#include "DepthAiWrapper.h"

DepthAiWrapper::DepthAiWrapper() {
    
    // define source nodes
    auto camRgb = pipeline.create<dai::node::ColorCamera>();

    // define output nodes & name output streams
    auto xlinkOutPreview = pipeline.create<dai::node::XLinkOut>();
    auto xlinkOutVideo = pipeline.create<dai::node::XLinkOut>();
    xlinkOutPreview->setStreamName("rgbPreview");
    xlinkOutVideo->setStreamName("rgbVideo");

    // set nodes properties
    camRgb->setBoardSocket(dai::CameraBoardSocket::CAM_A);
    camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);
    camRgb->setInterleaved(false);
    camRgb->setColorOrder(dai::ColorCameraProperties::ColorOrder::RGB);
    camRgb->setVideoSize(1920, 1080);    
    camRgb->setPreviewSize(480 * 1920 / 1080, 480);

    // link nodes
    camRgb->preview.link(xlinkOutPreview->input);
    camRgb->video.link(xlinkOutVideo->input);

    // connect to device & start pipeline
    device = std::make_unique<dai::Device>(pipeline, dai::UsbSpeed::SUPER);

    // get output queue
    queueCamRgbPreview = device->getOutputQueue("rgbPreview", 4, false);
    queueCamRgbVideo = device->getOutputQueue("rgbVideo", 4, false);
}

DepthAiWrapper::~DepthAiWrapper() {

    // close OpenCV windows when the object is destroyed
    cv::destroyAllWindows(); 
}

cv::Mat DepthAiWrapper::GetCamRgbPreview() {
    return queueCamRgbPreview->get<dai::ImgFrame>()->getCvFrame();
}

cv::Mat DepthAiWrapper::GetCamRgbVideo() {
    return queueCamRgbVideo->get<dai::ImgFrame>()->getCvFrame();
}

