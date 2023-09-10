#pragma once

#include <thread>
#include <depthai/depthai.hpp>

class DepthAiWrapper {
private:
    dai::Pipeline pipeline;

    // dynamically allocated object
    std::unique_ptr<dai::Device> device;
    std::shared_ptr<dai::DataOutputQueue> queueCamRgbPreview;
    std::shared_ptr<dai::DataOutputQueue> queueCamRgbVideo;

public:
    DepthAiWrapper();
    ~DepthAiWrapper();
    cv::Mat GetCamRgbPreview();
    cv::Mat GetCamRgbVideo();
};

