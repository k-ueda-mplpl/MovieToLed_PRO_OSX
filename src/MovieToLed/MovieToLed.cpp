
#include "MovieToLed.hpp"

MovieToLed::MovieToLed() {
	MovieToLedRuntimeState::error_msg.clear();
	// create OUTPUT dir
#ifdef TARGET_OSX
	string path = ofFilePath::getAbsolutePath("../");
#elif defined(TARGET_WIN32)
	string path = ofFilePath::getAbsolutePath("..\\..\\");
#endif
	output_dir_path = path + "OUTPUT";
	MovieToLedFileUtils::createDir(output_dir_path);
	// Data Setting
	data_panel.setup("LED Data Setting");
	sound_number.addListener(this, &MovieToLed::soundNumberChanged);
	play_once.addListener(this, &MovieToLed::playOnceChanged);
	play_loop.addListener(this, &MovieToLed::playLoopChanged);
	data_panel.add(sound_number.set("Sound Number", 0, 0, MovieToLedData::MAX_SOUND_NUMBER));
	data_panel.add(label_m5led_loop_playback.setup("M5LED Loop Playback?", "NO"));
	data_panel.add(play_loop.set("YES (Loop Playback)", false));
	data_panel.add(play_once.set("NO (Play Once)", true));
	// Device Map
	device_map_panel.setup("Device Map");
	btn_create_device_map.addListener(this, &MovieToLed::createDeviceMap);
	device_map_panel.add(btn_create_device_map.setup("Create DEVICE MAP"));
	// M5LEDMETADATA
	// M5デバイスのHOSTデバイスに必要
	metadata_panel.setup("M5LED Metadata");
	btn_clear_metadata.addListener(this, &MovieToLed::clearMetadata);
	metadata_panel.add(btn_clear_metadata.setup("Clear M5LED METADATA"));

	extractor.setOutputDir(output_dir_path);
	extractor.setCallback(std::bind(&MovieToLed::onExtracted, this));
	converter.setOutputDir(output_dir_path);
	device_map.setOutputDir(output_dir_path);

	UIContext::setup();
}

MovieToLed::~MovieToLed() {
}

void MovieToLed::setup() {
	// Parameter.JSONをロード
	ofxJSONElement json;
	if (json.open(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON))) {
		LedGain gain;
		sound_number = json["Parameter"]["01 Sound Number"].asInt() & 0xff;
		gain.led_white = json["Parameter"]["02 Normal LED White Gain"].asInt() & 0xff;
		gain.led_rgb = json["Parameter"]["03 Normal LED Color Gain"].asInt() & 0xff;
		gain.panel_white = json["Parameter"]["04 Panel LED White Gain"].asInt() & 0xff;
		gain.panel_rgb = json["Parameter"]["05 Panel LED Color Gain"].asInt() & 0xff;
		led_color_correction.setLedGain(gain);
	}
	loadProject();
}

void MovieToLed::setupRender(int x, int y, int width, int height, int gui_width) {
	drag_area.x = x;
	drag_area.y = y;
	drag_area.width = width;
	drag_area.height = height;
	panel_width = gui_width;

	profile_manager.setupRender(drag_area.position.x + UIContext::pc_display_size.width / 4 + 32, drag_area.y + drag_area.height + 64, panel_width);

	device_map_panel.setPosition(drag_area.position.x + UIContext::pc_display_size.width / 4 + 32, profile_manager.getBottomY());
	device_map_panel.setWidthElements(panel_width);

	data_panel.setPosition(drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 162);
	data_panel.setWidthElements(panel_width);

	metadata_panel.setPosition(data_panel.getPosition().x, data_panel.getPosition().y + data_panel.getHeight());
	metadata_panel.setWidthElements(panel_width);

	led_color_correction.setupRender(drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 100, panel_width);
}

