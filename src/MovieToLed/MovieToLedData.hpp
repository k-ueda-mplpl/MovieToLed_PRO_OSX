#ifndef MOVIETOLED_DATA_HPP
#define MOVIETOLED_DATA_HPP

#include "LedProduct.hpp"
#include "MovieToLedUtils.hpp"
#include "ProductProfile.hpp"
#include <cstdint>
#include <string>

struct OutputFiles {
	std::string M5LED;
	std::string BIN_DEVICE_8LINE;
	std::string BIN_DEVICE_4LINE_ABCD;
	std::string BIN_DEVICE_4LINE_EFGH;
	std::string BIN_DEVICE_1000FPS;
};

class MovieToLedData {
public:
	MovieToLedData();
	~MovieToLedData();

	std::vector<ProductProfile> product_profiles;
	LedProductManager led_product { product_profiles };
	std::vector<OutputFiles> output_files;

	inline static const uint16_t BUFF_SIZE = LedProduct::Device::MAX_NUM_LED * 3 * LedProduct::Device::MAX_NUM_LINE;
	inline static const uint8_t MAX_NUM_SCENE = 70;
};

#endif
