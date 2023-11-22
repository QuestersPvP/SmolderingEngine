@echo off

pushd ..
Dependencies\Premake\Windows\premake5.exe --file=Build.lua vs2022
popd
pause