//
//  main.cpp
//  commandLoop
//
//  Created by Collin Burger on 7/28/14.
//
//

#include "main.h"


#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

int main(int argc, const char * argv[])
{
    
    string fileOutString;
    string fileInString;
    string fileProgressString;
    ofstream progress;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("input,i",po::value<std::string>(&fileInString)->required(), "path and filename of desired input video")
    ("output,o", po::value<std::string>(&fileOutString)->required(), "path and filename of desired output file. it will be comma separated")
    ("progress,p", po::value<std::string>(&fileProgressString), "path and filename of processing progress indicator")
    
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    
    if (vm.count("input")) {
        cout << "Input file: "
        << vm["input"].as<std::string>() << ".\n";
    } else {
        cout << "Input file was not set.\n";
        return -1;
    }
    
    if (vm.count("output")) {
        cout << "Output file: "
        << vm["output"].as<std::string>() << ".\n";
    } else {
        cout << "Output file was not set.\n";
        return -1;
    }
    
    if (vm.count("progress")) {
        cout << "Progress file: "
        << vm["output"].as<std::string>() << ".\n";
        progress.open(fileProgressString,ofstream::out);
    } else {
        cout << "Progress file was not set.\n";
        return -1;
    }
    
    
    //string filename = "../data/160 Greatest Arnold Schwarzenegger Quotes.mp4";
    //vid.open(argv[1]);
    vid.open(fileInString);
    //string fileString(argv[1]);
    double numFrames = vid.get(CV_CAP_PROP_FRAME_COUNT);
    double fps = vid.get(CV_CAP_PROP_FPS);
    cout << "num frames: " << numFrames << endl;
    cout << "fps: " << fps << endl;
    
    minMovementThresh = 3;
    minMovementBool = true;
    maxMovementThresh = 15;
    maxMovementBool = false;
    loopThresh = 85.0;
    minChangeRatio = FLT_MAX;
    minPeriodSecs = 1.0;
    maxPeriodSecs = 3.0;
    
    maxPeriodFrames = fps*maxPeriodSecs;
    minPeriodFrames = fps*minPeriodSecs;
    
    imResize = cvSize(64, 64);
    
    initFrameBuffer();
    
    bool showProgress = progress.is_open();
    for (frameStartIdx = 0; frameStartIdx <= ceil(numFrames) - maxPeriodFrames; frameStartIdx++){
        checkForLoop();
        frameBuffer.erase(frameBuffer.begin());
        fullFrameBuffer.erase(fullFrameBuffer.begin());
        Mat currFrame,currGray, smallGray;
        while (!vid.read(currFrame))
            cout << "couldn't get next frame" << endl;
        
        fullFrameBuffer.push_back(currFrame);
        cvtColor(currFrame, currGray, CV_RGB2GRAY);
        resize(currGray, smallGray, imResize,INTER_AREA);
        frameBuffer.push_back(smallGray);
        //cout << "frameStart: " << frameStartIdx << endl;
        if (showProgress && frameStartIdx%100 == 0) {
            progress << (int)(frameStartIdx/numFrames*100) << "% done" << endl;
            cout << (int)(frameStartIdx/numFrames*100) << "% done" << endl;
        }
        
    }
    
    if(showProgress){
        progress << 100 << "% done" << endl;
        progress.close();
    }
    
    
    //stringstream fileOut;
    //fileOut << "data/loopEnds_"<< fs::basename(fileString) << ".csv";
    //string fileOutOne = fileOut.str();
    ofstream outputOne;
    //outputOne.open(fileOutOne,ofstream::out);
    outputOne.open(fileOutString,ofstream::out);
    if (outputOne.is_open()) {
        cout << "Output Good" << endl;
        for (int i = 0; i < loopEnds.size(); i++) {
            outputOne << loopEnds.at(i)[0] << "," <<  loopEnds.at(i)[1] << endl;
        }
    }
    
    outputOne.close();
    
    
    saveGifs();
    
    vid.release();
    loopEnds.clear();
    frameBuffer.clear();
    fullFrameBuffer.clear();
    loopRatings.clear();
    //matchIndeces.clear();
    //bestMatches.clear();
    /*for (int i = 0; i < loopStartMats.size(); i++) {
     loopStartMats.at(i).deallocate();
     }*/
    loopStartMats.clear();
    /*for (int i = 0; i < frameBuffer.size(); i++) {
     frameBuffer.at(i).deallocate();
     }*/
    frameBuffer.clear();
    return 0;
}


