#include<iostream>

#include<opencv2/core.hpp>

#include<opencv2/imgcodecs.hpp>

#include<opencv2/highgui.hpp>

#include<opencv2/imgproc.hpp>

#include<opencv2/features2d.hpp>

using namespace cv;

using namespace std;

int main(int argc, char **argv) {

	CommandLineParser parser(argc, argv, "{help h||}{ @image | ../data/baboon.jpg | }");

	Mat src;

	string filename = parser.get<string>("@image");

	if ((src = imread(filename, IMREAD_COLOR)).empty())

	{

		cout << "Couldn't load image";

		return -1;

	}

	Mat gray, thresh, smooth, morph;

	cvtColor(src, src, COLOR_BGRA2BGR);

	cout << "Number of input channels=" << src.channels();

	bilateralFilter(src, smooth, 9, 30, 30);

	cvtColor(smooth, gray, COLOR_BGR2GRAY);

	GaussianBlur(gray, gray, Size(5, 3), 1.5);

	adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY

		, 11, 2);

	threshold(gray, thresh, 50, 255, THRESH_BINARY + THRESH_OTSU);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(25, 9));

	//morphologyEx(thresh, morph, MORPH_CLOSE, kernel);

	morphologyEx(thresh, morph, MORPH_OPEN, kernel);

	//

	Ptr<MSER> ms = MSER::create();

	vector<vector<Point> > regions;

	vector<Rect> mser_boundingbox;

	vector<Mat> license_characters;

	vector<String> fn;
	glob("C:/Users/barkas2/Desktop/Program 3/Program 3/Program 3/characters/*.jpg", fn, false);

	vector<Mat> template_vector;
	size_t count = fn.size();
	for(size_t i = 0; i < count; i++)
	{
		template_vector.push_back(imread(fn[i]));
	}

	ms->detectRegions(thresh, regions, mser_boundingbox);

	
	//Filter out all boundingboxes outside of ratio threshold to get license numbers
	for(auto it = mser_boundingbox.begin(); it != mser_boundingbox.end();)
	{
		auto it2 = regions.begin();
		//Find ratio of each bounding box
		double ratio = ((double)it->width) / ((double)it->height);

		//If bounding box is outside threshold, excise it from vector
		if (ratio < .2 || ratio > .6)
		{

			it = mser_boundingbox.erase(it);
			it2 = regions.erase(it2);
			
		} 
		else
		{
			++it;
			++it2;
		}
	}

	//For each bounding box, extract image into a new MAT
	for (int i = 0; i < mser_boundingbox.size(); i++)
	{
		license_characters.push_back(src
		(Rect(mser_boundingbox[i].x,
			mser_boundingbox[i].y,
			mser_boundingbox[i].width,
			mser_boundingbox[i].height)));
	}

	//Draw extracted and thresholded bounding boxes onto image
	for (int i = 0; i < regions.size(); i++)
	{
		rectangle(src, mser_boundingbox[i], CV_RGB(0, 255, 0));
	}


	//perform edge detection on extracted features
	for (int i = 0; i < mser_boundingbox.size(); i++)
	{
		cvtColor(license_characters[i], license_characters[i], COLOR_BGR2GRAY);
		GaussianBlur(license_characters[i], license_characters[i], Size(7, 7), 3.0);
		Canny(license_characters[i], license_characters[i], 20, 60);
	}

	//Create keypoints vector for each template image
	vector<vector<KeyPoint>> template_keypoints_vector;
	vector<KeyPoint> template_keypoints;

	//Create a mats for descriptors of keypoints for each template image
	Ptr<KAZE> kz = KAZE::create();
	vector<Mat> template_descriptors_vector;
	Mat template_descriptors;

	//Iterate through templates to find keypoints for each image
	for (int i = 0; i < template_vector.size(); i++)
	{
		kz->detectAndCompute(template_vector[i], noArray(), template_keypoints, template_descriptors);
		template_keypoints_vector.push_back(template_keypoints);
		template_descriptors_vector.push_back(template_descriptors);
	}



	//Create vector for extracted features
	vector<vector<KeyPoint>> feature_keypoints_vector;
	vector<KeyPoint> feature_keypoints;

	//Create mats for descriptors of keypoints for each extracted feature
	vector<Mat> feature_descriptors_vector;
	Mat feature_descriptors;

	//Iterate through templates to find keypoints for each image
	for (int i = 0; i < license_characters.size(); i++)
	{
		kz->detectAndCompute(license_characters[i], noArray(), feature_keypoints, feature_descriptors);
		feature_keypoints_vector.push_back(feature_keypoints);
		feature_descriptors_vector.push_back(feature_descriptors);
	}

	FlannBasedMatcher matcher;
	vector<vector<vector<DMatch>>> all_matches;
	vector<vector<DMatch>> match_vector;
	vector< DMatch > matches;
	double max_distance = 0;
	double min_distance = 100;	

	//compare templates to each extracted feature to see which matches best
	for (int i = 0; i < license_characters.size(); i++)
	{
		for (int j = 0; j < template_vector.size(); j++)
		{
			matcher.match(feature_descriptors_vector[i], template_descriptors_vector[j], matches);
			match_vector.push_back(matches);
		}
		for (int j = 0; j < template_vector.size(); j++)
		{
			for (int k = 0; k < feature_descriptors_vector[i].rows; k++)
			{
				double dist;
			}
		}
		all_matches.push_back(match_vector);
	}

	

	imshow("processed_image", morph);

	imshow("original", src);

	moveWindow("original", 10, 50);

	moveWindow("thresh", 30, 70);

	waitKey(0);

	return 0;



}