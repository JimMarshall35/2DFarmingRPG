if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
cmake --build . --config Release
robocopy "../WfAssets/" "game\Release/WfAssets/" /E /XO
robocopy "engine\src\Release" "game\Release" "StardaltonegenewEngine.dll" /E /XO
robocopy "engine\src\Release" "enginetest\Release" "StardewEngine.dll" /E /XO
robocopy "..\enginetest\data" "enginetest\Release\data" /E /XO
robocopy "atlastool\Release" "../engine/scripts" "AtlasTool.exe" /E /XO
robocopy "engine\src\Release" "../engine/scripts" "StardewEngine.dll" /E /XO

rem // This handles the exit code (`ErrorLevel`) returned by `robocopy` properly: (needed for CI)
if ErrorLevel 8 (exit /B 1) else (exit /B 0)

