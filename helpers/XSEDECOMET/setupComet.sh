#!/bin/bash
set -e
set -o pipefail
#script to setup and build DFT-FE

########################################################################
#Provide paths below for external libraries, compiler options and flags,
# and optimization flag

#Paths for external libraries
dealiiPetscRealDir="/software/community/dftfe/dealii/intel2018_real"
dealiiPetscComplexDir="/software/community/dftfe/dealii/intel2018_complex"
alglibDir="/software/community/dftfe/alglib/cpp/src"
libxcDir="/software/community/dftfe/libxc/intel2018_libxc"
spglibDir="/software/community/dftfe/spglib/intel2018_spglib"
xmlIncludeDir="/usr/include/libxml2"
xmlLibDir="/usr/lib64"

#If you have installed dealii by linking with intel mkl library set underlying flag to "ON",
#otherwise set it to "OFF"
withIntelMkl=ON

#Compiler options and flags
c_compiler=mpicc
cxx_compiler=mpicxx
c_flagsRelease="-O3"
cxx_flagsRelease="-O3"

#Option to link to ELPA
withELPA=OFF

#Optmization flag: 1 for optimized mode and 0 for debug mode compilation
optimizedMode=1

###########################################################################
#Usually, no changes are needed below this line
#
RCol='\e[0m'
Blu='\e[0;34m';
if [ $optimizedMode == 1 ]; then
  if [ -d "build/release" ]; then
    echo -e "${Blu}build/release directory already present${RCol}"
    # Control will enter here if build directory exists.
    cd build
    cd release
    echo -e "${Blu}Building Real executable in Optimized (Release) mode...${RCol}"
    mkdir -p real && cd real && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" -DCMAKE_C_FLAGS_RELEASE="$c_flagsRelease" -DCMAKE_BUILD_TYPE=Release -DDEAL_II_DIR=$dealiiPetscRealDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl -DWITH_ELPA=$withELPA -DELPA_LIB_DIR=$elpaLibDir -DELPA_INCLUDE_DIR=$elpaIncludeDir ../../../. && make -j 4 && cd ..
    echo -e "${Blu}Building Complex executable in Optimized (Release) mode...${RCol}"
    mkdir -p complex && cd complex && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" -DCMAKE_C_FLAGS_RELEASE="$c_flagsRelease" -DCMAKE_BUILD_TYPE=Release -DDEAL_II_DIR=$dealiiPetscComplexDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl ../../../. && make -j 4 && cd ../..
  else
    rm -rf build/release
    echo -e "${Blu}Creating build directory...${RCol}"
    mkdir -p build && cd build
    mkdir -p release && cd release
    echo -e "${Blu}Building Real executable in Optimized (Release) mode...${RCol}"
    mkdir -p real && cd real && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" -DCMAKE_C_FLAGS_RELEASE="$c_flagsRelease" -DCMAKE_BUILD_TYPE=Release -DDEAL_II_DIR=$dealiiPetscRealDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl -DWITH_ELPA=$withELPA -DELPA_LIB_DIR=$elpaLibDir -DELPA_INCLUDE_DIR=$elpaIncludeDir ../../../. && make -j 4 && cd ..
    echo -e "${Blu}Building Complex executable in Optimized (Release) mode...${RCol}"
    mkdir -p complex && cd complex && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" -DCMAKE_BUILD_TYPE=Release -DDEAL_II_DIR=$dealiiPetscComplexDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl  ../../../. && make -j 4 && cd ../..
  fi
else
  if [ -d "build/debug" ]; then
    echo -e "${Blu}build/debug directory already present${RCol}"
    # Control will enter here if build directory exists.
    cd build
    cd debug
    echo -e "${Blu}Building Real executable in Debug mode...${RCol}"
    mkdir -p real && cd real && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_BUILD_TYPE=Debug -DDEAL_II_DIR=$dealiiPetscRealDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl -DWITH_ELPA=$withELPA -DELPA_LIB_DIR=$elpaLibDir -DELPA_INCLUDE_DIR=$elpaIncludeDir ../../../. && make -j 4 && cd ..
    echo -e "${Blu}Building Complex executable in Debug mode...${RCol}"
    mkdir -p complex && cd complex && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_BUILD_TYPE=Debug -DDEAL_II_DIR=$dealiiPetscComplexDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl ../../../. && make -j 4 && cd ../..
  else
    rm -rf build/debug
    echo -e "${Blu}Creating build directory...${RCol}"
    mkdir -p build && cd build
    mkdir -p debug && cd debug
    echo -e "${Blu}Building Real executable in Debug mode...${RCol}"
    mkdir -p real && cd real && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_BUILD_TYPE=Debug -DDEAL_II_DIR=$dealiiPetscRealDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl -DWITH_ELPA=$withELPA -DELPA_LIB_DIR=$elpaLibDir -DELPA_INCLUDE_DIR=$elpaIncludeDir ../../../. && make -j 4 && cd ..
    echo -e "${Blu}Building Complex executable in Debug mode...${RCol}"
    mkdir -p complex && cd complex && cmake -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -DCMAKE_BUILD_TYPE=Debug -DDEAL_II_DIR=$dealiiPetscComplexDir -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir -DXML_INCLUDE_DIR=$xmlIncludeDir -DWITH_INTEL_MKL=$withIntelMkl ../../../. && make -j 4 && cd ../..
  fi
fi
echo -e "${Blu}Build complete.${RCol}"
