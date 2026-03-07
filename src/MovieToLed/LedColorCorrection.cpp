#include "LedColorCorrection.hpp"
#include "MovieToLedRuntimeState.hpp"
#include "MovieToLedUtils.hpp"

LedColorCorrection::LedColorCorrection() {
	for (int i = 0; i < 256; i++) {
		//ガンマ補正
		//切り上げのために+0.5
		gamma_table[i] = static_cast<uint8_t>(pow(i / 255.0, GAMMA) * 255.0 + 0.5);
	}

	// RGB Curve Plot
	param_panel.setup("RGB Parameter");
	slider_led_white_gain.addListener(this, &LedColorCorrection::sliderLedWhiteGainChanged);
	slider_led_rgb_gain.addListener(this, &LedColorCorrection::sliderLedRgbGainChanged);
	slider_panel_white_gain.addListener(this, &LedColorCorrection::sliderPanelWhiteGainChanged);
	slider_panel_rgb_gain.addListener(this, &LedColorCorrection::sliderPanelRgbGainChanged);
	param_panel.add(slider_led_white_gain.setup("Normal LED White Gain", 100, 0, 100));
	param_panel.add(slider_led_rgb_gain.setup("Normal LED Color Gain", 100, 0, 100));
	param_panel.add(slider_panel_white_gain.setup("Panel LED White Gain", 100, 0, 100));
	param_panel.add(slider_panel_rgb_gain.setup("Panel LED Color Gain", 100, 0, 100));
	slider_led_white_gain.setFillColor(ofColor(0, 153, 204, 255));
	slider_led_rgb_gain.setFillColor(ofColor(0, 102, 223, 255));
	slider_panel_white_gain.setFillColor(ofColor(254, 145, 0, 255));
	slider_panel_rgb_gain.setFillColor(ofColor(228, 0, 6, 255));
}

void LedColorCorrection::setLedGain(LedGain gain) {
	led_gain = gain;
	slider_led_white_gain = led_gain.led_white;
	slider_led_rgb_gain = led_gain.led_rgb;
	slider_panel_white_gain = led_gain.panel_white;
	slider_panel_rgb_gain = led_gain.panel_rgb;
}

LedGain LedColorCorrection::getLedGain() {
	return led_gain;
}

void LedColorCorrection::setupRender(int x, int y, int panel_width) {
	render_x = x;
	render_y = y;
	param_panel.setPosition(render_x, render_y + 10);
	param_panel.setWidthElements(panel_width);
	// plot init
	plot_area.set(param_panel.getPosition().x + param_panel.getWidth(), param_panel.getPosition().y, 255, 255);
	uint8_t gammma_max = 0;
	updatePlot(gamma_curve, 100, gammma_max);
	updatePlot(led_white_curve, led_gain.led_white, led_max.led_white);
	updatePlot(led_rgb_curve, led_gain.led_rgb, led_max.led_rgb);
	updatePlot(panel_white_curve, led_gain.panel_white, led_max.panel_white);
	updatePlot(panel_rgb_curve, led_gain.panel_rgb, led_max.panel_rgb);
}

