echo "Configuring and building visual_recognition ..."

if [ -d "build" ]; then
    rm -rf build
fi
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j12

