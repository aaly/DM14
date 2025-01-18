// Copyright (c) 2017, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#ifndef objectDistance_HPP
#define objectDistance_HPP

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
// #include <opencv2/core/types.hpp>
#include <inttypes.h>

// #include <stdio.h>
// #include <stdlib.h>
#include "Array.hpp"
#include <cstdlib>

using namespace std;
using namespace cv;

typedef std::vector<std::vector<Point>> pointsArray;

typedef cv::Mat openCvMat;
typedef cv::Scalar openCvScalar;
typedef VideoCapture cvVideoCapture;

class objectDistance {
public:
  objectDistance();
  ~objectDistance();

  enum sourceType { IMAGESRC, VIDEOSRC, CAMSRC };

  float realWidth;
  float realHeight;
  float realDistance;
  float pixelsPerCM;
  float focalLength;

  int Init();

  int minimumArea; // minimum area of detected rectangle around the object

  int getCurrentDistance();

  int setLowHSV(int R, int G, int B);
  int setHighHSV(int R, int G, int B);

  int setSrcPath(string path);
  string getSrcPath();

  int setSrcType(sourceType type);
  int getSrcType();

  bool newDataAvailable();

  void help();

  float printFocalLength();

private:
  float determineFocalLength(float realWidth, float pixlesWidth, float distance,
                             float matWidth = 0);
  float pixlesToCM(float pixlesWidth);
  float calculateDistance(Rect rect);
  float calculateDistance(float pixelsWidth);
  bool initImageSource(string srcPath);
  int drawBoundingRects(pointsArray &contours, openCvMat &frame);
  openCvMat GetThresholdedImageHSV(Mat *img);
  openCvMat getImage();
  int findOneItemDistance(pointsArray &contours, openCvMat &frame,
                          int minimumArea);

  sourceType imageSource;
  string srcPath;
  unsigned int readCounter;    // =0;
  cvVideoCapture videoCapture; // source of a video
  openCvMat videoFrame;        // from from the video
  openCvMat currentImage;      // the mat to work on
  bool initDone;               // = false;

  openCvScalar lowHSV;
  openCvScalar highHSV;
};

#endif // objectDistance_HPP
