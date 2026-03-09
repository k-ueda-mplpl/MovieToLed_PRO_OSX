#ifndef MOVIE_TO_LED_UTILS_HPP
#define MOVIE_TO_LED_UTILS_HPP

#include "LedProduct.hpp"
#include <ofTrueTypeFont.h>

#define SOFTWARE_VER "1.0.5"

class MovieToLedUtils {
public:
	struct DirPaths {
		inline static const std::string PRODUCT_DIR = "Product";
		inline static const std::string CONTENTS_DIR = "Contents";
		inline static const std::string VIDEO_DIR = "Video";
	};

	struct FilePaths {
		inline static const std::string MtLContents = "Contents/1_MtL_Contents.conf";
		inline static const std::string PARAMETER_JSON = "Parameter/Parameter.JSON";
		inline static const std::string PRODUCT_SETTING_JSON = "Parameter/ProductSetting.JSON";
		inline static const std::string DEVICE_MAP_FILE = "DeviceMap.md";
		inline static const std::string METADATA_FILE = "HOST_M5LED_METADATA.csv";
	};

	struct BinDirPaths {
		inline static const std::string DEVICE_8LINE_DIR = "/8LINE Device 255LED";
		inline static const std::string DEVICE_1000FPS_DIR = "/1000FPS Device 256LED";
		inline static const std::string DEVICE_4LINE_ABCD_DIR = "/4LINE Device/LINE0-3";
		inline static const std::string DEVICE_4LINE_EFGH_DIR = "/4LINE Device/LINE4-7";
	};
};

#endif