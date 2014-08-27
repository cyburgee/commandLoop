//
//  main.h
//  commandLoop
//
//  Created by Collin Burger on 7/29/14.
//
//

#include <array>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>



#ifndef commandLoop_main_h
#define commandLoop_main_h

using namespace std;
using namespace cv;

void initFrameBuffer();
bool checkForLoop();
bool hasMovement(Mat start, Scalar startMean, float startSum, int potentialEndIdx);
void getBestLoop(Mat start,Scalar startMean,float startSum);
bool ditchSimilarLoop();
void saveGifs();

VideoCapture vid;

float minMovementThresh;
bool minMovementBool;
float maxMovementThresh;
bool maxMovementBool;
float loopThresh;
float minChangeRatio;
float minPeriodSecs;
float maxPeriodSecs;

int maxPeriodFrames;
int minPeriodFrames;

int frameStartIdx;

CvSize imResize;

vector<array<int, 2>> loopEnds;
vector<Mat> frameBuffer;
vector<Mat> fullFrameBuffer;
vector<vector<Mat>> potentialGifs;
vector<float> loopRatings;
vector<int> matchIndeces;
vector<Mat> bestMatches;
vector<Mat> loopStartMats;



#endif
