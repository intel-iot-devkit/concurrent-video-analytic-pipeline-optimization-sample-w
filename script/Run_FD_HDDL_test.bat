set PATH=%ffmpeg_installed_dir%\ffmpeg-20191224-287620f-win64-shared\bin;C:\Program Files (x86)\IntelSWTools\openvino\deployment_tools\inference_engine\external\hddl\bin;C:\Program Files (x86)\IntelSWTools\openvino\opencv\bin;C:\Program Files (x86)\IntelSWTools\openvino\deployment_tools\inference_engine\external\tbb\bin;C:\Program Files (x86)\IntelSWTools\openvino\deployment_tools\ngraph\lib;C:\Program Files (x86)\IntelSWTools\openvino\deployment_tools\inference_engine\bin\intel64\Release;%PATH%
set HDDL_INSTALL_DIR=C:\Program Files (x86)\IntelSWTools\openvino_2020.2.117\deployment_tools\inference_engine\external\hddl
cd ..\video_e2e_sample\
..\_build\x64\Release\video_e2e_sample.exe -par ..\par_file\n4_1080p_fd_HDDL.par
