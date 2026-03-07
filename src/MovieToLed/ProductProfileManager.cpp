#include "ProductProfileManager.hpp"
#include "MovieToLedFileUtils.hpp"
#include "MovieToLedRuntimeState.hpp"
#include "MovieToLedUtils.hpp"
#include <ofxJSON.h>

ProductProfileManager::ProductProfileManager(std::vector<ProductProfile> & profiles_ref)
	: product_profiles(profiles_ref) {
	setting_panel.setup("Product Profile");
	setting_panel.add(profile_index.set("Select Product", 1, 1, 1));
	setting_panel.add(label_product_name_a.setup("Product", ""));

	setting_panel.add(label_data_generate.setup("Generate LED Data?", "YES"));
	setting_panel.add(generate_data.set("YES", true));
	setting_panel.add(not_generate_data.set("NO", false));

	setting_panel.add(label_product_id_range.setup("Product ID Range", "0-0"));
	setting_panel.add(start_id.set("Start Product ID", 0, 0, 0));
	setting_panel.add(end_id.set("End Product ID", 0, 0, 0));
	setting_panel.add(btn_all_product_id.setup("All Product ID"));

	setting_panel.add(label_output_8line_bin.setup("Output 8Line BIN?", "YES"));
	setting_panel.add(output_8line_bin.set("YES", true));
	setting_panel.add(not_output_8line_bin.set("NO", false));
	setting_panel.add(label_output_4line_bin.setup("Output 4Line BIN?", "NO"));
	setting_panel.add(output_4line_bin.set("YES", false));
	setting_panel.add(not_output_4line_bin.set("NO", true));

	setting_panel.add(label_id_format.setup("ID Format", "DEC (00-99)"));
	setting_panel.add(id_format_dec.set("DEC (00-99)", true));
	setting_panel.add(id_format_hex.set("HEX (00-FF)", false));

	edit_panel.setup("Device Profile");
	edit_panel.add(label_product_name_b.setup("Product", ""));
	edit_panel.add(is_edit.set(false));
	is_edit.addListener(this, &ProductProfileManager::isEditChanged);

	create_panel.setup("Device Profile Editor");
	create_panel.add(label_device_type.setup("Which Device Type?", "8Line"));
	create_panel.add(device_type_8line.set("8Line", true));
	create_panel.add(device_type_4line.set("4Line", false));
	create_panel.add(num_device.setup("How Many Devices", 1));
	create_panel.add(btn_create.setup("Enter"));
}

void ProductProfileManager::setupRender(int x, int y, int w) {
	setting_panel.setPosition(x, y);
	setting_panel.setWidthElements(w);
	edit_panel.setPosition(x + w, y);
	edit_panel.setWidthElements(w);
	create_panel.setPosition(x + w, y + edit_panel.getHeight());
	create_panel.setWidthElements(w);
}

// void ProductConfig::setProduct(uint8_t idx) {
// 	if (isValid()) {
// 		if (!contents->empty() && idx < static_cast<uint8_t>(contents->size())) {
// 			index = idx;
// 			name = contents->at(index).product_name;
// 			device_type = contents->at(index).device_type;
// 			label_name = name;
// 			create_panel.clear();
// 			type_8line.removeListener(this, &ProductConfig::type8LineChanged);
// 			type_4line.removeListener(this, &ProductConfig::type4LineChanged);
// 			btn_create.removeListener(this, &ProductConfig::create);
// 			create_panel.setName(name + ".conf");
// 			create_panel.add(label_device_type.setup("Which Device Type?", device_type == DeviceType::LINE4 ? "4Line" : "8Line"));
// 			create_panel.add(type_8line.set("8Line", device_type == DeviceType::LINE8));
// 			create_panel.add(type_4line.set("4Line", device_type == DeviceType::LINE4));
// 			create_panel.add(num_device.setup("How Many Devices", contents->at(idx).num_device));
// 			create_panel.add(btn_create.setup("Update " + name + ".conf"));
// 			type_8line.addListener(this, &ProductConfig::type8LineChanged);
// 			type_4line.addListener(this, &ProductConfig::type4LineChanged);
// 			btn_create.addListener(this, &ProductConfig::create);
// 			if (found_config[index]) {
// 				// confがある場合
// 				is_edit_config = false;
// 			} else {
// 				// confがない場合
// 				is_edit_config = true;
// 				btn_create.setName("Create " + name + ".conf");
// 			}
// 		}
// 	}
// }

