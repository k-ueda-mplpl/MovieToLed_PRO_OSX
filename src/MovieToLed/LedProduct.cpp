#include "LedProduct.hpp"
#include <ofFileUtils.h>
#include <ofUtils.h>

void LedProductManager::selectProfile(uint8_t index) {
	if (product_profiles.empty()) return;
	if (index >= static_cast<uint8_t>(product_profiles.size())) return;
	profile_index = index;
	setupProducts(product_profiles[index]);
}

void LedProductManager::setupProducts(const ProductProfile & profile) {
	products.clear();
	uint16_t start_id = profile.start_product_id;
	uint16_t end_id = profile.end_product_id;
	uint16_t num_device = profile.num_device;
	for (uint16_t i = start_id; i <= end_id; i++) {
		LedProduct product;
		for (uint16_t j = 0; j < num_device; j++) {
			LedProduct::Device device;
			device.initLed();
			for (int k = 0; k < LedProduct::Device::MAX_NUM_LINE; k++) {
				device.num_led[k] = 0;
			}
			product.devices.push_back(device);
		}
		products.push_back(product);
	}
}

void LedProductManager::setLed(std::string path, uint16_t head_id, std::string & error_msg) {
	ofFile file(path);
	if (products.empty()) {
		printf("MtL_Contents, Products Empty\r\n");
		return;
	}
	if (!file.exists()) {
		printf("Not Found %s\r\n", path.c_str());
		error_msg = path + "\r\n File Not Found.";
		return;
	}
	printf("Load %s\r\n", path.c_str());
	const ProductProfile & profile = product_profiles[profile_index];
	ofBuffer buff(file);
	uint16_t product_id = 0;
	uint16_t device_id = 0;
	uint8_t led_line_id = 0;
	uint8_t num_line = profile.device_type == ProductProfile::DeviceType::LINE4 ? 4 : 8;
	uint16_t start_id = profile.start_product_id;
	uint16_t end_id = profile.end_product_id;
	for (auto line : buff.getLines()) {
		std::vector<std::string> dat = ofSplitString(line, ",");
		if (dat.size() >= 8) {
			int product_count = ofToInt(dat[0]);
			product_id = head_id + product_count;
			if (product_id > end_id) break;
			if (product_id >= start_id) {
				uint16_t product_array_index = product_id - start_id;
				int line_count = ofToInt(dat[1]);
				int led_id = ofToInt(dat[2]);
				int led_pos_x = ofToInt(dat[3]);
				int led_pos_y = ofToInt(dat[4]);
				int led_type = ofToInt(dat[7]);
				if (product_array_index < static_cast<uint16_t>(products.size()) && device_id < static_cast<uint16_t>(products[0].devices.size())) {
					device_id = line_count / num_line;
					led_line_id = line_count % num_line;
					products[product_array_index].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].x = led_pos_x & 0xffff;
					products[product_array_index].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].y = led_pos_y & 0xffff;
					products[product_array_index].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].type = led_type == 1 ? Led::LedType::PANEL : Led::LedType::NORMAL;
				}
			}
		}
	}
}

void LedProductManager::setNumLed() {
	ProductProfile & product_profile = product_profiles[profile_index];
	std::string product_name = product_profile.product_name;
	printf("Count %s LED\r\n", product_name.c_str());
	uint16_t start = product_profile.start_product_id;
	uint16_t end = product_profile.end_product_id;
	for (uint16_t product_id = start; product_id <= end; product_id++) {
		uint16_t product_array_index = product_id - start;
		LedProduct & product = products[product_array_index];
		for (uint16_t device_id = 0; device_id < product_profile.num_device; device_id++) {
			LedProduct::Device & device = product.devices[device_id];
			for (uint8_t line_id = 0; line_id < LedProduct::Device::MAX_NUM_LINE; line_id++) {
				for (uint16_t led_id = LedProduct::Device::MAX_NUM_LED - 1; led_id > 0; led_id--) {
					if (device.led[line_id][led_id].x == SHRT_MIN && device.led[line_id][led_id].y == SHRT_MIN) {
						if (device.led[line_id][led_id - 1].x != SHRT_MIN || device.led[line_id][led_id - 1].y != SHRT_MIN) {
							device.num_led[line_id] = led_id;
							return;
							// break;
						}
					} else {
						device.num_led[line_id] = led_id;
						return;
						// break;
					}
					device.num_led[line_id] = 0;
				}
				// printf("Product ID = %d, Device ID = %d, Line = %d LED Count = %d\r\n", product_id, device_id, line_id, device.num_led[line_id]);
			}
		}
	}
}

std::string LedProductManager::getName() {
	return product_profiles[profile_index].product_name;
}

uint16_t LedProductManager::getNumProduct() {
	return product_profiles[profile_index].num_product;
}

uint16_t LedProductManager::getNumDevice() {
	return product_profiles[profile_index].num_device;
}

ProductProfile::DeviceType LedProductManager::getDeviceType() {
	return product_profiles[profile_index].device_type;
}

uint16_t LedProductManager::getStartProductId() {
	return product_profiles[profile_index].start_product_id;
}

uint16_t LedProductManager::getEndProductId() {
	return product_profiles[profile_index].end_product_id;
}

ProductProfile::DeviceIdFormat LedProductManager::getDeviceIdFormat() {
	return product_profiles[profile_index].device_id_format;
}

bool LedProductManager::isGenerateData() {
	return product_profiles[profile_index].isGenerateDate();
}

bool LedProductManager::isOutputBin() {
	return product_profiles[profile_index].isOutputBin();
}

bool LedProductManager::isOutput4lineBin() {
	return product_profiles[profile_index].isOutput4lineBin();
}

bool LedProductManager::isOutput8lineBin() {
	return product_profiles[profile_index].isOutput8lineBin();
}

uint16_t LedProductManager::getNumLed(uint16_t product_id, uint16_t device_id, uint8_t line_id) {
	uint16_t start_id = product_profiles[profile_index].start_product_id;
	uint16_t end_id = product_profiles[profile_index].end_product_id;
	uint16_t num_device = product_profiles[profile_index].num_device;
	if(product_id >= start_id && product_id <= end_id){
		if(device_id >= 0 && device_id < num_device){
			return products[product_id - start_id].devices[device_id].num_led[line_id % LedProduct::Device::MAX_NUM_LINE];
		}
	}
	return 0;
}

const Led * LedProductManager::getLed(uint16_t product_id, uint16_t device_id, uint8_t line_id, uint8_t led_id) {
	uint16_t start_id = product_profiles[profile_index].start_product_id;
	uint16_t end_id = product_profiles[profile_index].end_product_id;
	uint16_t num_device = product_profiles[profile_index].num_device;
	if (product_id >= start_id && product_id <= end_id) {
		if (device_id >= 0 && device_id < num_device) {
			return &products[product_id - start_id].devices[device_id].led[line_id % LedProduct::Device::MAX_NUM_LINE][led_id % LedProduct::Device::MAX_NUM_LED];
		}
	}
	return &dummy_led;
}

uint8_t LedProductManager::getProfileIndex() {
	return profile_index;
}
