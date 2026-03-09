#ifndef PRODUCT_PROFILE_LOADER
#define PRODUCT_PROFILE_LOADER

#include "MovieToLedFileUtils.hpp"
#include "MovieToLedRuntimeState.hpp"
#include "ProductProfile.hpp"
#include "ProductProfileManager.hpp"
#include <ofFileUtils.h>
#include <ofxJSON.h>
#include <vector>

class ProductProfileLoader {
public:
	static bool loadProductProfile(std::vector<ProductProfile> & profiles) {
		profiles.clear();
		// MtL_Contentsの読み込み
		ofFile file(MovieToLedUtils::FilePaths::MtLContents);
		if (file.exists()) {
			ofBuffer buff(file);
			for (auto line : buff.getLines()) {
				std::vector<std::string> dat = ofSplitString(line, ",");
				if (dat.size() < 2) {
					continue;
				}
				const std::string & name = dat[0];
				uint16_t num_product = static_cast<uint16_t>(ofToInt(dat[1]));
				if (name.empty() || num_product == 0) {
					continue;
				}
				ProductProfile product_profile;
				product_profile.setProductInfo(name, num_product);
				profiles.push_back(product_profile);
			}
			file.close();
			buff.clear();
			return true;
		}
		return false;
	};

	static void loadProductDeviceProfile(std::vector<ProductProfile> & profiles, ProductProfileManager & manager) {
		if (profiles.empty()) return;
		// 各プロダクトの.confがあるか確認
		manager.verify();
		for (uint8_t i = 0; i < static_cast<uint8_t>(profiles.size()); i++) {
			if (manager.isExist(i)) {
				std::string path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + profiles[i].product_name + ".conf";
				ofFile file(path);
				if (!file.exists()) {
					MovieToLedRuntimeState::error_msg = path + "\r\nFile Not Found.";
					return;
				}
				ofBuffer buff(file);
				uint16_t num_device = 1;
				ProductProfile::DeviceType device_type = ProductProfile::DeviceType::LINE8;
				for (auto line : buff.getLines()) {
					// ファイルの構造
					// Name:name
					// DeviceType:4 or 8
					// Devices:num device
					std::vector<std::string> dat = ofSplitString(line, ":");
					if (dat.size() < 2) {
						continue;
					}
					if (dat[0] == "DeviceType") {
						device_type = ofToInt(dat[1]) == 4 ? ProductProfile::DeviceType::LINE4 : ProductProfile::DeviceType::LINE8;
					} else if (dat[0] == "Devices") {
						num_device = static_cast<uint16_t>(ofToInt(dat[1]));
					}
				}
				profiles[i].setDeviceInfo(device_type, num_device);
				file.close();
				buff.clear();
			}
		}
	};
	static bool loadPreviousProductProfileSetting(std::vector<ProductProfile> & profiles) {
		ofxJSONElement json;
		if (json.open(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON))) {
			if (compareProductProfilesAndJson(profiles, json)) {
				for (int i = 0; i < static_cast<int>(profiles.size()); i++) {
					uint16_t start_id = static_cast<uint16_t>(json["Product Setting"][i]["05 Start Product ID"].asInt());
					uint16_t end_id = static_cast<uint16_t>(json["Product Setting"][i]["06 End Product ID"].asInt());
					profiles[i].setGenerationRange(start_id, end_id);
					profiles[i].generate_data = json["Product Setting"][i]["07 Data Generation"].asBool();
					profiles[i].output_8line_bin = json["Product Setting"][i]["08 BIN Output"]["8Line"].asBool();
					profiles[i].output_4line_bin = json["Product Setting"][i]["08 BIN Output"]["4Line"].asBool();
					profiles[i].device_id_format = json["Product Setting"][i]["09 ID Format"].asString() == "HEX" ? ProductProfile::DeviceIdFormat::HEX : ProductProfile::DeviceIdFormat::DEC;
				}
				return true;
			}
		}
		return false;
	}

	static bool compareProductProfilesAndJson(std::vector<ProductProfile> & profiles, ofxJSONElement & json) {
		if (static_cast<int>(profiles.size()) == static_cast<int>(json["Product Setting"].size())) {
			for (int i = 0; i < static_cast<int>(profiles.size()); i++) {
				std::string product_name = profiles[i].product_name;
				std::string json_product_name = json["Product Setting"][i]["01 Product Name"].asString();
				if (product_name != json_product_name) return false;

				uint16_t num_product = profiles[i].num_product;
				uint16_t json_num_product = static_cast<uint16_t>(json["Product Setting"][i]["02 Number of Products"].asInt());
				if (num_product != json_num_product) return false;

				uint16_t num_device = profiles[i].num_device;
				uint16_t json_num_device = static_cast<uint16_t>(json["Product Setting"][i]["03 Number of Devices"].asInt());
				if (num_device != json_num_device) return false;

				ProductProfile::DeviceType device_type = profiles[i].device_type;
				ProductProfile::DeviceType json_device_type = json["Product Setting"][i]["04 Device Type"].asString() == "4Line" ? ProductProfile::DeviceType::LINE4 : ProductProfile::DeviceType::LINE8;
				if (device_type != json_device_type) return false;
			}
			return true;
		}
		return false;
	}
};

#endif
