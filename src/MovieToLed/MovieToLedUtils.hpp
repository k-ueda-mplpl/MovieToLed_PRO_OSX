#ifndef MOVIE_TO_LED_UTILS_HPP
#define MOVIE_TO_LED_UTILS_HPP

#include "LedProduct.hpp"
#include <ofTrueTypeFont.h>

#define SOFTWARE_VER "1.0.5"

class MovieToLedUtils {
public:
	struct DirPaths {
		inline static const std::string PROJECT_DIR = "Formation/OUTPUT";
		inline static const std::string PRODUCT_DIR = "Address/Product";
		inline static const std::string PRODUCT_DIR_OLD_FORMATION = "MtL/ForOldFormation/Product";
		inline static const std::string CONTENTS_DIR_OLD_FORMATION = "MtL/ForOldFormation/Contents";
		inline static const std::string VIDEO_DIR = "MtL/Video";
	};
	
	struct FilePaths {
		inline static const std::string MtLContents = "MtL/Contents/1_MtL_Contents.conf";
		inline static const std::string PARAMETER_JSON = "MtL/Parameter/Parameter.JSON";
		inline static const std::string PRODUCT_SETTING_JSON = "MtL/Parameter/ProductSetting.JSON";
		inline static const std::string DEVICE_MAP_FILE = "DeviceMap.md";
		inline static const std::string METADATA_FILE = "HOST_M5LED_METADATA.csv";
	};
	
	struct BinDirPahts {
		inline static const std::string DEVICE_8LINE_DIR = "/8LINE Device 255LED";
		inline static const std::string DEVICE_1000FPS_DIR = "/1000FPS Device 256LED";
		inline static const std::string DEVICE_4LINE_ABCD_DIR = "/4LINE Device/LINE0-3";
		inline static const std::string DEVICE_4LINE_EFGH_DIR = "/4LINE Device/LINE4-7";
	};
	
	struct DisplaySize {
		inline static const uint16_t FULL_HD_WIDTH = 1920;
		inline static const uint16_t FULL_HD_HEIGHT = 1080;
		inline static const uint16_t UHD_4K_WIDTH = 3840;
		inline static const uint16_t UHD_4K_HEIGHT = 2160;
	};
	
	struct OutputFiles {
		std::string M5LED;
		std::string BIN_DEVICE_8LINE;
		std::string BIN_DEVICE_4LINE_ABCD;
		std::string BIN_DEVICE_4LINE_EFGH;
		std::string BIN_DEVICE_1000FPS;
	};
	
	// データ作成に必要なオブジェクト
	struct OutputData {
		LedProductManager led_product;
		std::vector<OutputFiles> output_files;
	};
	
	struct MtLStates {
		struct MainProcess {
			inline static bool is_old_formation = false;
			inline static bool is_ready = false;
			inline static bool is_extracting = false;
			inline static bool is_converting = false;
			inline static bool is_completed = false;
			inline static uint16_t process_count = 0;
		};
		struct Preprocess {
			inline static bool config_all_exist = false;
			inline static bool m5led_already_exists = false;
		};
		struct Converter {
			inline static bool output_8line_bin = false;
			inline static bool output_4line_bin = false;
		};
		
		static bool isOldFormatinoData(){
			return MainProcess::is_old_formation;
		};
		static void onReady(bool is_load_video) {
			MainProcess::is_ready = is_load_video && Preprocess::config_all_exist;
		};
		static void startExtract() {
			MainProcess::is_ready = false;
			MainProcess::is_extracting = true;
			MainProcess::is_converting = false;
			MainProcess::is_completed = false;
			printf("Start Create M5LED Process\r\n");
		};
		static void startConvert() {
			MainProcess::is_ready = false;
			MainProcess::is_extracting = false;
			MainProcess::is_converting = true;
			MainProcess::is_completed = false;
		};
		static void stop() {
			MainProcess::is_ready = false;
			MainProcess::is_extracting = false;
			MainProcess::is_converting = false;
			MainProcess::is_completed = false;
		};
		static bool isFirstTime() {
			uint16_t prev_count = MainProcess::process_count;
			MainProcess::process_count++;
			return prev_count == 0;
		};
		static bool isProcessing() {
			return MainProcess::is_extracting || MainProcess::is_converting;
		};
		static bool isOutputBin() {
			return Converter::output_8line_bin || Converter::output_4line_bin;
		};
		static void onExtracted() {
			if (MainProcess::is_extracting) {
				printf("Finish Create M5LED Process\r\n");
				MainProcess::is_extracting = false;
				MainProcess::is_converting = isOutputBin();
				if (MainProcess::is_converting) printf("Start Convert to BIN Process\r\n");
			}
		};
		static void onCompleted() {
			MainProcess::is_ready = false;
			MainProcess::is_extracting = false;
			MainProcess::is_converting = false;
			MainProcess::is_completed = true;
			MainProcess::process_count = 0;
			printf("Finish All Process\r\n");
		};
	};
	
