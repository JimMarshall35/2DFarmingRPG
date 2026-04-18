if not exist build mkdir build
cd build
mkdir ./install_dir
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBAKE_ITEM_DEFS=%2 -DSTARDEW_PLATFORM=%3 -DSTARDEW_GL_API_TYPE=%4
cmake --build . --config %1
cmake --install . --prefix ./install_dir

@REM robocopy "../WfAssets/" "game\Release/WfAssets/" /E /XO
@REM robocopy engine\src\%1 game\%1 "StardewEngine.dll" /E /XO
@REM robocopy engine\src\%1 enginetest\%1 "StardewEngine.dll" /E /XO
@REM robocopy "..\enginetest\data" enginetest\%1\data /E /XO
@REM robocopy atlastool\%1 "../engine/scripts" "AtlasTool.exe" /E /XO
@REM robocopy engine\src\%1 "../engine/scripts" "StardewEngine.dll" /E /XO

rem // This handles the exit code (`ErrorLevel`) returned by `robocopy` properly: (needed for CI)
if ErrorLevel 8 (exit /B 1) else (exit /B 0)

