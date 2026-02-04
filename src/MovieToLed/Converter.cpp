#include "Converter.hpp"

Converter::Converter() {
}

Converter::~Converter() {
	output_data = nullptr;
}

void Converter::setup(MovieToLedUtils::OutputData * data_ptr, std::vector<std::string> * completed_file_ptr) {
	output_data = data_ptr;
	completed_file = completed_file_ptr;
}

void Converter::setOutputDir(std::string path) {
	output_dir = path;
}

bool Converter::isCreateDir(char * buff, int buff_size, uint16_t product_id, uint8_t device_id, DeviceIdFormat id_format, uint16_t start_product_id, uint16_t max_product_count) {
	// ディレクトリの作成タイミングとディレクトリ名を決定
	bool is_create = false;
	if (max_product_count > 0) {
		// 1プロダクトがBINの範囲に収まるとき
		// BINが00-99(10進数)の場合、デバイス100台以下のプロダクト
		// BINが00-FF(16進数)の場合、デバイス256台以下のプロダクト
		is_create = device_id == 0 && (product_id == start_product_id || product_id % max_product_count == 0);
		if (is_create) {
			uint16_t head_id = (product_id / max_product_count) * max_product_count;
			if (id_format == DeviceIdFormat::DEC) {
				snprintf(buff, buff_size, "/%03d-%03d-DEC", head_id, head_id + max_product_count - 1);
			} else if (id_format == DeviceIdFormat::HEX) {
				snprintf(buff, buff_size, "/%03X-%03X-HEX", head_id, head_id + max_product_count - 1);
			}
		}
	} else {
		// 1プロダクトがBINの範囲を超えるとき
		// デバイスが256 or 100台ごとにディレクトリを作成
		uint16_t max_device_count = id_format == DeviceIdFormat::HEX ? 256 : 100;
		is_create = device_id % max_device_count == 0;
		if (is_create) {
			if (id_format == DeviceIdFormat::DEC) {
				snprintf(buff, buff_size, "/%03X-%03X-HEX", product_id, device_id);
			} else {
				snprintf(buff, buff_size, "/%03d-%03d-DEC", product_id, device_id);
			}
		}
	}
	return is_create;
}

void Converter::setBINName(char * buff, int buff_size, uint8_t sound_num, uint16_t product_id, uint16_t device_id, DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count) {
	if (max_product_count > 0) {
		// 1プロダクトがBINの範囲に収まるとき
		uint8_t bin_id = (product_id % max_product_count) * block_size + device_id;
		if (id_format == DeviceIdFormat::DEC) {
			snprintf(buff, buff_size, "/%05X_%02d.BIN", sound_num, bin_id);
		} else if (id_format == DeviceIdFormat::HEX) {
			snprintf(buff, buff_size, "/%05X_%02X.BIN", sound_num, bin_id);
		}
	} else {
		// 1プロダクトがBINの範囲を超えるとき
		uint16_t max_device_count = id_format == DeviceIdFormat::HEX ? 256 : 100;
		snprintf(buff, buff_size, "/%05X_%02X.BIN", sound_num, device_id % max_device_count);
	}
}

void Converter::removePreviousBIN(std::string dir_path, uint8_t sound_num, uint16_t product_id, uint16_t num_device, DeviceIdFormat id_format, uint16_t block_size, uint16_t max_product_count) {
	char prev_file_name[14];
	for (uint16_t device_id = 0; device_id < num_device; device_id++) {
		setBINName(prev_file_name, 14, sound_num, product_id, device_id, id_format, block_size, max_product_count);
		ofFile prev_file(dir_path + prev_file_name);
		if (prev_file.exists()) prev_file.remove();
	}
}

void Converter::createBIN8LINE(std::string parent_path, uint8_t sound_num) {
	if (isValid()) {
		// 8ラインデバイスと1000FPSデバイスのディレクトリを作成
		FileUtils::createDir(parent_path + MovieToLedUtils::BinDirPahts::DEVICE_8LINE_DIR);
		FileUtils::createDir(parent_path + MovieToLedUtils::BinDirPahts::DEVICE_1000FPS_DIR);
		std::string dir_8LINE, dir_1000FPS;
		uint16_t start_product_id = output_data->led_product.getStartProductId();
		uint16_t end_product_id = output_data->led_product.getEndProductId();
		uint16_t num_device = output_data->led_product.getNumDevice();
		DeviceIdFormat id_format = output_data->led_product.getDeviceIdFormat();
		// ブロックサイズ: 1プロダクトが消費するBIN名の数
		uint16_t block_size = getBlockSize(num_device, id_format);
		// プロダクト収容数: 同じディレクトリに入れられるBINのプロダクト数
		uint16_t max_product_count = getMaxProductCount(num_device, id_format);
		char child_dir_name[13];
		for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				if (isCreateDir(child_dir_name, 13, product_id, device_id, id_format, start_product_id, max_product_count)) {
					// ディレクトリの作成
					dir_8LINE = parent_path + MovieToLedUtils::BinDirPahts::DEVICE_8LINE_DIR + child_dir_name;
					dir_1000FPS = parent_path + MovieToLedUtils::BinDirPahts::DEVICE_1000FPS_DIR + child_dir_name;
					FileUtils::createDir(dir_8LINE);
					FileUtils::createDir(dir_1000FPS);
					// 以前のBINファイルの削除
					removePreviousBIN(dir_8LINE, product_id, sound_num, num_device, id_format, block_size, max_product_count);
					removePreviousBIN(dir_1000FPS, product_id, sound_num, num_device, id_format, block_size, max_product_count);
				}
				// BINファイルの生成
				setBINName(file_name, 14, sound_num, product_id, device_id, id_format, block_size, max_product_count);
				FileUtils::createFile(dir_8LINE + file_name);
				FileUtils::createFile(dir_1000FPS + file_name);
				// 対応したグループにパスを登録
				for (MovieToLedUtils::OutputFiles & files : output_data->output_files) {
					if (files.M5LED.find(output_data->led_product.getName()) != std::string::npos) {
						char m5led[29];
						snprintf(m5led, 29, "%05d-%05d/%02X_%03d-%03d.M5LED", (product_id >> 8) << 8, ((product_id >> 8) << 8) + 0xff, sound_num, product_id & 0xff, device_id & 0xff);
						if (files.M5LED.find(m5led) != std::string::npos) {
							files.BIN_DEVICE_8LINE = dir_8LINE + file_name;
							files.BIN_DEVICE_1000FPS = dir_1000FPS + file_name;
						}
					}
				}
			}
		}
	}
}

