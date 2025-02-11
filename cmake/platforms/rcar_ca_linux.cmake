set (CMAKE_SYSTEM_NAME      "Linux")
set (CMAKE_SYSTEM_PROCESSOR "arm64")
set (MACHINE                "rcar_ca" CACHE STRING "")

set (CROSS_PREFIX           "aarch64-poky-linux-")
set (CMAKE_C_COMPILER       "${CROSS_PREFIX}gcc")
set (CMAKE_CXX_COMPILER     "${CROSS_PREFIX}g++")

set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM  NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY  NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE  NEVER CACHE STRING "")

if(DEFINED ENV{CC})
    string(REGEX MATCH "--sysroot=([^ ]+)" SYSROOT_PATH $ENV{CC})

    if(SYSROOT_PATH)
        string(REPLACE "--sysroot=" "" SYSROOT_PATH ${SYSROOT_PATH})
        set(CMAKE_SYSROOT ${SYSROOT_PATH})
	include_directories("${CMAKE_SYSROOT}/usr/include")
    endif()
endif()


