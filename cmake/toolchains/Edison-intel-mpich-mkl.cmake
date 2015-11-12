# NOTE: You will need to have the GNU environment loaded, e.g., via the command
#       
# module swap PrgEnv-intel PrgEnv-gnu
#

#For bug mentioned http://public.kitware.com/pipermail/cmake/2014-November/059154.html
list(APPEND CMAKE_MODULE_PATH "/global/homes/s/schatz/rote/cmake/MyCMakeModules")
set(CMAKE_SYSTEM_NAME "Edison-Intel")

# The Cray wrappers
set(COMPILER_DIR /opt/cray/craype/2.2.1/bin)
#set(CMAKE_C_COMPILER       ${COMPILER_DIR}/cc)
#set(CMAKE_CXX_COMPILER     ${COMPILER_DIR}/CC)
set(CMAKE_Fortran_COMPILER ${COMPILER_DIR}/ftn)

# This is just a hack, as this machine always uses the above wrappers
set(MPI_C_COMPILER ${CMAKE_C_COMPILER})
set(MPI_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(MPI_Fortran_COMPILER ${CMAKE_Fortran_COMPILER})

#MKL flags obtained from $MKL_INC and $MKL after loading 'module mkl' and unloading 'module unload cray-libsci'

# TODO: Check if -std=c++11 if the right way to add C++11 support (probably not)
if(CMAKE_BUILD_TYPE MATCHES PureDebug OR
   CMAKE_BUILD_TYPE MATCHES HybridDebug)
  set(C_FLAGS   " -DPROFILE -g -static -Wl,-Bstatic")
  set(CXX_FLAGS " -DPROFILE -g -static -Wl,-Bstatic -std=c++98 ")
else()
  set(C_FLAGS   " -DPROFILE -O2 -static -Wl,-Bstatic")
  set(CXX_FLAGS " -DPROFILE -O2 -static -Wl,-Bstatic -std=c++98 ")
endif()

set(OpenMP_CXX_FLAGS "-openmp")

#set(MATH_LIBS "/opt/xt-libsci/11.0.06/gnu/46/mc12/lib/libsci_gnu.a;/opt/gcc/4.7.1/snos/lib64/libgfortran.a;-lm")
#set(MATH_LIBS "/opt/cray/libsci/default/GNU/47/sandybridge/lib/libsci_gnu.a;/opt/gcc/4.7.2/snos/lib64/libgfortran.a;-lm")
#set(MATH_LIBS "/opt/cray/libsci/default/GNU/47/sandybridge/lib/libsci_gnu.a;/opt/gcc/4.7.2/snos/lib64/libgfortran.a;/opt/cray/xc-sysroot/5.0.15/usr/lib64/libm.a")
set(MATH_LIBS "-mkl=parallel -craype-verbose")
set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
