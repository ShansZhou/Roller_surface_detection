// 喷涂房辊子表面检测.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <string>
#include <ctime>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <pylon/PylonIncludes.h>


using namespace cv;
using namespace std;
using namespace Pylon;


static const uint32_t c_countOfImagesToGrab = 10000;

/* testing variables */
double INPUT_TIME = 1;
double INPUT_X = 300;
double INPUT_Y = 400;
double INPUT_WIDTH = 1500;
double INPUT_HEIGHT = 600;

double Canny_low = 30;
double Canny_high = 80;


class detector {
public:

	Mat dodection(Mat img)
	{
		//////////////////////////////////////////// process /////////////////////////////////////////


		clock_t begin = clock();
		
		Mat image;
		//GaussianBlur(img, image, Size(3, 3), 1);

		//cv::GaussianBlur(img, image, cv::Size(1, 1), 1);
		//cv::addWeighted(img,3.8, image, -2.5, 0, image);

		Mat edgeBW;
		Canny(img, edgeBW, Canny_low, Canny_high);
		//imshow("bw edge", edgeBW);

		Mat dilate_bw;
		Mat kernel = getStructuringElement(0, Size(5, 5));
		dilate(edgeBW, dilate_bw, kernel);

		//Mat edgeBW;
		//Canny(img, edgeBW, 10, 30);
		//imshow("Display window", edgeBW);


		//////////////////////////////////////////// anylsis////////////////////////////////////////////////
		Mat labs_edgebw;
		Mat labs,stats;
		Mat centers;

		connectedComponentsWithStats(dilate_bw, labs_edgebw, stats, centers,8);
		//imshow("test",labs_edgebw);
		//waitKey(0);

		/**/
		//std::cout << labels << std::endl;
		std::cout << "================="<< std::endl;
		std::cout << "num of defects: " << stats.rows-1 << std::endl;
		//std::cout << centroids << std::endl;
		for (int i = 1; i<stats.rows; i++)
		{
			int x = stats.at<int>(Point(0, i));
			int y = stats.at<int>(Point(1, i));
			int w = stats.at<int>(Point(2, i));
			int h = stats.at<int>(Point(3, i));

			std::cout << "[ " << x << ", " << y << ", " << w << ", " << h <<" ]"<< std::endl;
			Scalar color(255, 255, 255);
			Rect rect(x, y, w, h);
			rectangle(dilate_bw, rect, color);
			Scalar color1(0, 0, 0);
			rectangle(img, rect, color1);
		}
		std::cout << "------------"<< std::endl;


		namedWindow("frame", WINDOW_NORMAL);
		resizeWindow("frame", 680, 480);
		imshow("frame", img);

		namedWindow("BW", WINDOW_NORMAL);
		resizeWindow("BW", 680, 480);
		imshow("BW", dilate_bw);
		cv::waitKey(1);
		
		//string resultspath = "imgs/results/";
		//imwrite(resultspath + "1.bmp", edgeBW);

		/*
		filestorage fs(resultspath + "1.xml", filestorage::write);
		fs << "images" << edgebw;
		*/

		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		std::cout <<"DPS: "<< elapsed_secs << endl;

		return stats;
	}

	double maxTimeOwned(double dia, double rpm, double s_height) {
		double v_linear = (dia*3.14*rpm) / 60;
		double t_All = s_height / v_linear;
		return t_All;
	}

	Mat calframeROI(Mat img, double x,double y,double w,double h) {

		Rect roi;
		roi.x = x;
		roi.y = y;
		roi.width = w;
		roi.height = h;
		img = img(roi);

		return img;
	}

	
};

/* access frame, stats, index*/
class detectorResult {
public:
	Mat outputFrame;
	Mat outputStats;
	int outputIndex;
	detectorResult(Mat frame,Mat stats,int index) {
		outputFrame = frame;
		outputStats = stats;
		outputIndex = index;

	}

	Mat getFrame() {
		return outputFrame;
	}
	Mat getStats() {
		return outputStats;
	}
	int getIndex() {
		return outputIndex;
	}
};

/* access defect image, index, ROI*/
class defects {
public:
	Mat _image;
	int _index;
	Rect _roi;

	defects(Mat img,int idx, Rect rect) {
		_image = img;
		_index = idx;
		_roi = rect;
	}

	Mat getDefectImg() {
		return _image;
	}
	int getDefectIdx() {
		return _index;
	}
	Rect getDefectRect() {
		return _roi;
	}


};