void ProductProfileManager::verify() {
	if (product_profiles.empty()) return;
	found_config.clear();
	for (int i = 0; i < static_cast<int>(product_profiles.size()); i++) {
		found_config.push_back(false);
	}
	ofDirectory dir(MovieToLedUtils::DirPaths::PRODUCT_DIR);
	dir.listDir();
	for (int i = 0; i < static_cast<int>(product_profiles.size()); i++) {
		for (auto & file : dir.getFiles()) {
			if (file.isFile() && file.getFileName() == product_profiles[i].product_name + ".conf") {
				found_config[i] = true;
				break;
			}
		}
	}
	MovieToLedRuntimeState::config_all_exist = isAllExist();
}

bool ProductProfileManager::isAllExist() {
	if (found_config.empty()) return false;
	for (bool found : found_config) {
		if (!found) return false;
	}
	return true;
}

bool ProductProfileManager::isExist(uint8_t index) {
	if (found_config.empty()) return false;
	if (index < static_cast<uint8_t>(found_config.size())) {
		return found_config[index];
	}
	return false;
}

void ProductProfileManager::selectProfile(uint16_t index) {
	if (index < static_cast<uint8_t>(product_profiles.size())) {
		profile_index.removeListener(this, &ProductProfileManager::profileIndexChanged);
		profile_index = index + 1;
		profile_index.setMax(static_cast<uint8_t>(product_profiles.size()));
		product_name = product_profiles[index].product_name;
		label_product_name_a = product_name;
		label_product_name_b = product_name;
		updateSettingPanel(product_profiles[index]);
		updateEditPanel(product_profiles[index]);
		if (found_config[index]) {
			is_edit = false;
		} else {
			is_edit = true;
			// btn_create.setName("Create " + product_name + ".conf");
		}
		profile_index.addListener(this, &ProductProfileManager::profileIndexChanged);
	}
}

void ProductProfileManager::create() {
	if (product_profiles.empty()) return;
	uint16_t index = profile_index - 1;
	std::string product_name = product_profiles[index].product_name;
	ProductProfile::DeviceType device_type = product_profiles[index].device_type;
	uint16_t num_device = product_profiles[index].num_device;

	std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + product_name + ".conf";
	ofFile file(file_path);
	MovieToLedFileUtils::createFile(file_path, false);
	MovieToLedFileUtils::openFile(file, file_path, ofFile::Append, false);
	std::ostringstream out;
	out << "Name:" << product_name << "\n";
	out << "DeviceType:" << ofToString(device_type == ProductProfile::DeviceType::LINE8 ? 8 : 4) << "\n";
	out << "Devices:" << ofToString(num_device) << "\n";
	file.write(out.str().c_str(), out.str().size());
	file.close();
	verify();
}

void ProductProfileManager::draw() {
	setting_panel.draw();
	edit_panel.draw();
	if (is_edit) {
		create_panel.draw();
	}
}

void ProductProfileManager::saveProductSetting() {
	ofxJSONElement json;
	json.open(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON));
	json.clear();
	for (uint8_t i = 0; i < static_cast<uint8_t>(product_profiles.size()); i++) {
		ofxJSONElement product;
		ProductProfile & profile = product_profiles[i];
		product["01 Product Name"] = profile.product_name;
		product["02 Number of Products"] = profile.num_product;
		product["03 Number of Devices"] = profile.num_device;
		product["04 Device Type"] = profile.device_type == ProductProfile::DeviceType::LINE4 ? "4Line" : "8Line";
		product["05 Start Product ID"] = profile.start_product_id;
		product["06 End Product ID"] = profile.end_product_id;
		product["07 Data Generation"] = profile.generate_data;
		product["08 BIN Output"]["4Line"] = profile.output_4line_bin;
		product["08 BIN Output"]["8Line"] = profile.output_8line_bin;
		product["09 ID Format"] = profile.device_id_format == ProductProfile::DeviceIdFormat::HEX ? "HEX" : "DEC";
		json["Product Setting"].append(product);
	}
	json.save(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON), true);
}

