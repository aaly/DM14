export CC=clang
export CXX=clang++

rm -rf build
mkdir build
cd build
cmake  ..
make -j -l 8

#cd ../examples
#../build/DMC --sources distarray.m14 -I ../src/includes/
time ./DMC --sources ../examples/comm.m14 -I ../src/includes/


#time ./DMC --sources ../examples/image.m14 -I ../src/includes
#valgrind --tool=callgrind ./DMC --sources ../examples/image2.m14 -I ../src/includes


