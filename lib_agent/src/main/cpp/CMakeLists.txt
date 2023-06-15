# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("agent")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#LOCAL_PATH := $(call my-dir)
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := ssl
#LOCAL_SRC_FILES := libs/openssl/${ANDROID_ABI}/lib/libssl.a
#LOCAL_EXPORT_CFLAGS := -I$(LOCAL_PATH)/libs/openssl/${ANDROID_ABI}/include
#include $(PREBUILT_STATIC_LIBRARY)
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := crypto
#LOCAL_SRC_FILES := libs/openssl/${ANDROID_ABI}/lib/libcrypto.a
#LOCAL_EXPORT_CFLAGS := -I$(LOCAL_PATH)/libs/openssl/${ANDROID_ABI}/include
#include $(PREBUILT_STATIC_LIBRARY)
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := curl
#LOCAL_SRC_FILES := libs/curl/${ANDROID_ABI}/lib/libcurl.a
#LOCAL_EXPORT_CFLAGS := -I$(LOCAL_PATH)/libs/curl/${ANDROID_ABI}/include
#include $(PREBUILT_STATIC_LIBRARY)
#
#include $(CLEAR_VARS)
#LOCAL_C_INCLUDES :=
#LOCAL_MODULE := curl-a-website
#LOCAL_SRC_FILES := jni.c curl-a-website/curl_a_website.c
#LOCAL_STATIC_LIBRARIES := libcurl libssl libcrypto
#LOCAL_LDLIBS := -lz
#include $(BUILD_SHARED_LIBRARY)

include_directories(
        include/curl
        jsoncpp/json
)




add_library( # Sets the name of the library.
        curl

        # Sets the library as a shared library.
        STATIC

        IMPORTED
        )
set_target_properties( curl
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libcurl.so )


set_target_properties(curl PROPERTIES LINKER_LANGUAGE CXX)



file(
        GLOB
        lib_json_src
        jsoncpp/*.cpp
)

#
#add_library(
#        json
#        SHARED
#        ${lib_json_src}
#)


add_library( # Sets the name of the library.
        agent

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        TerminalUtil.cpp
        vps_so.cpp
        zxselect.cpp
        zxsocket.cpp
        zxstring.cpp
        semx.cpp
        VpsMgr.cpp
        WorkNode.cpp
        FIFONode.cpp
        ChannelMgr.cpp
        zxpipe.cpp
        flowStatistics.cpp
        DataQueue.cpp
#        VpsConfigMgr.cpp
        VpsLoginMgr.cpp
        HttpPostModule.cpp
#        httpUtil.cpp
        #curl_http.cpp
        md5.cpp
        ${lib_json_src}

        )


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        agent

        # Links the target library to the log library
        # included in the NDK.
        curl
#        json
        ${log-lib})