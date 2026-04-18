if not exist build mkdir build
conan profile detect --force
cd build
conan install .. -s build_type=%1 --deployer=full_deploy --deployer-folder=./ --build=missing
