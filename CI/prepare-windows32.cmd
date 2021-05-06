set DepsPath32=D:\code\obs-studio\dependencies2019\win32
set QTDIR32=D:\Qt\5.15.2\msvc2019
set OBSPath=D:\code\obs-studio
set build_config=RelWithDebInfo
set DepsBasePath=D:\code\obs-studio\dependencies2019
cmake -G "Visual Studio 16 2019" -A Win32 -DCMAKE_SYSTEM_VERSION=10.0 -DQTDIR="%QTDIR32%" -Dw32-pthreads_DIR="%OBSPath%\build32\deps\w32-pthreads" -DW32_PTHREADS_LIB="%OBSPath%\build32\deps\w32-pthreads\%build_config%\w32-pthreads.lib" -DLibObs_DIR="%OBSPath%\build32\libobs" -DLIBOBS_INCLUDE_DIR="%OBSPath%\libobs" -DLIBOBS_LIB="%OBSPath%\build32\libobs\%build_config%\obs.lib" -DOBS_FRONTEND_LIB="%OBSPath%\build32\UI\obs-frontend-api\%build_config%\obs-frontend-api.lib" -DFFmpegPath="%DepsBasePath%" ..
