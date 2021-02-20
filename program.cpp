#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool finished = false;
vector<Point2f> pts_src;
vector<Point2f> pts_dst;

void CallBackFunc(int event,int x,int y,int flags,void* userdata)
{
    if(event==EVENT_LBUTTONDOWN) {
        cout << "Left mouse button clicked at (" << x << ", " << y << ")" << endl;
        pts_src.push_back(Point2f(x,y));
        if(pts_src.size() == 4) finished = true;
        return;
    }
}


int main(int argc, char* argv[]) {
    Mat im_src = imread("empty.jpg");
    if(im_src.empty()) {
        cout << "Error reading source image\n";
        exit(1);
    }
    namedWindow("Original Frame", 1);
    setMouseCallback("Original Frame", CallBackFunc, nullptr);
    while(!finished){
      imshow("Original Frame", im_src);
      waitKey(50);
    }
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));
    Mat h = findHomography(pts_src, pts_dst);
}
