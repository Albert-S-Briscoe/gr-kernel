find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_KERNEL gnuradio-kernel)

FIND_PATH(
    GR_KERNEL_INCLUDE_DIRS
    NAMES gnuradio/kernel/api.h
    HINTS $ENV{KERNEL_DIR}/include
        ${PC_KERNEL_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_KERNEL_LIBRARIES
    NAMES gnuradio-kernel
    HINTS $ENV{KERNEL_DIR}/lib
        ${PC_KERNEL_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-kernelTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_KERNEL DEFAULT_MSG GR_KERNEL_LIBRARIES GR_KERNEL_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_KERNEL_LIBRARIES GR_KERNEL_INCLUDE_DIRS)
