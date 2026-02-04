//
//  MovieToLed.hpp
//  MovieToLED
//
//  Created by ueda on 2025/03/04.
//

#ifndef MovieToLed_hpp
#define MovieToLed_hpp

#include "Converter.hpp"
#include "DeviceMap.hpp"
#include "Extractor.hpp"
#include "M5LedMetadata.hpp"
#include "ProductConfig.hpp"
#include <ofxGui.h>
#include <ofxJSON.h>
#include <stdio.h>

class MovieToLed : public Extractor, Converter {
private:
	vector<ProductContent> product_contents;
	bool isSameProduct(ProductContent & content, ofxJSONElement json);
	void loadProductSetting();
	void onLoadContents();
	ProductConfig product_config;
	int content_index, end_content_index;
	void updateContentIndex(bool is_init = false);

	// プロダクトのLED情報
	// M5LEDとBINのペア
	MovieToLedUtils::OutputData output_data;
	vector<string> completed_file;
	int convert_count, max_convert_count;
	
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
	// callback
	void onExtracted();

	// UI
	int panel_width;
	ofRectangle drag_area;
	// Data Setting
	ofxPanel data_panel;
	ofParameter<uint8_t> sound_number;
	void soundNumberChanged(uint8_t & sound_num);
	ofxLabel label_m5led_loop_playback;
	ofParameter<bool> play_once, play_loop;
	void playOnceChanged(bool & is_once);
	void playLoopChanged(bool & is_loop);
	// Product Setting
	ofxPanel product_panel;
	ofParameter<uint8_t> product_idx;
	ofxLabel label_product_name;
	ofxLabel label_data_generate;
	ofParameter<bool> data_generate, not_data_generate;
	ofxLabel label_id_format;
	ofParameter<bool> id_format_dec, id_format_hex;
	ofxLabel label_product_id_range;
	ofxButton btn_all_product_id;
	ofParameter<uint16_t> product_id_start, product_id_end;
	void productIdxChanged(uint8_t & idx);
	void dataGenerateChanged(bool & is_generate);
	void notDataGenerateChanged(bool & not_generate);
	void idFormatDecChanged(bool & is_dec);
	void idFormatHexChanged(bool & is_hex);
	void setAllProductId();
	void productIdStartChanged(uint16_t & id_start);
	void productIdEndChanged(uint16_t & id_end);
	// RGB Curve Plot
	ofRectangle plot_area;
	ofPolyline gamma_curve, led_white_curve, led_rgb_curve, panel_white_curve, panel_rgb_curve;
	void updatePlot(ofPolyline & curve, uint8_t gain, uint8_t & max);
	ofxPanel param_panel;
	uint8_t led_white_max, led_rgb_max, panel_white_max, panel_rgb_max;
	ofxSlider<uint8_t> slider_led_white_gain;
	ofxSlider<uint8_t> slider_led_rgb_gain;
	ofxSlider<uint8_t> slider_panel_white_gain;
	ofxSlider<uint8_t> slider_panel_rgb_gain;
	void sliderLedWhiteGainChanged(uint8_t & gain);
	void sliderLedRgbGainChanged(uint8_t & gain);
	void sliderPanelWhiteGainChanged(uint8_t & gain);
	void sliderPanelRgbGainChanged(uint8_t & gain);

	char str[128];

	string window_mode;
	int window_width, window_height;

	ofxPanel bin_panel;
	ofxLabel label_output_8line_bin;
	ofParameter<bool> output_8line_bin, not_output_8line_bin;
	ofxLabel label_output_4line_bin;
	ofParameter<bool> output_4line_bin, not_output_4line_bin;
	void output8LineBinChanged(bool & output_bin);
	void notOutput8LineBinChanged(bool & not_output_bin);
	void output4LineBinChanged(bool & output_bin);
	void notOutput4LineBinChanged(bool & not_output_bin);

	ofxPanel voltage_panel;
	ofxLabel label_voltage;
	ofParameter<bool> use_5V, use_12V;
	void use5VChanged(bool & is_use);
	void use12VChanged(bool & is_use);

public:
	MovieToLed();
	~MovieToLed();
	void setup();
	void setupRender(int x, int y, int width, int height, int gui_width);
	bool load();

	void onDroppedVideo(const ofFile file);
	void dragEvent(ofDragInfo info);

	void setWindowSize(int w, int h);
	void switchWindowSize();

	void saveParameter();
	void saveProductSetting();
	void exit();

	void start();
	void draw();
	void update();
};

#endif /* MovieToLed_hpp */

