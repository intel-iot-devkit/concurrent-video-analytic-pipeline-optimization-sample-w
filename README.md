# DISCONTINUATION OF PROJECT #
This project will no longer be maintained by Intel.
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.
Intel no longer accepts patches to this project.
# Concurrent Video Analytic Pipeline Optimzation Sample 
Support users to quickly setup and adjust the core concurrent video analysis workload through configuration file to obtain the best performance of video codec, post-processing and inference based on Intel® integrated GPU according to their product requirements.
Users can use the sample application video_e2e_sample to complete runtime performance evaluation or as a reference for debugging core video workload issues.

## Typical workloads
Sample par files can be found in par_files directory. Verfied on i7-8559U. Performance differs on other platforms.
* 16 1080p H264 decoding, scaling, face detection inference, rendering inference results, composition, saving composition results to local H264 file, and display
* 4 1080p H264 decoding, scaling, human pose estimation inference, rendering inference results, composition and display
* 4 1080p H264 decoding, scaling, vehicle and vehicle attributes detection inference, rendering inference results, composition and display
* 16 1080p RTSP H264 stream decoding, scaling, face detection inference, rendering inference results, composition and display.
* 16 1080p H264 decoding, scaling, face detection inference, rendering inference results, composition and display. Plus 16 1080p H264 decoding, composition and showing on second display. 
* Multi-display support with multi par file, eg : video_e2e_sample -par ..\par_file\n4_1080p_fd.par ..\par_file\n4_1080p_vd.par

# Dependencies
The sample application depends on [Intel® Media SDK](https://software.intel.com/en-us/media-sdk/), [Intel® OpenVINO™](https://software.intel.com/en-us/openvino-toolkit) and [FFmpeg](https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z)

# FAQ
See doc/FAQ.md

# Table of contents

  * [License](#license)
  * [Documentation](#documentation)
  * [System requirements](#system-requirements)
  * [How to build](#how-to-build)
  * [How to run demo](#how-to-run-demo)
  * [Known limitations](#know-limitations)

# License
The sample application is licensed under MIT license. See [LICENSE](./video_e2e_sample/LICENSE) for details.

# Documentation
See [user guide](./doc/concurrent_video_analytic_sample_application_user_guide.pdf)

# System requirements

**Operating System:**
Windows 10 Enterprise

**Software:**
* [MediaSDK 2020 R1](https://software.intel.com/en-us/media-sdk/)
* [OpenVINO™ 2021.3](https://software.intel.com/en-us/openvino-toolkit)

**Hardware:** 
* Intel® platforms including Core™ or Atom™ processors which supported by the MediaSDK 2020 R1 and OpenVINO 2021.3. 
* For Media SDK, the major platform dependency comes from the Intel GPU driver.
* For OpenVINO™, see details from here: https://software.intel.com/en-us/openvino-toolkit/documentation/system-requirements

# How to build

Run .\script\compile.bat to build sample application video_e2e_sample. 

Please refer to ”Installation Guide“ in [user guide](./doc/concurrent_video_analytic_sample_application_user_guide.pdf) for details.

# How to run demo

Before this, you should download the IR file by the .\script\download_IR_file.py to .\video_e2e_sample\model\ folder
Replace the video clips in par file. Such as download face detection demo video from https://github.com/elainewangprc/testvideo/ with 1080p.h264, 
and use above video clips to replace the file location in .\par_file\*.par, (-i::h264 content\1080p.h264)
The run: cd .\script\ ;  Run_FD_test.bat

# Known limitations

The sample application has been validated on Intel® platforms Skylake(i7-6770HQ), Coffee Lake(i7-8559U i7-8700) and Whiskey Lake(i7-8665UE).


