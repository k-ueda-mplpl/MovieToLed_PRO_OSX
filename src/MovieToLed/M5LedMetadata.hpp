#ifndef M5LED_METADATA_HPP
#define M5LED_METADATA_HPP

#include "MovieToLedUtils.hpp"

struct METADATA {
	uint8_t sound_number;
	uint16_t duration;
	uint8_t play_mode;
};

class M5LedMetadata {
public:
	M5LedMetadata();
	~M5LedMetadata();

	void set(uint8_t sound_num, uint16_t duration, uint8_t play_mode);
	void write(std::string dir_path, uint8_t sound_num);
	void clear(std::string dir_path);

private:
	METADATA metadata[11];
};

#endif