void initFrameBuffer(){
    Mat currFrame, currGray, smallGray;
    
    for (int i = 0; i < maxPeriodFrames; i++) {
        
        while (!vid.read(currFrame))
            cout << "couldn't get next frame" << endl;
        
        fullFrameBuffer.push_back(currFrame);
        cvtColor(currFrame, currGray, CV_RGB2GRAY);
        resize(currGray, smallGray, imResize,INTER_AREA);
        frameBuffer.push_back(smallGray);
        
    }
}



bool checkForLoop(){
    Mat start;
    frameBuffer.at(0).copyTo(start);
    Scalar startMean = mean(start);
    subtract(start, startMean, start);
    float startSum = cv::sum(start)[0] + 1; //add one to get rid of division by 0 if screen is black
    Mat currEnd;
    
    for (int i = minPeriodFrames; i < frameBuffer.size(); i++) {
        Mat currEnd;
        frameBuffer.at(i).copyTo(currEnd);
        subtract(currEnd, startMean, currEnd);
        Mat diff;
        absdiff(start, currEnd, diff);
        float endDiff = cv::sum(diff)[0] + 1;
        float changeRatio = endDiff/startSum;
        
        int potentialEndIdx = -1;
        
        if (changeRatio < minChangeRatio) {
            minChangeRatio = changeRatio;
            //cout << "min Ratio: " << minChangeRatio << endl;
        }
        
        //cout << "frameStart: " << frameStart << endl;
        //cout << "compare Frame: " << frameStart + i << endl;
        //cout << "startSum: " << startSum << endl;
        //cout << "endSum: " << cv::sum(currEnd)[0] + 1 << endl;
        //cout << "endDiff: " << endDiff << endl;
        //cout << "change Ratio: " << changeRatio*100 << endl;
        //cout << "test Thresh: " << 100 - loopThresh << endl;
        
        if ((changeRatio*100) <=  (100 - loopThresh)){//set to change percentage
            //potentialEndIdx = frameStartIdx + i;
            potentialEndIdx = i;
            //array<int,2> idx = {frameStart, potentialEndIdx};
            //loopEnds.push_back(idx);
            
            //cout << "potentialEnd: " << potentialEndIdx << endl;
            if (minMovementBool || maxMovementBool){
                if  (hasMovement(start, startMean, startSum, potentialEndIdx)){
                    matchIndeces.push_back(potentialEndIdx);
                    bestMatches.push_back(currEnd);
                }
                else{
                    return false;
                }
            }
            else{
                matchIndeces.push_back(potentialEndIdx);
                bestMatches.push_back(currEnd);
            }
        }
    }
    
    if (bestMatches.size() > 0) {
        getBestLoop(start,startMean,startSum);
        return true;
    }
    
    return false;

    
}

bool hasMovement(Mat start, Scalar startMean, float startSum, int potentialEndIdx){
    float sumDiff = 0;
    
    cv::Mat currFrame;
    
    for (int i = 1; i <= potentialEndIdx; i++) {
        Mat currFrame;
        frameBuffer.at(i).copyTo(currFrame);
        subtract(currFrame, startMean, currFrame);
        Mat diff;
        absdiff(currFrame,start,diff);
        float endDiff = cv::sum(diff)[0] + 1;
        float changeRatio = endDiff;
        sumDiff += changeRatio;
        start = currFrame;
    }
    sumDiff /= startSum;
    
    //cout << "sum Diff: " << sumDiff << endl;
    //cout << "minThreshold: " << minMovementThresh*(potentialEndIdx - frameStart)/100 << endl;
    
    if (minMovementBool) {
        if(sumDiff < minMovementThresh/100 ){
            cout << "not enough movement" << endl;
            return false;
        }
    }
    else if (maxMovementBool){
        if(sumDiff > maxMovementThresh/100 ){
            cout << "too much movement" << endl;
            return false;
        }
    }
    else if (minMovementBool && maxMovementBool){
        if(sumDiff > maxMovementThresh/100 && sumDiff < minMovementThresh/100){
            cout << "not enough movement or too much" << endl;
            return false;
        }
    }
    
    return true;
}

