#ifndef LED_COLOR_CORRECTION_HPP
#define LED_COLOR_CORRECTION_HPP

#include <cstdint>
#include <ofxGui.h>

struct LedGain {
	uint8_t led_rgb = 70;
	uint8_t led_white = 70;
	uint8_t panel_rgb = 70;
	uint8_t panel_white = 70;
};

struct LedMax {
	uint8_t led_rgb = 250;
	uint8_t led_white = 250;
	uint8_t panel_rgb = 250;
	uint8_t panel_white = 250;
};

class LedColorCorrection {
public:
	LedColorCorrection();
	
	void setLedGain(LedGain gain);
	LedGain getLedGain();

	void setupRender(int x, int y, int panel_width);
	void draw();
	
	void applyLedColorCorrection(uint8_t (&src)[3]);
	void applyPanelColorCorrection(uint8_t (&src)[3]);

private:
	LedGain led_gain;
	LedMax led_max;

	inline static constexpr float GAMMA = 2.2; //gammma
	uint8_t gamma_table[256];

	ofRectangle plot_area;
	ofPolyline gamma_curve, led_white_curve, led_rgb_curve, panel_white_curve, panel_rgb_curve;
	void updatePlot(ofPolyline & curve, uint8_t gain, uint8_t & max);

	ofxPanel param_panel;
	ofxSlider<uint8_t> slider_led_white_gain;
	ofxSlider<uint8_t> slider_led_rgb_gain;
	ofxSlider<uint8_t> slider_panel_white_gain;
	ofxSlider<uint8_t> slider_panel_rgb_gain;
	void sliderLedWhiteGainChanged(uint8_t & gain);
	void sliderLedRgbGainChanged(uint8_t & gain);
	void sliderPanelWhiteGainChanged(uint8_t & gain);
	void sliderPanelRgbGainChanged(uint8_t & gain);

	int render_x, render_y;
};

#endif