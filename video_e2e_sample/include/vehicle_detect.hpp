// Copyright (C) 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <string>
#include <vector>

#include <inference_engine.hpp>
#include <opencv2/core/core.hpp>

struct VehicleDetectResult {
    int label;
    float confidence;
    cv::Rect location;
    std::string color;
    std::string type;
};

class VehicleDetect {
public:

    VehicleDetect(bool mEnablePerformanceReport = false);
    void Init(const std::string& detectorModelPath,
            const std::string& VAModelPath,
            const std::string& targetDeviceName);
    void Detect(const cv::Mat& image, std::vector<VehicleDetectResult>& results);
    void SetSrcImageSize(int width, int height);
    void RenderVDResults(std::vector<VehicleDetectResult>& results, cv::Mat& image);
    ~VehicleDetect();

private:

    static std::mutex mInitLock;
    float mDetectThreshold;
	InferenceEngine::Core ie;
    InferenceEngine::CNNNetwork mDetectorNetwork;
    InferenceEngine::ExecutableNetwork mVDExecutableNetwork;
    InferenceEngine::InferRequest mDetectorRequest;
    InferenceEngine::CNNNetReader mDetectorNetReader;
    int mDetectorMaxProposalCount;
    int mDetectorObjectSize;
    std::string mDetectorRoiBlobName;
    std::string mDetectorOutputName;
    bool mEnablePerformanceReport;
    cv::Size mSrcImageSize;

	InferenceEngine::Core ie1;
    InferenceEngine::CNNNetwork mVANetwork;
    InferenceEngine::ExecutableNetwork mVAExecutableNetwork;
    InferenceEngine::InferRequest mVARequest;
    InferenceEngine::CNNNetReader mVANetReader;
    std::string mVAOutputNameForColor;  // color is the first output
    std::string mVAOutputNameForType;  // type is the second output
    static const std::string mVAColors[];
    static const std::string mVATypes[];
};

