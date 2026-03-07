#include "Extractor.hpp"
#include "LedLoader.hpp"
#include "MovieToLedFileUtils.hpp"
#include "MovieToLedRuntimeState.hpp"

Extractor::Extractor(MovieToLedData & mtl_data_ref, LedColorCorrection & led_color_correction_ref)
	: mtl_data(mtl_data_ref)
	, led_color_correction(led_color_correction_ref) {
	video_file_name.clear();
	// video_pixels.allocate(FULL_HD_WIDTH, FULL_HD_HEIGHT, OF_PIXELS_RGB);
	video_image.allocate(MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT, OF_IMAGE_COLOR);
	frame_mat = cv::Mat(MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT, CV_8UC3);
	curr_frame = 0;
	prev_frame = 0;
	total_frame = 0;
	duration_min = 0;
	duration_sec = 0;
	memset(buff, 0, MovieToLedData::BUFF_SIZE);
	is_error_frame = false;
}

Extractor::~Extractor() {
}

void Extractor::setOutputDir(string path) {
	output_dir = path + "/M5LED/";
}

void Extractor::setCallback(std::function<void()> func) {
	callback = func;
}

void Extractor::loadVideo(string path) {
	if (ofFilePath::getFileExt(path) == "mov" || ofFilePath::getFileExt(path) == "mp4") {
		// if (video_player.load(path)) {
		// 	video_file_name = ofFilePath::getFileName(path);
		// 	video_player.setLoopState(OF_LOOP_NONE);
		// 	video_player.setPaused(true);
		// 	video_player.setFrame(0);
		// 	total_frame = video_player.getTotalNumFrames();
		// 	duration_min = total_frame / 30 / 60;
		// 	duration_sec = total_frame / 30 % 60;
		// }
		video_capture.open(path);
		if (video_capture.isOpened()) {
			video_file_name = ofFilePath::getFileName(path);
			getVideoFrame(0, video_image);
			total_frame = (uint16_t)video_capture.get(cv::CAP_PROP_FRAME_COUNT);
			duration_min = total_frame / 30 / 60;
			duration_sec = total_frame / 30 % 60;
		}
	}
}

void Extractor::clearVideo() {
	video_capture.release();
	video_file_name.clear();
	total_frame = 0;
	duration_min = 0;
	duration_sec = 0;
}

void Extractor::allocate(int w, int h) {
	video_image.allocate(w, h, OF_IMAGE_COLOR);
	frame_mat = cv::Mat(w, h, CV_8UC3);
}

void Extractor::setSoundNumber(uint8_t num) {
	sound_number = num;
}

void Extractor::setLoopPlayback(bool loop) {
	loop_playback = loop;
}

void Extractor::createLogBlackFile(string path) {
	log_black_path = path;
	MovieToLedFileUtils::createFile(log_black_path, false);
}

void Extractor::createLogSkipFile(string path) {
	log_skip_path = path;
	MovieToLedFileUtils::createFile(log_black_path, false);
}

void Extractor::createLogFrameFile(string path) {
	log_frame_path = path;
	MovieToLedFileUtils::createFile(log_black_path, false);
}

void Extractor::createLogScene(string path) {
	log_scene_path = path;
	MovieToLedFileUtils::createFile(log_scene_path, false);
}

bool Extractor::isExistM5LED(ProductProfile & profile, uint8_t sound_num) {
	snprintf(file_name, 4, "%02X_", (int)sound_num);
	string dir_path = output_dir + profile.product_name;
	ofDirectory dir(dir_path);
	if (!dir.exists()) return false;
	dir.listDir();
	for (auto & f : dir.getFiles()) {
		if (f.isDirectory()) {
			ofDirectory sub(f.getAbsolutePath());
			sub.listDir();
			for (auto file : sub) {
				if (file.getFileName().find(file_name) != string::npos) {
					return true;
				}
			}
		}
	}
	return false;
}