int main(int argc, char** argv)
{

	// Using gige Camera
	/*
	PylonAutoInitTerm atuoInitTerm;
	try {
		CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
		cout << "using device" << camera.GetDeviceInfo().GetModelName() << endl;
		GenApi::INodeMap& nodemap = camera.GetNodeMap();

		camera.Open();
		// set exposure time
		const GenApi::CFloatPtr  exposureTime = nodemap.GetNode("ExposureTimeAbs");
		exposureTime->SetValue(35);

		GenApi::CIntegerPtr width = nodemap.GetNode("Width");
		GenApi::CIntegerPtr height = nodemap.GetNode("Height");

		camera.MaxNumBuffer = 5;

		CImageFormatConverter formatConverter;
		formatConverter.OutputPixelFormat = PixelType_BGR8packed;
		CPylonImage pylonImage;

		//int grabbedImages = 0;

		//VideoWriter cvVideoCreator;
		Mat openCvImage;
		//string videoFileName = "openCvVideo.avi";
		//Size frameSize = Size((int)width->GetValue(), (int)height->GetValue());
		//cvVideoCreator.open(videoFileName, CV_FOURCC('D', 'I', 'V', 'X'), 14, frameSize, true);

		camera.StartGrabbing(c_countOfImagesToGrab, GrabStrategy_LatestImageOnly);

		CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing()) {
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
			if (ptrGrabResult->GrabSucceeded()) {
				//cout << "SizeX" << ptrGrabResult->GetWidth() << endl;
				//cout << "SizeY" << ptrGrabResult->GetHeight() << endl;
				formatConverter.Convert(pylonImage, ptrGrabResult);
				openCvImage = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint_t *)pylonImage.GetBuffer());
				
				//Mat Image_resized;
				//Size size(500, 380);
				//resize(openCvImage, Image_resized, size);//resize image
				//namedWindow("result", WINDOW_NORMAL);
				//resizeWindow("frame", 680, 480);
				//imshow("result", Image_resized);
				

				clock_t begin = clock();
				detector my;
				my.dodection(openCvImage);
				clock_t end = clock();
				double elapsed_secs = double(end - begin);



				waitKey(INPUT_TIME - elapsed_secs);
			}

		}
	}
	catch (Exception e) {
		cout << "basler camera failed to connected" << endl;
		waitKey(0);
	}
	
	*/

	/* Read video sample */
	/**/
	VideoCapture cap("imgs/t2.avi");

	//cap.open();
	if (!cap.isOpened()) {
		std::cout << "Error opening video" << endl;
		return -1;
	}
	
	Mat frame;
	
	//int framewidth = cap.get(CAP_PROP_FRAME_WIDTH);
	//int frameheight = cap.get(CAP_PROP_FRAME_HEIGHT);
	//VideoWriter output("output.avi", VideoWriter::fourcc('M','J', 'P', 'G'), 30, Size(framewidth, frameheight));


	int frameidex = 0;
	vector<detectorResult> resultsList;
	vector<defects> defectsList;


	while (cap.read(frame)) {

		cout << ">>>>>>>>>>>>>>>>>>>>>> Frame ID: " << frameidex << endl;
		//if ((cap.get(CV_CAP_PROP_POS_FRAMES) + 1) < cap.get(CV_CAP_PROP_FRAME_COUNT)) {
		//	cap.read(frame);
		//}
		//output.write(frame);
		//imshow("Frame", frame);
		//waitKey(1500);
		clock_t begin = clock();

		// Main defection framework 
		// detect single frame in ROI
		detector my;
		frame = my.calframeROI(frame, INPUT_X, INPUT_Y, INPUT_WIDTH, INPUT_HEIGHT);
		Mat stats = my.dodection(frame);

		// adding current result to defect reuslts
		detectorResult curResult(frame,stats,frameidex);
		resultsList.push_back(curResult);

		int defectsize = defectsList.size();
		
		// adding defects to list and checking exsits
		for (int i = 1; i<stats.rows; i++)
		{
			bool isDefect = false;
			int x = stats.at<int>(Point(0, i));
			int y = stats.at<int>(Point(1, i));
			int w = stats.at<int>(Point(2, i));
			int h = stats.at<int>(Point(3, i));
			Rect defect(x, y, w, h);

			Mat defectImg = my.calframeROI(frame,x,y,w,h);

			defects thisDefect(defectImg,frameidex,defect);

			
			if (defectsize == 0) {
				defectsList.push_back(thisDefect);
			}
			else if(stats.rows < 10 && stats.rows > 1){
				for (int n = 0; n < defectsize; n++) {
					defects oldDefect = defectsList.at(n);
					if (x == oldDefect.getDefectRect().x && w == oldDefect.getDefectRect().width && h == oldDefect.getDefectRect().height) {
						Mat cur_defectImg = my.calframeROI(frame, x, y, w, h);
						Mat old_defectImg = oldDefect.getDefectImg();

						//imshow("cur",cur_defectImg);
						//waitKey(1);
						//imshow("old",old_defectImg);
						//waitKey(1);
						// compare two defect image similarity

						Mat diff;
						subtract(cur_defectImg, old_defectImg, diff);

						double sumelement = sum(diff)[0];

						// Duplicate defect handling 
						if (sumelement == 0) {
							cout << "duplicated defect found" << endl;
							isDefect = false;
							break; 
						}
						else {
							isDefect = true;
						}
					}
					else {
						isDefect = true;	
					}
				}	
				if (isDefect) {
					//cout << "defect found" << endl;
					defectsList.push_back(thisDefect);
				}
			}
			else {
				cout << "large amount of errors" << endl;
			}

		}
		
		
		clock_t end = clock();

		cout << "FPS: " << double(end - begin) / CLOCKS_PER_SEC << endl;

		defectsize = defectsList.size();
		cout << "current size: " << defectsize << endl;

		double elapsed_secs = double(end - begin);
		double waittime = INPUT_TIME - elapsed_secs;
		if (waittime > 0) {
			waitKey(waittime);
		}
		else {
			std::cout << "input time too small" << endl;
		}

		frameidex++;
	}

	std::cout << "\n=====Detection Completed=====" << endl;
	std::cout << "Defects Amount: "<<defectsList.size() << endl;

	//output.release();
	cap.release();
	cv::waitKey(0);




	// test local images
	/*
	
	string resultspath = "imgs/Image__2019-03-06__10-15-55.bmp";
	Mat img;
	img = imread(resultspath, IMREAD_COLOR);
	if (img.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}
	Mat image;
	//GaussianBlur(img, image,Size(3, 3), 5);
	//addWeighted(img,2, image, -4, 0, image);
	//blur(img,image,Size(3,3),Point(1,1),0);
	//bilateralFilter(img, image,3,5,5);

	Mat edgeBW;
	Canny(image, edgeBW, 255*0.15, 255*0.42);

	//imwrite("imgs/edgeBW.bmp", edgeBW);

	namedWindow("", WINDOW_NORMAL);
	resizeWindow("",1000,700);
	imshow("",edgeBW);
	waitKey(0);


	//Mat dilate_bw;
	//Mat kernel = getStructuringElement(0,Size(3,3));
	//dilate(edgeBW, dilate_bw, kernel);
	//imshow("", edgeBW);
	//waitKey(0);

	Mat labs_edgebw;
	Mat labs, stats;
	Mat centers;

	connectedComponentsWithStats(edgeBW, labs_edgebw, stats, centers, 8);
	//std::cout << labels << std::endl;
	std::cout << "stats.size()=" << stats.size() << std::endl;
	//std::cout << centroids << std::endl;
	for (int i = 0; i<stats.rows; i++)
	{
		int x = stats.at<int>(Point(0, i));
		int y = stats.at<int>(Point(1, i));
		int w = stats.at<int>(Point(2, i));
		int h = stats.at<int>(Point(3, i));

		//std::cout << "x=" << x << " y=" << y << " w=" << w << " h=" << h << std::endl;
		Scalar color(0, 0, 255);
		Rect rect(x, y, w, h);
		rectangle(img, rect, color);
	}
	namedWindow("1", WINDOW_NORMAL);
	resizeWindow("1", 1000, 700);
	imshow("1", image);
	waitKey(0);

	*/
}

