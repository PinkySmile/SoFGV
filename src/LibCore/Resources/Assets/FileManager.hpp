//
// Created by andgel on 08/08/23
//

#ifndef SOFGV_FILEMANAGER_HPP
#define SOFGV_FILEMANAGER_HPP


#include <vector>
#include <string>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

namespace SpiralOfFate
{
	class FileManager {
	public:
		struct DirectoryEntry {
			bool dir;
			std::string path;
		};

		FileManager() = default;
		std::vector<DirectoryEntry> listDirectory(const std::string &path);
		bool exists(const std::string &path);
		std::string readFull(const std::string &path);
	};
}


#endif //SOFGV_FILEMANAGER_HPP
