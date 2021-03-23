#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool finished;
Mat h, empty;
vector<Point2f> pts_src;
vector<Point2f> pts_dst;

vector<Mat> frames;
vector<double> queue;
vector<double> dynamic;
vector<double> baseline_queue;
vector<double> baseline_dynamic;
int frame_number=0;
int resolve=1;
int x=5;

void CallBackFunc(int event,int x,int y,int flags,void* userdata) {
    // Store the coordinates of corners of the road

    if(event==EVENT_LBUTTONDOWN) {
        pts_src.push_back(Point2f(x,y));
        if(pts_src.size() == 4) finished = true;
        return;
    }
}

Mat processImage(Mat frame) {
    // Transform and crop the frame to a perpendicular view

    if(pts_src.size() != 4) {
        // If corner coordinates are not stored, take input from the user

        namedWindow("Original Frame", 1);
        setMouseCallback("Original Frame", CallBackFunc, nullptr);
        finished = false;
        while(!finished){
            imshow("Original Frame", frame);
            waitKey(50);
        }

        pts_dst.push_back(Point2f(472, 52));
        pts_dst.push_back(Point2f(472, 830));
        pts_dst.push_back(Point2f(800, 830));
        pts_dst.push_back(Point2f(800, 52));
        h = findHomography(pts_src, pts_dst);

        Mat im_trans;
        warpPerspective(frame, im_trans, h, frame.size());
        empty = im_trans(Rect(472, 52, 328, 778));

        return empty;
    } else {
        Mat im_trans;
        warpPerspective(frame, im_trans, h, frame.size());
        Mat processed = im_trans(Rect(472, 52, 328, 778));

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

void printData(vector<int> frames, vector<double> &queue, vector<double> &dynamic) {
    // Normalise and print data

    double ma = 0;
    double mi = queue[0];
    for(double e : queue) {
        ma = max(ma, e);
        mi= min(mi,e);
    }
    for(double &e: queue) {
        e = (e-mi)/(ma-mi);
    }
    ma = 0;
    mi = dynamic[0];
    for(double e : dynamic) {
        ma = max(ma, e);
        mi= min(mi,e);
    }
    for(double &e: dynamic) {
        e = (e-mi)/(ma-mi);
    }

    cout << "Frame,Queue,Moving\n";
    for(int i = 0; i < frames.size(); i++) cout << frames[i] << "," << queue[i] << "," << dynamic[i] << "\n";
}


void get_frames(string video){
    VideoCapture cap(video);

    if(!cap.isOpened()) {
        cout << "Error opening video file " << argv[1] << "\n";
        exit(1);
    }

    while(true){
        Mat frame; 
        cap >> frame;
        if(frame.empty()) break;
        frame_number++;
        frames.push_back(frame);
    }
}

void every_x(){
    Mat prev_frame;
    for(int i=0;i<frame_number;i++){
        if (i%x==0){
            prev_frame=processImage(frames[i]);
        }
        frames[i]=prev_frame;
    }
}

void resolution(){
    for(int i=0;i<frame_number;i++){                                // %x ?
        frames[i]=prev_frame;
        resize(frames[i],frames[i],Size(frames[i].cols/resolve,frames[i].rows/resolve));    // to take double value in size?
    }
}

void get_density(){

    double diff;
    double dyn_diff;

    Mat prev_frame;
    
    for(int i=0;i<frame_number;i++){
        if (i%x==0){
            diff = calcDiff(frames[i], empty);  
            if(i == 0) dyn_diff = 0.0;
            else dyn_diff = calcDiff(frame, prev_frame);  
            prev_frame=frames[i];
        }    
        queue.push_back(diff);
        dynamic.push_back(dyn_diff);  
    }
}

void get_baseline(){                                   // get baseline queue and dyn vector from csv

}

void utility_cal(){
    double utility_queue=0,utility_dynamic=0;
    for(int i=0;i<frame_number;i++){
        utility_queue+=((queue[i]-baseline_queue[i])*(queue[i]-baseline_queue[i]))/baseline_queue[i];        // is this division good?
        utility_dynamic+=((dynamic[i]-baseline_dynamic[i])*(dynamic[i]-baseline_dynamic[i]))/baseline_dynamic[i];
    }    
    utility_queue=utility_queue/frame_number;
    utility_dynamic=utility_dynamic/frame_number;
        
}


int main(int argc, char* argv[]) {
    
                                                    // take resolve and x also as arguement
    if(im_src.empty()) {
        cout << "Error reading image " << argv[2] << "\n";
        exit(1);
    }
    processImage(im_src);
    destroyAllWindows();

    get_frames(argv[1]);
    every_x();
    resolution();
    get_density();

    utility_cal();

}
