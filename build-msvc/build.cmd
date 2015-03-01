rd /s /q output
md output

set MINGW_ROOT=C:\Users\SAPikachu\AppData\Local\PortablePrograms\MinGW
set INSTALL_PREFIX=local-install
set Z7="C:\Users\SAPikachu\AppData\Local\PortablePrograms\7-Zip\7z.exe"
set CONFIGURATION=Release

pushd ..
for /f "usebackq" %%l in (`%MINGW_ROOT%\bin\git merge-base upstream/master HEAD`) do set FFMS_REV=%%l
for /f "usebackq" %%l in (`%MINGW_ROOT%\bin\git rev-parse --short %FFMS_REV%`) do set FFMS_REV=%%l
popd

if "x%FFMS_REV%" == "x" set FFMS_REV=unknown

call :build libav libav apps\libav\libav
call :build ffmpeg ffmpeg apps\ffmpeg\ffmpeg
call :build ffmbc ffmpeg apps\FFmbc-0.7.1

goto :eof

:build
setlocal

set VARIANT=%1
set FORK_NAME=%2
set LIBAV_PATH=%3

pushd %MINGW_ROOT%\%LIBAV_PATH%
for /f "usebackq" %%l in (`%MINGW_ROOT%\bin\git rev-parse --short HEAD`) do set REV=%%l
popd

if "x%REV%" == "x" set REV=unknown

if "%VARIANT%" == "ffmbc" set REV=%~nx3

set BUILD_NAME=ffms2-%FFMS_REV%-%VARIANT%-%REV%
set OUTPUT=output\%BUILD_NAME%
mkdir %OUTPUT%

rd /s /q Win32
rd /s /q ffmsindex_local
rd /s /q %CONFIGURATION%

msbuild ffms2_local.vcxproj /t:clean /p:configuration=%CONFIGURATION% /p:platform=Win32 /p:BuiltWithGCC=true /p:WithPthread=false /p:ForkName=%FORK_NAME% /p:Variant=%VARIANT% /p:"IncludePath=%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\include;%INCLUDE%;%MINGW_ROOT%\include" /p:"LibraryPath=%LIB%;%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\lib;%MINGW_ROOT%\lib"

msbuild ffmsindex_local.vcxproj /t:clean /p:configuration=%CONFIGURATION% /p:platform=Win32 /p:Variant=%VARIANT% /p:BuildProjectReferences=false /p:"IncludePath=%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\include;%INCLUDE%;D:\Microsoft Visual Studio 10.0\Projects\msinttypes;%MINGW_ROOT%\include"

msbuild ffms2_local.vcxproj /t:build /p:configuration=%CONFIGURATION% /p:platform=Win32 /p:BuiltWithGCC=true /p:WithPthread=false /p:ForkName=%FORK_NAME% /p:Variant=%VARIANT% /p:"IncludePath=%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\include;%INCLUDE%;%MINGW_ROOT%\include" /p:"LibraryPath=%LIB%;%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\lib;%MINGW_ROOT%\lib\gcc\mingw32\4.6.1;%MINGW_ROOT%\lib"

msbuild ffmsindex_local.vcxproj /t:build /p:configuration=%CONFIGURATION% /p:platform=Win32 /p:Variant=%VARIANT% /p:BuildProjectReferences=false /p:"IncludePath=%MINGW_ROOT%\%LIBAV_PATH%\%INSTALL_PREFIX%\include;%INCLUDE%;D:\Microsoft Visual Studio 10.0\Projects\msinttypes;%MINGW_ROOT%\include"

copy Win32\Release\ffms2.dll %OUTPUT%
copy Release\ffmsindex.exe %OUTPUT%

%Z7% a %OUTPUT%.7z .\%OUTPUT%\*

endlocal
