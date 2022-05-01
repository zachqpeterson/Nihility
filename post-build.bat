@echo off

echo "Compiling shaders..."

@REM echo "assets/shaders/Builtin.MaterialShader.vert.glsl -> assets/shaders/Builtin.MaterialShader.vert.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.MaterialShader.vert.glsl -o assets/shaders/Builtin.MaterialShader.vert.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

@REM echo "assets/shaders/Builtin.MaterialShader.frag.glsl -> assets/shaders/Builtin.MaterialShader.frag.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.MaterialShader.frag.glsl -o assets/shaders/Builtin.MaterialShader.frag.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

@REM echo "assets/shaders/Builtin.UIShader.vert.glsl -> assets/shaders/Builtin.UIShader.vert.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.UIShader.vert.glsl -o assets/shaders/Builtin.UIShader.vert.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

@REM echo "assets/shaders/Builtin.UIShader.frag.glsl -> assets/shaders/Builtin.UIShader.frag.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.UIShader.frag.glsl -o assets/shaders/Builtin.UIShader.frag.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Done."