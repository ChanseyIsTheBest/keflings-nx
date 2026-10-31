@echo off
rem keflings-nx: the Switch build's shader inputs from the PC build (docs/banjo-nx/deko3d-renderer.md sections 5 and 30):
rem   1. Renut's XenosRecomp (the PC build's) on out\renut_shader_dump in deko3d mode (BANJO_DEKO3D_SPIRV_DIR), with the
rem      PC build's early-alpha HLSL (BANJO_HLSL_OVERRIDE), so the console's shaders are the PC's;
rem   2. tools\renut_switch_shader_cache.py: Renut's shader table without DXIL (app\out\renut_switch);
rem   3. tools\deko3d_shaders.py (SPIR-V -> uam GLSL -> DKSH check), tools\deko3d_glsl_opt.py (every pass);
rem   4. tools\deko3d_shader_package.py: the GLSL package embedded in the NRO (app\out\renut_switch).
rem Everything written is game data (out\ and app\out\ only, never committed).
set "ROOT=%~dp0.."
set "PATH=C:\Python314;%PATH%"
set "PC=%ROOT%\app\out\pc-nx"
set "RECOMP=%PC%\xenos_recomp\build\XenosRecomp\XenosRecomp.exe"
set "COMMON=%ROOT%\app\thirdparty\reblue-XenosRecomp\XenosRecomp\shader_common.h"
if exist "%ROOT%\out\deko3d_shaders" rmdir /s /q "%ROOT%\out\deko3d_shaders"
mkdir "%ROOT%\out\deko3d_shaders" || exit /b 1
set "BANJO_DEKO3D_SPIRV_DIR=%ROOT%\out\deko3d_shaders\spirv"
set "BANJO_HLSL_OVERRIDE=%PC%\renut_gpu\early_alpha_hlsl"
"%RECOMP%" "%ROOT%\out\renut_shader_dump" "%ROOT%\out\deko3d_shaders\scratch_cache.cpp" "%COMMON%" || exit /b 1
set BANJO_DEKO3D_SPIRV_DIR=
set BANJO_HLSL_OVERRIDE=
python "%ROOT%\tools\renut_switch_shader_cache.py" --pc "%PC%" || exit /b 1
python "%ROOT%\tools\deko3d_shaders.py" --spirv "%ROOT%\out\deko3d_shaders\spirv" --out "%ROOT%\out\deko3d_shaders" || exit /b 1
python "%ROOT%\tools\deko3d_glsl_opt.py" --in "%ROOT%\out\deko3d_shaders\glsl" --out "%ROOT%\out\deko3d_shaders\glsl_opt" || exit /b 1
python "%ROOT%\tools\deko3d_shader_package.py" --glsl "%ROOT%\out\deko3d_shaders\glsl_opt" --out "%ROOT%\app\out\renut_switch" || exit /b 1
echo switch_shaders: done
