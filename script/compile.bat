set PATH=C:\Program Files (x86)\ffmpeg\ffmpeg-20191224-287620f-win64-shared\bin;C:\Program Files (x86)\IntelSWTools\openvino\deployment_tools\inference_engine\bin\intel64\Release;%PATH%
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin;%PATH%
call "C:\Program Files (x86)\IntelSWTools\openvino\bin\setupvars.bat"
cd ..\video_e2e_sample\
MSBuild.exe sample_multi_transcode.vcxproj /p:configuration=release /p:platform=x64 /p:VisualStudioVersion=15.0 /t:rebuild
