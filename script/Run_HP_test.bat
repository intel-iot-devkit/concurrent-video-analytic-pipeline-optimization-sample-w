set PATH=C:\Program Files (x86)\ffmpeg\bin;C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\inference_engine\external\hddl\bin;C:\Program Files (x86)\Intel\openvino_2021\opencv\bin;C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\inference_engine\external\tbb\bin;C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\ngraph\lib;C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\inference_engine\bin\intel64\Release;%PATH%
cd ..\video_e2e_sample\
..\build\win_x64\release\bin\video_e2e_sample.exe -par ..\par_file\n4_1080p_hp.par
