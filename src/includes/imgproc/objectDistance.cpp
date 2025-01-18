#include "objectDistance.hpp"
objectDistance::objectDistance() {
  initDone = false;
  readCounter = 0;
  minimumArea = 0;

  realWidth = 13;
  realHeight = 8.2;
  realDistance = 31;
  pixelsPerCM = 24.6154;
  focalLength = 763.077;

  srcPath = "0";
}

Mat frame;

bool valueInRange(int value, int min, int max) {
  return (value >= min) && (value <= max);
}

bool rectOverlap(int Ax, int Ay, int Awidth, int Aheight, int Bx, int By,
                 int Bwidth, int Bheight) {
  bool xOverlap =
      valueInRange(Ax, Bx, Bx + Bwidth) || valueInRange(Bx, Ax, Ax + Awidth);

  bool yOverlap =
      valueInRange(Ay, By, By + Bheight) || valueInRange(By, Ay, Ay + Aheight);

  return xOverlap && yOverlap;
}

objectDistance::~objectDistance() {}

// calculates focal length in mm
float objectDistance::determineFocalLength(float realWidth, float pixlesWidth,
                                           float distance, float matWidth = 0) {
  cerr << "Calcualte with : " << pixlesWidth << ":" << distance << ":"
       << realWidth << endl;
  focalLength = (pixlesWidth * distance) / realWidth;
  pixelsPerCM = pixlesWidth / realWidth;
  cerr << "FOV:" << pixelsPerCM / pixlesWidth << endl;
  cerr << "FOV:" << pixlesWidth << endl;
  return focalLength;
}

float objectDistance::printFocalLength() {
  cerr << "Focal Length : " << focalLength << endl;
  cerr << "Pixels per CM : " << pixelsPerCM << endl;
  return 0;
}

// converts length in pixles to length in centimeters !
float objectDistance::pixlesToCM(float pixlesWidth) {
  return pixlesWidth / pixelsPerCM;
}

float objectDistance::calculateDistance(Rect rect) {
  float distance = (realWidth * focalLength) / (rect.br().y - rect.tl().y);
  // float distance =
  // (pixlesToCM(rect.br().y-rect.tl().y)*focalLength)/(rect.br().y-rect.tl().y);
  // cerr << "Distance : " << distance << endl << flush;
  // cerr << "Size : " << pixlesToCM(rect.br().y-rect.tl().y) << endl << flush;
  return distance;
}

float objectDistance::calculateDistance(float pixelsWidth) {
  // cerr << realWidth << ":" << focalLength << ":" << pixelsWidth << endl;
  // cerr << pixelsWidth/pixelsPerCM << endl;
  float distance = (realWidth * focalLength) / (pixelsWidth);
  // float distance =
  // (pixlesToCM(rect.br().y-rect.tl().y)*focalLength)/(rect.br().y-rect.tl().y);
  // cerr << "Distance : " << distance << endl << flush;
  // cerr << "Size : " << pixlesToCM(rect.br().y-rect.tl().y) << endl << flush;
  return distance;
}

bool objectDistance::initImageSource(string srcPath) {
  if (imageSource == IMAGESRC) {
    currentImage = imread(srcPath.c_str(), IMREAD_COLOR);
    if (currentImage.empty()) {
      cerr << "[Error]: Can not open image file : " << srcPath << endl << flush;
      return false;
    }
  } else if (imageSource == VIDEOSRC || imageSource == CAMSRC) {

    if (imageSource == VIDEOSRC) {
      videoCapture = VideoCapture(srcPath.c_str());
    } else {
      videoCapture = VideoCapture(strtoumax(srcPath.c_str(), NULL, 10));
    }

    // videoCapture = cvCaptureFromFile(srcPath.c_str());
    if (!videoCapture.isOpened()) {
      if (imageSource == VIDEOSRC) {
        cerr << "[Error]: Could not open video file : " << srcPath << endl
             << flush;
      } else {
        cerr << "[Error]: Cannot open the web cam" << endl;
      }
      return false;
    }
  } else {
    cerr << "[Error]: unknown soource oath or type " << endl << flush;
    return false;
  }

  // initDone = true;
  return true;
}

bool objectDistance::newDataAvailable() {
  if ((imageSource == IMAGESRC) && readCounter) {
    return false;
  } else if ((imageSource == VIDEOSRC) &&
             readCounter > videoCapture.get(CV_CAP_PROP_FRAME_COUNT)) {
    return false;
  }

  // if(imageSource == CAMSRC)
  {
    // return true;
  }

  return true;
}

