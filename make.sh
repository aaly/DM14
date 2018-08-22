rm -rf build
mkdir build
cd build
cmake ..
make
cd ../examples
../build/DMC --sources distarray.m14
