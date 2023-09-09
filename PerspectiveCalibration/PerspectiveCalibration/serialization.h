#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


void serializeCvMat(cv::Mat inputMat, std::string outputFileName);
void deserializeCvMat(cv::Mat outputMat, std::string inputFileName);

