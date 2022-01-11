# Fuzzable Atari simulator

Build instructions: first build ALE & install it to "$ALE_ROOT/install". Then you can build this simulator:

```sh
pwd  # should show path to your git checkout (e.g. /home/sam/repos/Arcade-Learning-Environment for me)
# first we build the library
mkdir -p build \
  && cd build \
  && cmake ../ -DCMAKE_BUILD_TYPE=Release -DBUILD_CPP_LIB=ON -DBUILD_PYTHON_LIB=OFF \
       -DCMAKE_INSTALL_PREFIX=/home/sam/repos/Arcade-Learning-Environment/install/ \
  && cmake --build . --target install
# now we build the sim
cd ../fuzzable-sim \
  && mkdir -p build \
  && cd build \
  && cmake ../ -DCMAKE_PREFIX_PATH=/home/sam/repos/Arcade-Learning-Environment/install/lib/cmake/
  && cmake --build .
```
