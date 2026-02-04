#ifndef EXTRACTOR_HPP
#define EXTRACTOR_HPP

#include "MovieToLedUtils.hpp"
#include <functional>
#include <ofxGui.h>
#include <ofxOpenCv.h>

#ifdef TARGET_WIN32
	#include <opencv2/videoio.hpp>
	#include <windows.h>
#endif

class Extractor {
public:
	Extractor();
	virtual ~Extractor();
	void setup(MovieToLedUtils::OutputData * data_ptr);
	void setOutputDir(string path);
	void setCallback(std::function<void()> func);

	void loadVideo(string path);
	void loadLed(uint8_t scene_num);

	bool isExistM5LED(ProductContent & content, uint8_t sound_num);
	bool isCreateDir(char * buff, int buff_size, uint16_t product_id, uint16_t start_product_id);
	void createM5LED(uint8_t sound_num);
	void writeM5LED(ofFile & file, string product_name, uint8_t sound_num, uint16_t product_id, uint16_t device_id, uint8_t * dat, int dat_size);

	void setHeader(uint16_t product_id, uint16_t device_id);
	void writeFirstData(uint8_t sound_num);

	void correct(uint8_t (&dat)[3], LedType type);
	void logBlack(ofFile & file, string path, bool is_src_blk, bool is_dat_blk, uint16_t product_id, uint16_t device_id);
	void logSkip(ofFile & file, string path, int cur_pos, int prev_pos);
	void logFrame(ofFile & file, string path, unsigned int & count, uint16_t frame, bool is_end = false);

	bool getVideoFrame(int frame, ofImage & img);
	void updateScene(bool is_first = false);
	void writeVideoData(uint8_t sound_num);
	void extract();

protected:
	// video
	string video_file_name;
	// ofVideoPlayer video_player;
	// ofPixels video_pixels;
	// OSXの場合
	// ofVideoPlayerを使うとフレームが正確に1ずつ更新できない -> openCVのVideoCaptureを使用
	ofImage video_image;
	cv::VideoCapture video_capture;
	cv::Mat frame_mat;
	uint16_t num_frame, prev_frame, total_frame;
	uint8_t duration_min, duration_sec;
	// scene
	uint8_t curr_scene, prev_scene;
	// data correct
	inline static constexpr float gamma = 2.2; //gammma
	uint8_t gamma_table[256];
	uint8_t led_white_gain;
	uint8_t led_rgb_gain;
	uint8_t panel_white_gain;
	uint8_t panel_rgb_gain;
	bool loop_playback;
	// sound number
	uint8_t sound_number;
	// log
	uint8_t num_black, num_skip;
	string log_black_msg[32], log_skip_msg[32];
	string log_black_path, log_skip_path, log_frame_path;
	unsigned int num_count;
	ofFile log_file;
	char log_buff[64];
	// LED 5V or 12V
	bool use_led_5V;

	string product_name;
	uint16_t num_device;
	uint16_t start_product_id;
	uint16_t end_product_id;

private:
	// buffer
	uint8_t buff[MovieToLedUtils::BUFF_SIZE];
	ofFile dat_file;
	char file_name[18];
	string output_dir;
	char dir_name[13];

	std::function<void()> callback;

	MovieToLedUtils::OutputData * output_data = nullptr;
	bool isValid() {
		return output_data;
	};

	uint64_t error_time;
	uint16_t error_frame;
	bool is_error_frame;
};

#endif