	inline static const uint16_t BUFF_SIZE = LedProduct::Device::MAX_NUM_LED * 3 * LedProduct::Device::MAX_NUM_LINE;
	inline static const uint8_t MAX_NUM_SCENE = 70;
	inline static std::string error_msg;
	
	struct Font {
		inline static ofTrueTypeFont Tiny;
		inline static ofTrueTypeFont Small;
		inline static ofTrueTypeFont Middle;
		inline static ofTrueTypeFont Large;
	};
	
	static void collectFilesRecursive(const ofDirectory & dir, std::vector<ofFile> & out) {
		ofDirectory d = dir;
		d.listDir();
		for (auto & f : d.getFiles()) {
			if (f.isDirectory()) {
				ofDirectory sub(f.getAbsolutePath());
				MovieToLedUtils::collectFilesRecursive(sub, out);
			} else {
				out.push_back(f);
			}
		}
	}
	
	static bool isOldFormationData(std::vector<ofFile> files) {
		// Old Formation
		static std::string CONTENTS_CONF_NAME = "1_MtL_Contents.conf";
		static std::string LED_MAP_NAME = "suit_led_s";
		bool is_mtl_contents = false;
		bool is_led_csv = false;
		for (ofFile file : files) {
			if (!is_mtl_contents) is_mtl_contents = file.getFileName() == CONTENTS_CONF_NAME;
			if (!is_led_csv) is_led_csv = file.getFileName().find(LED_MAP_NAME) != std::string::npos && ofFilePath::getFileExt(file) == "csv";
			if (is_mtl_contents && is_led_csv) {
				MtLStates::MainProcess::is_old_formation = true;
				break;
			}
		}
		return is_mtl_contents && is_led_csv;
	}
	
	static bool isFormationData(std::vector<ofFile> files){
		// New Formation
		static std::string CONFIG_NAME = "MtL_config.json";
		static std::string LED_MAP_NAME = "suit_led_s";
		bool is_mtl_config = false;
		bool is_led_csv = false;
		for (ofFile file : files) {
			if (!is_mtl_config) is_mtl_config = file.getFileName() == CONFIG_NAME;
			if (!is_led_csv) is_led_csv = file.getFileName().find(LED_MAP_NAME) != std::string::npos && ofFilePath::getFileExt(file) == "csv";
			if (is_mtl_config && is_led_csv) {
				MtLStates::MainProcess::is_old_formation = false;
				break;
			}
		}
		return is_mtl_config && is_led_csv;
	}
	
	static void onDroppedContent(const ofDirectory & dir, std::function<void()> callback) {
		std::vector<ofFile> files;
		MovieToLedUtils::collectFilesRecursive(dir, files);
		if(MovieToLedUtils::isFormationData(files)){
			printf("This File is New Formation Data\r\n");
			ofDirectory d(MovieToLedUtils::DirPaths::PROJECT_DIR);
			if (d.exists()) d.listDir();
			ofFile::copyFromTo(dir.getAbsolutePath(), MovieToLedUtils::DirPaths::PROJECT_DIR, true, true);
			if (callback) callback();
			
		}else if (MovieToLedUtils::isOldFormationData(files)) {
			printf("This File is Old Formation Data\r\n");
			printf("Remove Directory or File in Dir:%s\r\n", MovieToLedUtils::DirPaths::CONTENTS_DIR_OLD_FORMATION.c_str());
			ofDirectory d(MovieToLedUtils::DirPaths::CONTENTS_DIR_OLD_FORMATION);
			if (d.exists()) d.listDir();
			for (int i = 0; i < static_cast<int>(d.size()); i++) {
				if (d.getFile(i).exists()) d.getFile(i).remove(true);
			}
			ofFile::copyFromTo(dir.getAbsolutePath(), MovieToLedUtils::DirPaths::CONTENTS_DIR_OLD_FORMATION, true, true);
			if (callback) callback();
		}
	}
};

class FileUtils {
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
};

#endif
