// Copyright (C) 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>

#include <opencv2/imgproc/imgproc.hpp>
#include <samples/ocv_common.hpp>
#include <samples/common.hpp>

#include "vehicle_detect.hpp"


using namespace InferenceEngine;

VehicleDetect::VehicleDetect(bool enablePerformanceReport)
    :mDetectThreshold(0.65f),
    mEnablePerformanceReport(enablePerformanceReport)
{
    return;
}

void VehicleDetect::Init(const std::string& detectorModelPath,
        const std::string& vehicleAttribsModelPath,
        const std::string& targetDeviceName)
{
    static std::mutex initLock;
    std::lock_guard<std::mutex> lock(initLock);

    //Load Vehicle Detector Network
    mDetectorNetReader.ReadNetwork(detectorModelPath);
    std::string binFileName = fileNameNoExt(detectorModelPath) + ".bin";
	//std::cout << binFileName << std::endl;
    mDetectorNetReader.ReadWeights(binFileName);
    mDetectorNetwork = mDetectorNetReader.getNetwork();
    InferenceEngine::InputInfo::Ptr inputInfo = mDetectorNetwork.getInputsInfo().begin()->second;
    inputInfo->setPrecision(Precision::U8);

    InferenceEngine::OutputsDataMap outputInfo = mDetectorNetwork.getOutputsInfo();
    auto outputBlobsIt = outputInfo.begin();
    mDetectorRoiBlobName = outputBlobsIt->first;

    DataPtr& output = outputInfo.begin()->second;
    const SizeVector outputDims = output->getTensorDesc().getDims();
    mDetectorOutputName = outputInfo.begin()->first;
    mDetectorMaxProposalCount = outputDims[2];
    mDetectorObjectSize = outputDims[3];
    output->setPrecision(Precision::FP32);
    output->setLayout(Layout::NCHW);

	mVDExecutableNetwork = ie.LoadNetwork(mDetectorNetwork, targetDeviceName);
    mDetectorRequest = mVDExecutableNetwork.CreateInferRequest();

    //Load Vehicle Attribute Network
    mVANetReader.ReadNetwork(vehicleAttribsModelPath);
    binFileName = fileNameNoExt(vehicleAttribsModelPath) + ".bin";
    mVANetReader.ReadWeights(binFileName);
    mVANetwork = mVANetReader.getNetwork();
    mVANetwork.setBatchSize(1);
    inputInfo = mVANetwork.getInputsInfo().begin()->second;
    inputInfo->setPrecision(Precision::U8);
    inputInfo->getInputData()->setLayout(Layout::NCHW);

    outputInfo = mVANetwork.getOutputsInfo();
    outputBlobsIt = outputInfo.begin();
    mVAOutputNameForColor = (outputBlobsIt++)->second->getName();  // color is the first output
    mVAOutputNameForType = (outputBlobsIt++)->second->getName();  // type is the second output

	mVAExecutableNetwork = ie1.LoadNetwork(mVANetwork, "GPU");
    mVARequest = mVAExecutableNetwork.CreateInferRequest();
}

const std::string VehicleDetect::mVAColors[] =
{
    "white", "gray", "yellow", "black", "green", "red", "black"
};
const std::string VehicleDetect::mVATypes[] =
{
    "car", "bus", "truck", "van"
};

void VehicleDetect::SetSrcImageSize(int width, int height)
{
    mSrcImageSize.height = height;
    mSrcImageSize.width = width;
}

