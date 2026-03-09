#ifndef PRODUCT_PROFILE_HPP
#define PRODUCT_PROFILE_HPP

#include <cstdint>
#include <string>

// バイナリデータ作成に必要なプロダクトの基本情報
struct ProductProfile {
	enum DeviceType {
		LINE4,
		LINE8,
		LINE8_1000FPS,
		UNKNOWN
	};

	enum DeviceIdFormat {
		DEC,
		HEX
	};

	std::string product_name;
	uint16_t num_product, num_device;
	uint16_t start_product_id, end_product_id;
	DeviceType device_type;
	DeviceIdFormat device_id_format;
	bool generate_data;
	bool output_8line_bin;
	bool output_4line_bin;

	ProductProfile() {
		product_name.clear();
		num_product = 1;
		num_device = 1;
		start_product_id = 0;
		end_product_id = num_product - 1;
		device_type = DeviceType::LINE8;
		device_id_format = DeviceIdFormat::DEC;
		generate_data = true;
		output_8line_bin = true;
		output_4line_bin = false;
	};

	void setProductInfo(std::string name, uint16_t num) {
		product_name = name;
		num_product = num;
		start_product_id = 0;
		end_product_id = num_product - 1;
	}

	void setDeviceInfo(DeviceType type, uint16_t num) {
		device_type = type;
		num_device = num;
		// デバイス数が10台以上16台以下なら自動でHEXに
		// 1プロダクトのデバイスが収まるから
		device_id_format = (num_device > 10 && num_device <= 16) ? DeviceIdFormat::HEX : DeviceIdFormat::DEC;
		if (device_type == DeviceType::LINE8) {
			output_8line_bin = true;
			output_4line_bin = false;
		} else if (device_type == DeviceType::LINE4) {
			output_8line_bin = false;
			output_4line_bin = true;
		}
	}

	bool setGenerationRange(uint16_t from, uint16_t to) {
		if (from >= num_product || to >= num_product || from > to) return false;
		start_product_id = from;
		end_product_id = to;
		return true;
	}

	bool isGenerateDate() {
		return generate_data;
	}

	bool isOutputBin() {
		return output_4line_bin || output_8line_bin;
	}

	bool isOutput4lineBin() {
		return output_4line_bin;
	}

	bool isOutput8lineBin() {
		return output_8line_bin;
	}
};

#endif