LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
LOCAL_MODULE    := glm

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES) #export includes

LOCAL_STATIC_LIBRARIES += cpufeatures

include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
$(call import-add-path,../..)
