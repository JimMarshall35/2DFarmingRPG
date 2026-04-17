if not exist build mkdir build
conan profile detect --force
cd build
conan install .. -s build_type=Debug --deployer=full_deploy --deployer-folder=./
conan install .. -s build_type=Release --deployer=full_deploy --deployer-folder=./
