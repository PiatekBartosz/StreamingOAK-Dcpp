#include <chrono>
#include <iostream>

// Inludes common necessary includes for development using depthai library
#include "depthai/depthai.hpp"
#include "DepthAiWrapper.h"

int main() {
    DepthAiWrapper app;

    while (true) {
        cv::Mat camRgbPreviewFrame = app.GetCamRgbPreview();
        cv::Mat camRgbVideoFrame = app.GetCamRgbVideo();

        cv::imshow("Preview", camRgbPreviewFrame);
        cv::imshow("Video", camRgbVideoFrame);

        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }

    return 0;
}