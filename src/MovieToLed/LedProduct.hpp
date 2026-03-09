#ifndef LED_PRODUCT_HPP
#define LED_PRODUCT_HPP

#include "ProductProfile.hpp"
#include <cstdint>
#include <vector>

struct Led {
	enum LedType {
		NORMAL,
		PANEL
	};
	short x, y;
	LedType type;
	Led() {
		x = SHRT_MIN;
		y = SHRT_MIN;
		type = LedType::NORMAL;
	};
	void setup(short pos_x, short pos_y, LedType led_type) {
		x = pos_x;
		y = pos_y;
		type = led_type;
	};
};

class LedProduct {
public:
	struct Device {
		inline static const uint16_t MAX_NUM_LED = 256;
		inline static const uint8_t MAX_NUM_LINE = 8;
		Led led[MAX_NUM_LINE][MAX_NUM_LED];
		uint16_t num_led[MAX_NUM_LED];

		void initLed() {
			for (int i = 0; i < MAX_NUM_LINE; i++) {
				for (int j = 0; j < MAX_NUM_LED; j++) {
					led[i][j] = Led();
				}
			}
		};
	};

	std::vector<Device> devices;

	LedProduct() {
		devices.clear();
	};

	~LedProduct() {
		devices.clear();
	};
};

class LedProductManager {
public:
	LedProductManager(std::vector<ProductProfile> & profiles_ref)
		: product_profiles(profiles_ref) { products.clear(); };

	~LedProductManager() { products.clear(); };

	void selectProfile(uint8_t index);

	// void setLed(std::string path, uint16_t start_id, std::string & error_msg);
	void setLed(std::string path, uint16_t head_id, std::string & error_msg);

	void setNumLed();

	std::string getName();
	uint16_t getNumProduct();
	uint16_t getNumDevice();
	ProductProfile::DeviceType getDeviceType();
	uint16_t getStartProductId();
	uint16_t getEndProductId();
	ProductProfile::DeviceIdFormat getDeviceIdFormat();
	
	bool isGenerateData();
	bool isOutputBin();
	bool isOutput4lineBin();
	bool isOutput8lineBin();

	uint16_t getNumLed(uint16_t product_id, uint16_t device_id, uint8_t line_id);
	const Led * getLed(uint16_t product_id, uint16_t device_id, uint8_t line_id, uint8_t led_id);

	uint8_t getProfileIndex();

private:
	std::vector<ProductProfile> & product_profiles;
	uint8_t profile_index;
	std::vector<LedProduct> products;
	inline static const Led dummy_led = Led();

	void setupProducts(const ProductProfile & profile);
};

#endif