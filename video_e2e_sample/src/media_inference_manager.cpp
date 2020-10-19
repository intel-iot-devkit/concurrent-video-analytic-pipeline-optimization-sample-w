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

#include "sample_utils.h"
#include "media_inference_manager.h"

#include "human_pose_estimator.hpp"
#include "render_human_pose.hpp"
#include <fstream>

#define FDUMP 0
#define LESS_P 1

#define IR_PATH_MAX_LEN 1024

using namespace cv;
using namespace std;

MediaInferenceManager::MediaInferenceManager():
    mInferType(0),
    mInferInterval(5),
    mTargetDevice("GPU"),
    mMaxObjNum(-1)
{
    mInit = false;
}

MediaInferenceManager::~MediaInferenceManager()
{
    delete mHPEstimator;
    mHPEstimator = nullptr;
    delete mFaceDetector;
    mFaceDetector = nullptr;
    delete mVehicleDetector;
    mVehicleDetector = nullptr;
}

int MediaInferenceManager::Init(int dec_w, int dec_h, int infer_type,
	msdk_char *model_dir, enum InferDeviceType device, int maxObjNum)
{
    int ret = 0;
    mInferType = infer_type;
    mDecW = dec_w;
    mDecH = dec_h;
    mMaxObjNum = maxObjNum;

    switch(device)
    {
        case InferDeviceGPU:
            mTargetDevice = "GPU";
            break;
        case InferDeviceCPU:
            mTargetDevice = "CPU";
            break;
        case InferDeviceHDDL:
            mTargetDevice = "HDDL";
            break;
        default:
            break;
    }

    switch(mInferType)
    {
        case InferTypeFaceDetection:
            ret = InitFaceDetection(model_dir);
            break;
        case InferTypeVADetect:
            ret = InitVehicleDetect(model_dir);
            break;
        case InferTypeHumanPoseEst:
            ret = InitHumanPose(model_dir);
            break;
        default:
            msdk_printf(MSDK_STRING("ERROR:Unsupported inference type %d\n"), mInferType);
	    ret = -1;
	    break;
    }
    if (ret == 0)
    {
        mInit = true;
    }
    return ret;
}

int MediaInferenceManager::RunInfer(mfxFrameData *pData, bool inferOffline)
{
    if (!mInit)
    {
        return -1;
    }

    switch(mInferType)
    {
        case InferTypeFaceDetection:
            RunInferFD(pData, inferOffline);
            break;
        case InferTypeVADetect:
            RunInferVDVA(pData, inferOffline);
            break;
        case InferTypeHumanPoseEst:
            RunInferHP(pData, inferOffline);
            break;
        default:
            msdk_printf(MSDK_STRING("ERROR:Unsupported inference type %d\n"), mInferType);
            return -1;
    }
    return 0;
}

int MediaInferenceManager::RenderRepeatLast(mfxFrameData *pData)
{
    if (!mInit)
    {
        return -1;
    }

    switch(mInferType)
    {
        case InferTypeFaceDetection:
            RenderRepeatLastFD(pData);
            break;
        case InferTypeVADetect:
            RenderRepeatLastVD(pData);
            break;
        case InferTypeHumanPoseEst:
            RenderRepeatLastHP(pData);
            break;
        default:
            msdk_printf(MSDK_STRING("ERROR:Unsupported inference type %d\n"), mInferType);
            return -1;
    }
    return 0;
}

int MediaInferenceManager::RenderRepeatLastFD(mfxFrameData *pData)
{
    if (mFaceDetector && mFaceDetector->results.size() > 0) {
        Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pData->B);
        mFaceDetector->RenderFDResults(frameRGB4);
    }
    return 0;
}

int MediaInferenceManager::RenderRepeatLastHP(mfxFrameData *pData)
{
    //std::cout << "MediaInferenceManager::RenderRepeatLastHP " << mDecH << "  " << mDecW << std::endl;
    if (mPoses.size() > 0) {
        Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pData->B);
        renderHumanPose(mPoses, frameRGB4);
    }

    return 0;
}

int MediaInferenceManager::RenderRepeatLastVD(mfxFrameData *pData)
{
    if (mVehicleDetector && (mVDResults.size() > 0)) {
        Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pData->B);
        mVehicleDetector->RenderVDResults(mVDResults, frameRGB4);
    }

    return 0;
}

