if not exist build mkdir build
cd build
mkdir ./install_dir
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBAKE_ITEM_DEFS=%2 -DSTARDEW_PLATFORM=%3 -DSTARDEW_GL_API_TYPE=%4
cmake --build . --config %1
cmake --install . --prefix ./install_dir --config %1

