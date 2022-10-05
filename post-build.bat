@echo off

echo "Compiling shaders..."

for %%i in (assets\shaders\*.vert) do (
	%VULKAN_SDK%\bin\glslc.exe %%i -o %%i.spv
)

for %%i in (assets\shaders\*.frag) do (
	%VULKAN_SDK%\bin\glslc.exe %%i -o %%i.spv
)

pause