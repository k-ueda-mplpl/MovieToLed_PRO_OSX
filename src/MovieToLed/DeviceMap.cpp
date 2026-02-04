#include "DeviceMap.hpp"

DeviceMap::DeviceMap() {
}

DeviceMap::~DeviceMap() {
}

void DeviceMap::setOutputDir(std::string path) {
	output_dir_path = path;
}

void DeviceMap::create(std::string product_name, uint16_t num_product, uint16_t num_device, DeviceType device_type, DeviceIdFormat id_format) {
	// DeviceMap.mdの削除
	std::string dir_path = output_dir_path + "/BIN/" + product_name;
	ofDirectory file_list(dir_path);
	file_list.listDir();
	for (auto file : file_list) {
		if (file.getFileName().find(MovieToLedUtils::FilePaths::DEVICE_MAP_FILE) != std::string::npos) {
			file.remove();
		}
	}
	// DeviceMap.mdを作成
	std::string file_path = dir_path + "/" + MovieToLedUtils::FilePaths::DEVICE_MAP_FILE;
	FileUtils::createFile(file_path, false);
	ofFile file;
	FileUtils::openFile(file, file_path, ofFile::Append, false);
	uint16_t block_size = Converter::getBlockSize(num_device, id_format);
	uint16_t max_product_count = Converter::getMaxProductCount(num_device, id_format);

	std::ostringstream out;
	out << "# Device ID Map\n";
	out << "- Product: " << product_name << "\n";
	out << "- Total Products: " << ofToString(num_product) << "\n";
	out << "- Devices per Product: " << ofToString(num_device) << "\n";
	out << "- Total Devices: " << ofToString(num_product * num_device) << "\n";
	if (device_type == DeviceType::LINE4) {
		out << "- Device Type: 4 Line Device (incl. POLE and RIBBON Device)\n";
	} else if (device_type == DeviceType::LINE8) {
		out << "- Device Type: 8 Line Device (incl. POMPOM Device(PB))\n";
	}
	if (id_format == DeviceIdFormat::HEX) {
		out << "- ID Format: HEX(16)\n";
	} else if (id_format == DeviceIdFormat::DEC) {
		out << "- ID Format: DEC(10)\n";
	}
	char date[20];
	snprintf(date, 20, "%04d-%02d-%02d %02d:%02d:%02d", ofGetYear(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
	out << "- Generated: " << date << "\n";
	out << "\n";

	out << "## Summary\n";
	char dir_name[13];
	char devices[768];
	char col[1024];
	if (max_product_count > 0) {
		// 1プロダクトがBINの範囲に収まるとき
		for (uint16_t product_id = 0; product_id < num_product; product_id++) {
			if (product_id == 0) {
				// Header
				if (device_type == DeviceType::LINE4) {
					out << "| Product | BIN Directory       | Device ID";
				} else if (device_type == DeviceType::LINE8) {
					out << "| Product | BIN Directory | Device ID";
				}
				if (num_device > 3) {
					unsigned int count = 3;
					while (count < num_device) {
						out << "   ";
						count++;
					}
				}
				out << "|\n";
			}
			// ディレクトリ名
			uint16_t head_id = (product_id / max_product_count) * max_product_count;
			if (id_format == DeviceIdFormat::DEC) {
				snprintf(dir_name, 12, "%03d-%03d-DEC", head_id, head_id + max_product_count - 1);
			} else if (id_format == DeviceIdFormat::HEX) {
				snprintf(dir_name, 12, "%03X-%03X-HEX", head_id, head_id + max_product_count - 1);
			}
			// BINに対応したデバイスID
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				if (num_device == 1) {
					if (id_format == DeviceIdFormat::HEX) {
						snprintf(devices, 10, "%02X       ", product_id % max_product_count);
					} else if (id_format == DeviceIdFormat::DEC) {
						snprintf(devices, 10, "%02d       ", product_id % max_product_count);
					}
				} else {
					uint8_t bin_id = (product_id % max_product_count) * block_size + device_id;
					if (id_format == DeviceIdFormat::HEX) {
						snprintf(&devices[device_id * 3], 4, "%02X ", bin_id);
					} else if (id_format == DeviceIdFormat::DEC) {
						snprintf(&devices[device_id * 3], 4, "%02d ", bin_id);
					}
					if (num_device == 2) {
						snprintf(&devices[6], 4, "   ");
					}
				}
			}
			if (device_type == DeviceType::LINE4) {
				snprintf(col, 1024, "| %03d     | LINE0-3/%s | %s|", product_id, dir_name, devices);
			} else if (device_type == DeviceType::LINE8) {
				snprintf(col, 1024, "| %03d     | %s   | %s|", product_id, dir_name, devices);
			}
			out << col << "\n";
		}
		file.write(out.str().c_str(), out.str().size());
		file.close();
	} else {
		// 1プロダクトがBINの範囲を超えるとき
		for (uint16_t product_id = 0; product_id < num_product; product_id++) {
			if (product_id == 0) {
				// Header
				if (device_type == DeviceType::LINE4) {
					out << "| Product | BIN Directory       | Device ID";
				} else if (device_type == DeviceType::LINE8) {
					out << "| Product | BIN Directory | Device ID";
				}
				out << "|\n";
			}
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				// ディレクトリ名
				if (id_format == DeviceIdFormat::HEX && (device_id & 0xff) == 0) {
					snprintf(dir_name, 12, "%03X-%03X-HEX", product_id, device_id);
				} else if (id_format == DeviceIdFormat::DEC && device_id % 100 == 0) {
					snprintf(dir_name, 12, "%03d-%03d-DEC", product_id, device_id);
				}
				// BINに対応したデバイスID
				uint16_t max_device_count = id_format == DeviceIdFormat::HEX ? 256 : 100;
				if (id_format == DeviceIdFormat::HEX) {
					snprintf(&devices[(device_id % max_device_count) * 3], 4, "%02X ", device_id);
				} else if (id_format == DeviceIdFormat::DEC) {
					snprintf(&devices[(device_id % max_device_count) * 3], 4, "%02d ", device_id);
				}
				if (device_id % max_device_count == max_device_count - 1) {
					if (device_type == DeviceType::LINE4) {
						snprintf(col, 1024, "| %03d     | LINE0-3/%s | %s|", product_id, dir_name, devices);
					} else if (device_type == DeviceType::LINE8) {
						snprintf(col, 1024, "| %03d     | %s   | %s|", product_id, dir_name, devices);
					}
					out << col << "\n";
				}
			}
		}
		file.write(out.str().c_str(), out.str().size());
		file.close();
	}
}

void DeviceMap::create(ProductContent & content) {
	create(content.product_name, content.num_product, content.num_device, content.device_type, content.id_format);
}