openCvMat objectDistance::getImage() {
  if (!initDone) {
    cerr << "[Error] : tried to get a mage before initilizating the source"
         << endl
         << flush;
  }

  if (imageSource == IMAGESRC) {
    // if(!readCounter)
    {
      readCounter++;
      return currentImage;
    }

  } else if (imageSource == VIDEOSRC || imageSource == CAMSRC) {
    // bool blnFrameReadSuccessfully = videoCapture.read(currentImage);
    if (videoCapture.isOpened() && videoCapture.grab() &&
        videoCapture.retrieve(currentImage)) {
      readCounter++;
      return currentImage;
    }
    /*videoCapture >> currentImage;
    //videoFrame = cvQueryFrame(videoCapture);
    if(!currentImage.empty())
    {
            //currentImage = cv::cvarrToMat(videoFrame, false);
            readCounter++;
            return currentImage;
    }*/
    else {
      cerr << "[Error] : empty frame" << endl << flush;
    }
  }

  return Mat();
}

void objectDistance::help() {
  cout << "\nthis application is used to Detect red led and determine the "
          "distance \n"
          "Usage:\n"
          "-h,--h,-help,--help, this help message\n"
          "-I <image_name>, image file\n"
          "-V <video_file_name>, video file\n"
          "-C <Camera_number>, cam source number\n"
          "Ex : -I test.jpg\n"
       << endl;
}

// Get thresholded image in HSV format
/*openCvMat objectDistance::GetGreyImage(Mat* img)
{
        Mat grayMAT;
        /// Convert image to gray and blur it
        cvtColor(*img, grayMAT, CV_BGR2GRAY );
        blur( src_gray, grayMAT, Size(3,3) );


}*/

openCvMat objectDistance::GetThresholdedImageHSV(Mat *img) {
  // Create an HSV format image from image passed
  Mat imgHSV; // = img->clone();

  // GaussianBlur(), blur(), medianBlur() or bilateralFilter()
  // docs.opencv.org/2.4/doc/tutorials/imgproc/gausian_median_blur_bilateral_filter/gausian_median_blur_bilateral_filter.html
  /// medianBlur(imgHSV, imgHSV, 5);

  /// cv::dilate(imgHSV, imgHSV, 0);        // Dilate Filter Effect
  /// cv::erode(imgHSV, imgHSV, 0);         // Erode Filter Effect

  cvtColor(*img, imgHSV, CV_BGR2HSV);

  cv::GaussianBlur(imgHSV, imgHSV, cv::Size(3, 3), 0); // Blur Effect

  // Create binary thresholded image acc. to max/min HSV ranges
  // Mat imgThresholded;

  inRange(imgHSV, lowHSV, highHSV,
          imgHSV); // Threshold the image

  // morphological opening (removes small objects from the foreground)
  // Morphological opening
  erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

  // morphological closing (removes small holes from the foreground)
  // Morphological closing
  dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

  if (imgHSV.empty()) {
    cerr << "[Error] : failed to get threshold image !" << endl << flush;
  }

  Mat hsv_channels[3];
  cv::split(imgHSV, hsv_channels);
  // same size as src and CV_8U type.
  return hsv_channels[0];
}

/*bool objectDistance::circleInsideCircle(Point2f center1, float radius1,
Point2f center2, float radius2)
{

        return false;
}*/

