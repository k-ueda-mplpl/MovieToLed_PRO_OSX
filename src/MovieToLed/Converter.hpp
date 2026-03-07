#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include "MovieToLedData.hpp"
#include "ProductProfile.hpp"
#include <cstdint>
#include <ofFileUtils.h>
#include <vector>

class Converter {
public:
	std::vector<std::string> completed_file;

	Converter(MovieToLedData & mtl_data_ref)
		: mtl_data(mtl_data_ref) { completed_file.clear(); };
	~Converter();
	void setOutputDir(std::string path);

	void setSoundNumber(uint8_t num);

	bool isCreateDir(char * buff, int buff_size, uint16_t product_id, uint8_t device_id, ProductProfile::DeviceIdFormat id_format, uint16_t start_product_id, uint16_t max_product_count);
	void setBINName(char * buff, int buff_size, uint8_t sound_num, uint16_t product_id, uint16_t device_id, ProductProfile::DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count);
	void removePreviousBIN(std::string dir_path, uint8_t sound_num, uint16_t product_id, uint16_t num_device, ProductProfile::DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count);

	void createBIN8LINE(std::string parent_path, uint8_t sound_num);
	void createBIN4LINE(std::string parent_path, uint8_t sound_num);

	void setHeader8LINE(ofFile & file, uint8_t * buff, int buf_size, uint16_t frame_size, uint16_t device_id);
	void setHeader4LINE_ABCD(ofFile & file, uint8_t * buff, int buf_size, uint16_t frame_size, uint16_t device_id);
	void setHeader4LINE_EFGH(ofFile & file, uint8_t * buff, int buf_size, uint16_t frame_size, uint16_t device_id);
	void regulatorOn(uint8_t * buff, uint8_t num_line);
	bool isBlack(uint8_t * buff, uint8_t num_led, int buff_size);

	void convertRGB8LINE(uint8_t * src_dat, uint8_t * dst_buff);
	void convertRGB4LINE(uint8_t * src_dat, uint8_t * dst_buff);

	void convert8LINE(ofFile & src_file, ofFile & dst_file, unsigned int & dst_size);
	void convert1000FPS(ofFile & src_file, ofFile & dst_file, unsigned int & dst_size);
	void convert4LINE(ofFile & src_file, ofFile * dst_file, unsigned int * dst_size, int num_dst = 2);

	void createBIN();
	bool convert(OutputFiles & output_files, uint8_t sound_number, uint16_t product_id, uint16_t device_id);
	void process();
	bool isFinish() {
		return convert_count >= max_convert_count;
	}

	int getConvertCount(){
		return convert_count;
	}

	int getMaxConvertCount(){
		return max_convert_count;
	}

	static uint16_t getBlockSize(uint16_t num_device, ProductProfile::DeviceIdFormat id_format);
	static uint16_t getMaxProductCount(uint16_t num_device, ProductProfile::DeviceIdFormat id_format);

private:
	MovieToLedData & mtl_data;

	uint8_t dst[MovieToLedData::BUFF_SIZE], src[MovieToLedData::BUFF_SIZE];
	uint8_t next[MovieToLedData::BUFF_SIZE], prev[MovieToLedData::BUFF_SIZE];

	ofFile conv_file[2], m5led_file;
	char file_name[14];
	std::string output_dir;
	uint16_t num_total_frame;

	ofFile md_file;
	std::string md_path;
	char md_buff[128];

	std::string log_conv_path;
	uint8_t sound_number;
	int convert_count, max_convert_count;
};

#endif
