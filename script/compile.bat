set PATH=C:\Program Files (x86)\ffmpeg\bin;C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\inference_engine\bin\intel64\Release;%PATH%
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin;%PATH%
call "C:\Program Files (x86)\Intel\openvino_2021\bin\setupvars.bat"
cd ..\video_e2e_sample\
MSBuild.exe sample_multi_transcode.vcxproj /p:configuration=release /p:platform=x64 /p:VisualStudioVersion=15.0 /t:rebuild
