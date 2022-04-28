LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sfml-example

LOCAL_SRC_FILES := ../../../../../src/Core/Inputs/ControllerInput.cpp ../../../../../src/Core/Inputs/KeyboardInput.cpp ../../../../../src/Core/Inputs/RemoteInput.cpp ../../../../../src/Core/Inputs/ReplayInput.cpp ../../../../../src/Core/Logger.cpp ../../../../../src/Core/Objects/Character.cpp ../../../../../src/Core/Objects/Characters/Stickman.cpp ../../../../../src/Core/Objects/FakeObject.cpp ../../../../../src/Core/Objects/Object.cpp ../../../../../src/Core/Objects/Platform.cpp ../../../../../src/Core/Objects/Projectile.cpp ../../../../../src/Core/Resources/BattleManager.cpp ../../../../../src/Core/Resources/FrameData.cpp ../../../../../src/Core/Resources/Game.cpp ../../../../../src/Core/Resources/MoveListData.cpp ../../../../../src/Core/Resources/NetManager.cpp ../../../../../src/Core/Resources/PracticeBattleManager.cpp ../../../../../src/Core/Resources/Screen.cpp ../../../../../src/Core/Resources/SoundManager.cpp ../../../../../src/Core/Resources/TextureManager.cpp ../../../../../src/Core/Scenes/CharacterSelect.cpp ../../../../../src/Core/Scenes/InGame.cpp ../../../../../src/Core/Scenes/NetplayCharacterSelect.cpp ../../../../../src/Core/Scenes/NetplayInGame.cpp ../../../../../src/Core/Scenes/PracticeInGame.cpp ../../../../../src/Core/Scenes/ReplayInGame.cpp ../../../../../src/Core/Scenes/TitleScreen.cpp ../../../../../src/Core/Utils.cpp ../../../../../src/Game/main.cpp

LOCAL_C_INCLUDES := ../../../../../include
LOCAL_SHARED_LIBRARIES := sfml-system-d
LOCAL_SHARED_LIBRARIES += sfml-window-d
LOCAL_SHARED_LIBRARIES += sfml-graphics-d
LOCAL_SHARED_LIBRARIES += sfml-audio-d
LOCAL_SHARED_LIBRARIES += sfml-network-d
LOCAL_SHARED_LIBRARIES += sfml-activity-d
LOCAL_SHARED_LIBRARIES += openal
LOCAL_WHOLE_STATIC_LIBRARIES := sfml-main-d

include $(BUILD_SHARED_LIBRARY)

$(call import-module,third_party/sfml)
