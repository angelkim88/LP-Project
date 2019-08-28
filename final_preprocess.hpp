#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
bool vertify_plate(RotatedRect mr)
{
	float size = mr.size.area();
	float aspect = (float)mr.size.height / mr.size.width;
	if (aspect < 1)  aspect = 1 / aspect;

	bool  ch1 = size > 2000 && size < 30000;	
	bool  ch2 = aspect > 1.3 && aspect < 6.4;		

	return  ch1 && ch2;
}
void draw_rotatedRect(Mat &img, RotatedRect mr, Scalar color, int thickness = 2)
{
	Point2f  pts[4];
	mr.points(pts);

	for (int i = 0; i <4; ++i) {
		line(img, pts[i], pts[(i + 1) % 4], color, thickness);
	}
}
void find_candidates(Mat img, vector<RotatedRect>& candidates)
{
	vector< vector< Point> > contours;															
	findContours(img.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i< (int)contours.size(); i++)
	{
		RotatedRect  rot_rect = minAreaRect(contours[i]);	
		if (vertify_plate(rot_rect))				
			candidates.push_back(rot_rect);			
	}
}
void calc_direct(Mat Gy, Mat Gx, Mat& direct)
{
	direct.create(Gy.size(), CV_8U);

	for (int i = 0; i < direct.rows; i++) {
		for (int j = 0; j < direct.cols; j++) {
			float gx = Gx.at<float>(i, j);
			float gy = Gy.at<float>(i, j);
			int theat = int(fastAtan2(gy, gx) / 45);
			direct.at<uchar>(i, j) = theat % 4;
		}
	}
}
void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)		
{
	dst = Mat(sobel.size(), CV_32F, Scalar(0));

	for (int i = 1; i < sobel.rows - 1; i++) {
		for (int j = 1; j < sobel.cols - 1; j++)
		{
			int   dir = direct.at<uchar>(i, j);	
			float v1, v2;
			if (dir == 0) {			
				v1 = sobel.at<float>(i, j - 1);
				v2 = sobel.at<float>(i, j + 1);
			}
			else if (dir == 1) {		
				v1 = sobel.at<float>(i + 1, j + 1);
				v2 = sobel.at<float>(i - 1, j - 1);
			}
			else if (dir == 2) {	
				v1 = sobel.at<float>(i - 1, j);
				v2 = sobel.at<float>(i + 1, j);
			}
			else if (dir == 3) {	
				v1 = sobel.at<float>(i + 1, j - 1);
				v2 = sobel.at<float>(i - 1, j + 1);
			}
			float center = sobel.at<float>(i, j);
			dst.at<float>(i, j) = (center > v1 && center > v2) ? center : 0;
		}
	}
}

void trace(Mat max_so, Mat& pos_ck, Mat& hy_img, Point pt, int low)
{
	Rect rect(Point(0, 0), pos_ck.size());
	if (!rect.contains(pt)) return;			

	if (pos_ck.at<uchar>(pt) == 0 && max_so.at<float>(pt) > low)
	{
		pos_ck.at<uchar>(pt) = 1;		
		hy_img.at<uchar>(pt) = 255;												
		trace(max_so, pos_ck, hy_img, pt + Point(-1, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(0, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(+1, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(-1, 0), low);
		trace(max_so, pos_ck, hy_img, pt + Point(+1, 0), low);
		trace(max_so, pos_ck, hy_img, pt + Point(-1, +1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(0, +1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(+1, +1), low);
	}
}

void  hysteresis_th(Mat max_so, Mat&  hy_img, int low, int high)
{
	Mat pos_ck(max_so.size(), CV_8U, Scalar(0));
	hy_img = Mat(max_so.size(), CV_8U, Scalar(0));
	for (int i = 0; i < max_so.rows; i++) {
		for (int j = 0; j < max_so.cols; j++)
		{
			if (max_so.at<float>(i, j) > high)
				trace(max_so, pos_ck, hy_img, Point(j, i), low);
		}
	}
}


Mat  preprocessing(Mat image)
{
	Mat gray, blur_img, morph, Gx, Gy, direct, sobel, max_sobel, hy_img;
	Mat gau_img, canny, filter;
	Mat kernel(5, 15, CV_8UC1, Scalar(1));	
	cvtColor(image, gray, CV_BGR2GRAY);	
	blur(gray, blur_img, Size(5, 5));
	Sobel(blur_img, Gx, CV_32F, 1, 0, 3);
	Sobel(blur_img, Gy, CV_32F, 0, 1, 3);
	sobel = abs(Gx);
	calc_direct(Gy, Gx, direct);
	supp_nonMax(sobel, direct, max_sobel);
	hysteresis_th(max_sobel, hy_img, 120, 170);
	morphologyEx(hy_img, morph, MORPH_CLOSE, kernel, Point(-1, -1), 2);
	return morph;
}