void MovieToLed::dragEvent(ofDragInfo info) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		if (drag_area.inside(info.position.x, info.position.y)) {
			ofFile dropped_file(info.files[0]);
			if (dropped_file.isDirectory()) {
				MovieToLedFileUtils::onDroppedContent(ofDirectory(dropped_file.getAbsolutePath()), std::bind(&MovieToLed::loadProject, this));
			} else if (dropped_file.isFile() && (ofFilePath::getFileExt(dropped_file) == "mov" || ofFilePath::getFileExt(dropped_file) == "mp4")) {
				onDroppedVideo(dropped_file);
			}
		}
	}
}

void MovieToLed::start() {
	if (!MovieToLedRuntimeState::error_msg.empty()) return;
	if (MovieToLedRuntimeState::isReady()) {
		if (MovieToLedRuntimeState::process_count == 0) {
			bool found_start = false;
			bool found_end = false;
			uint8_t last_index = static_cast<uint8_t>(mtl_data.product_profiles.size());
			for (uint8_t i = 0; i < last_index; i++) {
				// データ生成する最初のプロダクトを探索
				if (!found_start) {
					if (mtl_data.product_profiles[i].generate_data) {
						curr_product_index = i;
						found_start = true;
					}
				}
				// データ生成する最後のプロダクトを探索
				if (!found_end) {
					if (mtl_data.product_profiles[(last_index - 1) - i].generate_data) {
						end_product_index = i;
						found_end = true;
					}
				}
				if (found_start && found_end) break;
			}
			if (!found_start || !found_end) return;
		}
		mtl_data.led_product.selectProfile(curr_product_index);
		extractor.createM5LED();
		m5led_metadata.set(sound_number, extractor.getTotalFrame(), extractor.isLoopPlayback() ? 1 : 0);
		m5led_metadata.write(output_dir_path + "/M5LED/" + mtl_data.led_product.getName(), sound_number);
		// video_player.update();
		// num_frame = video_player.getCurrentFrame();
		string log_black_path = output_dir_path + "/LOG/" + mtl_data.led_product.getName() + "/BlackData.csv";
		string log_skip_path = output_dir_path + "/LOG/" + mtl_data.led_product.getName() + "/SkipFrame.csv";
		string log_frame_path = output_dir_path + "/LOG/" + mtl_data.led_product.getName() + "/Frame.csv";
		string log_scene_path = output_dir_path + "/LOG/" + mtl_data.led_product.getName() + "/Scenes_" + ofToString((int)sound_number, 2, '0') + ".csv";
		extractor.createLogBlackFile(log_black_path);
		extractor.createLogSkipFile(log_skip_path);
		extractor.createLogFrameFile(log_frame_path);
		extractor.createLogScene(log_scene_path);
		extractor.ready();
		converter.completed_file.clear();
		MovieToLedRuntimeState::process_count++;
		MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::EXTRACTING;
		return;
	}
	if (MovieToLedRuntimeState::isCompleted()) {
		extractor.clearVideo();
		MovieToLedRuntimeState::process_count = 0;
		MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::WAITING;
	}
}

void MovieToLed::update() {
	if (MovieToLedRuntimeState::isExtracting()) {
		// M5LED作成
		extractor.extract();
	} else if (MovieToLedRuntimeState::isConverting()) {
		int convert_count = converter.getConvertCount();
		int max_convert_count = converter.getMaxConvertCount();
		printf("Convert BIN : %d/%d\r\n", convert_count, max_convert_count);
		if (converter.isFinish()) {
			// すべてのM5LEDのBIN変換が終了
			if (curr_product_index == end_product_index) {
				// 最後のプロダクトのデータ生成が終了
				MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::COMPLETED;
			} else {
				// 次のプロダクトのデータ生成を開始
				uint8_t prev_index = curr_product_index;
				for (uint8_t i = prev_index + 1; i < static_cast<uint8_t>(mtl_data.product_profiles.size()); i++) {
					if (mtl_data.product_profiles[i].generate_data) {
						curr_product_index = i;
					}
				}
				start();
			}
		} else {
			// M5LEDをBINに変換
			converter.process();
		}
	}
}

