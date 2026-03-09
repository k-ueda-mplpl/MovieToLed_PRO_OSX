#ifndef DEVICE_MAP_HPP
#define DEVICE_MAP_HPP

#include "Converter.hpp"
#include "ProductProfile.hpp"
#include <cstdint>
#include <string>

class DeviceMap {
public:
	DeviceMap();
	~DeviceMap();
	void setOutputDir(std::string path);
	void create(std::string product_name, uint16_t num_product, uint16_t num_device, ProductProfile::DeviceType device_type, ProductProfile::DeviceIdFormat id_format);
	void create(const ProductProfile & profile);

protected:
	std::string output_dir_path;
};

#endif