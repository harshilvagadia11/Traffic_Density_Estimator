#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool finished;
Mat h, empty;
vector<Point2f> pts_src;
vector<Point2f> pts_dst;

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


int main(int argc, char* argv[]) {

    if(argc < 3) {
        cout << "Error: No input video and/or image provided\n Please execute as ./program <video> <empty_frame>\n";
        exit(1);
    } else if(argc > 4) {
        cout << "Warning: More than three arguments provided\n";
    }

    if(argc == 4) {
        freopen(argv[3], "w", stdout);
    }

    Mat im_src = imread(argv[2]);
    if(im_src.empty()) {
        cout << "Error reading image " << argv[2] << "\n";
        exit(1);
    }
    processImage(im_src);
    destroyAllWindows();

    VideoCapture cap(argv[1]);

    if(!cap.isOpened()) {
    cout << "Error opening video file " << argv[1] << "\n";
    return -1;
    }

    vector<int> frames;
    vector<double> queue;
    vector<double> dynamic;
    int frame_number = 0;
    
    Mat prev_frame;

    while(true) {
        frame_number++;
        Mat frame; cap >> frame;
        if(frame_number%5 != 1) continue;
        if(frame.empty()) break;

        frame = processImage(frame);
        double diff = calcDiff(frame, empty);

        double dyn_diff;
        if(frame_number == 1) dyn_diff = 0.0;
        else dyn_diff = calcDiff(frame, prev_frame);
        prev_frame = frame;

        frames.push_back(frame_number);
        queue.push_back(diff);
        dynamic.push_back(dyn_diff);
    }

    cap.release();
    destroyAllWindows();
    printData(frames, queue, dynamic);
}
