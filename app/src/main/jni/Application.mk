APP_STL := gnustl_static
NDK_TOOLCHAIN_VERSION := 4.9
APP_CPPFLAGS := -frtti -fexceptions -mfloat-abi=softfp -O3 -Ofast -ffast-math -ftree-vectorize -mfpu=neon -std=gnu++0x -Wno-deprecated -pthread -std=c++11
APP_ABI := armeabi armeabi-v7a
#APP_ABI := armeabi-v7a
APP_OPTIM := release
#APP_OPTIM := debug
APP_PLATFORM := android-14
APP_ALLOW_MISSING_DEPS=true
