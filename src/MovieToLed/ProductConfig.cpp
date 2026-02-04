#include "ProductConfig.hpp"

ProductConfig::ProductConfig() {
	panel.setup("Product Config");
	panel.add(label_name.setup("Product", ""));
	is_edit_config.addListener(this, &ProductConfig::isEditConfigChanged);
	panel.add(is_edit_config.set(false));

	create_panel.setup();
}

ProductConfig::~ProductConfig() {
	contents = nullptr;
}

void ProductConfig::setup(std::vector<ProductContent> * contents_ptr) {
	contents = contents_ptr;
	// verifyAllConfig();
}

void ProductConfig::setupRender(int x, int y, int w) {
	panel.setPosition(x, y);
	panel.setWidthElements(w);
	create_panel.setPosition(x, y + panel.getHeight());
	create_panel.setWidthElements(w);
}

void ProductConfig::setProduct(uint8_t idx) {
	if (isValid()) {
		if (!contents->empty() && idx < static_cast<uint8_t>(contents->size())) {
			index = idx;
			name = contents->at(index).product_name;
			device_type = contents->at(index).device_type;
			label_name = name;
			create_panel.clear();
			type_8line.removeListener(this, &ProductConfig::type8LineChanged);
			type_4line.removeListener(this, &ProductConfig::type4LineChanged);
			btn_create.removeListener(this, &ProductConfig::create);
			create_panel.setName(name + ".conf");
			create_panel.add(label_device_type.setup("Which Device Type?", device_type == DeviceType::LINE4 ? "4Line" : "8Line"));
			create_panel.add(type_8line.set("8Line", device_type == DeviceType::LINE8));
			create_panel.add(type_4line.set("4Line", device_type == DeviceType::LINE4));
			create_panel.add(num_device.setup("How Many Devices", contents->at(idx).num_device));
			create_panel.add(btn_create.setup("Update " + name + ".conf"));
			type_8line.addListener(this, &ProductConfig::type8LineChanged);
			type_4line.addListener(this, &ProductConfig::type4LineChanged);
			btn_create.addListener(this, &ProductConfig::create);
			if (found_config[index]) {
				// confがある場合
				is_edit_config = false;
			} else {
				// confがない場合
				is_edit_config = true;
				btn_create.setName("Create " + name + ".conf");
			}
		}
	}
}

void ProductConfig::verify(std::vector<ProductContent> & contents) {
	if (contents.empty()) return;
	found_config.clear();
	for (int i = 0; i < static_cast<int>(contents.size()); i++) {
		found_config.push_back(false);
	}
	ofDirectory dir(MovieToLedUtils::DirPaths::PRODUCT_DIR);
	dir.listDir();
	for (int i = 0; i < static_cast<int>(contents.size()); i++) {
		for (auto & file : dir.getFiles()) {
			if (file.isFile() && file.getFileName() == contents[i].product_name + ".conf") {
				found_config[i] = true;
				break;
			}
		}
	}
	MovieToLedUtils::MtLStates::Preprocess::config_all_exist = isAllExist();
}

bool ProductConfig::isAllExist() {
	if (found_config.empty()) return false;
	for (bool found : found_config) {
		if (!found) return false;
	}
	return true;
}

bool ProductConfig::isExist(uint8_t idx) {
	if (found_config.empty()) return false;
	if (idx < 0 || idx > static_cast<uint8_t>(found_config.size() - 1)) return false;
	return found_config[idx];
}

void ProductConfig::setDeviceInfo(uint8_t idx) {
	if (isValid()) {
		if (!contents->empty() && idx < static_cast<uint8_t>(contents->size())) {
			std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + contents->at(idx).product_name + ".conf";
			ofFile file(file_path);
			if (!file.exists()) {
				MovieToLedUtils::error_msg = file_path + "\r\nFile Not Found.";
				return;
			}
			ofBuffer buff(file);
			uint16_t num_device = 1;
			DeviceType device_type = DeviceType::LINE8;
			for (auto line : buff.getLines()) {
				// ファイルの構造
				// Name:name
				// DeviceType:4 or 8
				// Devices:num device
				std::vector<std::string> dat = ofSplitString(line, ":");
				if (dat[0] == "DeviceType") {
					device_type = ofToInt(dat[1]) == 4 ? DeviceType::LINE4 : DeviceType::LINE8;
				} else if (dat[0] == "Devices") {
					num_device = static_cast<uint16_t>(ofToInt(dat[1]));
				}
			}
			contents->at(idx).setDeviceInfo(device_type, num_device);
			file.close();
			buff.clear();
		}
	}
}