int objectDistance::findOneItemDistance(pointsArray &contours,
                                        openCvMat &imageThreshMat,
                                        int minimumArea) {
  ////std::vector<std::vector<Point> > contours;
  // blur( imageThreshMat, imageThreshMat, Size(3,3) );
  // Canny( imageThreshMat, imageThreshMat, 100, 50*3, 3);

  ////findContours( imageThreshMat, contours, CV_RETR_EXTERNAL,
  ///CV_CHAIN_APPROX_SIMPLE );

  // cvNamedWindow("thresh",WINDOW_NORMAL);
  // cvResizeWindow("thresh", 200,200);
  // imshow("thresh", imageThreshMat);
  //??cvWaitKey(100);

  Array<Array<Point>> contor_poly(contours.size());
  std::vector<std::vector<Point>> final_contours;
  std::vector<Point2f> center(contours.size());
  std::vector<float> radius(contours.size());

  std::vector<Point2f> finalCenter;
  std::vector<float> finalRadius;

  // cerr << "SIZE:" << contours.size() << endl << flush;
  for (int i = 0; i < contours.size(); i++) {
    approxPolyDP(Mat(contours[i]), contor_poly[i],
                 0.01 * arcLength(contours[i], true), true);
    // circle( imageThreshMat, finalCenter[i], (int)finalRadius[i],
    // cv::Scalar(255, 0, 0), 2, 8, 0 );
    if (contor_poly[i].size() > 4) {
      final_contours.push_back(contours[i]);
      minEnclosingCircle((Mat)contor_poly[i], center[i], radius[i]);
      circle(imageThreshMat, center[i], (int)radius[i], cv::Scalar(255, 0, 0),
             2, 8, 0);
    }
  }

  for (int i = 1000000; i < center.size(); i++) {
    if (radius[i] == -1) {
      continue;
    }

    bool push = true;

    int clearArea = 0;
    int topLeftX = center[i].x - (radius[i] + clearArea);
    int topLeftY = center[i].y - (radius[i] + clearArea);
    int bottomRightX = center[i].x + (radius[i] + clearArea);
    int bottomRightY = center[i].y + (radius[i] + clearArea);

    for (int k = 0; k < center.size(); k++) {
      if (i == k || radius[k] == -1) {
        continue;
      }

      int topLeftX1 = center[k].x - (radius[k] + clearArea);
      int topLeftY1 = center[k].y - (radius[k] + clearArea);
      int bottomRightX1 = center[k].x + (radius[k] + clearArea);
      int bottomRightY1 = center[k].y + (radius[k] + clearArea);

      // if( center[i].x > topLeftX && center[i].y > topLeftY &&
      //	center[i].x < bottomRightX && center[i].y < bottomRightY )
      // if( topLeftX > topLeftX1 && topLeftY1 < topLeftY &&
      //	bottomRightX1 < bottomRightX && bottomRightY1 > bottomRightY )

      int intersection =
          rectOverlap(topLeftX, topLeftY, radius[i] * 2, radius[i] * 2,
                      topLeftX1, topLeftY1, radius[k] * 2, radius[k] * 2);
      if (intersection) {
        if (radius[i] > radius[k]) {
          radius[i] += (radius[k] - radius[i]) * -1;
          radius[k] = -1;
          center[i] = center[i] + center[k];
        } else {
          radius[k] += (radius[i] - radius[k]) * -1;
          radius[i] = -1;
          center[k] = center[k] + center[i];
          break;
        }
        // push = false;
        // break;
      }
    }

    // remove me
    /*push = true;

    if (push)
    {
            finalCenter.push_back(center[i]);
            finalRadius.push_back(radius[i]);
    }*/
  }

  // determineFocalLength(realWidth, radius[0]*2, realDistance);
  // printFocalLength();

  // cerr << "SIZE : " << center.size() << endl;

  for (int i = 0; i < center.size(); i++) {
    if (radius[i] != -1) {
      finalCenter.push_back(center[i]);
      finalRadius.push_back(radius[i]);
    }
  }

  int index = -1;
  int biggestRadius = -1;

  for (size_t i = 0; i < finalCenter.size(); i++) {
    // drawContours(frame, final_contours, idx, cv::Scalar(255, 0, 0),
    // CV_FILLED);
    /// drawContours(imageThreshMat, final_contours, i, cv::Scalar(255, 0, 0));
    circle(frame, finalCenter[i], (int)finalRadius[i], cv::Scalar(255, 0, 0), 2,
           8, 0);

    char readCounterString[20];
    snprintf(readCounterString, sizeof(readCounterString), "frame : %d",
             readCounter);
    putText(frame, readCounterString, cvPoint(10, 20),
            FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200, 200, 250), 1, CV_AA);

    char objectDistanceString[100];
    snprintf(objectDistanceString, sizeof(objectDistanceString),
             "distance : %.f cm", calculateDistance(finalRadius[i] * 2));
    putText(frame, objectDistanceString,
            cvPoint(finalCenter[i].x, finalCenter[i].y + finalRadius[i]),
            FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(250, 250, 250), 1, CV_AA);

    if (finalRadius[i] > biggestRadius) {
      index = i;
    }
  }

  // cvNamedWindow("video",WINDOW_NORMAL);
  // cvResizeWindow("video", 200,200);
  // imshow("video", frame);
  //??cvWaitKey(100);

  if (index != -1) {
    // cerr << "DISTANCCE:" << calculateDistance(finalRadius[index]*2) << endl;
    return calculateDistance(finalRadius[index] * 2);
  }

  return -1;

  /*

  Array<RotatedRect> boundRect(contours.size());
  Array<Array<Point> > contor_poly(contours.size());
  for(int i =0; i< contours.size(); i++)
  {
          approxPolyDP(Mat(contours[i]), contor_poly[i], 3, true);
          boundRect[i] = minAreaRect(Mat(contor_poly[i]));
  }

  int index = -1;
  int biggestObject = -1;
  for(int i =0; i< boundRect.size(); i++)
  {
          int a = boundRect[i].boundingRect().area();
          if(a > minimumArea && a > biggestObject)
          {
                  biggestObject = a;
                  index = i;
          }
  }

  if (index != -1)
  {
          return calculateDistance(boundRect[index].boundingRect());
  }
  return -1;
  * */
}

