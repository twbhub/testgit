# 跳一跳简单外挂



**开发环境: VS2022, opencv4.2.0-c++**

项目视频地址: https://www.bilibili.com/video/BV1Ht4y1V7bd/?vd_source=939818502857687a4a334c7a40d2c98a


## 1.基本实现原理

### 1.1 总体印象

1. 电脑与手机通过USB数据线相连, 用于发出指令, 传输图片.
2. 程序接收到图片, 对其进行处理, 得到小人的位置, 并得到目标位置, 计算出二者之间的距离.
3. 根据距离计算出需要按压屏幕的时间.
4. 通过指令模拟按压屏幕一段时间, 实现小人跳到下一个小墩上.

### 1.2 具体细节

1. 电脑与手机使用数据线相连, 打开USB调试.
2. 电脑和手机之间的通信(包括做出动作, 图像传输)都是通过命令来完成的.
3. 要使用命令实现上面的功能, 需要使用ADB工具. 首先要安装这个工具.
4. 图像处理库使用opencv c++.
5. 首先通过命令让手机截屏, 然后把图片拉取到电脑指定目录中, 然后程序加载当前图像, 进行一些处理.
6. 因为图像太大, 为了加快处理速度, 进行了resize, 然后获取ROI. 然后转化为灰度图像.
7. 因为小人的外形不改变, 很明显要寻找图像中小人的位置, 使用模板匹配的方法比较好.
8. 接下来就要找下一个小墩的位置, 这里我采用了简单的处理方式, 使用边缘检测, 通过膨胀腐蚀, 使得边缘变的更粗, 然后从上往下寻找, 便可以找到最上面的点, 然后再竖直向下一段位置便可以大致确定小墩的中心位置. 
9. 对于这个需求, 我觉得不必非常准确的找到小墩的中心坐标也可以实现功能.



## 2.遇到的困难

### 2.1边缘不明显

在找下一个小墩的时候, 有时候虽然在RGB色彩空间下两种颜色区分度很大, 但是由于我转化为了灰度图像, 此时在RGB空间下区分度很大的颜色, 在灰度空间下看, 基本分辨不出来是两种颜色. 人眼都看不出来.

所以这就给找下一个小墩带来了困难. canny算子根本就找不到边缘.

我想了一下, 既然在RGB空间下可以区分出来, 那两种颜色一定会在某个通道上的灰度值差别较大, 如果每个通道的值都相同的话, 在RGB空间下应该也是看不出的啊. 

有了这个想法, 我就在转化为灰度图像前, 把3个通道给分离得到3张灰度图像, 然后分别对每个图像进行边缘检测, 最后再将得到的3张边缘图像相加.

这样一来基本就可以找到任意颜色的小墩的边缘了!

### 2.2模板的获取

另一个困难是获取小人模板, 直接用opencv把图像显示出来, 然后用截图工具截取, 这种方式就是始终不行, 我也不太清楚到底是为啥.....反正就是它截出来的大小跟实际的大小就不一样. 原因是imshow显示的时候把原图像变大了. 这可能跟imshow函数的实现有关吧.

我最后采用的是把图像放到画图工具里, 然后从那上面把要查找的模板抠出来, 这最后才算行! 

这个问题还卡了我好几个小时, 我用第一种方法始终截不出那么大的模板, 也不知道为啥, 给我气的要死! 唉!



## 3.自己实现的简单函数

那种找二值图像中的目标的最外围点的操作用的挺多的, 我就自己实现了一个通用的函数来实现这个功能:

```c
/**
  * Brief: 在二值图中查找目标的最外围点, 获取其坐标.
  * Author: 六边形饭桶
  * Date: 2022年7月20日13:12:27
  * param: src: 输入图像, 必须是单通道图像, 严格一点必须是二值图像.
           how: 查找方式, 送下面的const int变量中选一个.
           isWhite: 查找的是白色的点吗? ture是找白色点, false是找黑色的点.
           stepx: x方向的查找步长.
           stepy: y方向的查找步长.
           result: 坐标
  * Note: stepx和stepy用来在查找的速度和准确度之间做一个权衡, 使用时根据实际情况自行调整其值.
  */
const int TOP_TO_BOTTOM = 0, BOTTOM_TO_TOP = 1, LEFT_TO_RIGHT = 2, RIGHT_TO_LEFT = 3;
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
```