uint16_t ProductConfig::getDeviceNum(ProductContent & content) {
	std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + content.product_name + ".conf";
	ofFile file(file_path);
	if (!file.exists()) {
		MovieToLedUtils::error_msg = file_path + "\r\nFile Not Found.";
		return 0;
	}
	ofBuffer buff(file);
	uint16_t num_device = 0;
	for (auto line : buff.getLines()) {
		std::vector<std::string> dat = ofSplitString(line, ":");
		if (dat[0] == "Devices") {
			num_device = static_cast<uint16_t>(ofToInt(dat[1]));
		}
	}
	file.close();
	buff.clear();
	return num_device;
}

void ProductConfig::create() {
	if (isValid() && !contents->empty()) {
		bool prev_found_config = found_config[index];
		std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + name + ".conf";
		ofFile file(file_path);
		FileUtils::createFile(file_path, false);
		FileUtils::openFile(file, file_path, ofFile::Append, false);		
		std::ostringstream out;
		out << "Name:" << name << "\n";
		out << "DeviceType:" << ofToString(device_type == DeviceType::LINE8 ? 8 : 4) << "\n";
		out << "Devices:" << ofToString(static_cast<uint16_t>(num_device)) << "\n";
		file.write(out.str().c_str(), out.str().size());
		file.close();
		verify(*contents);
		setDeviceInfo(index);
		if (!prev_found_config) setProduct(index);
	}
}

void ProductConfig::draw() {
	panel.draw();
	if (is_edit_config) create_panel.draw();
}

void ProductConfig::isEditConfigChanged(bool & is_edit) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		is_edit = false;
	} else {
		if (isValid()) {
			if (!is_edit && !found_config[index]) is_edit = true;
		}
		is_edit_config.setName(is_edit ? "Edit Conf File? : YES" : "Edit Conf File? : NO");
	}
}

void ProductConfig::type4LineChanged(bool & type_4) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		type_4 = device_type == DeviceType::LINE4;
	} else {
		if (type_4) {
			type_8line = false;
			label_device_type = "4Line";
			device_type = DeviceType::LINE4;
		} else {
			if (!type_8line) type_4 = true;
		}
	}
}

void ProductConfig::type8LineChanged(bool & type_8) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		type_8 = device_type == DeviceType::LINE8;
	} else {
		if (type_8) {
			type_4line = false;
			label_device_type = "8Line";
			device_type = DeviceType::LINE8;
		} else {
			if (!type_4line) type_8 = true;
		}
	}
}

void ProductConfig::verifyAllConfig() {
	ofDirectory dir(MovieToLedUtils::DirPaths::PRODUCT_DIR);
	dir.listDir();
	int count = 0;
	for (auto & file : dir.getFiles()) {
		if (file.isFile()) {
			printf("-------------------------------\r\n");
			printf("File %d : %s\r\n", ++count, file.getFileName().c_str());
			bool no_problem[3] = { false, false, false };
			ofBuffer buff = ofBufferFromFile(file.getAbsolutePath());
			for (auto line : buff.getLines()) {
				std::vector<std::string> dat = ofSplitString(line, ":");
				if (dat.size() >= 2) {
					printf("%s:%s\r\n", dat[0].c_str(), dat[1].c_str());
					if (dat[0] == "Name") no_problem[0] = true;
					if (dat[0] == "DeviceType" && (dat[1] == "4" || dat[1] == "8")) no_problem[1] = true;
					if (dat[0] == "Devices") no_problem[2] = true;
				}
			}
			std::string result = no_problem[0] && no_problem[1] && no_problem[2] ? "Config File is OK\r\n" : "Config File is NG\r\n";
			printf("%s\r\n", result.c_str());
			printf("-------------------------------\r\n");
		}
	}
}