void MovieToLed::draw() {
	char str[128];
	ofSetBackgroundColor(0, 0, 0);
	if (!MovieToLedRuntimeState::error_msg.empty()) {
		// エラー
		ofSetBackgroundColor(228, 0, 6);
		ofSetColor(255, 255, 255, 255);
		UIContext::Font::Large.drawString(MovieToLedRuntimeState::error_msg, 10, UIContext::pc_display_size.height / 2);
		MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::WAITING;
		return;
	}
	if (MovieToLedRuntimeState::isCompleted()) {
		// BINデータ変換終了
		ofSetBackgroundColor(14, 167, 39);
		UIContext::Font::Large.drawString("Completed All Product Data", drag_area.position.x, drag_area.position.y + 58);
		UIContext::Font::Large.drawString("Press SPACE Key to Continue MtL", drag_area.position.x, drag_area.position.y + 168);
		for (int i = 0; i < static_cast<int>(converter.completed_file.size()); i++) {
			UIContext::Font::Tiny.drawString(converter.completed_file[i], drag_area.position.x + 200 * (i / 30), drag_area.position.y + 208 + 24 * (i % 30));
		}
	} else if (MovieToLedRuntimeState::isConverting()) {
		// BINデータ変換中
		string product_name = mtl_data.led_product.getName();
		uint16_t start_product_id = mtl_data.led_product.getStartProductId();
		uint16_t end_product_id = mtl_data.led_product.getEndProductId();
		uint16_t num_device = mtl_data.led_product.getNumDevice();
		string device_type = mtl_data.led_product.getDeviceType() == ProductProfile::DeviceType::LINE4 ? "4Line" : "8Line";
		string format = mtl_data.led_product.getDeviceIdFormat() == ProductProfile::DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
		string video_file_name = extractor.getVideoFileName();
		uint16_t frame = extractor.getCurrentFrame();
		uint16_t total_frame = extractor.getTotalFrame();
		uint8_t duration_min = extractor.getDurationMin();
		uint8_t duration_sec = extractor.getDurationSec();

		ofSetBackgroundColor(14, 167, 39);
		UIContext::Font::Middle.drawString(product_name + " Convert to BIN", drag_area.position.x, drag_area.position.y + 32);
		snprintf(str, 128, "Product ID Range : %d - %d / Devices per Product : %d\r\n", start_product_id, end_product_id, num_device);
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 58);
		UIContext::Font::Small.drawString("Device Type : " + device_type + " / ID Format : " + format, drag_area.position.x, drag_area.position.y + 84);
		snprintf(str, 128, "Video : %s", video_file_name.c_str());
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 110);
		snprintf(str, 128, "Frame : %d / %d (%02d:%02d)", frame, total_frame, duration_min, duration_sec);
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 136);
		snprintf(str, 32, "Completed BIN File: %llu", converter.completed_file.size());
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 178);
		for (int i = 0; i < static_cast<int>(converter.completed_file.size()); i++) {
			UIContext::Font::Tiny.drawString(converter.completed_file[i], drag_area.position.x + 200 * (i / 30), drag_area.position.y + 208 + 24 * (i % 30));
		}
	} else if (MovieToLedRuntimeState::isExtracting()) {
		// M5LED生成中
		string product_name = mtl_data.led_product.getName();
		uint16_t start_product_id = mtl_data.led_product.getStartProductId();
		uint16_t end_product_id = mtl_data.led_product.getEndProductId();
		uint16_t num_device = mtl_data.led_product.getNumDevice();
		string video_file_name = extractor.getVideoFileName();
		uint16_t frame = extractor.getCurrentFrame();
		uint16_t total_frame = extractor.getTotalFrame();
		uint8_t duration_min = extractor.getDurationMin();
		uint8_t duration_sec = extractor.getDurationSec();

		extractor.video_image.draw(0, 0);
		ofSetColor(255, 255, 255, 255);
		UIContext::Font::Middle.drawString(product_name + " Create M5LED", drag_area.position.x, drag_area.position.y + 32);
		snprintf(str, 128, "Product ID Range : %d - %d / Devices per Product : %d\r\n", start_product_id, end_product_id, num_device);
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 58);
		snprintf(str, 128, "Sound Number : %d", (int)sound_number);
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 84);
		UIContext::Font::Small.drawString(extractor.isLoopPlayback() ? "M5LED Loop Playback : ON" : "M5LED Loop Playback : OFF", drag_area.position.x, drag_area.position.y + 110);
		if (mtl_data.led_product.isOutputBin()) {
			string device_type = mtl_data.led_product.getDeviceType() == ProductProfile::DeviceType::LINE4 ? "4Line" : "8Line";
			string format = mtl_data.led_product.getDeviceIdFormat() == ProductProfile::DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
			UIContext::Font::Small.drawString("BIN : Device Type : " + device_type + " / ID Format : " + format, drag_area.position.x, drag_area.position.y + 136);
		}
		snprintf(str, 128, "Video : %s", video_file_name.c_str());
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 162);
		snprintf(str, 128, "Frame : %d / %d (%02d:%02d)", frame, total_frame, duration_min, duration_sec);
		UIContext::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 188);
		extractor.drawLogSkip(UIContext::pc_display_size.width - 340, UIContext::pc_display_size.height / 2 + 40);
	} else if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		// 処理前
		string video_file_name = extractor.getVideoFileName();
		uint16_t total_frame = extractor.getTotalFrame();
		uint8_t duration_min = extractor.getDurationMin();
		uint8_t duration_sec = extractor.getDurationSec();
		if (extractor.isLoadedVideo()) {
			// 動画ロード完了
			extractor.video_image.draw(0, 0);
		}
		ofSetColor(36, 36, 36, 128);
		ofDrawRectangle(0, 0, UIContext::display_size.width, UIContext::display_size.height);
		ofSetColor(255, 255, 255);
		ofNoFill();
		ofDrawRectangle(drag_area);
		ofFill();
		ofSetColor(255, 255, 255, 255);
		UIContext::Font::Large.drawString("Drag & Drop Here", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 32);
		UIContext::Font::Small.drawString("Drag & Drop a Directory (Formation SAVEDATA/csv) to Load New MtL Contents", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 96);
		UIContext::Font::Small.drawString("Drag & Drop a MOV or MP4 File to Load New Video", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 128);

		UIContext::Font::Middle.drawString("Products", drag_area.x, drag_area.y + drag_area.height + 48);
		if (mtl_data.product_profiles.empty()) {
			UIContext::Font::Small.drawString("No Products", drag_area.x, drag_area.y + drag_area.height + 48 + 26);
		} else {
			if (profile_manager.isExist(profile_manager.getProfileIndex())) {
				UIContext::Font::Middle.drawString("Product Profile", drag_area.position.x + UIContext::pc_display_size.width / 4 + 32, drag_area.y + drag_area.height + 48);
			} else {
				UIContext::Font::Middle.drawString("Product Profile : New Product", drag_area.position.x + UIContext::pc_display_size.width / 4 + 32, drag_area.y + drag_area.height + 48);
			}
			profile_manager.draw();
			device_map_panel.draw();

			for (int i = 0; i < static_cast<int>(mtl_data.product_profiles.size()); i++) {
				string product_name = mtl_data.product_profiles[i].product_name;
				bool color_change = false;
				if (!mtl_data.product_profiles[i].generate_data) {
					color_change = true;
					ofSetColor(192, 186, 162);
				}
				if (i == profile_manager.getProfileIndex()) {
					color_change = true;
					ofSetColor(254, 128, 51);
				}
				UIContext::Font::Small.drawString(ofToString(i + 1) + ". " + product_name, drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 84 * i);
				if (profile_manager.isExist(i)) {
					uint16_t num_product = mtl_data.product_profiles[i].num_product;
					uint16_t num_device = mtl_data.product_profiles[i].num_device;
					uint16_t start_product_id = mtl_data.product_profiles[i].start_product_id;
					uint16_t end_product_id = mtl_data.product_profiles[i].end_product_id;
					UIContext::Font::Tiny.drawString(ofToString(num_product) + " Products x " + ofToString(num_device) + " Devices", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 18 + 84 * i);
					if (mtl_data.product_profiles[i].generate_data) {
						if (start_product_id == 0 && end_product_id == num_product - 1) {
							UIContext::Font::Tiny.drawString("Product ID Range : " + ofToString(start_product_id) + " - " + ofToString(end_product_id) + " : All Product ID", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
						} else {
							UIContext::Font::Tiny.drawString("Product ID Range : " + ofToString(start_product_id) + " - " + ofToString(end_product_id), drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
						}
						if (mtl_data.product_profiles[i].isOutputBin()) {
							std::string device_type = mtl_data.product_profiles[i].device_type == ProductProfile::DeviceType::LINE4 ? "4Line" : "8Line";
							std::string format = mtl_data.product_profiles[i].device_id_format == ProductProfile::DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
							UIContext::Font::Tiny.drawString("BIN : Device Type : " + device_type + " / ID Format : " + format, drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 54 + 84 * i);
						}
					} else {
						UIContext::Font::Tiny.drawString("Data Generation : OFF. No LED Data will be Generated.", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
					}
				} else {
					UIContext::Font::Tiny.drawString("New Product. Enter Product Configuration.", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 18 + 84 * i);
				}
				if (color_change) {
					ofSetColor(255, 255, 255);
				}
			}
		}
		if (mtl_data.product_profiles.empty() || MovieToLedRuntimeState::config_all_exist) {
			UIContext::Font::Middle.drawString("Video", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48);
			snprintf(str, 128, "Name : %s", video_file_name.c_str());
			UIContext::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 26);
			snprintf(str, 128, "Duration : %d Frame (%02d:%02d)", total_frame, duration_min, duration_sec);
			UIContext::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 52);

			UIContext::Font::Middle.drawString("Data Setting", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 100);
			snprintf(str, 128, "Sound Number : %d", (int)sound_number);
			UIContext::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 126);
			UIContext::Font::Small.drawString(extractor.isLoopPlayback() ? "M5LED Loop Playback : ON" : "M5LED Loop Playback : OFF", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 152);

			data_panel.draw();
			metadata_panel.draw();
			led_color_correction.draw();

			if (MovieToLedRuntimeState::isReady()) {
				// 動画ロード完了 and .conf存在
				ofSetColor(255, 255, 255, 255);
				UIContext::Font::Large.drawString("Press SPACE Key Start MtL", 10, UIContext::pc_display_size.height * 4 / 5);
				if (mtl_data.led_product.isOutputBin()) {
					snprintf(str, 128, "Create: %02X_XXX-XXX.M5LED and %05X_XX.BIN File", (int)sound_number, (int)sound_number);
					UIContext::Font::Middle.drawString(str, 10, UIContext::pc_display_size.height * 4 / 5 + 60);
					if (MovieToLedRuntimeState::m5led_already_exists) {
						snprintf(str, 128, "Warnig: %02X_XXX-XXX.M5LED and %05X_XX.BIN File will be Overwritten", (int)sound_number, (int)sound_number);
						UIContext::Font::Middle.drawString(str, 10, UIContext::pc_display_size.height * 4 / 5 + 110);
					}
				} else {
					snprintf(str, 128, "Create: %02X_XXX-XXX.M5LED", (int)sound_number);
					UIContext::Font::Middle.drawString(str, 10, UIContext::pc_display_size.height * 4 / 5 + 60);
					if (MovieToLedRuntimeState::m5led_already_exists) {
						snprintf(str, 128, "Warnig: %02X_XXX-XXX.M5LED will be Overwritten", (int)sound_number);
						UIContext::Font::Middle.drawString(str, 10, UIContext::pc_display_size.height * 4 / 5 + 110);
					}
				}
			} else if (!extractor.isLoadedVideo()) {
				ofSetColor(255, 255, 255, 255);
				UIContext::Font::Middle.drawString("Press SHIFT Key Switch Window Size FULL HD or UHD 4K", 10, UIContext::pc_display_size.height * 4 / 5);
			}
		}
	}
	// 共通
	ofSetColor(255, 255, 255, 255);
	UIContext::Font::Small.drawString("MovieToLED PRO Ver." SOFTWARE_VER, drag_area.position.x, drag_area.position.y - 8);
	UIContext::Font::Small.drawString(UIContext::display_mode, drag_area.position.x + drag_area.width - 96, drag_area.position.y - 8);
}