/*
static void SetupCamera(Pylon::CInstantCamera& camera, int index)
{
	using namespace GenApi;
	INodeMap &cameraNodeMap = camera.GetNodeMap();

	if (index == 0)
	{
		CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode("TriggerSelector");
		ptrTriggerSel->FromString("FrameStart");
		CEnumerationPtr  ptrTrigger = cameraNodeMap.GetNode("TriggerMode");
		ptrTrigger->SetIntValue(0);
	}
	else if (index == 1)
	{
		CEnumerationPtr  ptrTriggerSel = cameraNodeMap.GetNode("TriggerSelector");
		ptrTriggerSel->FromString("FrameStart");
		CEnumerationPtr  ptrTrigger = cameraNodeMap.GetNode("TriggerMode");
		ptrTrigger->SetIntValue(1);
		CEnumerationPtr  ptrTriggerSource = cameraNodeMap.GetNode("TriggerSource");
		ptrTriggerSource->FromString("Line1");
	}
	else if (index == 2)
	{
		const CFloatPtr exposureTime = cameraNodeMap.GetNode("ExposureTimeAbs");
		exposureTime->SetValue(theApp.m_iExposeTime);
	}
	else if (index == 3)
	{
		const CIntegerPtr cameraGen = cameraNodeMap.GetNode("GainRaw");
		cameraGen->SetValue(theApp.m_iGain);
	}
	else if (index == 4)
	{
		const CBooleanPtr frameRate = cameraNodeMap.GetNode("AcquisitionFrameRateEnable");
		frameRate->SetValue(TRUE);
		const CFloatPtr frameRateABS = cameraNodeMap.GetNode("AcquisitionFrameRateAbs");
		frameRateABS->SetValue(theApp.m_iHZ);
	}
	else if (index == 5)
	{
		const CIntegerPtr widthPic = cameraNodeMap.GetNode("Width");
		widthPic->SetValue(theApp.m_Width);
	}
	else if (index == 6)
	{
		const CIntegerPtr heightPic = cameraNodeMap.GetNode("Height");
		heightPic->SetValue(theApp.m_Height);
	}
	else if (index == 7)
	{
		CEnumerationPtr  ptrLineSource = cameraNodeMap.GetNode("LineSource");
		ptrLineSource->SetIntValue(2);
	}

}
*/