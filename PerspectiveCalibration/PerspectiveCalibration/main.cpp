#include "depthai/depthai.hpp"
#include <stdio.h>
#include <vector>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


struct MouseParameters {
    std::vector<cv::Point> corners;
    bool warped = false;
    cv::Mat transformMat;
};

void serializeCvMat(cv::Mat& inputMat, std::string& outputFileName);
void MouseCallbackFunction(int event, int x, int y, int flags, void* userdata);

void serializeCvMat(cv::Mat& inputMat, std::string& outputFileName) {
    std::ofstream outputFileStream(outputFileName, std::ios::binary);
    boost::archive::text_oarchive outArchive(outputFileStream);

    int rows = inputMat.rows;
    int cols = inputMat.cols;
    int type = inputMat.type();

    // serialize cv::Mat metadata
    outArchive << rows;
    outArchive << cols;
    outArchive << type;

    // serialize every element of cv::Mat, assuming that the matrix is continous   
    const  size_t dataSize = rows * cols * inputMat.elemSize();
    outArchive << boost::serialization::make_array(inputMat.ptr(), dataSize);
}

void MouseCallbackFunction(int event, int x, int y, int flags, void* userdata) {
    MouseParameters* mp = (MouseParameters*)userdata;

    // get mouse position after LPM click
    if (event == cv::EVENT_LBUTTONDOWN && !(mp->warped)) {
        mp->corners.push_back(cv::Point(x, y));
    }
}

int main(void) {
    std::string outputFileName = "perspectiveCalibrationMatrix.dat";
    MouseParameters mp;
    cv::Scalar colors[4] = { cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar (0, 255, 255) };

    // create pipeline
    dai::Pipeline pipeline;

    // define source and output
    auto camRgb = pipeline.create<dai::node::ColorCamera>();
    auto xoutRgb = pipeline.create<dai::node::XLinkOut>();

    xoutRgb->setStreamName("rgb");

    // properties
    camRgb->setPreviewSize(416, 416);
    camRgb->setBoardSocket(dai::CameraBoardSocket::CAM_A);
    camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);
    camRgb->setInterleaved(false);
    camRgb->setColorOrder(dai::ColorCameraProperties::ColorOrder::RGB);

    // linking
    camRgb->preview.link(xoutRgb->input);

    // connect to device and start pipeline
    dai::Device device(pipeline, dai::UsbSpeed::SUPER);

    std::cout << "Connected cameras: " << device.getConnectedCameraFeatures() << std::endl;

    // print USB speed
    std::cout << "Usb speed: " << device.getUsbSpeed() << std::endl;

    // bootloader version
    if (device.getBootloaderVersion()) {
        std::cout << "Bootloader version: " << device.getBootloaderVersion()->toString() << std::endl;
    }

    // device name
    std::cout << "Device name: " << device.getDeviceName() << std::endl;

    // output queue will be used to get the rgb frames from the output defined above
    auto qRgb = device.getOutputQueue("rgb", 4, false);

    std::cout << "Mark conrners of the warp in this order:" << std::endl;
    std::cout << "TOP_LEFT, BOT_LEFT, BOT_RIGHT, TOP_RIGHT" << std::endl;
    
    while (true) {
        // get frame
        std::shared_ptr<dai::ImgFrame> inRgb = qRgb->get<dai::ImgFrame>();
        cv::Mat frame = inRgb->getCvFrame();
        int height = frame.size().height;
        int width = frame.size().width;

        for (int i = 0; i < mp.corners.size(); ++i) {
            cv::circle(frame, mp.corners[i], 5, colors[i], -1);
        }

        cv::namedWindow("Perspective Calibration");

        cv::putText(frame, "Mark conrners of the woarp in this order:",
            cv::Point2i(10, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        cv::putText(frame, "TOP_LEFT, BOT_LEFT, BOT_RIGHT, TOP_RIGHT",
            cv::Point2i(10, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        cv::imshow("Perspective Calibration", frame);

        // set callback
        cv::setMouseCallback("Perspective Calibration", MouseCallbackFunction, &mp);

        if (mp.corners.size() == 4 && !(mp.warped)) {
            // create dst points
            cv::Point2f topLeft = mp.corners[0];
            cv::Point2f botLeft = mp.corners[1];
            cv::Point2f botRight = mp.corners[2];
            cv::Point2f topRight = mp.corners[3];
            cv::Point2f srcPoints[4] = {topLeft, botLeft, botRight, topRight};

            // create src points
            cv::Point2f dstPoints[4] = {cv::Point2f(0, 0), cv::Point2f(0, height), cv::Point2f(width, height), cv::Point2f(width, 0) };

            // get perspective transform matrix
            mp.transformMat = cv::getPerspectiveTransform(srcPoints, dstPoints);
            mp.warped = true;
        }

        else if (mp.warped) {
            // warp frame
            cv::Mat warpedFrame;
            cv::warpPerspective(frame, warpedFrame, mp.transformMat, cv::Size(width, height));

            cv::putText(warpedFrame, "If the warp is correct press y,", 
                cv::Point2i(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
            cv::putText(warpedFrame, "otherwise press n", 
                cv::Point2i(20, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

            cv::imshow("Warped perspecive", warpedFrame);

            int key = cv::waitKey(1);
            if (key == 'y' || key == 'Y') {
                serializeCvMat(mp.transformMat, outputFileName);
                break;
            }

            else if (key == 'n' || key == 'N') {
                std::cout << "perspectiveCalibrationMatrix.dat";
                cv::destroyAllWindows();
            }
        }

        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
    return 0;
}