void MovieToLed::switchDisplaySize() {
	if (MovieToLedRuntimeState::runtime_state == MovieToLedRuntimeState::RuntimeState::WAITING) {
		int width = UIContext::display_size.width;
		if (width == UIContext::FULL_HD.width) {
			// FULL HD -> 4K
			UIContext::setDisplaySize(UIContext::UHD_4K);
			UIContext::display_mode = "UHD 4K";
		} else if (width == UIContext::UHD_4K.width) {
			// 4K -> FULL HD
			UIContext::setDisplaySize(UIContext::FULL_HD);
			UIContext::display_mode = "FULL HD";
		}
		extractor.allocate(UIContext::display_size.width, UIContext::display_size.height);
	}
}

void MovieToLed::saveParameter() {
	ofxJSONElement json;
	json.open(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON));
	LedGain led_gain = led_color_correction.getLedGain();
	json["Parameter"]["01 Sound Number"] = (int)sound_number;
	json["Parameter"]["02 Normal LED White Gain"] = led_gain.led_white;
	json["Parameter"]["03 Normal LED Color Gain"] = led_gain.led_rgb;
	json["Parameter"]["04 Panel LED White Gain"] = led_gain.panel_white;
	json["Parameter"]["05 Panel LED Color Gain"] = led_gain.panel_rgb;
	char update_date[20];
	snprintf(update_date, 20, "%04d/%02d/%02d %02d:%02d:%02d", ofGetYear(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
	json["Parameter"]["06 LAST UPDATE"] = (string)update_date;
	json.save(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON), true);
}

