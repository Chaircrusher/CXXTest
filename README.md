To build you need CMake and git installed: http://www.cmake.org

Build instructions:

mkdir <somedir>/kwscratch
cd <somedir>/kwscratch
git clone https://github.com/Chaircrusher/CXXTest.git
mkdir build
cd build
cmake ../CXXTest
make

Running instructions

The easy way is via CTest:

cd <somedir>/kwscratch/build/CxxTest-build
ctest -V

This runs the program, displaying the output.

To run it directly

<somedir>/kwscratch/build/CxxTest-build/CxxTest <somedir>/kwscratch/CXXTest/catalog.js other

I've done this the way I customarily work -- I didn't really want to write a json parser, so I found an open source library jsoncpp to handle it.

I moved the javascript array to the catalog.js file.  I had to change it slightly for jsoncpp to digest it; in particular, attribute names needed to be in quotes.
