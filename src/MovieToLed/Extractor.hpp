#ifndef EXTRACTOR_HPP
#define EXTRACTOR_HPP

#include "LedColorCorrection.hpp"
#include "LedProduct.hpp"
#include "MovieToLedData.hpp"
#include "MovieToLedUtils.hpp"
#include "ProductProfile.hpp"
#include <cstdint>
#include <functional>
#include <ofxGui.h>
#include <ofxOpenCv.h>

#ifdef TARGET_WIN32
	#include <opencv2/videoio.hpp>
	#include <windows.h>
#endif

class Extractor {
public:
	ofImage video_image;
	Extractor(MovieToLedData & mtl_data_ref, LedColorCorrection & led_color_correction_ref);
	~Extractor();
	void setOutputDir(string path);
	void setCallback(std::function<void()> func);

	void loadVideo(string path);
	void clearVideo();
	bool isLoadedVideo() {
		return video_capture.isOpened();
	}
	string getVideoFileName() {
		return video_file_name;
	}

	// void loadLed(uint8_t scene_num);

	void allocate(int w, int h);

	void setSoundNumber(uint8_t num);
	void setLoopPlayback(bool loop);

	bool isLoopPlayback() {
		return loop_playback;
	}

	uint16_t getTotalFrame() {
		return total_frame;
	}

	uint16_t getCurrentFrame() {
		return curr_frame;
	}

	uint8_t getDurationMin() {
		return duration_min;
	}

	uint8_t getDurationSec() {
		return duration_sec;
	}

	void createLogBlackFile(string path);
	void createLogSkipFile(string path);
	void createLogFrameFile(string path);
	void createLogScene(string path);

	bool isExistM5LED(ProductProfile & profile, uint8_t sound_num);
	bool isCreateDir(char * buff, int buff_size, uint16_t product_id, uint16_t start_product_id);
	void createM5LED();
	void writeM5LED(ofFile & file, string product_name, uint8_t sound_num, uint16_t product_id, uint16_t device_id, uint8_t * dat, int dat_size);

	void setHeader(uint16_t product_id, uint16_t device_id);
	void writeFirstData(uint8_t sound_num);

	void applyColorCorrection(uint8_t (&dat)[3], Led::LedType type);
	void logBlack(ofFile & file, string path, bool is_src_blk, bool is_dat_blk, uint16_t product_id, uint16_t device_id);
	void logSkip(ofFile & file, string path, int cur_pos, int prev_pos);
	void logFrame(ofFile & file, string path, unsigned int & count, uint16_t frame, bool is_end = false);

	void ready();
	bool getVideoFrame(int frame, ofImage & img);
	void logScene(uint8_t scene, uint16_t frame);
	void updateScene(uint16_t frame, bool is_first = false);
	void writeVideoData(uint8_t sound_num);
	void extract();

	void drawLogSkip(int x, int y);

private:
	// MtL Data
	MovieToLedData & mtl_data;
	// Led Color
	LedColorCorrection & led_color_correction;

	// buffer
	uint8_t buff[MovieToLedData::BUFF_SIZE];
	ofFile dat_file;
	char file_name[18];
	string output_dir;
	char dir_name[13];

	std::function<void()> callback;

	uint64_t error_time;
	uint16_t error_frame;
	bool is_error_frame;

	// video
	string video_file_name;
	// ofVideoPlayer video_player;
	// ofPixels video_pixels;
	// OSXの場合
	// ofVideoPlayerを使うとフレームが正確に1ずつ更新できない -> openCVのVideoCaptureを使用
	cv::VideoCapture video_capture;
	cv::Mat frame_mat;
	uint16_t curr_frame, prev_frame, total_frame;
	uint8_t duration_min, duration_sec;
	// scene
	uint8_t curr_scene, prev_scene;
	string log_scene_path;

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
};

#endif
