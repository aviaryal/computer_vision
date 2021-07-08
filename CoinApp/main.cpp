//
//    Copyright 2021 Christopher D. McMurrough
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*******************************************************************************************************************//**
 * @file cv_ellipse.cpp
 * @brief C++ example of Canny edge detection and ellipse model fitting in OpenCV
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1

/*******************************************************************************************************************//**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
int main(int argc, char **argv)
{
    int penny = 0, nickle = 0, dime = 0, quater = 0;
    cv::Mat imageIn;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);

        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
    }

    // get the image size
    std::cout << "image width: " << imageIn.size().width << std::endl;
    std::cout << "image height: " << imageIn.size().height << std::endl;
    std::cout << "image channels: " << imageIn.channels() << std::endl;

    cv::Mat imageResize;
    cv::resize(imageIn,imageResize,cv::Size(imageIn.cols / 4, imageIn.rows / 4));
    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageResize, imageGray, cv::COLOR_BGR2GRAY);

    // find the image edges
    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 200;
    const int cannyAperture = 3;
    cv::Canny(imageGray, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);
    
    // erode and dilate the edges to remove noise
    int morphologySize = 1;
    cv::Mat edgesDilated;
    cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1, -1), morphologySize);
    cv::Mat edgesEroded;
    cv::erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1, -1), morphologySize);
    
    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    //std::vector<int> hierarchy;
    cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // draw the contours
    cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    cv::RNG rand(12345);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::drawContours(imageContours, contours, i, color);
    }

    // compute minimum area bounding rectangles
    std::vector<cv::RotatedRect> minAreaRectangles(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute a minimum area bounding rectangle for the contour
        minAreaRectangles[i] = cv::minAreaRect(contours[i]);
    }

    // draw the rectangles
    cv::Mat imageRectangles = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::Point2f rectanglePoints[4];
        minAreaRectangles[i].points(rectanglePoints);
        for(int j = 0; j < 4; j++)
        {
            cv::line(imageRectangles, rectanglePoints[j], rectanglePoints[(j+1) % 4], color);
        }
    }

    // fit ellipses to contours containing sufficient inliers
    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
        if(contours.at(i).size() > 5)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    // draw the ellipses
    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    const int minEllipseInliers = 50;
    for(int i = 0; i < contours.size(); i++)
    {
        // draw any ellipse with sufficient inliers
        if(contours.at(i).size() > minEllipseInliers)
        {
            cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
            
            cv::Point2f vtx[4];
            fittedEllipses[i].points(vtx);
            double radius = cv::norm(vtx[0]-vtx[2]);
            //std::cout<<radius<<std::endl;
            if(radius>250 && radius < 260)
            {
                // draw quater in green
                cv::ellipse(imageEllipse, fittedEllipses[i], cv::Scalar(0,255,0), 2);
                cv::ellipse(imageResize,fittedEllipses[i], cv::Scalar(0,255,0), 2);
                quater++;
            }
            else if(radius > 215 && radius < 230)
            {
                // draw nickel in yellow
                cv::ellipse(imageEllipse, fittedEllipses[i],cv::Scalar(0,255,255), 2);
                cv::ellipse(imageResize,fittedEllipses[i], cv::Scalar(0,255,255), 2);
                nickle++;
            }
            else if(radius > 190 && radius < 200)
            {
                // draw pennny in red
                cv::ellipse(imageEllipse, fittedEllipses[i],cv::Scalar(0,0,255), 2);
                cv::ellipse(imageResize,fittedEllipses[i], cv::Scalar(0,0,255), 2);
                penny++;
            }
            else if(radius < 190 && radius > 180)
            {
                // draw dime blue
                cv::ellipse(imageEllipse, fittedEllipses[i],cv::Scalar(255,0,0), 2);
                cv::ellipse(imageResize,fittedEllipses[i], cv::Scalar(255,0,0), 2);
                dime++;
            }
            
        }
    }
    std::cout<<"Penny :-" << penny <<std::endl;
    std::cout<<"Nickle :-" << nickle <<std::endl;
    std::cout<<"Dime :-" << dime <<std::endl;
    std::cout<<"Quater :-" << quater <<std::endl;
    double value = quater*0.25 + dime * 0.10 + nickle*0.05 + penny*0.01;
    std::cout<<"Total :- $"<<value<<std::endl;
    // display the images
    // cv::imshow("imageIn", imageResize);
    // cv::imshow("imageGray", imageGray);
    // cv::imshow("imageEdges", imageEdges);
    // cv::imshow("edges dilated", edgesDilated);
    // cv::imshow("edges eroded", edgesEroded);
    // cv::imshow("imageContours", imageContours);
    // cv::imshow("imageRectangles", imageRectangles);
    // cv::imshow("imageEllipse", imageEllipse);
    cv::resize(imageResize,imageResize,cv::Size(imageResize.cols / 2, imageResize.rows / 2));
    cv::imshow("imageIn", imageResize);
        
    cv::waitKey();
}
