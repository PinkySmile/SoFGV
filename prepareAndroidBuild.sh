rm -rf android/app/src/main/assets/assets

cp -r assets android/app/src/main/assets
echo -n 'LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sfml-example

LOCAL_SRC_FILES :=' > android/app/src/main/jni/Android.mk
cd android/app/src/main/jni
for file in `find ../../../../../src/LibCore -name "*.cpp"`; do
	echo -n " $file" >> Android.mk
done
for file in `find ../../../../../src/Game -name "*.cpp"`; do
	echo -n " $file" >> Android.mk
done
echo '

LOCAL_SHARED_LIBRARIES := sfml-system-d
LOCAL_SHARED_LIBRARIES += sfml-window-d
LOCAL_SHARED_LIBRARIES += sfml-graphics-d
LOCAL_SHARED_LIBRARIES += sfml-audio-d
LOCAL_SHARED_LIBRARIES += sfml-network-d
LOCAL_SHARED_LIBRARIES += sfml-activity-d
LOCAL_SHARED_LIBRARIES += openal
LOCAL_WHOLE_STATIC_LIBRARIES := sfml-main-d

include $(BUILD_SHARED_LIBRARY)

$(call import-module,third_party/sfml)' >> Android.mk
cd -