void Converter::createBIN4LINE(std::string parent_path, uint8_t sound_num) {
	if (isValid()) {
		// 4ラインデバイスのディレクトリを作成
		FileUtils::createDir(parent_path + MovieToLedUtils::BinDirPahts::DEVICE_4LINE_ABCD_DIR);
		FileUtils::createDir(parent_path + MovieToLedUtils::BinDirPahts::DEVICE_4LINE_EFGH_DIR);
		std::string dir_4LINE[2];
		uint16_t start_product_id = output_data->led_product.getStartProductId();
		uint16_t end_product_id = output_data->led_product.getEndProductId();
		uint16_t num_device = output_data->led_product.getNumDevice();
		DeviceIdFormat id_format = output_data->led_product.getDeviceIdFormat();
		// ブロックサイズ: 1プロダクトが消費するBIN名の数
		uint16_t block_size = getBlockSize(num_device, id_format);
		// プロダクト収容数: 同じディレクトリに入れられるBINのプロダクト数
		uint16_t max_product_count = getMaxProductCount(num_device, id_format);
		char child_dir_name[13];
		for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				if (isCreateDir(child_dir_name, 13, product_id, device_id, id_format, start_product_id, max_product_count)) {
					// ディレクトリの作成
					dir_4LINE[0] = parent_path + MovieToLedUtils::BinDirPahts::DEVICE_4LINE_ABCD_DIR + child_dir_name;
					dir_4LINE[1] = parent_path + MovieToLedUtils::BinDirPahts::DEVICE_4LINE_EFGH_DIR + child_dir_name;
					FileUtils::createDir(dir_4LINE[0]);
					FileUtils::createDir(dir_4LINE[1]);
					// 以前のBINファイルの削除
					removePreviousBIN(dir_4LINE[0], product_id, sound_num, num_device, id_format, block_size, max_product_count);
					removePreviousBIN(dir_4LINE[1], product_id, sound_num, num_device, id_format, block_size, max_product_count);
				}
				// BINファイルの生成
				setBINName(file_name, 14, sound_num, product_id, device_id, id_format, block_size, max_product_count);
				FileUtils::createFile(dir_4LINE[0] + file_name);
				FileUtils::createFile(dir_4LINE[1] + file_name);
				// 対応したグループにパスを登録
				for (MovieToLedUtils::OutputFiles & files : output_data->output_files) {
					if (files.M5LED.find(output_data->led_product.getName()) != std::string::npos) {
						char m5led[29];
						snprintf(m5led, 29, "%05d-%05d/%02X_%03d-%03d.M5LED", (product_id >> 8) << 8, ((product_id >> 8) << 8) + 0xff, sound_num, product_id & 0xff, device_id & 0xff);
						if (files.M5LED.find(m5led) != std::string::npos) {
							files.BIN_DEVICE_4LINE_ABCD = dir_4LINE[0] + file_name;
							files.BIN_DEVICE_4LINE_EFGH = dir_4LINE[1] + file_name;
						}
					}
				}
			}
		}
	}
}

void Converter::setHeader8LINE(ofFile & file, uint8_t * buff, int buf_size, uint16_t num_frame, uint16_t device_id) {
	if (isValid()) {
		memset(buff, 0, buf_size);
		buff[0] = 3; // 256 x n x 8 (n = 3 -> RGB)
		buff[1] = 3;
		buff[2] = 30; // FPS
		buff[3] = 30;
		buff[4] = 1;
		buff[5] = 1;
		buff[6] = 1;
		buff[7] = 1;
		buff[10] = 1; // format = 1
		buff[11] = 1;
		buff[12] = (num_frame >> 8) & 0xff; // Frame Duration High
		buff[13] = (num_frame >> 8) & 0xff;
		buff[14] = num_frame & 0xff; // Frame Duration Low
		buff[15] = num_frame & 0xff;
		buff[16] = output_data->led_product.getNumLed(device_id, 0); // led
		buff[17] = output_data->led_product.getNumLed(device_id, 1);
		buff[18] = output_data->led_product.getNumLed(device_id, 2);
		buff[19] = output_data->led_product.getNumLed(device_id, 3);
		buff[20] = output_data->led_product.getNumLed(device_id, 4);
		buff[21] = output_data->led_product.getNumLed(device_id, 5);
		buff[22] = output_data->led_product.getNumLed(device_id, 6);
		buff[23] = output_data->led_product.getNumLed(device_id, 7);
		file.write((const char *)(buff), buf_size);
	}
}

void Converter::setHeader4LINE_ABCD(ofFile & file, uint8_t * buff, int buf_size, uint16_t num_frame, uint16_t device_id) {
	if (isValid()) {
		memset(buff, 0, buf_size);
		buff[0] = 3; // 256 x n x 8 (n = 3 -> RGB)
		buff[1] = 30; // FPS
		buff[2] = 1;
		buff[3] = 1;
		buff[4] = 1; // format = 1
		buff[5] = (num_frame >> 8) & 0xff; // Frame Duration High
		buff[6] = num_frame & 0xff; // Frame Duration Low
		buff[7] = output_data->led_product.getNumLed(device_id, 0); // led
		buff[8] = output_data->led_product.getNumLed(device_id, 1);
		buff[9] = output_data->led_product.getNumLed(device_id, 2);
		buff[10] = output_data->led_product.getNumLed(device_id, 3);
		file.write((const char *)(buff), buf_size);
	}
}

void Converter::setHeader4LINE_EFGH(ofFile & file, uint8_t * buff, int buf_size, uint16_t num_frame, uint16_t device_id) {
	if (isValid()) {
		memset(buff, 0, buf_size);
		buff[0] = 3; // 256 x n x 8 (n = 3 -> RGB)
		buff[1] = 30; // FPS
		buff[2] = 1;
		buff[3] = 1;
		buff[4] = 1; // format = 1
		buff[5] = ((num_frame + 1) >> 8) & 0xff; // Frame Duration High
		buff[6] = (num_frame + 1) & 0xff; // Frame Duration Low
		buff[7] = output_data->led_product.getNumLed(device_id, 4); // led
		buff[8] = output_data->led_product.getNumLed(device_id, 5);
		buff[9] = output_data->led_product.getNumLed(device_id, 6);
		buff[10] = output_data->led_product.getNumLed(device_id, 7);
		file.write((const char *)(buff), buf_size);
	}
}

void Converter::regulatorOn(uint8_t * buff, uint8_t num_line) {
	memset(&buff[LedProduct::Device::MAX_NUM_LED * 3 * num_line + (LedProduct::Device::MAX_NUM_LED - 1) * 3], 252, 3);
}

bool Converter::isBlack(uint8_t * buff, uint8_t num_led, int buff_size) {
	if (num_led * 3 + 2 < buff_size) {
		return buff[num_led * 3] == 0 && buff[num_led * 3 + 1] == 0 && buff[num_led * 3 + 2] == 0;
	}
	return true;
}

