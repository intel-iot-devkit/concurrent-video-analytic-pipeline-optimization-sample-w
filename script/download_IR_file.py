import requests
import os
r = requests.get(" https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/face-detection-retail-0004/FP16/face-detection-retail-0004.xml")
isExists=os.path.exists(os.path.abspath("..\\video_e2e_sample\\model"))
if not isExists:
	os.makedirs(os.path.abspath("..\\video_e2e_sample\\model"))

with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"face-detection-retail-0004.xml"),"wb") as f:
    f.write(r.content)

print("face-detection-retail-0004.xml downloaded");
	
r = requests.get(" https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/face-detection-retail-0004/FP16/face-detection-retail-0004.bin")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"face-detection-retail-0004.bin"),"wb") as f:
    f.write(r.content)

print("face-detection-retail-0004.bin downloaded");

r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/vehicle-license-plate-detection-barrier-0106/FP16/vehicle-license-plate-detection-barrier-0106.xml")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"vehicle-license-plate-detection-barrier-0106.xml"),"wb") as f:
    f.write(r.content)

print("vehicle-license-plate-detection-barrier-0106.xml downloaded");

r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/vehicle-license-plate-detection-barrier-0106/FP16/vehicle-license-plate-detection-barrier-0106.bin")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"vehicle-license-plate-detection-barrier-0106.bin"),"wb") as f:
    f.write(r.content)

print("vehicle-license-plate-detection-barrier-0106.bin downloaded");

r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/vehicle-attributes-recognition-barrier-0039/FP16/vehicle-attributes-recognition-barrier-0039.xml")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"vehicle-attributes-recognition-barrier-0039.xml"),"wb") as f:
    f.write(r.content)

print("vehicle-attributes-recognition-barrier-0039.xml downloaded");

r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/vehicle-attributes-recognition-barrier-0039/FP16/vehicle-attributes-recognition-barrier-0039.bin")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"vehicle-attributes-recognition-barrier-0039.bin"),"wb") as f:
    f.write(r.content)
	
print("vehicle-attributes-recognition-barrier-0039.bin downloaded");
	
r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/human-pose-estimation-0001/FP16/human-pose-estimation-0001.xml")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"human-pose-estimation-0001.xml"),"wb") as f:
    f.write(r.content)
	
print("human-pose-estimation-0001.xml downloaded");	

r = requests.get("https://storage.openvinotoolkit.org/repositories/open_model_zoo/2021.3/models_bin/2/human-pose-estimation-0001/FP16/human-pose-estimation-0001.bin")
with open(os.path.join((os.path.abspath("..\\video_e2e_sample\\model\\")),"human-pose-estimation-0001.bin"),"wb") as f:
    f.write(r.content)

print("human-pose-estimation-0001.bin downloaded");

isExists=os.path.exists(os.path.abspath("..\\video_e2e_sample\\content"))
if not isExists:
	os.makedirs(os.path.abspath("..\\video_e2e_sample\\content"))