int objectDistance::drawBoundingRects(pointsArray &contours, openCvMat &frame) {
  Array<RotatedRect> boundRect(contours.size());
  Array<Array<Point>> contor_poly(contours.size());
  for (int i = 0; i < contours.size(); i++) {
    approxPolyDP(Mat(contours[i]), contor_poly[i], 3, true);
    boundRect[i] = minAreaRect(Mat(contor_poly[i]));
  }

  int index = -1;
  int biggestSize = 0;
  for (int i = 0; i < boundRect.size(); i++) {
    if (boundRect[i].boundingRect().area() > biggestSize) {
      biggestSize = boundRect[i].boundingRect().area();
      index = i;
    }
    continue;

    int a = boundRect[i].boundingRect().area();

    if (a > 600) {
      // rotated rectangle
      Point2f rect_points[4];
      boundRect[i].points(rect_points);
      for (int j = 0; j < 4; j++) {
        line(frame, rect_points[j], rect_points[(j + 1) % 4],
             Scalar(255, 25 * i, 0), 1, 8);
      }
      // rectangle(frame, boundRect[i].boundingRect().tl(),
      // boundRect[i].boundingRect().br(), Scalar(255,255,0), 2, 8, 0);
      char objectDistanceString[100];
      snprintf(objectDistanceString, sizeof(objectDistanceString),
               "distance : %.f cm",
               calculateDistance(boundRect[i].boundingRect()));
      // cout << boundRect[i].boundingRect()
      // snprintf(objectDistanceString, sizeof(objectDistanceString), "distance
      // : %d cm", calculateDistance(boundRect[i].boundingRect()));
      putText(frame, objectDistanceString,
              cvPoint(boundRect[i].boundingRect().tl().x,
                      boundRect[i].boundingRect().br().y + 15),
              FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(250, 250, 250), 1,
              CV_AA);
      // break; // only one object !
    }
  }

  if (index != -1) {
    // rotated rectangle
    Point2f rect_points[4];
    boundRect[index].points(rect_points);
    for (int j = 0; j < 4; j++) {
      line(frame, rect_points[j], rect_points[(j + 1) % 4],
           Scalar(255, 25 * index, 0), 1, 8);
    }
    // rectangle(frame, boundRect[i].boundingRect().tl(),
    // boundRect[i].boundingRect().br(), Scalar(255,255,0), 2, 8, 0);
    char objectDistanceString[100];
    snprintf(objectDistanceString, sizeof(objectDistanceString),
             "distance : %.f cm",
             calculateDistance(boundRect[index].boundingRect()));
    // cout << boundRect[i].boundingRect()
    // snprintf(objectDistanceString, sizeof(objectDistanceString), "distance :
    // %d cm", calculateDistance(boundRect[i].boundingRect()));
    putText(frame, objectDistanceString,
            cvPoint(boundRect[index].boundingRect().tl().x,
                    boundRect[index].boundingRect().br().y + 15),
            FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(250, 250, 250), 1, CV_AA);
  }
  return 0;
}

int objectDistance::setLowHSV(int R, int G, int B) {
  lowHSV = Scalar(R, G, B);
  // cerr << "L:" << R << G << B << endl;
  return 0;
}

int objectDistance::setHighHSV(int R, int G, int B) {
  // cerr << "H:" << R << G << B << endl;
  highHSV = Scalar(R, G, B);
  return 0;
}

int objectDistance::setSrcPath(string path) {
  srcPath = path;
  return 0;
}

string objectDistance::getSrcPath() { return srcPath; }

int objectDistance::setSrcType(sourceType type) {
  imageSource = type;
  return 0;
}

int objectDistance::getSrcType() { return imageSource; }

int objectDistance::Init() {
  if (!srcPath.size()) {
    cerr << "[Error] : source parh not specified" << endl << flush;
    // help();
    return -1;
  }

  if (!initImageSource(srcPath)) {
    cerr << "[Help] : is the source path specified correctly ?" << endl
         << flush;
    // help();
    return -1;
  }

  initDone = true;
  return 0;
}