int MediaInferenceManager::RunInferHP(mfxFrameData *pData, bool inferOffline)
{

#if VERBOSE_LOG 
	chrono::high_resolution_clock::time_point time1 = chrono::high_resolution_clock::now();
#endif

	//std::cout << "MediaInferenceManager::RunInferHP " << mDecH << " " << mDecW << std::endl;
	unsigned char *pbuf = (pData->B < pData->R) ? pData->B : pData->R;
	Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pbuf);
	Mat frame(mDecH, mDecW, CV_8UC3);

	cvtColor(frameRGB4, frame, COLOR_RGBA2BGR);

#if VERBOSE_LOG 
	chrono::high_resolution_clock::time_point time2 = chrono::high_resolution_clock::now();
	chrono::duration<double> diff = time2 - time1;
	cout << " pre-stage: " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
	time1 = chrono::high_resolution_clock::now();
#endif

	if (mPoses.size() > 0)
	{
		mPoses.clear();
	}
	mPoses = mHPEstimator->estimate(frame);

#if VERBOSE_LOG 
	time2 = chrono::high_resolution_clock::now();
	diff = time2 - time1;
	cout << " " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
	time1 = chrono::high_resolution_clock::now();
#endif
	if (!inferOffline) {
		renderHumanPose(mPoses, frameRGB4);
	}
#if VERBOSE_LOG 
	time2 = chrono::high_resolution_clock::now();
	diff = time2 - time1;
	cout << " " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
#endif

	return 0;
}

int MediaInferenceManager::RunInferFD(mfxFrameData *pData, bool inferOffline)
{
#if VERBOSE_LOG
    chrono::high_resolution_clock::time_point time1 = chrono::high_resolution_clock::now();		
#endif

	unsigned char *pbuf = (pData->B < pData->R) ? pData->B : pData->R;
	Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pbuf);

	Mat frame(mDecH, mDecW, CV_8UC3);

	cvtColor(frameRGB4, frame, COLOR_RGBA2BGR);

#if  VERBOSE_LOG
    chrono::high_resolution_clock::time_point time2 = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = time2 - time1;
    cout << "pre-stage: " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
    time1 = chrono::high_resolution_clock::now();		
#endif	

   // IE_Execute(mFDCtx, mInputW, mInputH, 1.0, frame.data, &mBatchId);
	if (mFaceDetector->results.size() > 0)
	{
		mFaceDetector->results.clear();
	}

	mFaceDetector->Detect(frame);

#if  VERBOSE_LOG
    time2 = chrono::high_resolution_clock::now();
    diff = time2 - time1;
    cout << "infer-stage: " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
#endif

	if (!inferOffline) {
		mFaceDetector->RenderFDResults(frameRGB4);
	}
    return 0;
}

int MediaInferenceManager::RunInferVDVA(mfxFrameData *pData, bool inferOffline)
{
#if VERBOSE_LOG
	chrono::high_resolution_clock::time_point time1 = chrono::high_resolution_clock::now();
	time1 = chrono::high_resolution_clock::now();
#endif

	unsigned char *pbuf = (pData->B < pData->R) ? pData->B : pData->R;
	Mat frameRGB4(mDecH, mDecW, CV_8UC4, (unsigned char *)pbuf);
	Mat frameScl(mInputH, mInputH, CV_8UC4);
	Mat frame(mInputH, mInputW, CV_8UC3);

	resize(frameRGB4, frameScl, Size(mInputW, mInputH));
	cvtColor(frameScl, frame, COLOR_RGBA2BGR);

	if (mVDResults.size() > 0)
	{
		mVDResults.clear();
	}

#if VERBOSE_LOG
	chrono::high_resolution_clock::time_point time2 = chrono::high_resolution_clock::now();
	chrono::duration<double> diff = time2 - time1;
	cout << " pre-stage: " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
	time1 = chrono::high_resolution_clock::now();
#endif	

	mVehicleDetector->Detect(frame, mVDResults, mMaxObjNum);

#if VERBOSE_LOG
	time2 = chrono::high_resolution_clock::now();
	diff = time2 - time1;
	cout << " " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
	time1 = chrono::high_resolution_clock::now();
#endif
	if (!inferOffline) {
		mVehicleDetector->RenderVDResults(mVDResults, frameRGB4);
	}

#if VERBOSE_LOG
	time2 = chrono::high_resolution_clock::now();
	diff = time2 - time1;
	cout << " " << fixed << "\t" << setprecision(2) << setfill('0') << diff.count()*1000.0 << "ms/frame" << endl;
#endif

	return 0;
}

