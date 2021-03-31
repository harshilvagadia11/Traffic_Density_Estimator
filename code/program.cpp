#include <opencv2/opencv.hpp>
#include <bits/stdc++.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "json.hpp"

using json = nlohmann::json;
namespace pt = boost::property_tree;

using namespace cv;
using namespace std;

// Image processing parameters
bool finished;
Mat h, empty_frame;
vector<Point2f> pts_src;
vector<Point2f> pts_dst;

// Storage vectors
vector<Mat> frames;
vector<double> Queue;
vector<double> dynamic;
vector<double> baseline_queue;
vector<double> baseline_dynamic;

// Method parameters
int frame_number = 0;
int resolve;
int x;
int space_threads;
int time_threads;
bool space_opt;
bool print_data;

Mat processImage(Mat frame) {
    // Transform and crop the frame to a perpendicular view

    if(pts_src.size() != 4) {
        // The road corners are hard-coded

        pts_src.push_back(Point2f(977, 225));   
        pts_src.push_back(Point2f(408, 990));
        pts_src.push_back(Point2f(1519, 993));
        pts_src.push_back(Point2f(1271, 218));
        
        pts_dst.push_back(Point2f(472, 52));
        pts_dst.push_back(Point2f(472, 830));
        pts_dst.push_back(Point2f(800, 830));
        pts_dst.push_back(Point2f(800, 52));
        h = findHomography(pts_src, pts_dst);

        Mat im_trans;
        warpPerspective(frame, im_trans, h, frame.size());
        empty_frame = im_trans(Rect(472, 52, 328, 778));
        resize(empty_frame, empty_frame, Size(empty_frame.cols/resolve, empty_frame.rows/resolve));

        return empty_frame;
    } else {
        Mat im_trans;
        warpPerspective(frame, im_trans, h, frame.size());
        Mat processed = im_trans(Rect(472, 52, 328, 778));
        resize(processed, processed, Size(processed.cols/resolve, processed.rows/resolve));

        return processed;
    }
}

double calcDiff(Mat img1, Mat img2) {
    // Calculate difference between two images (single number metric)

    Mat gray1, gray2, delta;
    cvtColor(img1, gray1, COLOR_BGR2GRAY);
    cvtColor(img2, gray2, COLOR_BGR2GRAY);
    absdiff(gray1, gray2, delta);

    return sum(delta)[0];
}

void get_frames(string video){
    // Get and store the frames from the video

    VideoCapture cap(video);

    if(!cap.isOpened()) {
        cout << "Error opening video file " << video << "\n";
        exit(1);
    }

    int count = 0;
    while(true){
        Mat frame; 
        cap >> frame;
        if(frame.empty()) break;
        if(count%x == 0) {
            frame_number++;
            frames.push_back(processImage(frame));
        }
        count++;
    }
}

void normalise(vector<double> &vec) {
    // Normalise a vector setting the max value as 1 and min value as 0

    double ma = 0;
    double mi = vec[0];
    for(double e : vec) {
        ma = max(ma, e);
        mi= min(mi,e);
    }
    for(double &e: vec) {
        e = (e-mi)/(ma-mi);
    }
}

struct thread_data {
   int c;
   int r;
};


void *cal_density_space(void *threadarg) {
    // The thread processes a part of a frame

    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    int c = my_data->c;
    int r = my_data->r;

    double diff;
    double dyn_diff;

    Mat prev_frame;

    for(int i = 0; i < frame_number; i++) {
        int cols = frames[i].cols/space_threads;
        int rows = frames[i].rows/space_threads;
        Mat temp1 = frames[i](Rect(c*cols, r*rows, cols, rows));
        Mat temp2 = empty_frame(Rect(c*cols, r*rows, cols, rows));
        Mat temp3;
        if(i != 0) temp3 = prev_frame(Rect(c*cols, r*rows, cols, rows));
        diff = calcDiff(temp1, temp2);  
        if(i == 0) dyn_diff = 0.0;
        else dyn_diff = calcDiff(temp1, temp3);  
        prev_frame = frames[i];   
        mutex m;
        m.lock();
        Queue[i] += diff;
        dynamic[i] += dyn_diff;
        m.unlock();
    }
    pthread_exit(NULL);
}