void MovieToLed::exit() {
	saveParameter();
	profile_manager.saveProductSetting();
	extractor.clearVideo();
	MovieToLedFileUtils::remove(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
}

void MovieToLed::loadProject() {
	if (ProductProfileLoader::loadProductProfile(mtl_data.product_profiles)) {
		ProductProfileLoader::loadProductDeviceProfile(mtl_data.product_profiles, profile_manager);
		if (!ProductProfileLoader::loadPreviousProductProfileSetting(mtl_data.product_profiles)) {
			clearMetadata();
		}
		profile_manager.selectProfile(0);

		if (MovieToLedRuntimeState::config_all_exist) {
			if (extractor.isLoadedVideo()) {
				for (auto & profile : mtl_data.product_profiles) {
					MovieToLedRuntimeState::m5led_already_exists = extractor.isExistM5LED(profile, sound_number);
					if (MovieToLedRuntimeState::m5led_already_exists) break;
				}
			}
		}

		if (MovieToLedRuntimeState::config_all_exist && extractor.isLoadedVideo()) {
			MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::READY;
		}

		if (mtl_data.product_profiles.empty()) {
			printf("No Products\r\n");
		} else {
			for (int i = 0; i < static_cast<int>(mtl_data.product_profiles.size()); i++) {
				std::string product_name = mtl_data.product_profiles[i].product_name;
				uint16_t num_product = mtl_data.product_profiles[i].num_product;
				uint16_t num_device = mtl_data.product_profiles[i].num_device;
				printf("Product Name:%s : %d Products x %d Devices\r\n", product_name.c_str(), num_product, num_device);
			}
		}
	}
}

void MovieToLed::onDroppedVideo(const ofFile file) {
	// Windows 動画ファイルをdataディレクトリに入れる必要あり
	string file_name = file.getFileName();
	// Videoディレクトリ生成
	MovieToLedFileUtils::createDir(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
	if (extractor.isLoadedVideo()) {
		extractor.clearVideo();
	}
	// Videoディレクトリ内のファイルを削除
	MovieToLedFileUtils::remove(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
	// ファイルをコピー
	ofFile::copyFromTo(file.getAbsolutePath(), MovieToLedUtils::DirPaths::VIDEO_DIR, true, true);
	string path = ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR) + "/" + file_name;
	extractor.loadVideo(path);

	if (MovieToLedRuntimeState::config_all_exist) {
		if (extractor.isLoadedVideo()) {
			for (int i = 0; i < static_cast<int>(mtl_data.product_profiles.size()); i++) {
				MovieToLedRuntimeState::m5led_already_exists = extractor.isExistM5LED(mtl_data.product_profiles[i], sound_number);
				if (MovieToLedRuntimeState::m5led_already_exists) break;
			}
		}
	}

	if (MovieToLedRuntimeState::config_all_exist && extractor.isLoadedVideo()) {
		MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::READY;
	}
}

void MovieToLed::onExtracted() {
	// M5LED生成完了後
	printf("Finish Create M5LED Process\r\n");
	if (mtl_data.led_product.isOutputBin()) {
		// BIN変換に移行する場合
		converter.createBIN();
		printf("%s Data Start Convert\r\n", mtl_data.led_product.getName().c_str());
		uint8_t index = mtl_data.led_product.getProfileIndex();
		device_map.create(mtl_data.product_profiles[index]);
		MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::CONVERTING;
	} else {
		// すべてのM5LEDのBIN変換が終了
		if (curr_product_index == end_product_index) {
			// 最後のプロダクトのデータ生成が終了
			MovieToLedRuntimeState::runtime_state = MovieToLedRuntimeState::RuntimeState::COMPLETED;
		} else {
			// 次のプロダクトのデータ生成を開始
			uint8_t prev_index = curr_product_index;
			for (uint8_t i = prev_index + 1; i < static_cast<uint8_t>(mtl_data.product_profiles.size()); i++) {
				if (mtl_data.product_profiles[i].generate_data) {
					curr_product_index = i;
				}
			}
			start();
		}
	}
}

void MovieToLed::createDeviceMap() {
	uint8_t index = profile_manager.getProfileIndex();
	if (profile_manager.isExist(index)) {
		device_map.create(mtl_data.product_profiles[index]);
	}
}

void MovieToLed::clearMetadata() {
	if (mtl_data.product_profiles.empty()) return;
	for (auto & profile : mtl_data.product_profiles) {
		m5led_metadata.clear(output_dir_path + "/M5LED/" + profile.product_name);
	}
}

void MovieToLed::soundNumberChanged(uint8_t & num) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		extractor.setSoundNumber(num);
		converter.setSoundNumber(num);
		if (!mtl_data.product_profiles.empty()) {
			for (auto & profile : mtl_data.product_profiles) {
				MovieToLedRuntimeState::m5led_already_exists = extractor.isExistM5LED(profile, num);
				if (MovieToLedRuntimeState::m5led_already_exists) break;
			}
		}
	}
}

void MovieToLed::playOnceChanged(bool & is_once) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		if (is_once) {
			play_loop = false;
			label_m5led_loop_playback = "NO";
			extractor.setLoopPlayback(false);
		} else {
			if (!play_loop) is_once = true;
		}
	}
}

void MovieToLed::playLoopChanged(bool & is_loop) {
	if (MovieToLedRuntimeState::isWaiting() || MovieToLedRuntimeState::isReady()) {
		if (is_loop) {
			play_once = false;
			label_m5led_loop_playback = "YES";
			extractor.setLoopPlayback(true);
		} else {
			if (!play_once) is_loop = true;
		}
	}
}
