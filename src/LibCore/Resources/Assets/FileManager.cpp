//
// Created by PinkySmile on 08/08/23
//

#ifndef __ANDROID__
#include <filesystem>
#include <sys/stat.h>
#else
#include <Activity.hpp>
#endif
#include "FileManager.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	std::vector<FileManager::DirectoryEntry> FileManager::listDirectory(const std::string &path)
	{
		std::vector<DirectoryEntry> result;

#ifdef __ANDROID__
		std::vector<std::string> files;
		sf::priv::ActivityStates *states = sf::priv::getActivity();

		sf::Lock(states->mutex);

		AAssetDir *dir = AAssetManager_openDir(states->activity->assetManager, path.c_str());
		const char *entry = AAssetDir_getNextFileName(dir);

		while (entry) {
			files.push_back(entry);
			entry = AAssetDir_getNextFileName(dir);
		}
		AAssetDir_close(dir);

		JNIEnv *env = nullptr;

		states->activity->vm->AttachCurrentThread(&env, nullptr);

		auto context_object = states->activity->clazz;
		auto getAssets_method = env->GetMethodID(env->GetObjectClass(context_object), "getAssets", "()Landroid/content/res/AssetManager;");
		auto assetManager_object = env->CallObjectMethod(context_object, getAssets_method);
		auto list_method = env->GetMethodID(env->GetObjectClass(assetManager_object), "list", "(Ljava/lang/String;)[Ljava/lang/String;");
		jstring path_object = env->NewStringUTF(path.c_str());
		auto files_object = (jobjectArray)env->CallObjectMethod(assetManager_object, list_method, path_object);

		env->DeleteLocalRef(path_object);

		auto length = env->GetArrayLength(files_object);

		for (int i = 0; i < length; i++) {
			jstring jstr = (jstring)env->GetObjectArrayElement(files_object, i);
			const char * filename = env->GetStringUTFChars(jstr, nullptr);

			if (filename != nullptr) {
				result.push_back({
					std::find(files.begin(), files.end(), filename) == files.end(),
					path + "/" + filename
				});
				env->ReleaseStringUTFChars(jstr, filename);
			}

			env->DeleteLocalRef(jstr);
		}

		states->activity->vm->DetachCurrentThread();
#else
		for (auto &entry : std::filesystem::directory_iterator(path))
			result.push_back({
				entry.is_directory(),
				absolute(entry.path()).string()
			});
#endif
		return result;
	}

	bool FileManager::exists(const std::string &path)
	{
#ifdef __ANDROID__
		sf::priv::ActivityStates *states = sf::priv::getActivity();
		AAsset *asset = AAssetManager_open(states->activity->assetManager, path.c_str(), AASSET_MODE_STREAMING);

		if (asset)
			AAsset_close(asset);
		return asset != nullptr;
#else
		return std::filesystem::exists(std::filesystem::path(path));
#endif
	}

	std::string FileManager::readFull(const std::string &path)
	{
		std::string result;
#ifdef __ANDROID__
		sf::priv::ActivityStates *states = sf::priv::getActivity();
		AAsset *asset = AAssetManager_open(states->activity->assetManager, path.c_str(), AASSET_MODE_STREAMING);

		my_assert2(asset, "Cannot open asset " + path);
		result.resize(AAsset_getLength(asset));
		AAsset_read(asset, result.data(), result.size());
		AAsset_close(asset);
#else
		struct stat s;

		my_assert2(stat(path.c_str(), &s) >= 0, "Cannot stat " + path + ": " + strerror(errno));
		result.resize(s.st_size);

		std::ifstream stream{path, std::ifstream::binary};

		my_assert2(stream, "Cannot open " + path + ": " + strerror(errno));
		stream.read(result.data(), result.size());
#endif
		return result;
	}
}
