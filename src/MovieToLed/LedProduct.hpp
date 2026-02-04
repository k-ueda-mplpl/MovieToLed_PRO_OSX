#ifndef LED_PRODUCT_HPP
#define LED_PRODUCT_HPP

#include <ofFileUtils.h>
#include <ofUtils.h>
#include <vector>

enum class LedType {
	NORMAL,
	PANEL
};

enum class DeviceType {
	LINE4,
	LINE8,
	LINE8_1000FPS,
	UNKNOWN
};

enum class DeviceIdFormat {
	DEC,
	HEX
};

// データ作成するプロダクト情報
struct ProductContent {
	std::string product_name;
	uint16_t num_product, num_device;
	DeviceType device_type;
	DeviceIdFormat id_format;
	uint16_t start_product_id, end_product_id;
	bool is_data_generate;
	ProductContent(std::string name, uint16_t num) {
		product_name = name;
		num_product = num;
		num_device = 1;
		device_type = DeviceType::LINE8;
		id_format = DeviceIdFormat::DEC;
		start_product_id = 0;
		end_product_id = num_product - 1;
		is_data_generate = true;
	}
	void setDeviceInfo(DeviceType type, uint16_t num) {
		num_device = num;
		device_type = type;
		// デバイス数が10台以上16台以下なら自動でHEXに
		// 1プロダクトのデバイスが収まるから
		if (num_device > 10 && num_device <= 16) id_format = DeviceIdFormat::HEX;
		// if (num_product > 10 && num_product <= 16) id_format = DeviceIdFormat::HEX;
	}
	bool setCreationRange(uint16_t from, uint16_t to) {
		if (from >= num_product || to >= num_product || from > to) return false;
		start_product_id = from;
		end_product_id = to;
		return true;
	}
};

struct Led {
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

private:
};

class LedProductManager {
public:
	LedProductManager();
	~LedProductManager();

	void setup(ProductContent * content_ptr);
	// void setLed(std::string path, uint16_t start_id, std::string & error_msg);
	void setLed(std::string path, uint16_t head_id, std::string & error_msg);

	void setNumLed();

	std::string getName();
	uint16_t getNumProduct();
	uint16_t getNumDevice();
	DeviceType getDeviceType();
	uint16_t getStartProductId();
	uint16_t getEndProductId();
	DeviceIdFormat getDeviceIdFormat();
	uint16_t getNumLed(uint16_t device_id, uint8_t line_id);
	const Led * getLed(uint16_t product_id, uint16_t device_id, uint8_t line_id, uint8_t led_id);

private:
	ProductContent * content = nullptr;
	std::vector<LedProduct> products;
	inline static const Led dummy_led = Led();

	bool isValid() {
		return content;
	};
};

#endif
