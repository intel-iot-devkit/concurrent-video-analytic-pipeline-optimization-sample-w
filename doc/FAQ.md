# Frequently asked questions (SVET sample application)

## The loading time of 16-channel face detection demo is too long
Please enable cl_cache by make folder named "cl_cache" in video_e2e_sample, it will accelerate the loading time.

## How to limit the fps of whole pipeline to 30?
Add "-fps 30" to every decoding session.

## How to limit the frame number of input to 1000?
Add "-n 1000" to every decoding dessions. However this option won't work if both "-vpp_comp_only" and "-vpp_comp" are set.

## Where can I find tutorials for inference engine?
Please refer to https://docs.openvinotoolkit.org/latest/_docs_IE_DG_Deep_Learning_Inference_Engine_DevGuide.html

## Where can I find information for the models?
Please refer to https://github.com/opencv/open_model_zoo/tree/master/models/intel. The names of models used in sample application are
face-detection-retail-0004, human-pose-estimation-0001, vehicle-attributes-recognition-barrier-0039, vehicle-license-plate-detection-barrier-0106.

## Can I use other OpenVINO version rather than 2020.2?
Yes, but you have to modify some code due to interfaces changing. And also you need to download the IR files and copy them to ./model manually. Please refer to script/download_and_copy_models.sh for how to download the IR files.
 
## How to fix the compile error for "LoadLibrary" function?
There is a issue for OpenVINO 2020.2 which is not compatible with #define UNICODE. you can fix it by put on the patch patch\win_shared_object_loader_h.patch in OpenVINO folder's win_shared_object_loader.h file.
