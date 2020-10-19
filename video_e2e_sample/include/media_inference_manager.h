/******************************************************************************\
Copyright (c) 2005-2020, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#pragma once

#include "sample_utils.h"
#include "vehicle_detect.hpp"
#include "face_detect.hpp"
#include "human_pose_estimator.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace human_pose_estimation;

class MediaInferenceManager
{
public:

    MediaInferenceManager();
    ~MediaInferenceManager();
    enum InferDeviceType {InferDeviceGPU, InferDeviceCPU, InferDeviceHDDL };
    int Init(int dec_w, int dec_h, int infer_type, msdk_char *model_dir, enum InferDeviceType device, int maxObjNum);
    /* If inferOffline is true, the results won't be render to input surface */
    int RunInfer(mfxFrameData *data, bool inferOffline);
    int RenderRepeatLast(mfxFrameData *data);

    const static int InferTypeNone = 0;
    const static int InferTypeFaceDetection = 1;
    const static int InferTypeHumanPoseEst = 2;
    const static int InferTypeVADetect = 3;

private:
    int InitFaceDetection(msdk_char *model_dir);
    int RunInferFD(mfxFrameData *pData, bool inferOffline);
    int RenderRepeatLastFD(mfxFrameData *pData);

    int InitVehicleDetect(msdk_char *model_dir);
    int RunInferVDVA(mfxFrameData *pData, bool inferOffline);
    int RenderRepeatLastVD(mfxFrameData *pData);

    int InitHumanPose(msdk_char *model_dir);
    int RunInferHP(mfxFrameData *pData, bool inferOffline);
    int RenderRepeatLastHP(mfxFrameData *pData);

    void raw_dumper_nv12(const char *name, int w, int h, int pitch, unsigned char *y, unsigned char *uv);
    void pitch_nv12_to_buffer(unsigned char *out, int w, int h, int pitch, unsigned char *y, unsigned char *uv);
    void  raw_dumper_rgb(const char *name, int w, int h, int ch, unsigned char *data);
    int mInferType;
    int mDecW;
    int mDecH;
    int mInputW;
    int mInputH;
    int mBatchId;
    int mMaxObjNum;
    std::string mTargetDevice;

    int mInferInterval;
    int mInferDevType;
    bool mInit;

    FaceDetect *mFaceDetector = nullptr;

    /*Vehicle and Vehicle attributes detection*/
    VehicleDetect *mVehicleDetector = nullptr;
    std::vector<VehicleDetectResult> mVDResults;

    /*Human Pose Estimation*/
    HumanPoseEstimator *mHPEstimator = nullptr;
    std::vector<HumanPose> mPoses;
};