void getBestLoop(Mat start,Scalar startMean,float startSum){
    float minChange = MAXFLOAT;
    int bestEnd = -1;
    //float startSum = cv::sum(start)[0] + 1; //add one to get rid of division by 0 if screen is black
    
    for (int i = 0; i < bestMatches.size(); i++) {
        Mat currEnd;
        frameBuffer.at(i).copyTo(currEnd);
        subtract(currEnd,startMean,currEnd);
        Mat diff;
        absdiff(start, currEnd, diff);
        float endDiff = cv::sum(diff)[0] + 1;
        float changeRatio = endDiff/startSum;
        
        //cout << "end of loop frame: " << matchIndeces.at(i) << endl;
        //add optical flow test here
        if (changeRatio <= minChange) {
            minChange = changeRatio;
            bestEnd = i;
        }
    }
    
    if (bestEnd >= 0) {
        array<int,2> endIndeces = {frameStartIdx, frameStartIdx + matchIndeces.at(bestEnd)};
        loopEnds.push_back(endIndeces);
        
        loopRatings.push_back(minChange);
        loopStartMats.push_back(start);
        
        vector<Mat> potGif;
        for (int i = 0; i < matchIndeces.at(bestEnd); i++) {
            potGif.push_back(fullFrameBuffer.at(i));
        }
        potentialGifs.push_back(potGif);
        
        ditchSimilarLoop();
    }
    
    bestMatches.clear();
    matchIndeces.clear();
}


//------------------------------------------------------------------------------------
bool ditchSimilarLoop(){
    if (loopStartMats.size() < 2) {
        return false;
    }
    
    cv::Mat prevLoopMat;
    loopStartMats.at(loopStartMats.size()-2).copyTo(prevLoopMat);
    cv::Mat newLoopMat;
    loopStartMats.at(loopStartMats.size()-1).copyTo(newLoopMat);
    
    float prevLoopSum = cv::sum(prevLoopMat)[0] + 1;
    
    cv::Mat diff;
    cv::absdiff(prevLoopMat, newLoopMat, diff);
    float endDiff = cv::sum(diff)[0] + 1;
    
    float changeRatio = endDiff/prevLoopSum;
    //float changePercent = (changeRatio*100);
    if((changeRatio*100) <= (100-loopThresh)){ // the first frames are very similar
        
        if (loopRatings.at(loopRatings.size() - 2) < loopRatings.at(loopRatings.size() -1)) {
            cout << "erasing last one" << endl;
            loopRatings.erase(loopRatings.end() - 1);
            //loopLengths.erase(loopLengths.end() - 1);
            //loopPlayIdx.erase(loopPlayIdx.end() - 1);
            //vector<ofImage *> disp = displayLoops.at(displayLoops.size() - 1);
            /*for (std::vector<ofImage *>::iterator i = disp.begin(); i != disp.end(); ++i){
                delete *i;
            }*/
            //displayLoops.erase(displayLoops.end() - 1);
            loopEnds.erase(loopEnds.end() - 1);
            loopStartMats.erase(loopStartMats.end() - 1);
            
            potentialGifs.at(potentialGifs.size() -1).clear();
            potentialGifs.erase(potentialGifs.end() -1);
        }
        else{
            cout << "erasing second to last" << endl;
            loopRatings.erase(loopRatings.end() - 2 );
            //loopLengths.erase(loopLengths.end() - 2);
            //loopPlayIdx.erase(loopPlayIdx.end() - 2);
            /*vector<ofImage *> disp = displayLoops.at(displayLoops.size() - 2);
            for (std::vector<ofImage *>::iterator i = disp.begin(); i != disp.end(); ++i){
                delete *i;
            }
            displayLoops.erase(displayLoops.end() - 2);*/
            loopEnds.erase(loopEnds.end() - 2);
            
            potentialGifs.at(potentialGifs.size() - 2).clear();
            potentialGifs.erase(potentialGifs.end() - 2);
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------
void saveGifs(){
    for (int i =0; i < potentialGifs.size(); i++) {
        for (int j = 0; j < potentialGifs.at(i).size(); j++) {
            stringstream imageName;
            imageName << "../data/loop_" << i << "_" << j << ".jpg";
            string filename = imageName.str();
            imwrite(filename.c_str(), potentialGifs.at(i).at(j));
        }
    }
    //clear everything
    for (int i = 0; i < potentialGifs.size(); i++) {
        potentialGifs.at(i).clear();
    }
    potentialGifs.clear();
}

