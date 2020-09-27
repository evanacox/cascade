# Cascade
The compiler for the Cascade language. 

### Building
Note: Building currently only tested on Linux
  1. Clone the repository and `cd` in
    - `git clone https://www.github.com/evanacox/cascade && cd cascade`
  2. Download the Git submodules
    - `git submodule update --init`
  3. Create build folder
    - `mkdir build && cd build`
  4. Run CMake, and give it the LLVM libraries it needs
    - `cmake .. -DCMAKE_BUILD_TYPE=Release -DLLVM_LIBS=$(llvm-config --ldflags --libs)`
  5. Run your generated build file
    - `ninja`, `make`, run Visual Studio, etc
  
### Testing
Testing currently relies on Clang/GCC `-fsanitize=undefined`