bool Extractor::isCreateDir(char * buff, int buff_size, uint16_t product_id, uint16_t start_product_id) {
	// ディレクトリ名
	// プロダクト名/プロダクトIDの範囲/M5LEDファイル という構造になる
	// 作成するIDの範囲を指定できるようにしたので、ID = 256の倍数の時でないときもディレクトリを作成するように
	bool is_create = (product_id == start_product_id || (product_id & 0xff) == 0);
	if (is_create) {
		uint16_t head_id = (product_id / 256) * 256;
		snprintf(buff, buff_size, "/%05d-%05d", head_id, head_id + 0xff);
	}
	return is_create;
}

void Extractor::createM5LED() {
	if (!mtl_data.output_files.empty()) {
		mtl_data.output_files.clear();
	}
	product_name = mtl_data.led_product.getName();
	start_product_id = mtl_data.led_product.getStartProductId();
	end_product_id = mtl_data.led_product.getEndProductId();
	num_device = mtl_data.led_product.getNumDevice();
	string dir_path = output_dir + product_name;
	for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
		if (isCreateDir(dir_name, 13, product_id, start_product_id)) {
			// ディレクトリ作成
			dir_path = output_dir + product_name + dir_name;
			MovieToLedFileUtils::createDir(dir_path);
			// 以前のM5LEDファイルを削除
			char prev_file_name[18];
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				snprintf(prev_file_name, 18, "/%02X_%03d-%03d.M5LED", sound_number, product_id & 0xff, device_id & 0xff);
				ofFile prev_file(dir_path + prev_file_name);
				if (prev_file.exists()) prev_file.remove();
			}
		}
		// M5LEDの生成
		// ファイル名: SS_PPP-DDD.M5LED
		// S = Sound Number, P = Product ID, D = Device ID
		for (uint16_t device_id = 0; device_id < num_device; device_id++) {
			snprintf(file_name, 18, "/%02X_%03d-%03d.M5LED", sound_number, product_id & 0xff, device_id & 0xff);
			MovieToLedFileUtils::createFile(dir_path + file_name);
			OutputFiles files;
			files.M5LED = dir_path + file_name;
			mtl_data.output_files.push_back(files);
		}
	}
}

void Extractor::writeM5LED(ofFile & file, string product_name, uint8_t sound_num, uint16_t product_id, uint16_t device_id, uint8_t * dat, int dat_size) {
	char dir_name[13];
	uint16_t base_id = (product_id >> 8) << 8; // 256の倍数に丸める
	snprintf(dir_name, 13, "/%05d-%05d", base_id, base_id + 0xff);
	snprintf(file_name, 18, "/%02X_%03d-%03d.M5LED", sound_num, product_id & 0xff, device_id & 0xff);
	string path = output_dir + product_name + dir_name + file_name;
	while (!MovieToLedFileUtils::openFile(file, path, ofFile::Append))
		;
	if (file && file.is_open()) {
		file.write((const char *)(dat), dat_size);
		file.close(); //save
	}
}

void Extractor::setHeader(uint16_t product_id, uint16_t device_id) {
	memset(buff, 0, MovieToLedData::BUFF_SIZE);
	buff[0] = 0x4d;
	buff[1] = 0x35;
	buff[2] = 3;
	buff[3] = 30;
	buff[4] = (total_frame >> 8) & 0xff;
	buff[5] = total_frame & 0xff;
	buff[6] = mtl_data.led_product.getNumLed(device_id, 0);
	buff[7] = mtl_data.led_product.getNumLed(device_id, 1);
	buff[8] = mtl_data.led_product.getNumLed(device_id, 2);
	buff[9] = mtl_data.led_product.getNumLed(device_id, 3);
	buff[10] = mtl_data.led_product.getNumLed(device_id, 4);
	buff[11] = mtl_data.led_product.getNumLed(device_id, 5);
	buff[12] = mtl_data.led_product.getNumLed(device_id, 6);
	buff[13] = mtl_data.led_product.getNumLed(device_id, 7);
	buff[14] = loop_playback ? 1 : 0; //ループするか
	buff[15] = 0xed;
}

void Extractor::writeFirstData(uint8_t sound_num) {
	for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
		for (uint16_t device_id = 0; device_id < num_device; device_id++) {
			// ヘッダーを入力
			setHeader(product_id, device_id);
			writeM5LED(dat_file, product_name, sound_num, product_id, device_id, buff, MovieToLedData::BUFF_SIZE);
			// 最初のフレーム位置にRGB = (0, 0, 0)のデータを入れる
			memset(buff, 0, MovieToLedData::BUFF_SIZE);
			writeM5LED(dat_file, product_name, sound_num, product_id, device_id, buff, MovieToLedData::BUFF_SIZE);
		}
	}
}

