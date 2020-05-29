/******************************************************************************\
Copyright (c) 2005-2020, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

\**********************************************************************************/

#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>

#include <opencv2/imgproc/imgproc.hpp>
#include <samples/ocv_common.hpp>
#include <samples/common.hpp>

#include "face_detect.hpp"


using namespace InferenceEngine;

#define SSD_EMPTY_DETECTIONS_INDICATOR -1.0

namespace {
	cv::Rect TruncateToValidRect(const cv::Rect& rect,
		const cv::Size& size) {
		auto tl = rect.tl(), br = rect.br();
		tl.x = std::max(0, std::min(size.width - 1, tl.x));
		tl.y = std::max(0, std::min(size.height - 1, tl.y));
		br.x = std::max(0, std::min(size.width, br.x));
		br.y = std::max(0, std::min(size.height, br.y));
		int w = std::max(0, br.x - tl.x);
		int h = std::max(0, br.y - tl.y);
		return cv::Rect(tl.x, tl.y, w, h);
	}

	cv::Rect IncreaseRect(const cv::Rect& r, float coeff_x,
		float coeff_y) {
		cv::Point2f tl = r.tl();
		cv::Point2f br = r.br();
		cv::Point2f c = (tl * 0.5f) + (br * 0.5f);
		cv::Point2f diff = c - tl;
		cv::Point2f new_diff{ diff.x * coeff_x, diff.y * coeff_y };
		cv::Point2f new_tl = c - new_diff;
		cv::Point2f new_br = c + new_diff;

		cv::Point new_tl_int{ static_cast<int>(std::floor(new_tl.x)), static_cast<int>(std::floor(new_tl.y)) };
		cv::Point new_br_int{ static_cast<int>(std::ceil(new_br.x)), static_cast<int>(std::ceil(new_br.y)) };

		return cv::Rect(new_tl_int, new_br_int);
	}
}

FaceDetect::FaceDetect(bool enablePerformanceReport)
	:mDetectThreshold(0.65f),
	mEnablePerformanceReport(enablePerformanceReport)
{
	return;
}

void FaceDetect::Init(const std::string& detectorModelPath,
	const std::string& targetDeviceName)
{
	static std::mutex initLock;
	std::lock_guard<std::mutex> lock(initLock);

	mDetectorNetReader.ReadNetwork(detectorModelPath);
	std::string binFileName = fileNameNoExt(detectorModelPath) + ".bin";
	mDetectorNetReader.ReadWeights(binFileName);
	mDetectorNetwork = mDetectorNetReader.getNetwork();

	InputsDataMap inputInfo(mDetectorNetReader.getNetwork().getInputsInfo());
	if (inputInfo.size() != 1) {
		THROW_IE_EXCEPTION << "Face Detection network should have only one input";
	}
	InputInfo::Ptr inputInfoFirst = inputInfo.begin()->second;
	inputInfoFirst->setPrecision(Precision::U8);
	inputInfoFirst->getInputData()->setLayout(Layout::NCHW);

	OutputsDataMap outputInfo(mDetectorNetReader.getNetwork().getOutputsInfo());
	if (outputInfo.size() != 1) {
		THROW_IE_EXCEPTION << "Face Detection network should have only one output";
	}
	DataPtr& _output = outputInfo.begin()->second;
	output_name_ = outputInfo.begin()->first;

	const CNNLayerPtr outputLayer = mDetectorNetReader.getNetwork().getLayerByName(output_name_.c_str());
	if (outputLayer->type != "DetectionOutput") {
		THROW_IE_EXCEPTION << "Face Detection network output layer(" + outputLayer->name +
			") should be DetectionOutput, but was " + outputLayer->type;
	}

	if (outputLayer->params.find("num_classes") == outputLayer->params.end()) {
		THROW_IE_EXCEPTION << "Face Detection network output layer (" +
			output_name_ + ") should have num_classes integer attribute";
	}

	const SizeVector outputDims = _output->getTensorDesc().getDims();
	max_detections_count_ = outputDims[2];
	object_size_ = outputDims[3];
	if (object_size_ != 7) {
		THROW_IE_EXCEPTION << "Face Detection network output layer should have 7 as a last dimension";
	}
	if (outputDims.size() != 4) {
		THROW_IE_EXCEPTION << "Face Detection network output dimensions not compatible shoulld be 4, but was " +
			std::to_string(outputDims.size());
	}
	_output->setPrecision(Precision::FP32);
	_output->setLayout(TensorDesc::getLayoutByDims(_output->getDims()));

	input_name_ = inputInfo.begin()->first;
	mVDExecutableNetwork = ie.LoadNetwork(mDetectorNetReader.getNetwork(), targetDeviceName);
	mDetectorRequest = mVDExecutableNetwork.CreateInferRequest();
}

void FaceDetect::SetSrcImageSize(int width, int height)
{
	mSrcImageSize.height = 300;// height;
	mSrcImageSize.width = 300;// width;
}

void FaceDetect::Detect(const cv::Mat& image)
{
	InferenceEngine::Blob::Ptr input = mDetectorRequest.GetBlob(mDetectorNetwork.getInputsInfo().begin()->first);
	matU8ToBlob<uint8_t>(image, input);
	width_ = static_cast<float>(image.cols);
	height_ = static_cast<float>(image.rows);
	mDetectorRequest.Infer();

	const float *data = mDetectorRequest.GetBlob(output_name_)->buffer().as<float *>();

	for (int det_id = 0; det_id < max_detections_count_; ++det_id) {
			const int start_pos = det_id * object_size_;

			const float batchID = data[start_pos];
			if (batchID == SSD_EMPTY_DETECTIONS_INDICATOR) {
				break;
			}

			const float score = std::min(std::max(0.0f, data[start_pos + 2]), 1.0f);
			const float x0 =
				std::min(std::max(0.0f, data[start_pos + 3]), 1.0f) * width_;
			const float y0 =
				std::min(std::max(0.0f, data[start_pos + 4]), 1.0f) * height_;
			const float x1 =
				std::min(std::max(0.0f, data[start_pos + 5]), 1.0f) * width_;
			const float y1 =
				std::min(std::max(0.0f, data[start_pos + 6]), 1.0f) * height_;

			FDDetectedObject object;
			object.confidence = score;
			object.rect = cv::Rect(cv::Point(static_cast<int>(round(static_cast<double>(x0))),
				static_cast<int>(round(static_cast<double>(y0)))),
				cv::Point(static_cast<int>(round(static_cast<double>(x1))),
					static_cast<int>(round(static_cast<double>(y1)))));


			object.rect = TruncateToValidRect(IncreaseRect(object.rect,
				1.15,
				1.15),
				cv::Size(static_cast<int>(width_), static_cast<int>(height_)));

			if (object.confidence > 0.6 && object.rect.area() > 0) {
				results.emplace_back(object);
			}
		}

	return;
}

void FaceDetect::RenderFDResults(cv::Mat& image)
{
	for (unsigned int i = 0; i < results.size(); i++) {
		cv::rectangle(image, results[i].rect, cv::Scalar(0, 255, 0), 2);
		putText(image, "Person", cv::Point(results[i].rect.x, results[i].rect.y), cv::FONT_HERSHEY_PLAIN, 2,
			cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
	}
	//cv::rectangle(*image, new_rect, cv::Scalar(255, 255, 255));
	return;
}

FaceDetect::~FaceDetect()
{
	return;
}
