rm -rf android/app/src/main/assets/assets

cp -r assets android/app/src/main/assets
echo 'LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sfml-example

LOCAL_SRC_FILES := \' > android/app/src/main/jni/Android.mk

for file in `find $(pwd)/src/LibCore -name "*.cpp"`; do
	echo -e "\t$file \\\\" >> android/app/src/main/jni/Android.mk
done
for file in `find $(pwd)/src/Game -name "*.cpp"`; do
	echo -e "\t$file \\\\" >> android/app/src/main/jni/Android.mk
done
echo "
LOCAL_SHARED_LIBRARIES := sfml-system-d
LOCAL_SHARED_LIBRARIES += sfml-window-d
LOCAL_SHARED_LIBRARIES += sfml-graphics-d
LOCAL_SHARED_LIBRARIES += sfml-audio-d
LOCAL_SHARED_LIBRARIES += sfml-network-d
LOCAL_SHARED_LIBRARIES += sfml-activity-d
LOCAL_SHARED_LIBRARIES += openal
LOCAL_WHOLE_STATIC_LIBRARIES := sfml-main-d

LOCAL_CFLAGS += -I $(pwd)/include -I $(pwd)/src/LibCore  -I $(pwd)/android/app/src/main/jni

include \$(BUILD_SHARED_LIBRARY)

\$(call import-module,third_party/sfml)" >> android/app/src/main/jni/Android.mk