void Extractor::applyColorCorrection(uint8_t (&dat)[3], Led::LedType type) {
	// 各種割合でデータを補正
	// ゲインの選択
	if (type == Led::LedType::PANEL) {
		led_color_correction.applyPanelColorCorrection(dat);
	} else if (type == Led::LedType::NORMAL) {
		led_color_correction.applyLedColorCorrection(dat);
	}
}

void Extractor::logBlack(ofFile & file, string path, bool is_src_blk, bool is_dat_blk, uint16_t product_id, uint16_t device_id) {
	// 動画データと抽出して補正をかけたデータでR=G=B=0かどうかを調べる
	// 動画データはR=G=B=0でないが抽出したデータはR=G=B=0の場合、LEDが一瞬光らないことがある(補正の影響あり)
	if (is_src_blk != is_dat_blk) {
		MovieToLedFileUtils::openFile(file, path, ofFile::Append, false);
		char msg[128];
		if (is_src_blk) {
			snprintf(msg, 128, "%06d Frame %05d Product %03d Device %03d / Video:Black LED:Color", num_black + 1, curr_frame, product_id % 1000, device_id % 1000);
			snprintf(log_buff, 64, "%d,%d,%d,Video:Black,LED:Color\n", curr_frame, product_id, device_id);
		} else if (is_dat_blk) {
			snprintf(msg, 128, "%06d Frame %05d Product %03d Device %03d / Video:Color LED:Black", num_black + 1, curr_frame, product_id % 1000, device_id % 1000);
			snprintf(log_buff, 64, "%d,%d,%d,Video:Color,LED:Black\n", curr_frame, product_id, device_id);
		}
		//snprintf(str, 126, "%06d Frame:%05ld Device:%02d / Data Video:%d LED:%d", black_count + 1, read_frame, device_id, is_raw_black, is_processed_black);
		log_black_msg[num_black++ % 32] = msg;
		if (file && file.is_open()) {
			file.write(log_buff, strlen(log_buff));
			file.close();
		}
	}
}

void Extractor::logSkip(ofFile & file, string path, int cur_pos, int prev_pos) {
	// 動画のフレームが飛んだかを調べる
	if (cur_pos >= 0 && prev_pos >= 0 && cur_pos - prev_pos != 1) {
		MovieToLedFileUtils::openFile(file, path, ofFile::Append, false);
		char msg[128];
		snprintf(msg, 128, "%06d Diff %d Frame / %d -> %d\r\n", num_skip + 1, cur_pos - prev_pos, prev_pos, cur_pos);
		log_skip_msg[num_skip++ % 32] = msg;
		snprintf(log_buff, 64, "%d,%d,%d\n", prev_pos, cur_pos, cur_pos - prev_pos);
		if (file && file.is_open()) {
			file.write(log_buff, strlen(log_buff));
			file.close();
		}
	}
}

void Extractor::logFrame(ofFile & file, string path, unsigned int & count, uint16_t frame, bool is_end) {
	MovieToLedFileUtils::openFile(file, path, ofFile::Append, false);
	if (!is_end) {
		snprintf(log_buff, 64, "%d,%d\n", count, frame);
		count++;
	} else {
		snprintf(log_buff, 64, "Video,%d\n", frame);
	}
	if (file && file.is_open()) {
		file.write(log_buff, strlen(log_buff));
		file.close();
	}
}

void Extractor::ready() {
	curr_frame = 1;
	prev_frame = 1;
	num_count = 1;
}

bool Extractor::getVideoFrame(int frame, ofImage & img) {
	if (!video_capture.isOpened()) return false;
	video_capture.set(cv::CAP_PROP_POS_FRAMES, frame);
	if (!video_capture.read(frame_mat)) return false;
	cv::cvtColor(frame_mat, frame_mat, cv::COLOR_BGR2RGB);
	img.setFromPixels(frame_mat.data, frame_mat.cols, frame_mat.rows, OF_IMAGE_COLOR);
	// 次に読み込まれるフレーム数を取得
	curr_frame = video_capture.get(cv::CAP_PROP_POS_FRAMES);
	return true;
}