// uint16_t ProductConfig::getDeviceNum(ProductContent & content) {
// 	std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + content.product_name + ".conf";
// 	ofFile file(file_path);
// 	if (!file.exists()) {
// 		MovieToLedRuntimeState::error_msg = file_path + "\r\nFile Not Found.";
// 		return 0;
// 	}
// 	ofBuffer buff(file);
// 	uint16_t num_device = 0;
// 	for (auto line : buff.getLines()) {
// 		std::vector<std::string> dat = ofSplitString(line, ":");
// 		if (dat[0] == "Devices") {
// 			num_device = static_cast<uint16_t>(ofToInt(dat[1]));
// 		}
// 	}
// 	file.close();
// 	buff.clear();
// 	return num_device;
// }

// void ProductProfileManager::create() {
// 	if (isValid() && !contents->empty()) {
// 		bool prev_found_config = found_config[index];
// 		std::string file_path = MovieToLedUtils::DirPaths::PRODUCT_DIR + "/" + name + ".conf";
// 		ofFile file(file_path);
// 		MovieToLedFileUtils::createFile(file_path, false);
// 		MovieToLedFileUtils::openFile(file, file_path, ofFile::Append, false);
// 		std::ostringstream out;
// 		out << "Name:" << name << "\n";
// 		out << "DeviceType:" << ofToString(device_type == DeviceType::LINE8 ? 8 : 4) << "\n";
// 		out << "Devices:" << ofToString(static_cast<uint16_t>(num_device)) << "\n";
// 		file.write(out.str().c_str(), out.str().size());
// 		file.close();
// 		verify(*contents);
// 		setDeviceInfo(index);
// 		if (!prev_found_config) setProduct(index);
// 	}
// }

// void ProductProfileManager::draw() {
// 	panel.draw();
// 	if (is_edit_config) create_panel.draw();
// }

