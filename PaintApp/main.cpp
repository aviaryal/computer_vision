// Avinash Aryal 1001727418
// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1
// Define enum for the mouse Flag
// Left clicked, right click, left release, right release, left double click, mouse move.
enum MouseButton {LDown,RDown,LUp,RightUp,LDC,MH};
class PaintProgram
{
    private:
        // Define necessary variable 
        int inital_pointx = 0, inital_pointy = 0,
            final_pointx = 0, final_pointy = 0,
            hoverx = 0 , hovery = 0,
            tool = 0;
        // Set InitalColor of pixel to white, save of inital Mat image, and save inital eyedropper value
        cv::Vec3b initalColor = cv::Vec3b(255,255,255);
        cv::Mat _initalImageIn;
        cv::Mat _imageIn;
        cv::Vec3b eyedropper = cv::Vec3b(255,255,255);
    public:
        PaintProgram(cv::Mat imageIn);
        void setInitalXY(int x, int y);
        void setFinalXY(int x, int y);
        void runCommand(MouseButton flag);
        void toolsSelected();
        void setEyeDropperValue();
        void cropImage();
        void painBucket(int pointx, int pointy);
        static void clickCallback(int event, int x, int y, int flags, void* userdata);
        void pencil(MouseButton flag);
        void setHoverPostion(int x,int y);
        int getTools();
};

PaintProgram::PaintProgram(cv::Mat imageIn): _initalImageIn{imageIn.clone()}, _imageIn{imageIn}
{
    // Constructor for Class
    cv::imshow("imageIn", _imageIn);
    cv::setMouseCallback("imageIn", clickCallback, this);
    cv::waitKey();

}
void PaintProgram::setInitalXY(int x, int y)
{
    // set Inital point x and y
    inital_pointx = x;
    inital_pointy = y;
    
}
void PaintProgram::setFinalXY(int x, int y)
{
    // set final point x and y
    final_pointx = x;
    final_pointy = y;
}
void PaintProgram::setHoverPostion(int x, int y)
{
    // set final point x and y 
    hoverx = x;
    hovery = y;
    
}
void PaintProgram::setEyeDropperValue()
{
    // set the eyedropper pixel value to the seletected pixel and print
    eyedropper = _imageIn.at<cv::Vec3b>(cv::Point(inital_pointx,inital_pointy));
    std::cout<< "BGR value: "<< eyedropper<<std::endl;
}
void PaintProgram::pencil(MouseButton flag)
{
    // if set the pixel value to eyedroper value when Left click or left click and drag
    if(flag == LDown)
    {
        _imageIn.at<cv::Vec3b>(cv::Point(inital_pointx,inital_pointy)) = eyedropper;
        cv::imshow("imageIn", _imageIn);
    }
    else if(flag == MH)
    {
        _imageIn.at<cv::Vec3b>(hovery,hoverx) = eyedropper;
        cv::imshow("imageIn", _imageIn);
    }
}   
void PaintProgram::cropImage()
{   
    // Check if the points are same
    if((inital_pointx - final_pointx)== 0 || (inital_pointy - final_pointy)==0 )
        return;
    if(final_pointx > _imageIn.rows || final_pointy > _imageIn.cols || final_pointx < 0 || final_pointy < 0) 
        return;
    cv::Rect myROI(cv::Point(inital_pointx,inital_pointy),cv::Point(final_pointx,final_pointy));
    cv::Mat imageROI = _imageIn(myROI);
    cv::resize(imageROI, _imageIn, cv::Size(imageROI.cols, imageROI.rows));
    cv::imshow("imageIn",_imageIn);

}
void PaintProgram::painBucket(int pointx, int pointy)
{
    // get the neighbouring point from each direction.
    int east = pointx + 1;
    int west = pointx - 1;
    int south = pointy + 1;
    int north = pointy - 1;

    /*  leftmost point can't be less than zero,
        upmost value can't be less than zero,
        righmost value can't be greater than number of column
        bottonmost value can't be greater than number of row
        check if the pixel value is the inital color value
        recursively call the function 

    */
    if(west >= 0 && _imageIn.at<cv::Vec3b>(cv::Point(west,pointy)) == initalColor)
    {
        _imageIn.at<cv::Vec3b>(cv::Point(west,pointy)) = eyedropper;
        painBucket(west,pointy);
    }

    if (north >=0 && _imageIn.at<cv::Vec3b>(cv::Point(pointx, north)) == initalColor)
    {
        _imageIn.at<cv::Vec3b>(cv::Point(pointx, north)) = eyedropper;
        painBucket(pointx,north);
    }
    if( east < _imageIn.cols && _imageIn.at<cv::Vec3b>(cv::Point(east,pointy)) == initalColor)
    {
        _imageIn.at<cv::Vec3b>(cv::Point(east,pointy)) = eyedropper;
        painBucket(east,pointy);
    }
    if( south < _imageIn.rows && _imageIn.at<cv::Vec3b>(cv::Point(pointx, south)) == initalColor)
    {
        _imageIn.at<cv::Vec3b>(cv::Point(pointx, south)) = eyedropper;
        painBucket(pointx,south);
    }

}
// function to print which tool selected.
void PaintProgram::toolsSelected()
{
    ++tool;
    if(tool == 6)
        tool = 1;
    switch(tool)
    {
        case 1:
            std::cout << "EyeDropper Selected" <<std::endl;
            break;
        case 2:
            std::cout << "Crop Selected" <<std::endl;
            break;
        case 3:
            std::cout << "Pencil Selected" <<std::endl;
            break;
        case 4:
            std::cout << "Paint Bucket  Selected" <<std::endl;
            break;
        case 5:
            std::cout << "Reset Selected" <<std::endl;
            break;
    }
}