void Extractor::logScene(uint8_t scene, uint16_t frame) {
	ofFile file;
	MovieToLedFileUtils::openFile(file, log_scene_path, ofFile::Append, false);
	snprintf(log_buff, 64, "%d,%d\n", frame, scene);
	if (file && file.is_open()) {
		file.write(log_buff, strlen(log_buff));
		file.close();
	}
}

void Extractor::updateScene(uint16_t frame, bool is_first) {
	// Sceneをチェック
	for (int i = 0; i < MovieToLedData::MAX_NUM_SCENE; i++) {
		// 指定のy座標のピクセルのポインタを取得
		cv::Vec3b * src = frame_mat.ptr<cv::Vec3b>(14);
		// x座標の色([0, 1, 2] = [R, G, B])
		// 四角が白色か黒色なので，RGBのうち[G] = [1]の値で判断
		if (src[25 * i + 140][1] > 200) {
			curr_scene = i;
			break;
		}
	}
	// Sceneの切り替え
	// 最初のScene or Sceneが更新されたらledの座標データをロード
	if (prev_scene != curr_scene || is_first) {
		printf("Update Scene %d\r\n", curr_scene);
		LedLoader::loadLed(mtl_data, curr_scene);
		logScene(curr_scene, frame);
		prev_scene = curr_scene;
	}
}

void Extractor::writeVideoData(uint8_t sound_num) {
	for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
		for (uint16_t device_id = 0; device_id < num_device; device_id++) {
			bool is_src_blk = true;
			bool is_dat_blk = true;
			for (uint8_t line_id = 0; line_id < LedProduct::Device::MAX_NUM_LINE; line_id++) {
				for (uint16_t led_id = 0; led_id < LedProduct::Device::MAX_NUM_LED; led_id++) {
					const Led * led = mtl_data.led_product.getLed(product_id, device_id, line_id & 0xff, led_id & 0xff);
					if (led->x >= 0 && led->y >= 0) {
						cv::Vec3b * src = frame_mat.ptr<cv::Vec3b>(led->y);
						if (is_src_blk && !(src[led->x][0] == 0 && src[led->x][1] == 0 && src[led->x][2] == 0)) {
							is_src_blk = false;
						}
						// 処理用の配列に値をコピー
						uint8_t dat[3];
						memcpy(dat, &src[led->x], 3);
						// RとGを入れ替え
						// 5VのLEDはデータ順がGRB, 12VのLEDはRGB
						if (use_led_5V) swap(dat[0], dat[1]);
						// データの補正
						applyColorCorrection(dat, led->type);
						if (is_dat_blk && !(dat[0] == 0 && dat[1] == 0 && dat[2] == 0)) {
							is_dat_blk = false;
						}
						// bufferにコピー
						memcpy(&buff[LedProduct::Device::MAX_NUM_LED * 3 * line_id + led_id * 3], dat, 3);
					} else {
						memset(&buff[LedProduct::Device::MAX_NUM_LED * 3 * line_id + led_id * 3], 0, 3);
					}
				}
			}
			// 1デバイス分のデータを出力
			writeM5LED(dat_file, mtl_data.led_product.getName(), sound_num, product_id, device_id, buff, MovieToLedData::BUFF_SIZE);
			memset(buff, 0, MovieToLedData::BUFF_SIZE);
			logBlack(log_file, log_black_path, is_src_blk, is_dat_blk, product_id, device_id);
		}
	}
}

