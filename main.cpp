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
	string tmpPath = "E:/΢����һ�����/����ͼƬ/С��.jpg";// ģ����һֱ�����, ����һ��ʼ�Ͷ�����
	Mat tmp = imread(tmpPath, IMREAD_GRAYSCALE);// �ԻҶȶ���ͼ��
	if (tmp.empty())
	{
		cout << "error: can not load template image.." << endl;
		exit(-1);
	}

	const char* screencap = "adb shell screencap -p /sdcard/ADB_screencap/1.jpg";// ��������
	const char* pullScreencap = "adb pull /sdcard/ADB_screencap/1.jpg E:/΢����һ�����/����ͼƬ";// ��ȡ��Ļ��ͼ����
	double coeff = 2.4;// ���ư�ѹʱ����ϵ��
	char jump[70] = { '\0' };


	while (true)
	{
		system(screencap);// ��ͼ
		system(pullScreencap);// ��ȡ��ͼ

		Mat src = imread("E:/΢����һ�����/����ͼƬ/1.jpg");// ���ؽ�ͼ, ���ǲ�ɫͼ��
		if (src.empty())
		{
			cout << "error: can not load src image..." << endl;
			exit(-1);
		}

		Mat resizedSrc;
		resize(src, resizedSrc, Size(600, 1200));// ������С

		Mat roiSrc = resizedSrc(Rect(0, 400, 600, 400)).clone();// ��ȡROI����

		Mat roiSrcGray;
		cvtColor(roiSrc, roiSrcGray, COLOR_BGR2GRAY);// ת��Ϊ�Ҷ�ͼ��

		Mat res;
		double min = 0.0, max = 0.0;
		Point minPos(0, 0), maxPos(0, 0);
		matchTemplate(roiSrcGray, tmp, res, TM_CCOEFF_NORMED);// ģ��ƥ��
		minMaxLoc(res, &min, &max, &minPos, &maxPos);

		vector<Mat> vec;
		split(roiSrc, vec);

		Mat blurSrc[3];
		Mat edges[3];
		Mat edgesDilatet[3];
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		for (int i = 0; i < 3; i++)
		{
			GaussianBlur(vec[i], blurSrc[i], Size(5, 5), 0.1, 0.1);// ��˹ģ��
			Canny(blurSrc[i], edges[i], 0, 150);// ��Ե���
			dilate(edges[i], edgesDilatet[i], element, Point(-1, -1), 1);// ����һ��, ����������
		}

		Mat edgeedge;
		edgeedge = edgesDilatet[0] + edgesDilatet[1] + edgesDilatet[2];

		Point result(0, 0);
		getFirst(edgeedge, TOP_TO_BOTTOM, true, 3, 3, result);// �õ���������

		Point start(maxPos.x + tmp.cols / 2, maxPos.y + tmp.rows);// ���
		Point target(result.x, result.y + 50);// �յ�

		rectangle(roiSrc, Rect(maxPos.x, maxPos.y, tmp.cols, tmp.rows), Scalar(255, 0, 0), 2, LINE_8);
		line(roiSrc, start, target, Scalar(0, 255, 0), 2, LINE_8);
		imshow("�Ҷ�ͼ", roiSrcGray);
		imshow("���ƽ��", roiSrc);
		imshow("edges", edgeedge);
		if (waitKey(3000) == 'q')
			break;

		double dist = distance(start, target);// ��������յ�֮��ľ���
		sprintf_s(jump, "adb shell input swipe 500 500 500 500 %d", static_cast<int>(coeff * dist));// ���㰴ѹʱ������
		system(jump);// ��ѹ��Ļ, ��һ��
		if (waitKey(2000) == 'q')
			break;
	}

	////cout << "position: " << maxPos << endl;
	////cout << "ƥ���: " << max << endl;
	//cout << dist << endl;

	////imshow("��˹ƽ��", blurSrc);
	////imshow("��Ե", edgesDilatet);

	//waitKey();
	destroyAllWindows();
	return 0;
}


void getFirst(Mat& src, int how, bool isWhite, int stepx, int stepy, Point& result)
{
	if (src.channels() != 1)
	{
		cout << "getFirst����ֻ�ܴ����ֵͼ��..." << endl;
		return;
	}

	int rows = src.rows;
	int cols = src.cols;
	bool flag = false;//��־λ���������ڶ���ѭ��, Ӧ������ߵ�Ч��.
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