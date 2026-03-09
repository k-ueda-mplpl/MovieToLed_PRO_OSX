#ifndef LED_LOADER_HPP
#define LED_LOADER_HPP

#include "MovieToLedData.hpp"
#include "MovieToLedRuntimeState.hpp"
#include "MovieToLedUtils.hpp"
#include <string>

class LedLoader {
public:
	static void loadLed(MovieToLedData & mtl_data, uint8_t scene_num) {
		char file_name[18];
		snprintf(file_name, 18, "/suit_led_s%d.csv", scene_num);
		char suffix[8];
		// デバイス数が1の場合、100プロダクトごとに
		// デバイス数が2以上の場合、10プロダクトごとにLEDのマップデータがある
		uint8_t product_count = mtl_data.led_product.getNumDevice() == 1 ? 100 : 10;
		uint16_t head_id = (mtl_data.led_product.getStartProductId() / product_count) * product_count;
		while (head_id <= mtl_data.led_product.getEndProductId()) {
			snprintf(suffix, 8, "%d-%d", head_id, head_id + product_count - 1);
			std::string dir_name = MovieToLedUtils::DirPaths::CONTENTS_DIR + "/" + mtl_data.led_product.getName() + suffix;
			// LEDのマップ情報を更新
			mtl_data.led_product.setLed(dir_name + file_name, head_id, MovieToLedRuntimeState::error_msg);
			head_id += product_count;
		}
		mtl_data.led_product.setNumLed();
	};
};

#endif