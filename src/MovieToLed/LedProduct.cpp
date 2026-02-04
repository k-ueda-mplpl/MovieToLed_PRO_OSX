#include "LedProduct.hpp"

LedProductManager::LedProductManager() {
	products.clear();
}

LedProductManager::~LedProductManager() {
	content = nullptr;
	products.clear();
}

void LedProductManager::setup(ProductContent * content_ptr) {
	products.clear();
	content = content_ptr;
	for (uint16_t i = content->start_product_id; i <= content->end_product_id; i++) {
		LedProduct product;
		for (uint16_t j = 0; j < content->num_device; j++) {
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

// void LedProductManager::setLed(std::string path, uint16_t start_id, std::string & error_msg) {
// 	if (isValid()) {
// 		ofFile file(path);
// 		if (products.empty()) {
// 			printf("MtL_Contents, Products Empty\r\n");
// 			return;
// 		}
// 		if (!file.exists()) {
// 			printf("Not Found %s\r\n", path.c_str());
// 			error_msg = path + "\r\n File Not Found.";
// 			return;
// 		}
// 		printf("Load %s\r\n", path.c_str());
// 		ofBuffer buff(file);
// 		uint16_t product_id = 0;
// 		uint16_t device_id = 0;
// 		uint8_t led_line_id = 0;
// 		uint8_t num_line = content->device_type == DeviceType::LINE4 ? 4 : 8;
// 		for (auto line : buff.getLines()) {
// 			std::vector<std::string> dat = ofSplitString(line, ",");
// 			if (dat.size() >= 8) {
// 				unsigned int product_count = static_cast<unsigned int>(ofToInt(dat[0]));
// 				unsigned int line_count = static_cast<unsigned int>(ofToInt(dat[1]));
// 				int led_id = ofToInt(dat[2]);
// 				int led_pos_x = ofToInt(dat[3]);
// 				int led_pos_y = ofToInt(dat[4]);
// 				int led_type = ofToInt(dat[7]);
// 				product_id = start_id + product_count;
// 				if (product_id < static_cast<uint16_t>(products.size()) && device_id < static_cast<uint16_t>(products[0].devices.size())) {
// 					device_id = line_count / num_line;
// 					led_line_id = line_count % num_line;
// 					products[product_id].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].x = led_pos_x & 0xffff;
// 					products[product_id].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].y = led_pos_y & 0xffff;
// 					products[product_id].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].type = led_type == 1 ? LedType::PANEL : LedType::NORMAL;
// 				}
// 			}
// 		}
// 	}
// }

void LedProductManager::setLed(std::string path, uint16_t head_id, std::string & error_msg) {
	if (isValid()) {
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
		ofBuffer buff(file);
		uint16_t product_id = 0;
		uint16_t device_id = 0;
		uint8_t led_line_id = 0;
		uint8_t num_line = content->device_type == DeviceType::LINE4 ? 4 : 8;
		for (auto line : buff.getLines()) {
			std::vector<std::string> dat = ofSplitString(line, ",");
			if (dat.size() >= 8) {
				int product_count = ofToInt(dat[0]);
				product_id = head_id + product_count;
				if (product_id > content->end_product_id) break;
				if (product_id >= content->start_product_id) {
					uint16_t product_array_index = product_id - content->start_product_id;
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
						products[product_array_index].devices[device_id].led[led_line_id][led_id % LedProduct::Device::MAX_NUM_LED].type = led_type == 1 ? LedType::PANEL : LedType::NORMAL;
					}
				}
			}
		}
	} else {
	}
}

void LedProductManager::setNumLed() {
	for (LedProduct & product : products) {
		for (LedProduct::Device & device : product.devices) {
			for (int i = 0; i < LedProduct::Device::MAX_NUM_LINE; i++) {
				for (int j = LedProduct::Device::MAX_NUM_LED - 1; j >= 1; j--) {
					if (device.led[i][j].x == SHRT_MIN && device.led[i][j].y == SHRT_MIN) {
						if (device.led[i][j - 1].x != SHRT_MIN || device.led[i][j - 1].y != SHRT_MIN) {
							device.num_led[i] = j;
							break;
						}
					}
					device.num_led[i] = 0;
				}
			}
		}
	}
}

std::string LedProductManager::getName() {
	if (isValid()) return content->product_name;
	return "UNKNOWN";
}

uint16_t LedProductManager::getNumProduct() {
	if (isValid()) return content->num_product;
	return 0;
}

uint16_t LedProductManager::getNumDevice() {
	if (isValid()) return content->num_device;
	return 0;
}

DeviceType LedProductManager::getDeviceType() {
	if (isValid()) return content->device_type;
	return DeviceType::UNKNOWN;
}

uint16_t LedProductManager::getStartProductId() {
	if (isValid()) return content->start_product_id;
	return 0;
}

uint16_t LedProductManager::getEndProductId() {
	if (isValid()) return content->end_product_id;
	return 0;
}

DeviceIdFormat LedProductManager::getDeviceIdFormat(){
	if(isValid()) return content->id_format;
	return DeviceIdFormat::DEC;
}

uint16_t LedProductManager::getNumLed(uint16_t device_id, uint8_t line_id) {
	if (isValid()) {
		if (device_id >= 0 && device_id < content->num_device) {
			if (line_id >= 0 && line_id < LedProduct::Device::MAX_NUM_LINE) {
				return products[0].devices[device_id].num_led[line_id];
			}
		}
	}
	return 0;
}

const Led * LedProductManager::getLed(uint16_t product_id, uint16_t device_id, uint8_t line_id, uint8_t led_id) {
	if (isValid()) {
		if (product_id >= content->start_product_id && product_id <= content->end_product_id) {
			if (device_id >= 0 && device_id < content->num_device) {
				return &products[product_id - content->start_product_id].devices[device_id].led[line_id % LedProduct::Device::MAX_NUM_LINE][led_id % LedProduct::Device::MAX_NUM_LED];
			}
		}
	}
	return &dummy_led;
}

