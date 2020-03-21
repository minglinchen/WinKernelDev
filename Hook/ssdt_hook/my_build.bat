@echo on
@echo 64MOD: %1 is "chk" or "fre". %2 is "WXP" or "W2K" or "WNET". %3 is the module name.%4 is "AMD64" or "IA64". %5 is "\a".
@echo 32MOD: %1 is "chk" or "fre". %2 is "WXP" or "W2K" or "WNET". %3 is the module name.%4 is "\a".

if "%4"=="/a" call my_clean
if "%5"=="/a" call my_clean

pushd.
if not "%4" == "/a" call %BASEDIR%\bin\setenv.bat %BASEDIR% %1 %4 %2
if "%4" == "/a" call %BASEDIR%\bin\setenv.bat %BASEDIR% %1 %2
popd

set INCLUDE=%INCLUDE%;%BASEDIR%\inc\ddk\%2;%BASEDIR%\inc\ddk\wdm\%2;..\inc

@echo on
build

if not exist ..\inc mkdir ..\inc
if not exist ..\inc\%3 mkdir ..\inc\%3
if not exist ..\lib mkdir ..\lib
copy *.h ..\inc\%3\


if "%4"=="" copy .\obj%1_%2_x86\i386\%3.lib ..\lib\%3_%1_%2.lib
if "%4"==""  goto :EOF
if "%4"=="/a" copy .\obj%1_%2_x86\i386\%3.lib ..\lib\%3_%1_%2.lib
if "%4"=="/a" goto :EOF
copy .\obj%1_%2_%_BUILDARCH%\%_BUILDARCH%\%3.lib ..\lib\%3_%1_%2_%_BUILDARCH%.lib