int objectDistance::getCurrentDistance() {

  if (!initDone) {
    cerr << "Forgot to call Init() ?" << endl << flush;
    return -1;
  }

  frame = getImage();

  if (frame.empty()) {
    cerr << "[Error] : Failed to get image !" << endl << flush;
    return -1;
  }

  // Mat imageThreshMat = GetThresholdedImageHSV(&frame);

  Mat imgHSV;
  cvtColor(frame, imgHSV, CV_BGR2HSV);
  cv::Mat lower_red_hue_range;
  cv::Mat upper_red_hue_range;

  // cv::inRange(imgHSV, cv::Scalar(165, 140, 120), cv::Scalar(180, 255, 255),
  // lower_red_hue_range);
  cv::inRange(imgHSV, cv::Scalar(165, 120, 110), cv::Scalar(180, 255, 255),
              lower_red_hue_range);
  cv::inRange(imgHSV, cv::Scalar(0, 130, 130), cv::Scalar(8, 255, 255),
              upper_red_hue_range); // orange? !
  // Combine the above two images
  cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0,
                  imgHSV);

  // cv::GaussianBlur(imgHSV, imgHSV, cv::Size(9, 9), 2, 2);

  // morphological opening (removes small objects from the foreground)
  // Morphological opening
  erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

  // morphological closing (removes small holes from the foreground)
  // Morphological closing
  dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

  Mat hsv_channels[3];
  cv::split(imgHSV, hsv_channels);
  // same size as src and CV_8U type.
  Mat imageThreshMat = hsv_channels[0];

  // cv::GaussianBlur(imageThreshMat, imageThreshMat, cv::Size(9, 9), 2, 2);

  std::vector<std::vector<Point>> contours;
  findContours(imageThreshMat, contours, CV_RETR_EXTERNAL,
               CV_CHAIN_APPROX_SIMPLE);

  cv::Mat structuringElement =
      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(80, 80));
  cv::morphologyEx(imageThreshMat, imageThreshMat, cv::MORPH_CLOSE,
                   structuringElement);

  // erode(imageThreshMat, imageThreshMat, getStructuringElement(MORPH_ELLIPSE,
  // Size(5, 5)) ); dilate(imageThreshMat, imageThreshMat,
  // getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

  return findOneItemDistance(contours, imageThreshMat, minimumArea);

  if (imgHSV.empty()) {
    cerr << "[Error] : failed to get threshold image !" << endl << flush;
  }

  /*Mat hsv_channels[3];
  cv::split( imgHSV, hsv_channels );
  //same size as src and CV_8U type.
Mat imageThreshMat = hsv_channels[0];
  */
  // imshow("video", imageThreshMat);
  // cvWaitKey(0);

  // Find the contours.

  /*
  Mat imgThresholded;

  inRange(frame,  lowHSV,
                                  highHSV,
                                  imgThresholded); //Threshold the image


  // smooth it, otherwise a lot of false circles may be detected
GaussianBlur( imgThresholded, imgThresholded, Size(9, 9), 2, 2 );
std::vector<Vec3f> circles;

HoughCircles(imgThresholded, circles, CV_HOUGH_GRADIENT,
           2, imgThresholded.rows/4, 200, 100 );
for( size_t i = 0; i < circles.size(); i++ )
{
   Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
   int radius = cvRound(circles[i][2]);
   // draw the circle center
   circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
   // draw the circle outline
   circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
}*/

  // openCvMat gray;
  // cvtColor(imageThreshMat, gray, CV_BGR2GRAY );

  //
  //	drawBoundingRects(contours, frame);
  //	char readCounterString[20];
  //	snprintf(readCounterString, sizeof(readCounterString), "frame : %d",
  //readCounter); 	putText(frame, readCounterString,  cvPoint(10,20),
  //FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
  // imshow("video", imageThreshMat);

  // cvNamedWindow("video",WINDOW_NORMAL);
  // cvResizeWindow("video", 400,400);
  // imshow("video", frame);

  // updateWindow("video");
  // cvWaitKey(50);
  // return -1;
  //

  // videoCapture.release();
  // initDone = false;

  // std::vector<std::vector<Point> > contours;
  // findContours( imageThreshMat, contours, CV_RETR_EXTERNAL,
  // CV_CHAIN_APPROX_SIMPLE );

  // return findOneItemDistance(contours, imageThreshMat, minimumArea);
}
