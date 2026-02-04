#ifndef DEVICE_MAP_HPP
#define DEVICE_MAP_HPP

#include "Converter.hpp"

class DeviceMap {
public:
	DeviceMap();
	virtual ~DeviceMap();
	void setOutputDir(std::string path);
	void create(std::string product_name, uint16_t num_product, uint16_t num_device, DeviceType device_type, DeviceIdFormat id_format);
	void create(ProductContent & content);

protected:
	std::string output_dir_path;
};

#endif
