#ifndef PRODUCT_PROFILE_MANAGER_HPP
#define PRODUCT_PROFILE_MANAGER_HPP

#pragma once

#include "ProductProfile.hpp"
#include <cstdint>
#include <ofxGui.h>
#include <vector>

class ProductProfileManager {
public:
	ProductProfileManager(std::vector<ProductProfile> & profiles_ref);
	void setupRender(int x, int y, int w);
	void verify();
	bool isAllExist();
	bool isExist(uint8_t index);
	void selectProfile(uint16_t index);
	void create();
	void draw();

	uint8_t getProfileIndex(){
		return profile_index - 1;
	}

	void saveProductSetting();

	int getBottomY(){
		return setting_panel.getPosition().y + setting_panel.getHeight();
	}

private:
	std::vector<ProductProfile> & product_profiles;
	std::vector<bool> found_config;

	std::string product_name;
	void updateSettingPanel(ProductProfile & profile);
	void updateEditPanel(ProductProfile & profile);

	// for Data Setting
	ofxPanel setting_panel;
	ofParameter<uint8_t> profile_index;
	ofxLabel label_product_name_a;
	ofxLabel label_data_generate;
	ofParameter<bool> generate_data, not_generate_data;
	ofxLabel label_product_id_range;
	ofxButton btn_all_product_id;
	ofParameter<uint16_t> start_id, end_id;
	ofxLabel label_output_8line_bin;
	ofParameter<bool> output_8line_bin, not_output_8line_bin;
	ofxLabel label_output_4line_bin;
	ofParameter<bool> output_4line_bin, not_output_4line_bin;
	ofxLabel label_id_format;
	ofParameter<bool> id_format_dec, id_format_hex;

	void profileIndexChanged(uint8_t & index);
	void generateDataChanged(bool & generate);
	void notGenerateDataChanged(bool & not_generate);
	void startIdChanged(uint16_t & id);
	void endIdChanged(uint16_t & id);
	void setAllProductId();
	void output8LineBinChanged(bool & output_bin);
	void notOutput8LineBinChanged(bool & not_output_bin);
	void output4LineBinChanged(bool & output_bin);
	void notOutput4LineBinChanged(bool & not_output_bin);
	void idFormatDecChanged(bool & dec);
	void idFormatHexChanged(bool & hex);

	// for Edit
	ofxPanel edit_panel;
	ofxLabel label_product_name_b;
	ofParameter<bool> is_edit;
	void isEditChanged(bool & edit);

	ProductProfile::DeviceType device_type;
	ofxPanel create_panel;
	ofxLabel label_device_type;
	ofParameter<bool> device_type_4line;
	ofParameter<bool> device_type_8line;
	ofxInputField<uint16_t> input_num_device;
	ofxButton btn_create;
	void deviceType4LineChanged(bool & type_4);
	void deviceType8LineChanged(bool & type_8);
	void inputNumDeviceChanged(uint16_t & num);

	void verifyAllConfig();
};

#endif