void LedColorCorrection::draw() {
	MovieToLedUtils::Font::Middle.drawString("LED Color Gain", render_x, render_y);
	MovieToLedUtils::Font::Small.drawString("Normal LED", render_x, render_y + 10 + param_panel.getHeight() + 26);
	MovieToLedUtils::Font::Small.drawString("Panel LED", render_x, render_y + 10 + param_panel.getHeight() + 104);

	param_panel.draw();

	// 補正をかけたRGB値を描画するグラフ
	ofSetColor(255, 255, 255, 30);
	ofDrawRectangle(plot_area);
	ofSetColor(255, 255, 255, 96);
	for (int i = 0; i < 7; i++) {
		ofDrawLine(plot_area.getPosition().x + 32 * (i + 1), plot_area.getPosition().y, plot_area.getPosition().x + 32 * (i + 1), plot_area.getPosition().y + plot_area.getHeight());
		ofDrawLine(plot_area.getPosition().x, plot_area.getPosition().y + 32 * (i + 1), plot_area.getPosition().x + plot_area.getWidth(), plot_area.getPosition().y + 32 * (i + 1));
	}
	ofSetColor(0, 153, 204, 255);
	MovieToLedUtils::Font::Tiny.drawString("WHT Normal LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32);
	led_white_curve.draw();
	MovieToLedUtils::Font::Small.drawString("White MAX:" + ofToString((int)led_max.led_white), render_x, render_y + 10 + param_panel.getHeight() + 52);
	ofSetColor(0, 102, 223, 255);
	MovieToLedUtils::Font::Tiny.drawString("RGB Normal LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 2);
	led_rgb_curve.draw();
	MovieToLedUtils::Font::Small.drawString("Other Color MAX:" + ofToString((int)led_max.led_rgb), render_x, render_y + 10 + param_panel.getHeight() + 78);
	ofSetColor(254, 145, 0, 255);
	MovieToLedUtils::Font::Tiny.drawString("WHT Panel LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 3);
	panel_white_curve.draw();
	MovieToLedUtils::Font::Small.drawString("White MAX:" + ofToString((int)led_max.panel_white), render_x, render_y + 10 + param_panel.getHeight() + 130);
	ofSetColor(228, 0, 6, 255);
	MovieToLedUtils::Font::Tiny.drawString("RGB Panel LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 4);
	panel_rgb_curve.draw();
	MovieToLedUtils::Font::Small.drawString("Other Color MAX:" + ofToString((int)led_max.panel_rgb), render_x, render_y + 10 + param_panel.getHeight() + 156);
	ofSetColor(14, 167, 39, 255);
	MovieToLedUtils::Font::Tiny.drawString("Gamma Curve", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20);
	gamma_curve.draw();
}

void LedColorCorrection::applyLedColorCorrection(uint8_t (&src)[3]) {
	uint8_t gain = (src[0] == src[1] && src[1] == src[2]) ? led_gain.led_white : led_gain.led_rgb;
	for (int i = 0; i < 3; i++) {
		// ゲイン適用
		src[i] = (src[i] * gain) / 100;
		// ガンマ補正
		src[i] = gamma_table[src[i]];
		// 最大値250に
		src[i] = (src[i] > 250) ? 250 : src[i];
	}
}

void LedColorCorrection::applyPanelColorCorrection(uint8_t (&src)[3]) {
	uint8_t gain = (src[0] == src[1] && src[1] == src[2]) ? led_gain.panel_white : led_gain.panel_rgb;
	for (int i = 0; i < 3; i++) {
		// ゲイン適用
		src[i] = (src[i] * gain) / 100;
		// ガンマ補正
		src[i] = gamma_table[src[i]];
		// 最大値250に
		src[i] = (src[i] > 250) ? 250 : src[i];
	}
}

void LedColorCorrection::updatePlot(ofPolyline & curve, uint8_t gain, uint8_t & max) {
	curve.clear();
	for (int i = 0; i < 256; i++) {
		if (i == 0 || i == 255) {
			// 最初と最後を２回描画すると曲線がなめらかになる
			// 補正をかける順番 -> gain(線形補正)をかけてからガンマ補正(非線形補正)をかける
			uint8_t val = i * gain / 100;
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
			if (i == 255) max = gamma_table[val] > 250 ? 250 : gamma_table[val];
		} else {
			uint8_t val = i * gain / 100;
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
		}
	}
}

void LedColorCorrection::sliderLedWhiteGainChanged(uint8_t & gain) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		led_gain.led_white = gain;
		updatePlot(led_white_curve, led_gain.led_white, led_max.led_white);
	}
}

void LedColorCorrection::sliderLedRgbGainChanged(uint8_t & gain) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		led_gain.led_rgb = gain;
		updatePlot(led_rgb_curve, led_gain.led_rgb, led_max.led_rgb);
	}
}

void LedColorCorrection::sliderPanelWhiteGainChanged(uint8_t & gain) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		led_gain.panel_white = gain;
		updatePlot(panel_white_curve, led_gain.panel_white, led_max.panel_white);
	}
}

void LedColorCorrection::sliderPanelRgbGainChanged(uint8_t & gain) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		led_gain.panel_rgb = gain;
		updatePlot(panel_rgb_curve, led_gain.panel_rgb, led_max.panel_rgb);
	}
}