void ProductProfileManager::updateSettingPanel(ProductProfile & profile) {
	generate_data.removeListener(this, &ProductProfileManager::generateDataChanged);
	not_generate_data.removeListener(this, &ProductProfileManager::notGenerateDataChanged);

	start_id.removeListener(this, &ProductProfileManager::startIdChanged);
	end_id.removeListener(this, &ProductProfileManager::endIdChanged);
	btn_all_product_id.removeListener(this, &ProductProfileManager::setAllProductId);

	output_8line_bin.removeListener(this, &ProductProfileManager::output8LineBinChanged);
	not_output_8line_bin.removeListener(this, &ProductProfileManager::notOutput8LineBinChanged);
	output_4line_bin.removeListener(this, &ProductProfileManager::output4LineBinChanged);
	not_output_4line_bin.removeListener(this, &ProductProfileManager::notOutput4LineBinChanged);

	id_format_dec.removeListener(this, &ProductProfileManager::idFormatDecChanged);
	id_format_hex.removeListener(this, &ProductProfileManager::idFormatHexChanged);

	if (profile.generate_data) {
		generate_data = true;
		not_generate_data = false;
	} else {
		not_generate_data = true;
		generate_data = false;
	}
	label_data_generate = generate_data ? "YES" : "NO";

	start_id.setMax(profile.num_product - 1);
	start_id = profile.start_product_id;
	end_id.setMax(profile.num_product - 1);
	end_id = profile.end_product_id;
	label_product_id_range = ofToString(start_id) + "-" + ofToString(end_id);

	if (profile.isOutput8lineBin()) {
		output_8line_bin = true;
		not_output_8line_bin = false;
	} else {
		not_output_8line_bin = true;
		output_8line_bin = false;
	}
	if (profile.isOutput4lineBin()) {
		output_4line_bin = true;
		not_output_4line_bin = false;
	} else {
		not_output_4line_bin = true;
		output_4line_bin = false;
	}

	label_output_8line_bin = output_8line_bin ? "YES" : "NO";
	label_output_4line_bin = output_4line_bin ? "YES" : "NO";

	id_format_dec = profile.device_id_format == ProductProfile::DeviceIdFormat::DEC;
	id_format_hex = profile.device_id_format == ProductProfile::DeviceIdFormat::HEX;
	label_id_format = id_format_dec ? "DEC (00-99)" : "HEX (00-FF)";

	generate_data.addListener(this, &ProductProfileManager::generateDataChanged);
	not_generate_data.addListener(this, &ProductProfileManager::notGenerateDataChanged);

	start_id.addListener(this, &ProductProfileManager::startIdChanged);
	end_id.addListener(this, &ProductProfileManager::endIdChanged);
	btn_all_product_id.addListener(this, &ProductProfileManager::setAllProductId);

	output_8line_bin.addListener(this, &ProductProfileManager::output8LineBinChanged);
	output_4line_bin.addListener(this, &ProductProfileManager::output4LineBinChanged);

	id_format_dec.addListener(this, &ProductProfileManager::idFormatDecChanged);
	not_output_8line_bin.addListener(this, &ProductProfileManager::notOutput8LineBinChanged);
	id_format_hex.addListener(this, &ProductProfileManager::idFormatHexChanged);
	not_output_4line_bin.addListener(this, &ProductProfileManager::notOutput4LineBinChanged);
}

void ProductProfileManager::updateEditPanel(ProductProfile & profile) {
	bool edit = is_edit;
	is_edit.setName(edit ? "Edit Device Profile?:YES" : "Edit Device Profile?:NO");
	device_type_8line.removeListener(this, &ProductProfileManager::deviceType8LineChanged);
	device_type_4line.removeListener(this, &ProductProfileManager::deviceType4LineChanged);

	btn_create.removeListener(this, &ProductProfileManager::create);

	// create_panel.setName(product_name + ".conf");

	device_type_8line = profile.device_type == ProductProfile::DeviceType::LINE8;
	device_type_4line = profile.device_type == ProductProfile::DeviceType::LINE4;
	label_device_type = device_type_4line ? "4Line" : "8Line";

	num_device = profile.num_device;

	// btn_create.setName("Update " + product_name + ".conf");

	device_type_8line.addListener(this, &ProductProfileManager::deviceType8LineChanged);
	device_type_4line.addListener(this, &ProductProfileManager::deviceType4LineChanged);

	btn_create.addListener(this, &ProductProfileManager::create);
}

void ProductProfileManager::profileIndexChanged(uint8_t & index) {
	if (!product_profiles.empty()) {
		if (index >= 1 && index <= static_cast<uint8_t>(product_profiles.size())) {
			product_name = product_profiles[index - 1].product_name;
			label_product_name_a = product_name;
			label_product_name_b = product_name;
			updateSettingPanel(product_profiles[index - 1]);
			updateEditPanel(product_profiles[index - 1]);
			if (found_config[index - 1]) {
				is_edit = false;
			} else {
				is_edit = true;
				// btn_create.setName("Create " + product_name + ".conf");
			}
		}
	}
}

void ProductProfileManager::generateDataChanged(bool & generate) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (generate) {
				not_generate_data = false;
				product_profiles[index].generate_data = true;
				label_data_generate = "YES";
			} else {
				if (!not_generate_data) generate = true;
			}
		}
	}
}

void ProductProfileManager::notGenerateDataChanged(bool & not_generate) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (not_generate) {
				generate_data = false;
				product_profiles[index].generate_data = false;
				label_data_generate = "NO";
			} else {
				if (!generate_data) not_generate = true;
			}
		}
	}
}