void Converter::convertRGB8LINE(uint8_t * src_dat, uint8_t * dst_buff) {
	// BINデータ変換
	unsigned char r[LedProduct::Device::MAX_NUM_LINE], g[LedProduct::Device::MAX_NUM_LINE], b[LedProduct::Device::MAX_NUM_LINE]; //LED1個のRGB8ライン分
	for (int led = 0; led < LedProduct::Device::MAX_NUM_LED; led++) {
		for (int line = 0; line < LedProduct::Device::MAX_NUM_LINE; line++) {
			g[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3];
			r[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3 + 1];
			b[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3 + 2];
		}
		// 8ライン分の1個のLEDのRGBを24(=8x3)バイトに変換
		// ex. BINのヘッダーの次のバイトから8バイトは各ラインの1個目のLEDのG, その次から8バイトが各ラインの1個目のLEDのR, その次から8バイトが各ラインのLEDのB
		// 2 ^ 4 = 6, 3 ^ 4 = 7, 4 ^ 4 = 0, 5 ^ 4 = 1
		dst_buff[led * 24] = (((g[0] >> 7) & 1) << 2) | (((g[1] >> 7) & 1) << 3) | (((g[2] >> 7) & 1) << 4) | (((g[3] >> 7) & 1) << 5) | (((g[0] >> 6) & 1) << (2 ^ 4)) | (((g[1] >> 6) & 1) << (3 ^ 4)) | (((g[2] >> 6) & 1) << (4 ^ 4)) | (((g[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 1] = (((g[4] >> 7) & 1) << 2) | (((g[5] >> 7) & 1) << 3) | (((g[6] >> 7) & 1) << 4) | (((g[7] >> 7) & 1) << 5) | (((g[4] >> 6) & 1) << (2 ^ 4)) | (((g[5] >> 6) & 1) << (3 ^ 4)) | (((g[6] >> 6) & 1) << (4 ^ 4)) | (((g[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 2] = (((g[0] >> 5) & 1) << 2) | (((g[1] >> 5) & 1) << 3) | (((g[2] >> 5) & 1) << 4) | (((g[3] >> 5) & 1) << 5) | (((g[0] >> 4) & 1) << (2 ^ 4)) | (((g[1] >> 4) & 1) << (3 ^ 4)) | (((g[2] >> 4) & 1) << (4 ^ 4)) | (((g[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 3] = (((g[4] >> 5) & 1) << 2) | (((g[5] >> 5) & 1) << 3) | (((g[6] >> 5) & 1) << 4) | (((g[7] >> 5) & 1) << 5) | (((g[4] >> 4) & 1) << (2 ^ 4)) | (((g[5] >> 4) & 1) << (3 ^ 4)) | (((g[6] >> 4) & 1) << (4 ^ 4)) | (((g[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 4] = (((g[0] >> 3) & 1) << 2) | (((g[1] >> 3) & 1) << 3) | (((g[2] >> 3) & 1) << 4) | (((g[3] >> 3) & 1) << 5) | (((g[0] >> 2) & 1) << (2 ^ 4)) | (((g[1] >> 2) & 1) << (3 ^ 4)) | (((g[2] >> 2) & 1) << (4 ^ 4)) | (((g[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 5] = (((g[4] >> 3) & 1) << 2) | (((g[5] >> 3) & 1) << 3) | (((g[6] >> 3) & 1) << 4) | (((g[7] >> 3) & 1) << 5) | (((g[4] >> 2) & 1) << (2 ^ 4)) | (((g[5] >> 2) & 1) << (3 ^ 4)) | (((g[6] >> 2) & 1) << (4 ^ 4)) | (((g[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 6] = (((g[0] >> 1) & 1) << 2) | (((g[1] >> 1) & 1) << 3) | (((g[2] >> 1) & 1) << 4) | (((g[3] >> 1) & 1) << 5) | (((g[0] >> 0) & 1) << (2 ^ 4)) | (((g[1] >> 0) & 1) << (3 ^ 4)) | (((g[2] >> 0) & 1) << (4 ^ 4)) | (((g[3] >> 0) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 7] = (((g[4] >> 1) & 1) << 2) | (((g[5] >> 1) & 1) << 3) | (((g[6] >> 1) & 1) << 4) | (((g[7] >> 1) & 1) << 5) | (((g[4] >> 0) & 1) << (2 ^ 4)) | (((g[5] >> 0) & 1) << (3 ^ 4)) | (((g[6] >> 0) & 1) << (4 ^ 4)) | (((g[7] >> 0) & 1) << (5 ^ 4));

		dst_buff[led * 24 + 8] = (((r[0] >> 7) & 1) << 2) | (((r[1] >> 7) & 1) << 3) | (((r[2] >> 7) & 1) << 4) | (((r[3] >> 7) & 1) << 5) | (((r[0] >> 6) & 1) << (2 ^ 4)) | (((r[1] >> 6) & 1) << (3 ^ 4)) | (((r[2] >> 6) & 1) << (4 ^ 4)) | (((r[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 9] = (((r[4] >> 7) & 1) << 2) | (((r[5] >> 7) & 1) << 3) | (((r[6] >> 7) & 1) << 4) | (((r[7] >> 7) & 1) << 5) | (((r[4] >> 6) & 1) << (2 ^ 4)) | (((r[5] >> 6) & 1) << (3 ^ 4)) | (((r[6] >> 6) & 1) << (4 ^ 4)) | (((r[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 10] = (((r[0] >> 5) & 1) << 2) | (((r[1] >> 5) & 1) << 3) | (((r[2] >> 5) & 1) << 4) | (((r[3] >> 5) & 1) << 5) | (((r[0] >> 4) & 1) << (2 ^ 4)) | (((r[1] >> 4) & 1) << (3 ^ 4)) | (((r[2] >> 4) & 1) << (4 ^ 4)) | (((r[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 11] = (((r[4] >> 5) & 1) << 2) | (((r[5] >> 5) & 1) << 3) | (((r[6] >> 5) & 1) << 4) | (((r[7] >> 5) & 1) << 5) | (((r[4] >> 4) & 1) << (2 ^ 4)) | (((r[5] >> 4) & 1) << (3 ^ 4)) | (((r[6] >> 4) & 1) << (4 ^ 4)) | (((r[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 12] = (((r[0] >> 3) & 1) << 2) | (((r[1] >> 3) & 1) << 3) | (((r[2] >> 3) & 1) << 4) | (((r[3] >> 3) & 1) << 5) | (((r[0] >> 2) & 1) << (2 ^ 4)) | (((r[1] >> 2) & 1) << (3 ^ 4)) | (((r[2] >> 2) & 1) << (4 ^ 4)) | (((r[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 13] = (((r[4] >> 3) & 1) << 2) | (((r[5] >> 3) & 1) << 3) | (((r[6] >> 3) & 1) << 4) | (((r[7] >> 3) & 1) << 5) | (((r[4] >> 2) & 1) << (2 ^ 4)) | (((r[5] >> 2) & 1) << (3 ^ 4)) | (((r[6] >> 2) & 1) << (4 ^ 4)) | (((r[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 14] = (((r[0] >> 1) & 1) << 2) | (((r[1] >> 1) & 1) << 3) | (((r[2] >> 1) & 1) << 4) | (((r[3] >> 1) & 1) << 5) | (((r[0] >> 0) & 1) << (2 ^ 4)) | (((r[1] >> 0) & 1) << (3 ^ 4)) | (((r[2] >> 0) & 1) << (4 ^ 4)) | (((r[3] >> 0) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 15] = (((r[4] >> 1) & 1) << 2) | (((r[5] >> 1) & 1) << 3) | (((r[6] >> 1) & 1) << 4) | (((r[7] >> 1) & 1) << 5) | (((r[4] >> 0) & 1) << (2 ^ 4)) | (((r[5] >> 0) & 1) << (3 ^ 4)) | (((r[6] >> 0) & 1) << (4 ^ 4)) | (((r[7] >> 0) & 1) << (5 ^ 4));

		dst_buff[led * 24 + 16] = (((b[0] >> 7) & 1) << 2) | (((b[1] >> 7) & 1) << 3) | (((b[2] >> 7) & 1) << 4) | (((b[3] >> 7) & 1) << 5) | (((b[0] >> 6) & 1) << (2 ^ 4)) | (((b[1] >> 6) & 1) << (3 ^ 4)) | (((b[2] >> 6) & 1) << (4 ^ 4)) | (((b[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 17] = (((b[4] >> 7) & 1) << 2) | (((b[5] >> 7) & 1) << 3) | (((b[6] >> 7) & 1) << 4) | (((b[7] >> 7) & 1) << 5) | (((b[4] >> 6) & 1) << (2 ^ 4)) | (((b[5] >> 6) & 1) << (3 ^ 4)) | (((b[6] >> 6) & 1) << (4 ^ 4)) | (((b[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 18] = (((b[0] >> 5) & 1) << 2) | (((b[1] >> 5) & 1) << 3) | (((b[2] >> 5) & 1) << 4) | (((b[3] >> 5) & 1) << 5) | (((b[0] >> 4) & 1) << (2 ^ 4)) | (((b[1] >> 4) & 1) << (3 ^ 4)) | (((b[2] >> 4) & 1) << (4 ^ 4)) | (((b[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 19] = (((b[4] >> 5) & 1) << 2) | (((b[5] >> 5) & 1) << 3) | (((b[6] >> 5) & 1) << 4) | (((b[7] >> 5) & 1) << 5) | (((b[4] >> 4) & 1) << (2 ^ 4)) | (((b[5] >> 4) & 1) << (3 ^ 4)) | (((b[6] >> 4) & 1) << (4 ^ 4)) | (((b[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 20] = (((b[0] >> 3) & 1) << 2) | (((b[1] >> 3) & 1) << 3) | (((b[2] >> 3) & 1) << 4) | (((b[3] >> 3) & 1) << 5) | (((b[0] >> 2) & 1) << (2 ^ 4)) | (((b[1] >> 2) & 1) << (3 ^ 4)) | (((b[2] >> 2) & 1) << (4 ^ 4)) | (((b[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 21] = (((b[4] >> 3) & 1) << 2) | (((b[5] >> 3) & 1) << 3) | (((b[6] >> 3) & 1) << 4) | (((b[7] >> 3) & 1) << 5) | (((b[4] >> 2) & 1) << (2 ^ 4)) | (((b[5] >> 2) & 1) << (3 ^ 4)) | (((b[6] >> 2) & 1) << (4 ^ 4)) | (((b[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 22] = (((b[0] >> 1) & 1) << 2) | (((b[1] >> 1) & 1) << 3) | (((b[2] >> 1) & 1) << 4) | (((b[3] >> 1) & 1) << 5) | (((b[0] >> 0) & 1) << (2 ^ 4)) | (((b[1] >> 0) & 1) << (3 ^ 4)) | (((b[2] >> 0) & 1) << (4 ^ 4)) | (((b[3] >> 0) & 1) << (5 ^ 4));
		dst_buff[led * 24 + 23] = (((b[4] >> 1) & 1) << 2) | (((b[5] >> 1) & 1) << 3) | (((b[6] >> 1) & 1) << 4) | (((b[7] >> 1) & 1) << 5) | (((b[4] >> 0) & 1) << (2 ^ 4)) | (((b[5] >> 0) & 1) << (3 ^ 4)) | (((b[6] >> 0) & 1) << (4 ^ 4)) | (((b[7] >> 0) & 1) << (5 ^ 4));
	}
}

void Converter::convertRGB4LINE(uint8_t * src_dat, uint8_t * dst_buff) {
	unsigned char r[LedProduct::Device::MAX_NUM_LINE], g[LedProduct::Device::MAX_NUM_LINE], b[LedProduct::Device::MAX_NUM_LINE]; //LED1個のRGB8ライン分
	for (int led = 0; led < LedProduct::Device::MAX_NUM_LED; led++) {
		for (int line = 0; line < LedProduct::Device::MAX_NUM_LINE; line++) {
			g[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3];
			r[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3 + 1];
			b[line] = src_dat[LedProduct::Device::MAX_NUM_LED * 3 * line + led * 3 + 2];
		}
		// LINE ABCDのLEDデータ
		dst_buff[led * 12] = (((g[0] >> 7) & 1) << 2) | (((g[1] >> 7) & 1) << 3) | (((g[2] >> 7) & 1) << 4) | (((g[3] >> 7) & 1) << 5) | (((g[0] >> 6) & 1) << (2 ^ 4)) | (((g[1] >> 6) & 1) << (3 ^ 4)) | (((g[2] >> 6) & 1) << (4 ^ 4)) | (((g[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 1] = (((g[0] >> 5) & 1) << 2) | (((g[1] >> 5) & 1) << 3) | (((g[2] >> 5) & 1) << 4) | (((g[3] >> 5) & 1) << 5) | (((g[0] >> 4) & 1) << (2 ^ 4)) | (((g[1] >> 4) & 1) << (3 ^ 4)) | (((g[2] >> 4) & 1) << (4 ^ 4)) | (((g[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 2] = (((g[0] >> 3) & 1) << 2) | (((g[1] >> 3) & 1) << 3) | (((g[2] >> 3) & 1) << 4) | (((g[3] >> 3) & 1) << 5) | (((g[0] >> 2) & 1) << (2 ^ 4)) | (((g[1] >> 2) & 1) << (3 ^ 4)) | (((g[2] >> 2) & 1) << (4 ^ 4)) | (((g[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 3] = (((g[0] >> 1) & 1) << 2) | (((g[1] >> 1) & 1) << 3) | (((g[2] >> 1) & 1) << 4) | (((g[3] >> 1) & 1) << 5) | (((g[0] >> 0) & 1) << (2 ^ 4)) | (((g[1] >> 0) & 1) << (3 ^ 4)) | (((g[2] >> 0) & 1) << (4 ^ 4)) | (((g[3] >> 0) & 1) << (5 ^ 4));

		dst_buff[led * 12 + 4] = (((r[0] >> 7) & 1) << 2) | (((r[1] >> 7) & 1) << 3) | (((r[2] >> 7) & 1) << 4) | (((r[3] >> 7) & 1) << 5) | (((r[0] >> 6) & 1) << (2 ^ 4)) | (((r[1] >> 6) & 1) << (3 ^ 4)) | (((r[2] >> 6) & 1) << (4 ^ 4)) | (((r[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 5] = (((r[0] >> 5) & 1) << 2) | (((r[1] >> 5) & 1) << 3) | (((r[2] >> 5) & 1) << 4) | (((r[3] >> 5) & 1) << 5) | (((r[0] >> 4) & 1) << (2 ^ 4)) | (((r[1] >> 4) & 1) << (3 ^ 4)) | (((r[2] >> 4) & 1) << (4 ^ 4)) | (((r[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 6] = (((r[0] >> 3) & 1) << 2) | (((r[1] >> 3) & 1) << 3) | (((r[2] >> 3) & 1) << 4) | (((r[3] >> 3) & 1) << 5) | (((r[0] >> 2) & 1) << (2 ^ 4)) | (((r[1] >> 2) & 1) << (3 ^ 4)) | (((r[2] >> 2) & 1) << (4 ^ 4)) | (((r[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 7] = (((r[0] >> 1) & 1) << 2) | (((r[1] >> 1) & 1) << 3) | (((r[2] >> 1) & 1) << 4) | (((r[3] >> 1) & 1) << 5) | (((r[0] >> 0) & 1) << (2 ^ 4)) | (((r[1] >> 0) & 1) << (3 ^ 4)) | (((r[2] >> 0) & 1) << (4 ^ 4)) | (((r[3] >> 0) & 1) << (5 ^ 4));

		dst_buff[led * 12 + 8] = (((b[0] >> 7) & 1) << 2) | (((b[1] >> 7) & 1) << 3) | (((b[2] >> 7) & 1) << 4) | (((b[3] >> 7) & 1) << 5) | (((b[0] >> 6) & 1) << (2 ^ 4)) | (((b[1] >> 6) & 1) << (3 ^ 4)) | (((b[2] >> 6) & 1) << (4 ^ 4)) | (((b[3] >> 6) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 9] = (((b[0] >> 5) & 1) << 2) | (((b[1] >> 5) & 1) << 3) | (((b[2] >> 5) & 1) << 4) | (((b[3] >> 5) & 1) << 5) | (((b[0] >> 4) & 1) << (2 ^ 4)) | (((b[1] >> 4) & 1) << (3 ^ 4)) | (((b[2] >> 4) & 1) << (4 ^ 4)) | (((b[3] >> 4) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 10] = (((b[0] >> 3) & 1) << 2) | (((b[1] >> 3) & 1) << 3) | (((b[2] >> 3) & 1) << 4) | (((b[3] >> 3) & 1) << 5) | (((b[0] >> 2) & 1) << (2 ^ 4)) | (((b[1] >> 2) & 1) << (3 ^ 4)) | (((b[2] >> 2) & 1) << (4 ^ 4)) | (((b[3] >> 2) & 1) << (5 ^ 4));
		dst_buff[led * 12 + 11] = (((b[0] >> 1) & 1) << 2) | (((b[1] >> 1) & 1) << 3) | (((b[2] >> 1) & 1) << 4) | (((b[3] >> 1) & 1) << 5) | (((b[0] >> 0) & 1) << (2 ^ 4)) | (((b[1] >> 0) & 1) << (3 ^ 4)) | (((b[2] >> 0) & 1) << (4 ^ 4)) | (((b[3] >> 0) & 1) << (5 ^ 4));

		// 4 x 256 x 3 = 3072bytes
		// LINE EFGHのLEDデータ
		dst_buff[3072 + led * 12] = (((g[4] >> 7) & 1) << 2) | (((g[5] >> 7) & 1) << 3) | (((g[6] >> 7) & 1) << 4) | (((g[7] >> 7) & 1) << 5) | (((g[4] >> 6) & 1) << (2 ^ 4)) | (((g[5] >> 6) & 1) << (3 ^ 4)) | (((g[6] >> 6) & 1) << (4 ^ 4)) | (((g[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 1] = (((g[4] >> 5) & 1) << 2) | (((g[5] >> 5) & 1) << 3) | (((g[6] >> 5) & 1) << 4) | (((g[7] >> 5) & 1) << 5) | (((g[4] >> 4) & 1) << (2 ^ 4)) | (((g[5] >> 4) & 1) << (3 ^ 4)) | (((g[6] >> 4) & 1) << (4 ^ 4)) | (((g[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 2] = (((g[4] >> 3) & 1) << 2) | (((g[5] >> 3) & 1) << 3) | (((g[6] >> 3) & 1) << 4) | (((g[7] >> 3) & 1) << 5) | (((g[4] >> 2) & 1) << (2 ^ 4)) | (((g[5] >> 2) & 1) << (3 ^ 4)) | (((g[6] >> 2) & 1) << (4 ^ 4)) | (((g[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 3] = (((g[4] >> 1) & 1) << 2) | (((g[5] >> 1) & 1) << 3) | (((g[6] >> 1) & 1) << 4) | (((g[7] >> 1) & 1) << 5) | (((g[4] >> 0) & 1) << (2 ^ 4)) | (((g[5] >> 0) & 1) << (3 ^ 4)) | (((g[6] >> 0) & 1) << (4 ^ 4)) | (((g[7] >> 0) & 1) << (5 ^ 4));

		dst_buff[3072 + led * 12 + 4] = (((r[4] >> 7) & 1) << 2) | (((r[5] >> 7) & 1) << 3) | (((r[6] >> 7) & 1) << 4) | (((r[7] >> 7) & 1) << 5) | (((r[4] >> 6) & 1) << (2 ^ 4)) | (((r[5] >> 6) & 1) << (3 ^ 4)) | (((r[6] >> 6) & 1) << (4 ^ 4)) | (((r[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 5] = (((r[4] >> 5) & 1) << 2) | (((r[5] >> 5) & 1) << 3) | (((r[6] >> 5) & 1) << 4) | (((r[7] >> 5) & 1) << 5) | (((r[4] >> 4) & 1) << (2 ^ 4)) | (((r[5] >> 4) & 1) << (3 ^ 4)) | (((r[6] >> 4) & 1) << (4 ^ 4)) | (((r[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 6] = (((r[4] >> 3) & 1) << 2) | (((r[5] >> 3) & 1) << 3) | (((r[6] >> 3) & 1) << 4) | (((r[7] >> 3) & 1) << 5) | (((r[4] >> 2) & 1) << (2 ^ 4)) | (((r[5] >> 2) & 1) << (3 ^ 4)) | (((r[6] >> 2) & 1) << (4 ^ 4)) | (((r[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 7] = (((r[4] >> 1) & 1) << 2) | (((r[5] >> 1) & 1) << 3) | (((r[6] >> 1) & 1) << 4) | (((r[7] >> 1) & 1) << 5) | (((r[4] >> 0) & 1) << (2 ^ 4)) | (((r[5] >> 0) & 1) << (3 ^ 4)) | (((r[6] >> 0) & 1) << (4 ^ 4)) | (((r[7] >> 0) & 1) << (5 ^ 4));

		dst_buff[3072 + led * 12 + 8] = (((b[4] >> 7) & 1) << 2) | (((b[5] >> 7) & 1) << 3) | (((b[6] >> 7) & 1) << 4) | (((b[7] >> 7) & 1) << 5) | (((b[4] >> 6) & 1) << (2 ^ 4)) | (((b[5] >> 6) & 1) << (3 ^ 4)) | (((b[6] >> 6) & 1) << (4 ^ 4)) | (((b[7] >> 6) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 9] = (((b[4] >> 5) & 1) << 2) | (((b[5] >> 5) & 1) << 3) | (((b[6] >> 5) & 1) << 4) | (((b[7] >> 5) & 1) << 5) | (((b[4] >> 4) & 1) << (2 ^ 4)) | (((b[5] >> 4) & 1) << (3 ^ 4)) | (((b[6] >> 4) & 1) << (4 ^ 4)) | (((b[7] >> 4) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 10] = (((b[4] >> 3) & 1) << 2) | (((b[5] >> 3) & 1) << 3) | (((b[6] >> 3) & 1) << 4) | (((b[7] >> 3) & 1) << 5) | (((b[4] >> 2) & 1) << (2 ^ 4)) | (((b[5] >> 2) & 1) << (3 ^ 4)) | (((b[6] >> 2) & 1) << (4 ^ 4)) | (((b[7] >> 2) & 1) << (5 ^ 4));
		dst_buff[3072 + led * 12 + 11] = (((b[4] >> 1) & 1) << 2) | (((b[5] >> 1) & 1) << 3) | (((b[6] >> 1) & 1) << 4) | (((b[7] >> 1) & 1) << 5) | (((b[4] >> 0) & 1) << (2 ^ 4)) | (((b[5] >> 0) & 1) << (3 ^ 4)) | (((b[6] >> 0) & 1) << (4 ^ 4)) | (((b[7] >> 0) & 1) << (5 ^ 4));
	}
}

void Converter::convert8LINE(ofFile & src_file, ofFile & dst_file, unsigned int & dst_size) {
	size_t pos = 0;
	size_t file_size = src_file.getSize();
	memset(next, 1, MovieToLedUtils::BUFF_SIZE);
	memset(prev, 1, MovieToLedUtils::BUFF_SIZE);
	// M5LEDのヘッダーと1フレーム目(0,0,0のデータ)を飛ばす
	src_file.seekg(0);
	src_file.seekg(MovieToLedUtils::BUFF_SIZE * 2);
	pos = MovieToLedUtils::BUFF_SIZE * 2;
	// 1フレーム目は0,0,0
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	for (int i = 0; i < LedProduct::Device::MAX_NUM_LINE; i++) {
		regulatorOn(src, i);
	}
	convertRGB8LINE(src, dst);
	// ファイルへ書き込み
	dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
	// 2フレーム目以降
	while (src_file.read(reinterpret_cast<char *>(next), MovieToLedUtils::BUFF_SIZE)) {
		if (pos == MovieToLedUtils::BUFF_SIZE * 2) {
			memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
			memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
			pos += MovieToLedUtils::BUFF_SIZE;
			continue;
		} else if (pos <= file_size) {
			for (uint8_t num_line = 0; num_line < LedProduct::Device::MAX_NUM_LINE; num_line++) {
				bool is_prev_black = true;
				bool is_black = true;
				bool is_next_black = true;
				unsigned short head_pos = LedProduct::Device::MAX_NUM_LED * 3 * num_line;
				for (int num_led = 0; num_led < LedProduct::Device::MAX_NUM_LED; num_led++) {
					if (is_prev_black) is_prev_black = isBlack(prev, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (is_black) is_black = isBlack(src, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (is_next_black) is_next_black = isBlack(next, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (!is_prev_black && !is_black && !is_next_black) break;
				}
				if (is_black) {
					if (is_next_black) {

					} else {
						if (is_prev_black) {
							// 1フレーム前はRGB = (0, 0, 0)、1フレーム後はRGB != (0, 0, 0)ならRegulator ON
							regulatorOn(src, num_line);
						} else {
						}
					}
				} else {
					// LEDが点灯しているときはRegulator ON
					regulatorOn(src, num_line);
				}
			}
		}
		convertRGB8LINE(src, dst);
		// ファイルへ書き込み
		dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
		memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
		memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
		pos += MovieToLedUtils::BUFF_SIZE;
	}
	// 最終フレーム
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	for (uint8_t i = 0; i < LedProduct::Device::MAX_NUM_LINE; i++) {
		regulatorOn(src, i);
	}
	// ファイルへ書き込み
	dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
	dst_file.flush();
	dst_size = dst_file.getSize() / MovieToLedUtils::BUFF_SIZE;
	dst_file.close();
	src_file.clear();
	src_file.seekg(0);
}

void Converter::convert1000FPS(ofFile & src_file, ofFile & dst_file, unsigned int & dst_size) {
	// レギュレーターONのデータは必要ない
	size_t pos = 0;
	size_t file_size = src_file.getSize();
	memset(next, 1, MovieToLedUtils::BUFF_SIZE);
	memset(prev, 1, MovieToLedUtils::BUFF_SIZE);
	// M5LEDのヘッダーと1フレーム目(0,0,0のデータ)を飛ばす
	src_file.seekg(MovieToLedUtils::BUFF_SIZE * 2);
	pos = MovieToLedUtils::BUFF_SIZE * 2;
	// 1フレーム目は0,0,0
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	convertRGB8LINE(src, dst);
	// ファイルへ書き込み
	dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
	// 2フレーム目以降
	while (src_file.read(reinterpret_cast<char *>(next), MovieToLedUtils::BUFF_SIZE)) {
		if (pos == MovieToLedUtils::BUFF_SIZE * 2) {
			memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
			memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
			pos += MovieToLedUtils::BUFF_SIZE;
			continue;
		} else if (pos <= file_size) {
		}
		convertRGB8LINE(src, dst);
		// ファイルへ書き込み
		dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
		memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
		memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
		pos += MovieToLedUtils::BUFF_SIZE;
	}
	// 最終フレーム
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	convertRGB8LINE(src, dst);
	// ファイルへ書き込み
	dst_file.write((const char *)(dst), MovieToLedUtils::BUFF_SIZE);
	dst_file.flush();
	dst_size = dst_file.getSize() / MovieToLedUtils::BUFF_SIZE;
	dst_file.close();
	src_file.clear();
	src_file.seekg(0);
}

void Converter::convert4LINE(ofFile & src_file, ofFile * dst_file, unsigned int * dst_size, int num_dst) {
	size_t pos = 0;
	size_t file_size = src_file.getSize();
	memset(next, 1, MovieToLedUtils::BUFF_SIZE);
	memset(prev, 1, MovieToLedUtils::BUFF_SIZE);
	// M5LEDのヘッダーと1フレーム目(0,0,0のデータ)を飛ばす
	src_file.seekg(MovieToLedUtils::BUFF_SIZE * 2);
	pos = MovieToLedUtils::BUFF_SIZE * 2;
	// 1フレーム目は0,0,0
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	for (uint8_t i = 0; i < LedProduct::Device::MAX_NUM_LINE; i++) {
		regulatorOn(src, i);
	}
	convertRGB4LINE(src, dst);
	// ファイルへ書き込み
	for (int i = 0; i < num_dst; i++) {
		dst_file[i].write((const char *)(&dst[3072 * i]), MovieToLedUtils::BUFF_SIZE / 2);
	}
	// 2フレーム目以降
	while (src_file.read(reinterpret_cast<char *>(next), MovieToLedUtils::BUFF_SIZE)) {
		if (pos == MovieToLedUtils::BUFF_SIZE * 2) {
			memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
			memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
			pos += MovieToLedUtils::BUFF_SIZE;
			continue;
		} else if (pos <= file_size) {
			for (uint8_t num_line = 0; num_line < LedProduct::Device::MAX_NUM_LINE; num_line++) {
				bool is_prev_black = true;
				bool is_black = true;
				bool is_next_black = true;
				unsigned short head_pos = LedProduct::Device::MAX_NUM_LED * 3 * num_line;
				for (int num_led = 0; num_led < LedProduct::Device::MAX_NUM_LED; num_led++) {
					if (is_prev_black) is_prev_black = isBlack(prev, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (is_black) is_black = isBlack(src, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (is_next_black) is_next_black = isBlack(next, head_pos + num_led, MovieToLedUtils::BUFF_SIZE);
					if (!is_prev_black && !is_black && !is_next_black) break;
				}
				if (is_black) {
					if (is_next_black) {

					} else {
						if (is_prev_black) {
							// 1フレーム前はRGB = (0, 0, 0)、1フレーム後はRGB != (0, 0, 0)ならRegulator ON
							regulatorOn(src, num_line);
						} else {
						}
					}
				} else {
					// LEDが点灯しているときはRegulator ON
					regulatorOn(src, num_line);
				}
			}
		}
		convertRGB4LINE(src, dst);
		// ファイルへ書き込み
		// 3072バイトずつ書き込み
		for (int i = 0; i < num_dst; i++) {
			dst_file[i].write((const char *)(&dst[3072 * i]), MovieToLedUtils::BUFF_SIZE / 2);
		}
		memcpy(prev, src, MovieToLedUtils::BUFF_SIZE);
		memcpy(src, next, MovieToLedUtils::BUFF_SIZE);
		pos += MovieToLedUtils::BUFF_SIZE;
	}
	// 最終フレーム
	memset(src, 0, MovieToLedUtils::BUFF_SIZE);
	for (uint8_t i = 0; i < LedProduct::Device::MAX_NUM_LINE; i++) {
		regulatorOn(src, i);
	}
	// ファイルへ書き込み
	// 3072バイトずつ書き込み
	for (int i = 0; i < num_dst; i++) {
		dst_file[i].write((const char *)(&dst[3072 * i]), MovieToLedUtils::BUFF_SIZE / 2);
		dst_file[i].flush();
		dst_size[i] = dst_file[i].getSize() / (MovieToLedUtils::BUFF_SIZE / 2);
		dst_file[i].close();
	}
	src_file.clear();
	src_file.seekg(0);
}

void Converter::createBIN(uint8_t sound_num) {
	if (isValid()) {
		std::string product_dir = output_dir + "/BIN/" + output_data->led_product.getName();
		if (MovieToLedUtils::MtLStates::isOutputBin()) {
			convert_count = 0;
			max_convert_count = static_cast<int>(output_data->output_files.size());
			if (MovieToLedUtils::MtLStates::Converter::output_8line_bin) createBIN8LINE(product_dir, sound_num);
			if (MovieToLedUtils::MtLStates::Converter::output_4line_bin) createBIN4LINE(product_dir, sound_num);
		}
	}
}

bool Converter::convert(MovieToLedUtils::OutputFiles & output_files, uint8_t sound_number, uint16_t product_id, uint16_t device_id) {
	char m5led[29];
	snprintf(m5led, 29, "%05d-%05d/%02X_%03d-%03d.M5LED", (product_id >> 8) << 8, ((product_id >> 8) << 8) + 0xff, sound_number, product_id & 0xff, device_id & 0xff);
	if (output_files.M5LED.find(m5led) != std::string::npos) {
		// printf("------------------------------\r\n");
		// printf("M5LED : %s\r\n", output_data->output_files[num_conv].M5LED.c_str());
		// printf("8LINE : %s\r\n", output_data->output_files[num_conv].BIN_DEVICE_8LINE.c_str());
		// printf("1000F : %s\r\n", output_data->output_files[num_conv].BIN_DEVICE_1000FPS.c_str());
		// printf("4LINE : %s\r\n", output_data->output_files[num_conv].BIN_DEVICE_4LINE_ABCD.c_str());
		// printf("4LINE : %s\r\n", output_data->output_files[num_conv].BIN_DEVICE_4LINE_EFGH.c_str());
		// printf("------------------------------\r\n");
		if (FileUtils::openFile(m5led_file, output_files.M5LED, ofFile::ReadOnly)) {
			std::string conv_file_name;
			unsigned int file_size[4];
			if (MovieToLedUtils::MtLStates::Converter::output_8line_bin) {
				if (FileUtils::openFile(conv_file[0], output_files.BIN_DEVICE_1000FPS, ofFile::Append) && FileUtils::openFile(conv_file[1], output_files.BIN_DEVICE_8LINE, ofFile::Append)) {
					// printf("Convert to BIN for 8LINE\r\n");
					conv_file_name = conv_file[0].getFileName();
					// ヘッダー
					// 8LINEのBINは1フレームのデータが6144(= 256 * 3 * 8)byte
					setHeader8LINE(conv_file[0], dst, MovieToLedUtils::BUFF_SIZE, num_total_frame, device_id);
					setHeader8LINE(conv_file[1], dst, MovieToLedUtils::BUFF_SIZE, num_total_frame, device_id);
					// M5LED -> BIN
					// 1000FPSのBIN
					convert1000FPS(m5led_file, conv_file[0], file_size[0]);
					// 8LINEのBIN
					convert8LINE(m5led_file, conv_file[1], file_size[1]);
				}
			}
			if (MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
				if (FileUtils::openFile(conv_file[0], output_files.BIN_DEVICE_4LINE_ABCD, ofFile::Append) && FileUtils::openFile(conv_file[1], output_files.BIN_DEVICE_4LINE_EFGH, ofFile::Append)) {
					// printf("Convert to BIN for 4LINE\r\n");
					// ヘッダー
					// 4LINEのBINは1フレームのデータが3072(= 256 * 3 * 4)byte
					setHeader4LINE_ABCD(conv_file[0], dst, MovieToLedUtils::BUFF_SIZE / 2, num_total_frame, device_id);
					setHeader4LINE_EFGH(conv_file[1], dst, MovieToLedUtils::BUFF_SIZE / 2, num_total_frame, device_id);
					// M5LED -> BIN
					// 4LINEのBIN(8ラインを4ラインずつ)
					convert4LINE(m5led_file, conv_file, &file_size[2], 2);
				}
			}
			// 変換が終了したら、ファイルサイズを比較
			// すべて同じサイズなら問題なし
			if (completed_file) {
				char completed[128];
				// printf("%d %d %d %d\r\n", file_size[0], file_size[1], file_size[2], file_size[3]);
				if (MovieToLedUtils::MtLStates::Converter::output_8line_bin && MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
					if (file_size[0] == file_size[1] && file_size[1] == file_size[2] && file_size[2] == file_size[3]) {
						snprintf(completed, 128, "%s (%d)", conv_file_name.c_str(), file_size[0]);
					} else {
						snprintf(completed, 128, "%s ERROR", conv_file_name.c_str());
					}
					completed_file->push_back(completed);
				} else if (MovieToLedUtils::MtLStates::Converter::output_8line_bin && !MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
					if (file_size[0] == file_size[1]) {
						snprintf(completed, 128, "%s (%d)", conv_file_name.c_str(), file_size[0]);
					} else {
						snprintf(completed, 128, "%s ERROR", conv_file_name.c_str());
					}
					completed_file->push_back(completed);
				} else if (!MovieToLedUtils::MtLStates::Converter::output_8line_bin && MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
					if (file_size[2] == file_size[3]) {
						snprintf(completed, 128, "%s (%d)", conv_file_name.c_str(), file_size[2]);
					} else {
						snprintf(completed, 128, "%s ERROR", conv_file_name.c_str());
					}
					completed_file->push_back(completed);
				}
			}
			return true;
		}
	}
	return false;
}

void Converter::process() {
	if (isValid()) {
		if (convert_count >= max_convert_count) return;
		uint16_t start_product_id = output_data->led_product.getStartProductId();
		uint16_t end_product_id = output_data->led_product.getEndProductId();
		uint16_t num_device = output_data->led_product.getNumDevice();
		for (uint16_t product_id = start_product_id; product_id <= end_product_id; product_id++) {
			for (uint16_t device_id = 0; device_id < num_device; device_id++) {
				if (convert(output_data->output_files[convert_count], sound_number, product_id, device_id)) {
					convert_count++;
					return;
				}
			}
		}
	}
}

uint16_t Converter::getBlockSize(uint16_t num_device, DeviceIdFormat id_format) {
	// ブロックサイズ = 1プロダクトが消費するBIN名の数
	// BINの命名規則
	// 1. BIN名のデバイスIDは2桁
	// 2. ID FormatよるデバイスIDの範囲 DEC: 00-99, HEX:00-FF
	// 3. 1プロダクト内のデバイスのBINは連番 00 01 02
	// 4. 次のプロダクトへ進むときは桁が変わる 00 10 20
	// デバイス数が2の場合00から09の範囲に収まる -> ブロックサイズ = 10(プロダクトごとに桁上がりするため)
	// デバイス数が11の場合00から19の範囲に収まる -> ブロックサイズ = 20

	if (num_device == 1) return 1; // デバイス数が1台の場合、ブロックサイズ = 1
	uint8_t base = id_format == DeviceIdFormat::HEX ? 16 : 10;
	uint8_t blocks = (num_device + (base - 1)) / base;
	return blocks * base;
}

uint16_t Converter::getMaxProductCount(uint16_t num_device, DeviceIdFormat id_format) {
	// プロダクト収容数: 同じディレクトリに入れられるBINのプロダクト数(1連のBIN名に収容できるプロダクト数)
	// ファイル名が10進数表記の場合 00-99
	// デバイス数が1 -> ブロックサイズ = 1 -> プロダクト収容数 = 100
	// デバイス数が2 - 10 -> ブロックサイズ = 10 -> プロダクト収容数 = 10
	// デバイス数が11 - 20 -> ブロックサイズ = 20 -> プロダクト収容数 = 5
	// デバイス数が21 - 30 -> ブロックサイズ = 30 -> プロダクト収容数 = 3
	// デバイス数が31 - 50 -> ブロックサイズ = 40 or 50 -> プロダクト収容数 = 2
	// デバイス数が51 - 100 -> ブロックサイズ = 60 - 100 -> プロダクト収容数 = 1

	// ファイル名が16進数表記の場合 00-FF
	// デバイス数が1 -> ブロックサイズ = 1 -> プロダクト収容数 = 255
	// デバイス数が2 - 16 -> ブロックサイズ = 16 -> プロダクト収容数 = 16
	// デバイス数が17 - 32 -> ブロックサイズ = 32 -> プロダクト収容数 = 8
	// デバイス数が33 - 48 -> ブロックサイズ = 48 -> プロダクト収容数 = 5
	// デバイス数が49 - 64 -> ブロックサイズ = 64 -> プロダクト収容数 = 4
	// デバイス数が65 - 80 -> ブロックサイズ = 80 -> プロダクト収容数 = 3
	// デバイス数が81 - 128 -> ブロックサイズ = 96 - 128 -> プロダクト収容数 = 2
	// デバイス数が129 - 256 -> ブロックサイズ = 144 - 256 -> プロダクト収容数 = 1

	if (num_device == 1) return id_format == DeviceIdFormat::HEX ? 256 : 100; // デバイス数が1台のプロダクトの場合、収容数 = 256 or 100
	uint8_t b = getBlockSize(num_device, id_format); // ブロックサイズ
	uint8_t base = id_format == DeviceIdFormat::HEX ? 16 : 10;
	uint8_t max_value = base * base - 1; //　BIN名の最大値
	uint8_t max_index = max_value - (num_device - 1); // デバイス数がBIN名の最大値を超えていないか
	if (max_index < 0) return 0; // 1プロダクトも入らない
	return (max_index / b) + 1;
}

