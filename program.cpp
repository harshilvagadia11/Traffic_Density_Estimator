#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool finished;
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

void CallBackFunc1(int event,int x,int y,int flags,void* userdata)
{
    if(event==EVENT_LBUTTONDOWN) {
        finished = true;
        return;
    }
}


int main(int argc, char* argv[]) {
    if(argc == 1) {
        cout << "Error: No input image provided\n Please execute as ./program <img_name>\n";
        exit(1);
    } else if(argc > 2) {
        cout << "Warning: Multiple arguments provided\n";
    }
    String image = argv[1];
    Mat im_src = imread(image);
    if(im_src.empty()) {
        cout << "Error reading image: " + image + "\n";
        exit(1);
    }
    namedWindow("Original Frame", 1);
    setMouseCallback("Original Frame", CallBackFunc, nullptr);
    finished = false;
    while(!finished){
      imshow("Original Frame", im_src);
      waitKey(50);
    }
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));
    Mat h = findHomography(pts_src, pts_dst);
    Mat im_trans;
    Size im_out_size(328,778);
    warpPerspective(im_src, im_trans, h, im_src.size());
    namedWindow("Transformed Frame", 1);
    setMouseCallback("Transformed Frame", CallBackFunc1, nullptr);
    finished = false;
    while(!finished){
      imshow("Transformed Frame", im_trans);
      waitKey(50);
    }
    imwrite("transformed_"+image, im_trans);
    Mat im_crop = im_trans(Rect(472, 52, 328, 778));
    namedWindow("Cropped Frame", 1);
    setMouseCallback("Cropped Frame", CallBackFunc1, nullptr);
    finished = false;
    while(!finished){
      imshow("Cropped Frame", im_crop);
      waitKey(50);
    }
    imwrite("cropped_"+image, im_crop);
}