void get_density_space() {
    // Create the treads and then join them once the work is done for method 3

    pthread_t threads[space_threads][space_threads];
    pthread_attr_t attr;
    void *status;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(int i = 0; i < space_threads; i++) {
        for(int j = 0; j < space_threads; j++) {
            struct thread_data td;
            td.c = i; td.r = j;
            int rc = pthread_create(&threads[i][j], &attr, cal_density_space, (void *)&td);
            if (rc) {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }
    }
    pthread_attr_destroy(&attr);

    for(int i = 0; i < space_threads; i++) {
        for(int j = 0; j < space_threads; j++) {
            int rc = pthread_join(threads[i][j], &status);
            if (rc) {
                cout << "Error:unable to join," << rc << endl;
                exit(-1);
            }
        }
    }
}

void *cal_density_time(void *threadid) {
    // The thread processes a part of the video

    int off = static_cast<int>(reinterpret_cast<intptr_t>(threadid)); 

    double diff;
    double dyn_diff;

    Mat prev_frame;

    for(int i = (off*frame_number)/time_threads; i < ((off+1)*frame_number)/time_threads; i++) {
        diff = calcDiff(frames[i], empty_frame);
        if(i == (off*frame_number)/time_threads) dyn_diff = 0.0;
        else dyn_diff = calcDiff(frames[i], prev_frame);  
        prev_frame = frames[i];   
        Queue[i] = diff;
        dynamic[i] = dyn_diff;
    }
    pthread_exit(NULL);
}

void get_density_time() {
    // Create the treads and then join them once the work is done for method 4

    pthread_t threads[time_threads];
    pthread_attr_t attr;
    void *status;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(int i = 0; i < time_threads; i++) {
        int rc = pthread_create(&threads[i], &attr, cal_density_time, (void *)i);
        if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    pthread_attr_destroy(&attr);

    for(int i = 0; i < time_threads; i++) {
        int rc = pthread_join(threads[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }
}

void get_density() {
    // Initialise Queue and dynamic vectors, make calls to relevant functions and then normalise Queue and dynamic

    for(int i = 0; i < frame_number; i++) {Queue.push_back(0); dynamic.push_back(0);}

    if(space_opt) get_density_space();
    else get_density_time();

    normalise(Queue);
    normalise(dynamic);
}

void get_baseline(string file_name) {
    // get baseline Queue and dyn vector from csv

    ifstream fin(file_name);
    int count = 0;
  
    vector<string> row;
    string line, word;
  
    while (getline(fin, line)) {
        row.clear();
        stringstream s(line);

        while (getline(s, word, ',')) {
            row.push_back(word);
        }

        count++;
        if (count == 1) continue;
        baseline_queue.push_back(stold(row[1]));
        baseline_dynamic.push_back(stold(row[2]));

    }
    
}

void error_cal() {
    // Calculate error for both queue and dynamic density using the baseline as reference

    double error_queue = 0, error_dynamic = 0;
    int count = 0;
    for(int i = 0; i < frame_number; i++) {
        for(int j = 0; j < x; j++) {
            if(i*x+j >= baseline_queue.size()) break;
            error_queue+=((Queue[i]-baseline_queue[i*x+j])*(Queue[i]-baseline_queue[i*x+j]));
            error_dynamic+=((dynamic[i]-baseline_dynamic[i*x+j])*(dynamic[i]-baseline_dynamic[i*x+j]));
            count++;
        }
    }    
    error_queue = sqrt(error_queue/count);
    error_dynamic = sqrt(error_dynamic/count);

    if(!print_data) {
        cout << fixed << error_queue << setprecision(5) << "\n";
        cout << fixed << error_dynamic << setprecision(5) << "\n";
    }

}

void printData() {
    // Normalise and print data

    cout << "Frame,Queue,Moving\n";
    for(int i = 0; i < frame_number; i++) cout << i << "," << Queue[i] << "," << dynamic[i] << "\n";
}

void load_parameters(string filename) {
    // Reads the parameters from the JSON input file

    pt::ptree config;
    pt::read_json(filename, config);
    x = config.get<int>("x");
    resolve = config.get<int>("resolve");
    space_threads = config.get<int>("space_threads");
    time_threads = config.get<int>("time_threads");
    space_opt = config.get<bool>("space_opt");
    print_data = config.get<bool>("print_data");
}

int main(int argc, char* argv[]) {

    if(argc < 3) {
        cout << "Error: No input video and/or image provided\n Please execute as ./part3 <video> <empty_frame>\n";
        exit(1);
    } else if(argc > 3) {
        cout << "Warning: More than three arguments provided\n";     
    }

    load_parameters("config.json");

    Mat im_src = imread(argv[2]);
    if(im_src.empty()) {
        cout << "Error reading image " << argv[2] << "\n";
        exit(1);
    }

    
    processImage(im_src);
    destroyAllWindows();

    get_frames(argv[1]);
    auto start = std::chrono::high_resolution_clock::now();
    get_density();
    auto stop = std::chrono::high_resolution_clock::now();
    get_baseline("baseline.csv");

    error_cal();

    if (print_data) printData();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    
    if(!print_data) cout << ((long double)duration.count())/((long double) 1e9) << "\n";

}