void VehicleDetect::Detect(const cv::Mat& image, std::vector<VehicleDetectResult>& results, int maxObjNum)
{
    InferenceEngine::Blob::Ptr input = mDetectorRequest.GetBlob(mDetectorNetwork.getInputsInfo().begin()->first);
    matU8ToBlob<uint8_t>(image, input);
    mDetectorRequest.Infer();

    const float *detections = mDetectorRequest.GetBlob(mDetectorOutputName)->buffer().as<float *>();
    if (maxObjNum < 0 || maxObjNum > mDetectorMaxProposalCount)
    {
        maxObjNum = mDetectorMaxProposalCount; 
    }

    for (int i = 0; i < mDetectorMaxProposalCount; i++)
    {
        float image_id = detections[i * mDetectorObjectSize + 0];  // in case of batch
        VehicleDetectResult r;
        r.label = static_cast<int>(detections[i * mDetectorObjectSize + 1]);
        r.confidence = detections[i * mDetectorObjectSize + 2];
        if (r.confidence <= mDetectThreshold || r.label != 1)
        {
            continue;
        }

        if (image_id < 0)
        {  // indicates end of detections
            break;
        }
        r.location.x = static_cast<int>(detections[i * mDetectorObjectSize + 3] * mSrcImageSize.width);
        r.location.y = static_cast<int>(detections[i * mDetectorObjectSize + 4] * mSrcImageSize.height);
        r.location.width = static_cast<int>(detections[i * mDetectorObjectSize + 5] * mSrcImageSize.width - r.location.x);
        r.location.height = static_cast<int>(detections[i * mDetectorObjectSize + 6] * mSrcImageSize.height - r.location.y);

        /* std::cout << "[" << i << "," << r.label << "] element, prob = " << r.confidence <<
            "    (" << r.location.x << "," << r.location.y << ")-(" << r.location.width << ","
            << r.location.height << ")"
            << (r.confidence  ) << std::endl; */

        results.push_back(r);
        if (results.size() >= (unsigned int)maxObjNum)
        {
            break;
        }
    }

    InferenceEngine::Blob::Ptr VAInput = mVARequest.GetBlob(mVANetwork.getInputsInfo().begin()->first);
    for (unsigned int i = 0; i < results.size(); i++)
    {
        //image's size can be different from source image
        auto clip = results[i].location & cv::Rect(0, 0, mSrcImageSize.width, mSrcImageSize.height);
        clip.x = clip.x * image.cols / mSrcImageSize.width;
        clip.width = clip.width * image.cols / mSrcImageSize.width;
        clip.y = clip.y * image.rows / mSrcImageSize.height;
        clip.height = clip.height * image.rows / mSrcImageSize.height;
        cv::Mat vehicle = image(clip);
        matU8ToBlob<uint8_t>(vehicle, VAInput);
        mVARequest.Infer();

        auto colorsValues = mVARequest.GetBlob(mVAOutputNameForColor)->buffer().as<float*>();
        // 4 possible types for each vehicle and we should select the one with the maximum probability
        auto typesValues  = mVARequest.GetBlob(mVAOutputNameForType)->buffer().as<float*>();

        const auto color_id = std::max_element(colorsValues, colorsValues + 7) - colorsValues;
        const auto type_id =  std::max_element(typesValues,  typesValues  + 4) - typesValues;
        results[i].color = mVAColors[color_id];
        results[i].type = mVATypes[type_id];
        //  std::cout<<"Car attribute: "<<results[i].color<<" "<<results[i].type<<"\n";
    }
    return ;
}


void VehicleDetect::RenderVDResults(std::vector<VehicleDetectResult>& results, cv::Mat& image)
{
    for (unsigned int i = 0; i < results.size(); i++) {
        cv::rectangle(image, results[i].location, cv::Scalar(0, 255, 0), 2);
        std::stringstream va_str;
        va_str<<results[i].color<<" "<<results[i].type;
        cv::putText(image, va_str.str(), cv::Point(results[i].location.x, results[i].location.y + 20),
                cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(255, 255, 255));
    }
    return;
}

VehicleDetect::~VehicleDetect()
{
    if (mEnablePerformanceReport)
    {
        std::cout << "Performance counts for vehicle detection:" << std::endl << std::endl;
        printPerformanceCounts(mDetectorRequest.GetPerformanceCounts(), std::cout, "GPU", false);
    }
}