int MediaInferenceManager::InitHumanPose(msdk_char *model_dir)
{
	mInputW = 456;
	mInputH = 256;
	std::string ir_file;
	fstream file;

	FILE *modelFile = NULL;
	TCHAR* xml_file = _T("\\human-pose-estimation-0001.xml");
	int len_xml = msdk_strlen(xml_file);
	int len_model_dir = msdk_strlen(model_dir);
	TCHAR* full_path = new TCHAR[len_model_dir + len_xml + 1];
	full_path[0] = _T('\0');
	lstrcat(full_path, model_dir);
	lstrcat(full_path, xml_file);

	MSDK_FOPEN(modelFile, full_path, MSDK_STRING("r"));
 	if (NULL != modelFile)
	{
		int iLength = WideCharToMultiByte(CP_ACP, 0, full_path, -1, NULL, 0, NULL, NULL);
 		char local[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, full_path, -1, local, iLength, NULL, NULL);
		local[iLength] = '\0';
		ir_file.append(local);
		delete[]full_path;
		fclose(modelFile);
 	}
 	else {
 		ir_file = "C:\\Program Files (x86)\\IntelSWTools\\openvino\\deployment_tools\\tools\\model_downloader\\intel\\human-pose-estimation-0001\\FP16\\human-pose-estimation-0001.xml";
		file.open(ir_file, ios::in);
		if (!file) {
			cout << "ERROR:Not able to open IR file " << ir_file << ". Please check if the IR file path is correct" << endl;
			file.close();
			delete[]full_path;
			return -1;
		}
		delete[]full_path;
		file.close();
	}
 
	mHPEstimator = new HumanPoseEstimator(ir_file, mTargetDevice, false);

	return 0;
}

int MediaInferenceManager::InitFaceDetection(msdk_char *model_dir)
{
	string fd_model_path;
	FILE *modelFile = NULL;
	TCHAR* xml_file = _T("\\face-detection-retail-0004.xml");
	int len_xml = msdk_strlen(xml_file);
	int len_model_dir = msdk_strlen(model_dir);
	fstream file;
 
	mInputW = 300;
	mInputH = 300;

	TCHAR* full_path = new TCHAR[len_model_dir + len_xml + 1];
	full_path[0] = _T('\0');
	lstrcat(full_path, model_dir);
	lstrcat(full_path, xml_file);

	MSDK_FOPEN(modelFile, full_path, MSDK_STRING("r"));

	if (NULL != modelFile)
	{
		int iLength = WideCharToMultiByte(CP_ACP, 0, full_path, -1, NULL, 0, NULL, NULL);
 		char local[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, full_path, -1, local, iLength, NULL, NULL);
		local[iLength] = '\0';
		fd_model_path.append(local);
		delete[]full_path;
		fclose(modelFile);
	} else {
 		fd_model_path = "C:\\Program Files (x86)\\IntelSWTools\\openvino\\deployment_tools\\tools\\model_downloader\\intel\\face-detection-retail-0004\\FP16\\face-detection-retail-0004.xml";
		file.open(fd_model_path, ios::in);
		if (!file) {
			cout << "ERROR:Not able to open IR file " << fd_model_path << ". Please check if the IR file path is correct" << endl;
			file.close();
			delete[]full_path;
			return -1;
		}
		delete[]full_path;
		file.close();
	}

	mFaceDetector = new FaceDetect(false);
	mFaceDetector->SetSrcImageSize(mDecW, mDecH);
	mFaceDetector->Init(fd_model_path, mTargetDevice);

    return 0;
}

