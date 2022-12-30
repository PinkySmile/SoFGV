LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := sfml-example

LOCAL_SRC_FILES := \
	/home/pinky/projects/Battle/src/LibCore/Resources/Screen.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/BattleManager.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/PracticeBattleManager.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/SoundManager.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/RandomWrapper.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Game.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/TextureManager.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Network/NetManager.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Network/Packet.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Network/ServerConnection.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Network/Connection.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/Network/ClientConnection.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/FrameData.cpp \
	/home/pinky/projects/Battle/src/LibCore/Resources/MoveListData.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/InGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/PracticeInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Menu.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/MenuItem.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/TitleScreen.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/LoadingScene.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/RollbackMachine.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/ClientCharacterSelect.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/NetworkInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/NetworkCharacterSelect.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/ServerInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/ServerCharacterSelect.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/ClientInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/Network/SyncTestInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/ReplayInGame.cpp \
	/home/pinky/projects/Battle/src/LibCore/Scenes/CharacterSelect.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/FakeObject.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/Character.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/Projectile.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/StageObjects/Cloud.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/StageObjects/StageObject.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/Object.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/Platform.cpp \
	/home/pinky/projects/Battle/src/LibCore/Objects/Characters/Stickman.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/RollbackInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/ReplayInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/ControllerInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/DelayInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/KeyboardInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/RemoteInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Inputs/NetworkInput.cpp \
	/home/pinky/projects/Battle/src/LibCore/Utils.cpp \
	/home/pinky/projects/Battle/src/LibCore/Logger.cpp \
	/home/pinky/projects/Battle/src/Game/main.cpp \

LOCAL_SHARED_LIBRARIES := sfml-system-d
LOCAL_SHARED_LIBRARIES += sfml-window-d
LOCAL_SHARED_LIBRARIES += sfml-graphics-d
LOCAL_SHARED_LIBRARIES += sfml-audio-d
LOCAL_SHARED_LIBRARIES += sfml-network-d
LOCAL_SHARED_LIBRARIES += sfml-activity-d
LOCAL_SHARED_LIBRARIES += openal
LOCAL_SHARED_LIBRARIES += sfml-activity-d
LOCAL_WHOLE_STATIC_LIBRARIES := sfml-main-d

LOCAL_CFLAGS += -I /home/pinky/projects/Battle/include -I /home/pinky/projects/Battle/src/LibCore

include $(BUILD_SHARED_LIBRARY)

$(call import-module,third_party/sfml)
