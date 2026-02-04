#ifndef PRODUCT_CONFIG_HPP
#define PRODUCT_CONFIG_HPP

#pragma once

#include "MovieToLedUtils.hpp"
#include <ofxGui.h>

class ProductConfig {
public:
	ProductConfig();
	~ProductConfig();
	void setup(std::vector<ProductContent> * contents_ptr);
	void setupRender(int x, int y, int w);
	void setProduct(uint8_t idx);
	void verify(std::vector<ProductContent> & contents);
	bool isAllExist();
	bool isExist(uint8_t idx);
	void setDeviceInfo(uint8_t idx);
	uint16_t getDeviceNum(ProductContent & content);
	void create();
	void draw();

private:
	std::vector<ProductContent> * contents = nullptr;
	std::vector<bool> found_config;
	uint8_t index;
	std::string name;
	DeviceType device_type;

	ofxPanel panel;
	ofxLabel label_name;
	ofParameter<bool> is_edit_config;
	
	ofxPanel create_panel;
	ofxLabel label_device_type;
	ofParameter<bool> type_4line;
	ofParameter<bool> type_8line;
	ofxInputField<uint16_t> num_device;
	ofxButton btn_create;
	void isEditConfigChanged(bool & is_edit);
	void type4LineChanged(bool & type_4);
	void type8LineChanged(bool & type_8);

	bool isValid() {
		return contents;
	}

	void verifyAllConfig();
};

#endif
