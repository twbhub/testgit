#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


const int TOP_TO_BOTTOM = 0, BOTTOM_TO_TOP = 1, LEFT_TO_RIGHT = 2, RIGHT_TO_LEFT = 3;
void getFirst(Mat& src, int how, bool isWhite, int stepx, int stepy, Point& result);
double distance(Point& point1, Point& point2);


int main(void)
{
	string tmpPath = "E:/微信跳一跳外挂/测试图片/小人.jpg";// 模板是一直不变的, 所以一开始就读进来
	Mat tmp = imread(tmpPath, IMREAD_GRAYSCALE);// 以灰度读入图像
	if (tmp.empty())
	{
		cout << "error: can not load template image.." << endl;
		exit(-1);
	}

	const char* screencap = "adb shell screencap -p /sdcard/ADB_screencap/1.jpg";// 截屏命令
	const char* pullScreencap = "adb pull /sdcard/ADB_screencap/1.jpg E:/微信跳一跳外挂/测试图片";// 获取屏幕截图命令
	double coeff = 2.4;// 控制按压时长的系数
	char jump[70] = { '\0' };


	while (true)
	{
		system(screencap);// 截图
		system(pullScreencap);// 获取截图

		Mat src = imread("E:/微信跳一跳外挂/测试图片/1.jpg");// 加载截图, 还是彩色图像
		if (src.empty())
		{
			cout << "error: can not load src image..." << endl;
			exit(-1);
		}

		Mat resizedSrc;
		resize(src, resizedSrc, Size(600, 1200));// 调整大小

		Mat roiSrc = resizedSrc(Rect(0, 400, 600, 400)).clone();// 获取ROI区域

		Mat roiSrcGray;
		cvtColor(roiSrc, roiSrcGray, COLOR_BGR2GRAY);// 转化为灰度图像

		Mat res;
		double min = 0.0, max = 0.0;
		Point minPos(0, 0), maxPos(0, 0);
		matchTemplate(roiSrcGray, tmp, res, TM_CCOEFF_NORMED);// 模板匹配
		minMaxLoc(res, &min, &max, &minPos, &maxPos);

		vector<Mat> vec;
		split(roiSrc, vec);

		Mat blurSrc[3];
		Mat edges[3];
		Mat edgesDilatet[3];
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		for (int i = 0; i < 3; i++)
		{
			GaussianBlur(vec[i], blurSrc[i], Size(5, 5), 0.1, 0.1);// 高斯模糊
			Canny(blurSrc[i], edges[i], 0, 150);// 边缘检测
			dilate(edges[i], edgesDilatet[i], element, Point(-1, -1), 1);// 膨胀一下, 让线条更粗
		}

		Mat edgeedge;
		edgeedge = edgesDilatet[0] + edgesDilatet[1] + edgesDilatet[2];

		Point result(0, 0);
		getFirst(edgeedge, TOP_TO_BOTTOM, true, 3, 3, result);// 得到顶点坐标

		Point start(maxPos.x + tmp.cols / 2, maxPos.y + tmp.rows);// 起点
		Point target(result.x, result.y + 50);// 终点

		rectangle(roiSrc, Rect(maxPos.x, maxPos.y, tmp.cols, tmp.rows), Scalar(255, 0, 0), 2, LINE_8);
		line(roiSrc, start, target, Scalar(0, 255, 0), 2, LINE_8);
		imshow("灰度图", roiSrcGray);
		imshow("绘制结果", roiSrc);
		imshow("edges", edgeedge);
		if (waitKey(3000) == 'q')
			break;

		double dist = distance(start, target);// 求起点与终点之间的距离
		sprintf_s(jump, "adb shell input swipe 500 500 500 500 %d", static_cast<int>(coeff * dist));// 计算按压时间命令
		system(jump);// 按压屏幕, 跳一跳
		if (waitKey(2000) == 'q')
			break;
	}

	////cout << "position: " << maxPos << endl;
	////cout << "匹配度: " << max << endl;
	//cout << dist << endl;

	////imshow("高斯平滑", blurSrc);
	////imshow("边缘", edgesDilatet);

	//waitKey();
	destroyAllWindows();
	return 0;
}


void getFirst(Mat& src, int how, bool isWhite, int stepx, int stepy, Point& result)
{
	if (src.channels() != 1)
	{
		cout << "getFirst函数只能处理二值图像..." << endl;
		return;
	}

	int rows = src.rows;
	int cols = src.cols;
	bool flag = false;//标志位用来跳出第二重循环, 应该能提高点效率.
	switch (how)
	{
	case TOP_TO_BOTTOM: {
		for (int row = 0; row < rows; row += stepy)
		{
			for (int col = 0; col < cols; col += stepx)
			{
				if (isWhite == true)
				{
					if (src.at<uchar>(row, col) > 200)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
				else
				{
					if (src.at<uchar>(row, col) < 50)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
			}

			if (flag == true)
				break;
		}
	}break;

	case BOTTOM_TO_TOP: {
		for (int row = rows - 1; row >= 0; row -= stepy)
		{
			for (int col = 0; col < cols; col += stepx)
			{
				if (isWhite == true)
				{
					if (src.at<uchar>(row, col) > 200)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
				else
				{
					if (src.at<uchar>(row, col) < 50)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
			}

			if (flag == true)
				break;
		}
	}break;

	case LEFT_TO_RIGHT: {
		for (int col = 0; col < cols; col += stepx)
		{
			for (int row = 0; row < rows; row += stepy)
			{
				if (isWhite == true)
				{
					if (src.at<uchar>(row, col) > 200)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
				else
				{
					if (src.at<uchar>(row, col) < 50)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
			}

			if (flag == true)
				break;
		}
	}break;

	case RIGHT_TO_LEFT: {
		for (int col = cols - 1; col > 0; col -= stepx)
		{
			for (int row = 0; row < rows; row += stepy)
			{
				if (isWhite == true)
				{
					if (src.at<uchar>(row, col) > 200)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
				else
				{
					if (src.at<uchar>(row, col) < 50)
					{
						result.y = row;
						result.x = col;
						flag = true;
						break;
					}
				}
			}

			if (flag == true)
				break;
		}
	}break;

	default: break;
	}
}


double distance(Point& point1, Point& point2)
{
	int dis_x = point2.x - point1.x;
	int dis_y = point2.y - point1.y;
	return sqrt(dis_x * dis_x + dis_y * dis_y);
}