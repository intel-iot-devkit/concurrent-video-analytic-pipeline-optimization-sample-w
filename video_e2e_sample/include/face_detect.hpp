/******************************************************************************\
Copyright (c) 2005-2020, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

\**********************************************************************************/

#include <string>
#include <vector>

#include <inference_engine.hpp>
#include <opencv2/core/core.hpp>


struct FaceDetectResult {
	int label;
	float confidence;
	cv::Rect location;
	std::string color;
	std::string type;
};

struct FDDetectedObject {
	cv::Rect rect;
	float confidence;

	explicit FDDetectedObject(const cv::Rect& rect = cv::Rect(), float confidence = -1.0f)
		: rect(rect), confidence(confidence) {}
};

using FDDetectedObjects = std::vector<FDDetectedObject>;


class FaceDetect {
public:

	FaceDetect(bool mEnablePerformanceReport = false);
	void Init(const std::string& detectorModelPath,
		const std::string& targetDeviceName);
	void Detect(const cv::Mat& image);
	void SetSrcImageSize(int width, int height);
	void RenderFDResults(cv::Mat& image);
	~FaceDetect();
	FDDetectedObjects results;

private:

	static std::mutex mInitLock;
	float mDetectThreshold;
	// InferenceEngine::InferencePlugin mPlugin;
	InferenceEngine::Core ie;
	InferenceEngine::CNNNetwork mDetectorNetwork;
	InferenceEngine::ExecutableNetwork mVDExecutableNetwork;
	InferenceEngine::InferRequest mDetectorRequest;
	InferenceEngine::CNNNetReader mDetectorNetReader;
	cv::Size mSrcImageSize;
	bool mEnablePerformanceReport;
	std::string input_name_;
	std::string output_name_;
	int max_detections_count_ = 0;
	int object_size_ = 0;
	int enqueued_frames_ = 0;
	float width_ = 0;
	float height_ = 0;
	bool results_fetched_ = false;
	
};