int MediaInferenceManager::InitVehicleDetect(msdk_char *model_dir)
{

#ifdef VD_LPD_MODEL
	mInputW = 300;
	mInputH = 300;
#else
	mInputW = 672;
	mInputH = 384;
#endif

	fstream file_vd;
	fstream file_va;
	std::string ir_file_vd;
	std::string ir_file_va;
	FILE *modelFile_vd = NULL;
	FILE *modelFile_va = NULL;
	TCHAR* xml_file_vd = _T("\\vehicle-license-plate-detection-barrier-0106.xml");
	TCHAR* xml_file_va = _T("\\vehicle-attributes-recognition-barrier-0039.xml");
	int len_xml_vd = msdk_strlen(xml_file_vd);
	int len_xml_va = msdk_strlen(xml_file_va);
	int len_model_dir = msdk_strlen(model_dir);

	TCHAR* full_path_vd = new TCHAR[len_model_dir + len_xml_vd + 1];
	full_path_vd[0] = _T('\0');
	lstrcat(full_path_vd, model_dir);
	lstrcat(full_path_vd, xml_file_vd);

	TCHAR* full_path_va = new TCHAR[len_model_dir + len_xml_va + 1];
	full_path_va[0] = _T('\0');
	lstrcat(full_path_va, model_dir);
	lstrcat(full_path_va, xml_file_va);
	MSDK_FOPEN(modelFile_va, full_path_va, MSDK_STRING("r"));
	MSDK_FOPEN(modelFile_vd, full_path_vd, MSDK_STRING("r"));

	if (NULL != modelFile_va && NULL != modelFile_vd)
 	{
		int iLength_vd = WideCharToMultiByte(CP_ACP, 0, full_path_vd, -1, NULL, 0, NULL, NULL);
		char local_vd[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, full_path_vd, -1, local_vd, iLength_vd, NULL, NULL);
		local_vd[iLength_vd] = '\0';
		ir_file_vd.append(local_vd);
		delete[]full_path_vd;
		fclose(modelFile_vd);

		int iLength_va = WideCharToMultiByte(CP_ACP, 0, full_path_va, -1, NULL, 0, NULL, NULL);
		char local_va[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, full_path_va, -1, local_va, iLength_va, NULL, NULL);
		local_va[iLength_va] = '\0';
		ir_file_va.append(local_va);
		delete[]full_path_va;
		fclose(modelFile_va);
 	}
 	else {
 		ir_file_vd = "C:\\Program Files (x86)\\IntelSWTools\\openvino\\deployment_tools\\tools\\model_downloader\\intel\\vehicle-license-plate-detection-barrier-0106\\FP16\\vehicle-license-plate-detection-barrier-0106.xml";
 		ir_file_va = "C:\\Program Files (x86)\\IntelSWTools\\openvino\\deployment_tools\\tools\\model_downloader\\intel\\vehicle-attributes-recognition-barrier-0039\\FP16\\vehicle-attributes-recognition-barrier-0039.xml";
		file_vd.open(ir_file_vd, ios::in);
		if (!file_vd) {
			cout << "ERROR:Not able to open IR file " << ir_file_vd << ". Please check if the IR file path is correct" << endl;
			file_vd.close();
			delete[]full_path_va;
			delete[]full_path_vd;
			if (modelFile_va)
				fclose(modelFile_va);
			if (modelFile_vd)
				fclose(modelFile_vd);
			return -1;
		}
		file_vd.close();

		file_va.open(ir_file_va, ios::in);
		if (!file_va) {
			cout << "ERROR:Not able to open IR file " << ir_file_va << ". Please check if the IR file path is correct" << endl;
			file_va.close();
			delete[]full_path_va;
			delete[]full_path_vd;
			if (modelFile_va)
				fclose(modelFile_va);
			if (modelFile_vd)
				fclose(modelFile_vd);

			return -1;
		}
		file_va.close();
		delete[]full_path_va;
		delete[]full_path_vd;
		if (modelFile_va)
			fclose(modelFile_va);
		if (modelFile_vd)
			fclose(modelFile_vd);

 	}

	mVehicleDetector = new VehicleDetect(false);
	mVehicleDetector->Init(ir_file_vd, ir_file_va, mTargetDevice);
	mVehicleDetector->SetSrcImageSize(mDecW, mDecH);
	//std::cout << ir_file_vd << std::endl;
	return 0;
}
void MediaInferenceManager::raw_dumper_nv12(const char *name, int w, int h, int pitch, unsigned char *y, unsigned char *uv)
{
    FILE *fp = NULL;
    int i = 0;

    fp = fopen(name, "a+");
    if (fp)
    {
        for (i=0; i<h; i++)
        {
            fwrite(y+i*pitch, w, 1, fp);
        }

        for (i=0; i<h/2; i++)
        {
            fwrite(uv+i*pitch, w, 1, fp);
        }
        fclose(fp);
    }

    return;
}

void MediaInferenceManager::pitch_nv12_to_buffer(unsigned char *out, int w, int h, int pitch, unsigned char *y, unsigned char *uv)
{
    int i = 0, j = 0;

    for (i=0; i<h; i++)
    {
        memcpy(out+i*w, y+i*pitch, w);
    }

    j = i;

    for (i=0; i<h/2; i++)
    {
        memcpy(out+(i+j)*w, uv+i*pitch, w);
    }

    return;
}

void  MediaInferenceManager::raw_dumper_rgb(const char *name, int w, int h, int ch, unsigned char *data)
{
    FILE *fp = NULL;

    fp = fopen(name, "a+");
    if (fp)
    {
        fwrite(data, w*h*ch, 1, fp);
        fclose(fp);
    }

    return;
}


