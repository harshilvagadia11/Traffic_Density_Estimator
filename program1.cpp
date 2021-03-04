#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool finished;
Mat h, empty;
vector<Point2f> pts_src;
vector<Point2f> pts_dst;

void CallBackFunc(int event,int x,int y,int flags,void* userdata) {
    if(event==EVENT_LBUTTONDOWN) {
        // cout << "Left mouse button clicked at (" << x << ", " << y << ")" << endl;
        pts_src.push_back(Point2f(x,y));
        if(pts_src.size() == 4) finished = true;
        return;
    }
}

Mat processImage(Mat frame) {
    if(pts_src.size() != 4) {
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
    Mat gray1, gray2;
    cvtColor(img1, gray1, COLOR_BGR2GRAY);
    cvtColor(img2, gray2, COLOR_BGR2GRAY);
    int histSize = 256;
    float range[] = { 0, 256} ;
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;
    int channels[] = {0};
    Mat h1,h2;
    calcHist(&gray1, 1, channels, Mat(), h1, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&gray2, 1, channels, Mat(), h2, 1, &histSize, &histRange, uniform, accumulate);
    return compareHist(h1, h2, HISTCMP_BHATTACHARYYA);
}

void printData(vector<double> &queue, vector<double> &dynamic) {
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
    for(int i = 0; i < queue.size(); i++) cout << i+1 << "," << queue[i] << "," << dynamic[i] << "\n";
}


int main(int argc, char* argv[]) {
    Mat im_src = imread("empty.jpg");
    if(im_src.empty()) {
        cout << "Error reading image\n";
        exit(1);
    }
    processImage(im_src);
    destroyAllWindows();

    VideoCapture cap("trafficvideo.mp4");

    if(!cap.isOpened()) {
    cout << "Error opening video stream or file" << endl;
    return -1;
    }

    vector<double> queue;
    vector<double> dynamic;
    int frame_number = 0;
    Mat prev_frame;

    while(true) {
        frame_number++;
        Mat frame;
        cap >> frame;
        if(frame_number%10 != 1) continue;
        if(frame.empty()) break;
        frame = processImage(frame);
        double diff = calcDiff(frame, empty);
        double dyn_diff;
        if(frame_number == 1) dyn_diff = 0.0;
        else dyn_diff = calcDiff(frame, prev_frame);
        prev_frame = frame;
        queue.push_back(diff);
        dynamic.push_back(dyn_diff);
    }

    cap.release();
    destroyAllWindows();
    printData(queue, dynamic);
}