// function to run command
void PaintProgram::runCommand(MouseButton flag)
{
    switch(tool)
    {
        case 1:
            // only set eyedropper color value when left click
            if(flag==LDown)
                setEyeDropperValue();
            break;
        case 2:
            // only crop when left click is released
            if(flag==LUp)
                cropImage();
            break;
        case 3:
            pencil(flag);
            break;
        case 4:
            // only fill the bucket when flg is down.
            if(flag == LDown)
            {
                initalColor = _imageIn.at<cv::Vec3b> (inital_pointy,inital_pointx);
                _imageIn.at<cv::Vec3b> (inital_pointy,inital_pointx) = eyedropper;
                painBucket(inital_pointx,inital_pointy);
                cv::imshow("imageIn", _imageIn);
            }
            
            break;
        case 5:
            // only run when mouse double clik
            if(flag==LDC)
            {
                _imageIn = _initalImageIn.clone();
                cv::imshow("imageIn",_imageIn);
            }
            break;
    }
}
// return whihc tool is selected.
int PaintProgram::getTools(){return tool;}

void PaintProgram::clickCallback(int event, int x, int y, int flags, void* userdata)
{
    PaintProgram *obj = static_cast <PaintProgram *> (userdata);

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        // std::cout << "LEFT CLICK (" << x << ", " << y << ")" << std::endl;
        obj->setInitalXY(x,y);
        obj->runCommand(LDown);

    }
    else if(event == cv::EVENT_RBUTTONDOWN)
    {
        // Right Buttom Down click
        // std::cout << "RIGHT CLICK (" << x << ", " << y << ")" << std::endl;
        obj->toolsSelected();
    }
    else if (event == cv::EVENT_LBUTTONUP)
    {   
        // Left buttom up
        obj->setFinalXY(x,y);
        obj->runCommand(LUp);
    }
    else if(event == cv::EVENT_LBUTTONDBLCLK )
    {
        // Left Buttom Double Click event
        obj->runCommand(LDC);
    }
    else if(event == cv::EVENT_MOUSEMOVE && flags == cv::EVENT_LBUTTONDOWN)
    {
        // Left Mouse Click and Drag
        obj->setHoverPostion(x,y);
        obj->runCommand(MH);
    }                                                
    
}


int main(int argc, char **argv)
{

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        cv::Mat imageIn, initalImageIn;
        imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);
        
        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
        // cv::Vec3b eyedropper;
        // eyedropper[0] = 255;
        // eyedropper[1] = 255;
        // eyedropper[2] = 255;
        
        PaintProgram myPaintProgram(imageIn);
        

        
        
    }
}