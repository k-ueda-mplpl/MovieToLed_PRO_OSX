#ifndef MOVEITOLED_HPP
#define MOVEITOLED_HPP

#pragma once

#include "Converter.hpp"
#include "DeviceMap.hpp"
#include "Extractor.hpp"
#include "LedColorCorrection.hpp"
#include "M5LedMetadata.hpp"
#include "MovieToLedData.hpp"
#include "MovieToLedFileUtils.hpp"
#include "MovieToLedRuntimeState.hpp"
#include "MovieToLedUtils.hpp"
#include "ProductProfileLoader.hpp"
#include "ProductProfileManager.hpp"
#include <ofxGui.h>
#include <ofxJSON.h>
#include <stdio.h>

class MovieToLed {
private:
	// プロダクトのLED情報
	MovieToLedData mtl_data;
	LedColorCorrection led_color_correction;
	Extractor extractor { mtl_data, led_color_correction };
	Converter converter { mtl_data };
	int convert_count, max_convert_count;
	ProductProfileManager profile_manager { mtl_data.product_profiles };

	uint8_t curr_product_index, end_product_index;
	
	// DeviceMap
	DeviceMap device_map;
	ofxPanel device_map_panel;
	ofxButton btn_create_device_map;
	void createDeviceMap();
	// M5LED Metadata
	M5LedMetadata m5led_metadata;
	ofxPanel metadata_panel;
	ofxButton btn_clear_metadata;
	void clearMetadata();

	string output_dir_path;

	// UI
	int panel_width;
	ofRectangle drag_area;
	// Data Setting
	ofxPanel data_panel;
	ofParameter<uint8_t> sound_number;
	void soundNumberChanged(uint8_t & num);
	ofxLabel label_m5led_loop_playback;
	ofParameter<bool> play_once, play_loop;
	void playOnceChanged(bool & is_once);
	void playLoopChanged(bool & is_loop);

	string window_mode;
	int window_width, window_height;

	void loadProject();
	void onDroppedVideo(const ofFile file);
	// callback
	void onExtracted();

public:
	MovieToLed();
	~MovieToLed();
	void setup();
	void setupRender(int x, int y, int width, int height, int gui_width);
	
	void dragEvent(ofDragInfo info);

	void start();
	void update();
	void draw();
	
	void setWindowSize(int w, int h);
	void switchWindowSize();

	void saveParameter();
	void saveProductSetting();
	void exit();
};

#endif /* MovieToLed_hpp */
