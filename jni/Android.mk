LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)



LOCAL_C_INCLUDES  :=  /home/yeheng/workspace_adt/BreathSensorJni/jni/include

LOCAL_SRC_FILES := breathSensor.cpp \
                                  		generateCmd.cpp  \
                                  		UIhandler.cpp
                                  		

LOCAL_LDLIBS := -lc 	-llog		
LOCAL_MODULE    := breathSensor_jni
include $(BUILD_SHARED_LIBRARY)