void ProductProfileManager::startIdChanged(uint16_t & id) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (product_profiles[index].setGenerationRange(id, end_id)) {
				label_product_id_range = ofToString(id) + "-" + ofToString(end_id);
			} else {
				if (id >= product_profiles[index].num_product) id = 0;
				if (id > end_id) end_id = id;
			}
		}
		// if (not_generate_data) generate_data = true;
	}
}

void ProductProfileManager::endIdChanged(uint16_t & id) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (product_profiles[index].setGenerationRange(start_id, id)) {
				label_product_id_range = ofToString(start_id) + "-" + ofToString(id);
			} else {
				if (id >= product_profiles[index].num_product) id = 0;
				if (id < start_id) start_id = id;
			}
		}
		// if (not_generate_data) generate_data = true;
	}
}

void ProductProfileManager::setAllProductId() {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			start_id = 0;
			end_id = product_profiles[index].num_product - 1;
		}
		// if (not_generate_data) generate_data = true;
	}
}

void ProductProfileManager::output8LineBinChanged(bool & output_bin) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (output_bin) {
				not_output_8line_bin = false;
				product_profiles[index].output_8line_bin = true;
				label_output_8line_bin = "YES";
			} else {
				if (!not_output_8line_bin) output_bin = true;
			}
		}
	}
}

void ProductProfileManager::notOutput8LineBinChanged(bool & not_output_bin) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (not_output_bin) {
				output_8line_bin = false;
				product_profiles[index].output_8line_bin = false;
				label_output_8line_bin = "NO";
			} else {
				if (!output_8line_bin) not_output_bin = true;
			}
		}
	}
}

void ProductProfileManager::output4LineBinChanged(bool & output_bin) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (output_bin) {
				not_output_4line_bin = false;
				product_profiles[index].output_4line_bin = true;
				label_output_4line_bin = "YES";
			} else {
				if (!not_output_4line_bin) output_bin = true;
			}
		}
	}
}

void ProductProfileManager::notOutput4LineBinChanged(bool & not_output_bin) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (not_output_bin) {
				output_4line_bin = false;
				product_profiles[index].output_4line_bin = false;
				label_output_4line_bin = "NO";
			} else {
				if (!output_4line_bin) not_output_bin = true;
			}
		}
	}
}

void ProductProfileManager::idFormatDecChanged(bool & dec) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (dec) {
				id_format_hex = false;
				product_profiles[index].device_id_format = ProductProfile::DeviceIdFormat::DEC;
				label_id_format = "DEC (00-99)";
			} else {
				if (!id_format_hex) dec = true;
			}
		}
	}
}

void ProductProfileManager::idFormatHexChanged(bool & hex) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (hex) {
				id_format_dec = false;
				product_profiles[index].device_id_format = ProductProfile::DeviceIdFormat::HEX;
				label_id_format = "HEX (00-FF)";
			} else {
				if (!id_format_dec) hex = true;
			}
		}
	}
}

void ProductProfileManager::isEditChanged(bool & edit) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (!edit && !found_config[index]) edit = true;
			is_edit.setName(edit ? "Edit Device Profile?:YES" : "Edit Device Profile?:NO");
		}
	}
}

void ProductProfileManager::deviceType4LineChanged(bool & type_4) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (type_4) {
				device_type_8line = false;
				label_device_type = "4Line";
				product_profiles[index].device_type = ProductProfile::DeviceType::LINE4;
			} else {
				if (!device_type_8line) type_4 = true;
			}
		}
	}
}

void ProductProfileManager::deviceType8LineChanged(bool & type_8) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		uint8_t index = profile_index - 1;
		if (index < static_cast<uint8_t>(product_profiles.size())) {
			if (type_8) {
				device_type_4line = false;
				label_device_type = "8Line";
				product_profiles[index].device_type = ProductProfile::DeviceType::LINE8;
			} else {
				if (!device_type_4line) type_8 = true;
			}
		}
	}
}

void ProductProfileManager::verifyAllConfig() {
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
