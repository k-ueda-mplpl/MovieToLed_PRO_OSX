#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include "MovieToLedUtils.hpp"

class Converter {
public:
	Converter();
	virtual ~Converter();
	void setup(MovieToLedUtils::OutputData * data_ptr, std::vector<std::string> * completed_file_ptr);
	void setOutputDir(std::string path);

	bool isCreateDir(char * buff, int buff_size, uint16_t product_id, uint8_t device_id, DeviceIdFormat id_format, uint16_t start_product_id, uint16_t max_product_count);
	void setBINName(char * buff, int buff_size, uint8_t sound_num, uint16_t product_id, uint16_t device_id, DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count);
	void removePreviousBIN(std::string dir_path, uint8_t sound_num, uint16_t product_id, uint16_t num_device, DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count);

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

	void createBIN(uint8_t sound_num);
	bool convert(MovieToLedUtils::OutputFiles & output_files, uint8_t sound_number, uint16_t product_id, uint16_t device_id);
	void process();
	bool isFinish() {
		return convert_count >= max_convert_count;
	}

	static uint16_t getBlockSize(uint16_t num_device, DeviceIdFormat id_format);
	static uint16_t getMaxProductCount(uint16_t num_device, DeviceIdFormat id_format);

protected:
	std::string log_conv_path;
	uint8_t sound_number;
	int convert_count, max_convert_count;

private:
	uint8_t dst[MovieToLedUtils::BUFF_SIZE], src[MovieToLedUtils::BUFF_SIZE];
	uint8_t next[MovieToLedUtils::BUFF_SIZE], prev[MovieToLedUtils::BUFF_SIZE];

	ofFile conv_file[2], m5led_file;
	char file_name[14];
	std::string output_dir;
	uint16_t num_total_frame;

	MovieToLedUtils::OutputData * output_data = nullptr;
	std::vector<std::string> * completed_file;
	bool isValid() {
		return output_data && completed_file;
	}

	ofFile md_file;
	std::string md_path;
	char md_buff[128];
};

#endif

