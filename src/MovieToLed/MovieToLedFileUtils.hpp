#ifndef MOVIETOLED_FILE_UTILS_HPP
#define MOVIETOLED_FILE_UTILS_HPP

#include "MovieToLedUtils.hpp"
#include <functional>
#include <ofFileUtils.h>
#include <string>
#include <vector>

class MovieToLedFileUtils {
public:
	static void remove(std::string parent_dir_path) {
		//parent_dir_path内のディレクトリとファイルを削除
		printf("Remove Directory or File in Dir:%s\r\n", parent_dir_path.c_str());
		ofDirectory dir(parent_dir_path);
		if (dir.exists()) dir.listDir();
		for (int i = 0; i < static_cast<int>(dir.size()); i++) {
			if (dir.getFile(i).exists()) dir.getFile(i).remove(true);
		}
	};

	static bool createDir(std::string path) {
		ofDirectory dir(path);
		return dir.create(true);
	};

	static bool createFile(std::string path, bool is_binary = true) {
		ofFile new_file(path, ofFile::WriteOnly, is_binary);
		printf("Create File:%s\r\n", new_file.path().c_str());
		return new_file.create();
	};

	static bool openFile(ofFile & file, std::string path, ofFile::Mode mode, bool is_binary = true) {
		if (file && file.is_open()) {
			file.close();
		}
		file.open(path, mode, is_binary);
		return file.is_open();
	};

	static void collectFilesRecursive(const ofDirectory & dir, std::vector<ofFile> & out) {
		ofDirectory d = dir;
		d.listDir();
		for (auto & f : d.getFiles()) {
			if (f.isDirectory()) {
				ofDirectory sub(f.getAbsolutePath());
				MovieToLedFileUtils::collectFilesRecursive(sub, out);
			} else {
				out.push_back(f);
			}
		}
	}

	static bool isMtLContents(const std::vector<ofFile> & files) {
		static const std::string MTL_CONTENTS = "1_MtL_Contents.conf";
		static const std::string LED_SCENE = "suit_led_s";

		bool is_mtl_contents = false;
		bool is_led_csv = false;
		for (ofFile file : files) {
			if (!is_mtl_contents) is_mtl_contents = file.getFileName() == MTL_CONTENTS;
			if (!is_led_csv) is_led_csv = file.getFileName().find(LED_SCENE) != std::string::npos && ofFilePath::getFileExt(file) == "csv";
			if (is_mtl_contents && is_led_csv) break;
		}
		return is_mtl_contents && is_led_csv;
	}

	static void onDroppedContent(const ofDirectory & dir, std::function<void()> callback) {
		std::vector<ofFile> files;
		MovieToLedFileUtils::collectFilesRecursive(dir, files);
		if (MovieToLedFileUtils::isMtLContents(files)) {
			printf("Remove Directory or File in Dir:%s\r\n", MovieToLedUtils::DirPaths::CONTENTS_DIR.c_str());
			ofDirectory d(MovieToLedUtils::DirPaths::CONTENTS_DIR);
			if (d.exists()) d.listDir();
			for (int i = 0; i < static_cast<int>(d.size()); i++) {
				if (d.getFile(i).exists()) d.getFile(i).remove(true);
			}
			ofFile::copyFromTo(dir.getAbsolutePath(), MovieToLedUtils::DirPaths::CONTENTS_DIR, true, true);
			if (callback) callback();
		}
	}
};

#endif