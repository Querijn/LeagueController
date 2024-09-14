@echo off
echo Building image shader (%~dp0/shaders/image.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/image.glsl --output %~dp0/src/shaders/image.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Building skerm_image shader (%~dp0/shaders/skerm_image.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/skerm_image.glsl --output %~dp0/src/shaders/skerm_image.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Building skerm_container shader (%~dp0/shaders/skerm_container.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/skerm_container.glsl --output %~dp0/src/shaders/skerm_container.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Building text shader (%~dp0/shaders/text.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/text.glsl --output %~dp0/src/shaders/text.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Building radial_menu shader (%~dp0/shaders/radial_menu.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/radial_menu.glsl --output %~dp0/src/shaders/radial_menu.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Building debug_renderer shader (%~dp0/shaders/debug_renderer.glsl)..
%~dp0/ext/sokol/tools/bin/win32/sokol-shdc --input %~dp0/shaders/debug_renderer.glsl --output %~dp0/src/shaders/debug_renderer.hpp --slang glsl330:glsl300es:hlsl5:glsl100
echo:

echo Done
