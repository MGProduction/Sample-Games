LOCAL_PATH := $(call my-dir)

#include $(CLEAR_VARS)
#LOCAL_MODULE    := libopenal 
#LOCAL_SRC_FILES := libopenal.a
#include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libopenal
LOCAL_SRC_FILES := libopenal.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := world2d
LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL \
                -DHAVE_SLEEP \
		-DHAVE_STAT \
		-DSOUNDS_ON \
		-ffast-math -O3 -funroll-loops
								
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../ \
					$(LOCAL_PATH)/../../../lib \
					$(LOCAL_PATH)/zip \
					$(LOCAL_PATH)/al
								
LOCAL_SRC_FILES := \
	../../../g_main.c \
	../../../lib/l_module.c \
	../../../lib/l_openGL.c \
	../../../lib/l_openAL.c \
	android_main.c \
	android_jni.c \
	al/alutBufferData.c \
	al/alutCodec.c \
	al/alutError.c \
	al/alutInit.c \
	al/alutInputStream.c \
	al/alutLoader.c \
	al/alutOutputStream.c	\
	al/alutUtil.c \
	al/alutVersion.c \
	al/alutWaveform.c \
	zip/adler32.c \
	zip/crc32.c \
	zip/infback.c \
	zip/inffast.c \
	zip/inflate.c \
	zip/inftrees.c \
	zip/ioapi.c \
	zip/mztools.c \
	zip/unzip.c \
	zip/zutil.c

LOCAL_SHARED_LIBRARIES := libopenal
LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