void Extractor::extract() {
	//if (num_frame < 0 || !video_player.isLoaded()) return;
	if (video_capture.isOpened()) {
		if (curr_frame == 1 && prev_frame == 1) {
			// 開始時
			printf("1st Frame\r\n");
			// Sceneを更新.最初のSceneのLEDデータをロード
			updateScene(curr_frame, true);
			mtl_data.led_product.setNumLed();
			writeFirstData(sound_number);
			// 最初のフレームはRGB=0のデータになるから動画データは出力しない
			// 動画の最初のフレームのLEDデータを出力
			// writeVideoData();
			logFrame(log_file, log_frame_path, num_count, curr_frame);
			// printf("num frame = %d / prev frame = %d\r\n", num_frame, prev_frame);
		}

		if (curr_frame < total_frame) {
			// Update Video Frame
			if (getVideoFrame(curr_frame, video_image)) {
				if (curr_frame - prev_frame != 1 && (curr_frame > 1 && prev_frame > 1)) {
					while (!getVideoFrame(prev_frame - 1, video_image)) {
						ofSleepMillis(1);
					}
				}
			} else {
			}
		}
		// printf("num frame = %d / prev frame = %d\r\n", num_frame, prev_frame);
		// Update Video
		//		video_player.nextFrame();
		//		video_player.update();
		//		if (num_frame < total_frame - 2) {
		//			while (!video_player.isFrameNew()) {
		//#ifdef TARGET_WIN32
		//				// WIN
		//				// windows.hをincludeして、WinAPIのWaitableTimerを使用
		//				HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
		//				if (!timer) return;
		//				LARGE_INTEGER li;
		//				li.QuadPart = -static_cast<int64_t>(1) * 10000LL;
		//				SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
		//				WaitForSingleObject(timer, INFINITE);
		//				CloseHandle(timer);
		//#else
		//				// OSX
		//				ofSleepMillis(1);
		//#endif
		//				video_player.update();
		//			}
		//		}
		//		num_frame = video_player.getCurrentFrame();

		// 動画のピクセルデータをcv::Matに変換
		//		video_pixels = video_player.getPixels();
		//		if (video_pixels.isAllocated()) {
		//			frame_mat = cv::Mat(video_pixels.getHeight(), video_pixels.getWidth(), CV_8UC3, video_pixels.getData());
		//			video_image.setFromPixels(frame_mat.data, frame_mat.cols, frame_mat.rows, OF_IMAGE_COLOR);
		//		}
		updateScene(curr_frame);
		writeVideoData(sound_number);
		logSkip(log_file, log_skip_path, curr_frame, prev_frame);
		prev_frame = curr_frame;
		logFrame(log_file, log_frame_path, num_count, curr_frame);
		// 動画が最後のフレームかチェック
		if (curr_frame >= total_frame - 1) {
			// 動画が最後のフレームの場合
			// 最後にRGB = (0, 0, 0)のデータを入れる
			memset(buff, 0, MovieToLedData::BUFF_SIZE);
			for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
				for (uint16_t device_id = 0; device_id < num_device; device_id++) {
					writeM5LED(dat_file, product_name, sound_number, product_id, device_id, buff, MovieToLedData::BUFF_SIZE);
					ofSleepMillis(1);
					// ファイルを閉じる
					snprintf(dir_name, 13, "/%05d-%05d", (product_id >> 8) << 8, ((product_id >> 8) << 8) + 0xff);
					snprintf(file_name, 18, "/%02X_%03d-%03d.M5LED", sound_number, product_id & 0xff, device_id & 0xff);
					string path = output_dir + product_name + dir_name + file_name;
					while (!MovieToLedFileUtils::openFile(dat_file, path, ofFile::ReadOnly))
						;
					// if (dat_file && dat_file.is_open()) {
					// 	printf("Completed File:%s Size:%llu byte = %llu Frame\r\n", dat_file.getFileName().c_str(), dat_file.getSize(), dat_file.getSize() / BUFF_SIZE);
					// }
					dat_file.flush();
					dat_file.close();
				}
			}
			// データ生成終了
			// 動画を0フレーム目に戻す
			// video_player.setFrame(0);
			// video_player.update();
			// num_frame = video_player.getCurrentFrame();
			getVideoFrame(0, video_image);
			curr_frame = 1;
			prev_frame = 1;
			num_count = 1;
			logFrame(log_file, log_frame_path, num_count, total_frame, true);
			if (callback) callback();
			return;
		}
	}
}

void Extractor::drawLogSkip(int x, int y) {
	for (int i = 0; i < 32; i++) {
		// ofDrawBitmapString(log_black_msg[i], display_width - 520, display_height - 10 - 15 * i);
		ofDrawBitmapString(log_skip_msg[i], x, y - 15 * i);
	}
}
