// Avinash Aryal 1001727418


// include necessary dependencies

#include <iostream>
#include <string>
#include <stdio.h>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1
enum MouseButton {LDown,RDown,LUp,RightUp};
class PaintProgram
{
    private:
        int inital_pointx, inital_pointy;
        int final_pointx, final_pointy;
        int tool = 0;
        cv::Mat _initalImageIn;
        cv::Mat _imageIn;

        
        int *_x;
        int _y;
        int eyedropper[3];
    public:
        PaintProgram(cv::Mat imageIn);
        void setInitalXY(int x, int y);
        void setFinalXY(int x, int y);
        void runCommand(MouseButton flag);
        void toolsSelected();
        void setEyeDropperValue();
        void cropImage();


};
PaintProgram::PaintProgram(cv::Mat imageIn): _initalImageIn{imageIn}, _imageIn{imageIn}, eyedropper{255,255,255}
{
    // ;
    cv::imshow("imageIn", _imageIn);

}
void PaintProgram::setInitalXY(int x, int y)
{

    inital_pointx = x;
    inital_pointy = y;
    
}
void PaintProgram::setFinalXY(int x, int y)
{
    final_pointx = x;
    final_pointy = y;
}
void PaintProgram::setEyeDropperValue()
{
    // std::cout << "Inital (" << inital_pointx << ", " << inital_pointy << ")" << std::endl;

    // x axis represnt column, while y-axis represet row. 
    // https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
    eyedropper[0] = _imageIn.at<cv::Vec3b>(inital_pointy, inital_pointx)[0];
    eyedropper[1] = _imageIn.at<cv::Vec3b>(inital_pointy, inital_pointx)[1];
    eyedropper[2] = _imageIn.at<cv::Vec3b>(inital_pointy, inital_pointx)[2];
    std::cout << "BGR( " << eyedropper[0] <<"," << eyedropper[1] << ","<<eyedropper[2] << ")" <<std::endl;
}
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
void PaintProgram::cropImage()
{   if((inital_pointx - final_pointx)== 0 || (inital_pointy - final_pointy)==0 )
        return;
    cv::Rect myROI(cv::Point(inital_pointx,inital_pointy),cv::Point(final_pointx,final_pointy));
    cv::Mat imageROI = _imageIn(myROI);
    cv::imshow("imageIn",imageROI);

}
void PaintProgram::runCommand(MouseButton flag)
{
    switch(tool)
    {
        case 1:
            setEyeDropperValue();
            break;
        case 2:
            if(flag==LUp)
                cropImage();
            break;
        case 3:
            
            break;
        case 4:
            
            break;
        case 5:
            
            break;
    }
}
static void clickCallback(int event, int x, int y, int flags, void* userdata)
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
        // std::cout << "RIGHT CLICK (" << x << ", " << y << ")" << std::endl;
        obj->toolsSelected();
    }
    else if (event == cv::EVENT_LBUTTONUP){
        obj->setFinalXY(x,y);
        obj->runCommand(LUp);
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
        cv::Mat imageIn;
        imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);
        
        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }

        PaintProgram myPaintProgram(imageIn);
        cv::setMouseCallback("imageIn", clickCallback, &myPaintProgram);
        cv::waitKey();
        
    }
}