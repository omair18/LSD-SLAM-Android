#include <android/native_activity.h>
#include <android/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <cstdio>

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "AndroidUtils.cmake", __VA_ARGS__))
#define LIB_PATH "/data/data/com.tc.tar.lsd/lib/"

void * load_lib(const char * l) {
    void * handle = dlopen(l, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) LOGE( "dlopen('%s'): %s", l, strerror(errno) );
    return handle;
}

void ANativeActivity_onCreate(ANativeActivity * app, void * ud, size_t udsize) {
    LOGE("ANativeActivity_onCreate ENTER\n");
    #include "LSD_shared_load.h"

    // Look for standard entrypoint in user lib
    void (*stdentrypoint)(ANativeActivity*, void*, size_t);
    *(void **) (&stdentrypoint) = dlsym(load_lib( LIB_PATH "libLSD.so"), "ANativeActivity_onCreate");
    LOGE("stdentrypoint=%p\n", stdentrypoint);
    if (stdentrypoint) {
        (*stdentrypoint)(app, ud, udsize);
    }else{
        // Look for deferred load entry point
        void (*exdentrypoint)(ANativeActivity*, void*, size_t, const char*);
        *(void **) (&exdentrypoint) = dlsym(load_lib( LIB_PATH "libLSD.so"), "DeferredNativeActivity_onCreate");
        if (!exdentrypoint) {
            // Look in specific shared lib
            *(void **) (&exdentrypoint) = dlsym(load_lib( LIB_PATH "libpangolin.so"), "DeferredNativeActivity_onCreate");
        }
        if(exdentrypoint) {
            (*exdentrypoint)(app, ud, udsize, LIB_PATH "libLSD.so" );
        }else{
            LOGE( "Unable to find compatible entry point" );
        }
    }
}
