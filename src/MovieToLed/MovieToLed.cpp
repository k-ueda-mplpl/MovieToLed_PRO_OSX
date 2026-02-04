//
//  MovieToLed.cpp
//  MovieToLED
//
//  Created by ueda on 2025/03/04.
//

#include "MovieToLed.hpp"

MovieToLed::MovieToLed() {
	MovieToLedUtils::error_msg.clear();
	// create OUTPUT dir
#ifdef TARGET_OSX
	string path = ofFilePath::getAbsolutePath("../");
#elif defined(TARGET_WIN32)
	string path = ofFilePath::getAbsolutePath("..\\..\\");
#endif
	output_dir_path = path + "OUTPUT";
	FileUtils::createDir(output_dir_path);
	// Data Setting
	data_panel.setup("LED Data Setting");
	sound_number.addListener(this, &MovieToLed::soundNumberChanged);
	play_once.addListener(this, &MovieToLed::playOnceChanged);
	play_loop.addListener(this, &MovieToLed::playLoopChanged);
	data_panel.add(sound_number.set("Sound Number", 0, 0, 10));
	data_panel.add(label_m5led_loop_playback.setup("M5LED Loop Playback?", "NO"));
	data_panel.add(play_loop.set("YES (Loop Playback)", false));
	data_panel.add(play_once.set("NO (Play Once)", true));
	// Product Setting
	// onLoadContents()でUIを追加する
	product_panel.setup("Product Data Setting");
	// Device Map
	device_map_panel.setup("Device Map");
	btn_create_device_map.addListener(this, &MovieToLed::createDeviceMap);
	device_map_panel.add(btn_create_device_map.setup("Create DEVICE MAP"));
	// M5LEDMETADATA
	// M5デバイスのHOSTデバイスに必要
	metadata_panel.setup("M5LED Metadata");
	btn_clear_metadata.addListener(this, &MovieToLed::clearMetadata);
	metadata_panel.add(btn_clear_metadata.setup("Clear M5LED METADATA"));
	// BIN Output
	bin_panel.setup("BIN Output");
	output_8line_bin.addListener(this, &MovieToLed::output8LineBinChanged);
	not_output_8line_bin.addListener(this, &MovieToLed::notOutput8LineBinChanged);
	output_4line_bin.addListener(this, &MovieToLed::output4LineBinChanged);
	not_output_4line_bin.addListener(this, &MovieToLed::notOutput4LineBinChanged);
	bin_panel.add(label_output_8line_bin.setup("Output 8Line BIN?", "YES"));
	bin_panel.add(output_8line_bin.set("YES", true));
	bin_panel.add(not_output_8line_bin.set("NO", false));
	bin_panel.add(label_output_4line_bin.setup("Output 4Line BIN?", "YES"));
	bin_panel.add(output_4line_bin.set("YES", true));
	bin_panel.add(not_output_4line_bin.set("NO", false));
	
	// RGB Curve Plot
	param_panel.setup("RGB Parameter");
	slider_led_white_gain.addListener(this, &MovieToLed::sliderLedWhiteGainChanged);
	slider_led_rgb_gain.addListener(this, &MovieToLed::sliderLedRgbGainChanged);
	slider_panel_white_gain.addListener(this, &MovieToLed::sliderPanelWhiteGainChanged);
	slider_panel_rgb_gain.addListener(this, &MovieToLed::sliderPanelRgbGainChanged);
	param_panel.add(slider_led_white_gain.setup("Normal LED White Gain", 100, 0, 100));
	param_panel.add(slider_led_rgb_gain.setup("Normal LED Color Gain", 100, 0, 100));
	param_panel.add(slider_panel_white_gain.setup("Panel LED White Gain", 100, 0, 100));
	param_panel.add(slider_panel_rgb_gain.setup("Panel LED Color Gain", 100, 0, 100));
	slider_led_white_gain.setFillColor(ofColor(0, 153, 204, 255));
	slider_led_rgb_gain.setFillColor(ofColor(0, 102, 223, 255));
	slider_panel_white_gain.setFillColor(ofColor(254, 145, 0, 255));
	slider_panel_rgb_gain.setFillColor(ofColor(228, 0, 6, 255));
	
	MovieToLedUtils::Font::Tiny.load("Font/NotoSansJP-Regular.ttf", 12);
	MovieToLedUtils::Font::Small.load("Font/NotoSansJP-Regular.ttf", 16);
	MovieToLedUtils::Font::Middle.load("Font/NotoSansJP-Medium.ttf", 32);
	MovieToLedUtils::Font::Large.load("Font/NotoSansJP-SemiBold.ttf", 64);
	
	Extractor::setup(&output_data);
	Extractor::setOutputDir(output_dir_path);
	Converter::setup(&output_data, &completed_file);
	Converter::setOutputDir(output_dir_path);
	Extractor::setCallback(std::bind(&MovieToLed::onExtracted, this));
	device_map.setOutputDir(output_dir_path);
	
	window_mode = "FULL HD";
	window_width = MovieToLedUtils::DisplaySize::FULL_HD_WIDTH;
	window_height = MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT;
	
	voltage_panel.setup("LED Voltage");
	use_5V.addListener(this, &MovieToLed::use5VChanged);
	use_12V.addListener(this, &MovieToLed::use12VChanged);
	voltage_panel.add(label_voltage.setup("Use 12V LED?", "NO (5V)"));
	voltage_panel.add(use_12V.set("Yes (12V)", false));
	voltage_panel.add(use_5V.set("No (5V)", true));
	
	product_config.setup(&product_contents);
}

MovieToLed::~MovieToLed() {
}

void MovieToLed::setup() {
	// Parameter.JSONをロード
	ofxJSONElement json;
	if (json.open(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON))) {
		sound_number = json["Parameter"]["01 Sound Number"].asInt() & 0xff;
		slider_led_white_gain = json["Parameter"]["02 Normal LED White Gain"].asInt() & 0xff;
		slider_led_rgb_gain = json["Parameter"]["03 Normal LED Color Gain"].asInt() & 0xff;
		slider_panel_white_gain = json["Parameter"]["04 Panel LED White Gain"].asInt() & 0xff;
		slider_panel_rgb_gain = json["Parameter"]["05 Panel LED Color Gain"].asInt() & 0xff;
		MovieToLedUtils::MtLStates::Converter::output_4line_bin = json["Parameter"]["06 BIN Output"]["4Line BIN"].asBool();
		MovieToLedUtils::MtLStates::Converter::output_8line_bin = json["Parameter"]["06 BIN Output"]["8Line BIN"].asBool();
		if (!MovieToLedUtils::MtLStates::Converter::output_8line_bin) not_output_8line_bin = true;
		if (!MovieToLedUtils::MtLStates::Converter::output_4line_bin) not_output_4line_bin = true;
	}
	// 1_MtL_Contents.confをロード
	load();
}

void MovieToLed::setupRender(int x, int y, int width, int height, int gui_width) {
	drag_area.x = x;
	drag_area.y = y;
	drag_area.width = width;
	drag_area.height = height;
	panel_width = gui_width;
	
	product_panel.setPosition(drag_area.position.x + MovieToLedUtils::DisplaySize::FULL_HD_WIDTH / 4 + 32, drag_area.y + drag_area.height + 64);
	product_panel.setWidthElements(panel_width);
	product_config.setupRender(product_panel.getPosition().x + panel_width, product_panel.getPosition().y, panel_width);
	device_map_panel.setPosition(product_panel.getPosition().x, product_panel.getPosition().y + product_panel.getHeight());
	device_map_panel.setWidthElements(panel_width);
	
	data_panel.setPosition(drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 162);
	data_panel.setWidthElements(panel_width);
	bin_panel.setPosition(data_panel.getPosition().x, data_panel.getPosition().y + data_panel.getHeight());
	bin_panel.setWidthElements(panel_width);
	metadata_panel.setPosition(bin_panel.getPosition().x, bin_panel.getPosition().y + bin_panel.getHeight());
	metadata_panel.setWidthElements(panel_width);
	
	param_panel.setPosition(drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110);
	param_panel.setWidthElements(panel_width);
	// plot init
	plot_area.set(param_panel.getPosition().x + param_panel.getWidth(), param_panel.getPosition().y, 255, 255);
	unsigned char gammma_max = 0;
	updatePlot(gamma_curve, 100, gammma_max);
	updatePlot(led_white_curve, (unsigned char)led_white_gain, led_white_max);
	updatePlot(led_rgb_curve, (unsigned char)led_rgb_gain, led_rgb_max);
	updatePlot(panel_white_curve, (unsigned char)panel_white_gain, panel_white_max);
	updatePlot(panel_rgb_curve, (unsigned char)led_rgb_gain, panel_rgb_max);
	
	voltage_panel.setPosition(plot_area.getPosition().x, plot_area.getPosition().y + plot_area.getHeight() + 6);
	voltage_panel.setWidthElements(panel_width);
}

bool MovieToLed::load() {
	product_contents.clear();
	product_panel.clear();
	label_product_name = "-";
	// MtL_Contentsの読み込み
	if(MovieToLedUtils::MtLStates::isOldFormatinoData()){
		ofFile file(MovieToLedUtils::FilePaths::MtLContents);
		if (file.exists()) {
			ofBuffer buff(file);
			for (auto line : buff.getLines()) {
				vector<string> dat = ofSplitString(line, ",");
				product_contents.push_back(ProductContent(dat[0], static_cast<unsigned int>(ofToInt(dat[1]))));
			}
			file.close();
			buff.clear();
			onLoadContents();
			return true;
		}
	}else{
		
	}
	return false;
}

bool MovieToLed::isSameProduct(ProductContent & content, ofxJSONElement json){
	if(content.product_name != json["01 Product Name"].asString()) return false;
	if(content.num_product != json["02 Number of Products"].asInt()) return false;
	if(content.num_device != json["03 Number of Devices"].asInt()) return false;
	if(content.device_type == DeviceType::LINE4 && json["04 Device Type"].asString() != "4Line") return false;
	if(content.device_type == DeviceType::LINE8 && json["04 Device Type"].asString() != "8Line") return false;
	return true;
}

void MovieToLed::loadProductSetting() {
	ofxJSONElement json;
	if (json.open(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON))) {
		if (static_cast<uint8_t>(product_contents.size()) == static_cast<uint8_t>(json["Product Setting"].size())) {
			for (uint8_t i = 0; i < static_cast<uint8_t>(json["Product Setting"].size()); i++) {
				if(!isSameProduct(product_contents[i], json["Product Setting"][i])) return;
			}
			for (uint8_t i = 0; i < static_cast<uint8_t>(json["Product Setting"].size()); i++) {
				product_contents[i].id_format = json["Product Setting"][i]["05 ID Format"].asString() == "HEX" ? DeviceIdFormat::HEX : DeviceIdFormat::DEC;
				product_contents[i].setCreationRange(json["Product Setting"][i]["06 Start Product ID"].asInt(), json["Product Setting"][i]["07 End Product ID"].asInt());
				product_contents[i].is_data_generate = json["Product Setting"][i]["08 Data Generation"].asBool();
			}
		}
	}
}

void MovieToLed::onLoadContents() {
	// 各プロダクトの.confがあるか確認
	product_config.verify(product_contents);
	ofxJSONElement json;
	for (uint8_t i = 0; i < static_cast<uint8_t>(product_contents.size()); i++) {
		// .confからデバイスタイプとデバイス数を取得
		if (product_config.isExist(i)) product_config.setDeviceInfo(i);
	}
	loadProductSetting();
	if (!product_contents.empty()) {
		product_panel.add(product_idx.set("Select Product", 1, 1, static_cast<uint8_t>(product_contents.size())));
		product_panel.add(label_product_name.setup("Product", product_contents[product_idx - 1].product_name));
		if (product_contents[product_idx - 1].is_data_generate) {
			product_panel.add(label_data_generate.setup("Generate LED Data?", "YES"));
			product_panel.add(data_generate.set("YES", true));
			product_panel.add(not_data_generate.set("NO", false));
		} else {
			product_panel.add(label_data_generate.setup("Generate LED Data?", "NO"));
			product_panel.add(data_generate.set("YES", false));
			product_panel.add(not_data_generate.set("NO", true));
		}
		if (product_contents[product_idx - 1].id_format == DeviceIdFormat::HEX) {
			product_panel.add(label_id_format.setup("ID Format", "HEX (00-FF)"));
			product_panel.add(id_format_dec.set("DEC (00-99)", false));
			product_panel.add(id_format_hex.set("HEX (00-FF)", true));
		} else if (product_contents[product_idx - 1].id_format == DeviceIdFormat::DEC) {
			product_panel.add(label_id_format.setup("ID Format", "DEC (00-99)"));
			product_panel.add(id_format_dec.set("DEC (00-99)", true));
			product_panel.add(id_format_hex.set("HEX (00-FF)", false));
		}
		product_panel.add(label_product_id_range.setup("Product ID Range", ofToString(product_contents[product_idx - 1].start_product_id) + "-" + ofToString(product_contents[product_idx - 1].end_product_id)));
		product_panel.add(product_id_start.set("Start Product ID", product_contents[product_idx - 1].start_product_id, 0, product_contents[product_idx - 1].num_product - 1));
		product_panel.add(product_id_end.set("End Product ID", product_contents[product_idx - 1].end_product_id, 0, product_contents[product_idx - 1].num_product - 1));
		product_panel.add(btn_all_product_id.setup("All Product ID"));
		product_idx.addListener(this, &MovieToLed::productIdxChanged);
		id_format_dec.addListener(this, &MovieToLed::idFormatDecChanged);
		id_format_hex.addListener(this, &MovieToLed::idFormatHexChanged);
		data_generate.addListener(this, &MovieToLed::dataGenerateChanged);
		not_data_generate.addListener(this, &MovieToLed::notDataGenerateChanged);
		product_id_start.addListener(this, &MovieToLed::productIdStartChanged);
		product_id_end.addListener(this, &MovieToLed::productIdEndChanged);
		btn_all_product_id.addListener(this, &MovieToLed::setAllProductId);
		device_map_panel.setPosition(product_panel.getPosition().x, product_panel.getPosition().y + product_panel.getHeight());
		product_config.setProduct(product_idx - 1);
	}
	MovieToLedUtils::MtLStates::onReady(video_capture.isOpened());
	if (MovieToLedUtils::MtLStates::Preprocess::config_all_exist) {
		if (video_capture.isOpened()) {
			for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
				MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists = Extractor::isExistM5LED(product_contents[i], sound_number);
				if (MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists) break;
			}
		}
	}
	if (product_contents.empty()) {
		printf("No Contents\r\n");
	} else {
		for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
			printf("Product Name:%s : %d Products x %d Devices\r\n", product_contents[i].product_name.c_str(), product_contents[i].num_product, product_contents[i].num_device);
		}
	}
}

void MovieToLed::onDroppedVideo(const ofFile file) {
	// Windows 動画ファイルをdataディレクトリに入れる必要あり
	string file_name = file.getFileName();
	// Videoディレクトリ生成
	FileUtils::createDir(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
	if (video_capture.isOpened()) video_capture.release();
	// Videoディレクトリ内のファイルを削除
	FileUtils::remove(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
	// ファイルをコピー
	ofFile::copyFromTo(file.getAbsolutePath(), MovieToLedUtils::DirPaths::VIDEO_DIR, true, true);
	string path = ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR) + "/" + file_name;
	Extractor::loadVideo(path);
	MovieToLedUtils::MtLStates::onReady(video_capture.isOpened());
	if (!product_contents.empty()) {
		for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
			MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists = Extractor::isExistM5LED(product_contents[i], sound_number);
			if (MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists) break;
		}
	}
}

void MovieToLed::dragEvent(ofDragInfo info) {
	if (!MovieToLedUtils::MtLStates::isProcessing()) {
		if (drag_area.inside(info.position.x, info.position.y)) {
			ofFile dropped_file(info.files[0]);
			if (dropped_file.isDirectory()) {
				MovieToLedUtils::onDroppedContent(ofDirectory(dropped_file.getAbsolutePath()), std::bind(&MovieToLed::load, this));
			} else if (dropped_file.isFile() && (ofFilePath::getFileExt(dropped_file) == "mov" || ofFilePath::getFileExt(dropped_file) == "mp4")) {
				onDroppedVideo(dropped_file);
			}
		}
	}
}

void MovieToLed::setWindowSize(int w, int h) {
	if (!video_capture.isOpened()) {
		if (w >= MovieToLedUtils::DisplaySize::UHD_4K_WIDTH || h >= MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT) {
			ofSetWindowShape(MovieToLedUtils::DisplaySize::UHD_4K_WIDTH, MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT);
			window_width = MovieToLedUtils::DisplaySize::UHD_4K_WIDTH;
			window_height = MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT;
			window_mode = "UHD 4K";
			video_image.allocate(MovieToLedUtils::DisplaySize::UHD_4K_WIDTH, MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT, OF_IMAGE_COLOR);
			frame_mat = cv::Mat(MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT, MovieToLedUtils::DisplaySize::UHD_4K_WIDTH, CV_8UC3);
		} else {
			ofSetWindowShape(MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT);
			window_width = MovieToLedUtils::DisplaySize::FULL_HD_WIDTH;
			window_height = MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT;
			window_mode = "FULL HD";
			video_image.allocate(MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT, OF_IMAGE_COLOR);
			frame_mat = cv::Mat(MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT, MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, CV_8UC3);
		}
		// setupRender(32, 32, ofGetWidth() - 64, ofGetHeight() / 3 - 64, 220);
		ofSetWindowPosition(0, 30);
	}
}

void MovieToLed::switchWindowSize() {
	if (!video_capture.isOpened()) {
		if (window_width == MovieToLedUtils::DisplaySize::UHD_4K_WIDTH) {
			setWindowSize(MovieToLedUtils::DisplaySize::FULL_HD_WIDTH, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT);
		} else {
			setWindowSize(MovieToLedUtils::DisplaySize::UHD_4K_WIDTH, MovieToLedUtils::DisplaySize::UHD_4K_HEIGHT);
		}
	}
}

void MovieToLed::saveParameter() {
	ofxJSONElement json;
	json.open(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON));
	json["Parameter"]["01 Sound Number"] = (int)sound_number;
	json["Parameter"]["02 Normal LED White Gain"] = (int)led_white_gain;
	json["Parameter"]["03 Normal LED Color Gain"] = (int)led_rgb_gain;
	json["Parameter"]["04 Panel LED White Gain"] = (int)panel_white_gain;
	json["Parameter"]["05 Panel LED Color Gain"] = (int)panel_rgb_gain;
	json["Parameter"]["06 BIN Output"]["4Line BIN"] = MovieToLedUtils::MtLStates::Converter::output_4line_bin;
	json["Parameter"]["06 BIN Output"]["8Line BIN"] = MovieToLedUtils::MtLStates::Converter::output_8line_bin;
	char update_date[20];
	snprintf(update_date, 20, "%04d/%02d/%02d %02d:%02d:%02d", ofGetYear(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
	json["Parameter"]["07 LAST UPDATE"] = (string)update_date;
	json.save(ofToDataPath(MovieToLedUtils::FilePaths::PARAMETER_JSON), true);
}

void MovieToLed::saveProductSetting() {
	ofxJSONElement json;
	json.open(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON));
	json.clear();
	for (uint8_t i = 0; i < static_cast<uint8_t>(product_contents.size()); i++) {
		ofxJSONElement product;
		product["01 Product Name"] = product_contents[i].product_name;
		product["02 Number of Products"] = product_contents[i].num_product;
		product["03 Number of Devices"] = product_contents[i].num_device;
		product["04 Device Type"] = product_contents[i].device_type == DeviceType::LINE4 ? "4Line" : "8Line";
		product["05 ID Format"] = product_contents[i].id_format == DeviceIdFormat::HEX ? "HEX" : "DEC";
		product["06 Start Product ID"] = product_contents[i].start_product_id;
		product["07 End Product ID"] = product_contents[i].end_product_id;
		product["08 Data Generation"] = product_contents[i].is_data_generate;
		json["Product Setting"].append(product);
	}
	json.save(ofToDataPath(MovieToLedUtils::FilePaths::PRODUCT_SETTING_JSON), true);
}

void MovieToLed::exit() {
	saveParameter();
	saveProductSetting();
	video_capture.release();
	FileUtils::remove(ofToDataPath(MovieToLedUtils::DirPaths::VIDEO_DIR));
}

void MovieToLed::start() {
	if (!MovieToLedUtils::error_msg.empty()) return;
	if (MovieToLedUtils::MtLStates::MainProcess::is_completed) {
		// BIN生成が終了した時
		video_capture.release();
		video_file_name.clear();
		total_frame = 0;
		duration_min = 0;
		duration_sec = 0;
		MovieToLedUtils::MtLStates::MainProcess::is_completed = false;
	} else {
		MovieToLedUtils::MtLStates::onReady(video_capture.isOpened());
		if (MovieToLedUtils::MtLStates::MainProcess::is_ready) {
			if (MovieToLedUtils::MtLStates::isFirstTime()) {
				updateContentIndex(true);
				createDeviceMap();
			}
			// データを作成するプロダクト情報をセット
			if (product_config.isExist(content_index)) output_data.led_product.setup(&product_contents[content_index]);
			Extractor::createM5LED(sound_number);
			m5led_metadata.set(sound_number, total_frame, loop_playback ? 1 : 0);
			m5led_metadata.write(output_dir_path + "/M5LED/" + output_data.led_product.getName(), sound_number);
			// video_player.update();
			// num_frame = video_player.getCurrentFrame();
			log_black_path = output_dir_path + "/LOG/" + output_data.led_product.getName() + "/BlackData.csv";
			log_skip_path = output_dir_path + "/LOG/" + output_data.led_product.getName() + "/SkipFrame.csv";
			log_frame_path = output_dir_path + "/LOG/" + output_data.led_product.getName() + "/Frame.csv";
			FileUtils::createFile(log_black_path, false);
			FileUtils::createFile(log_skip_path, false);
			FileUtils::createFile(log_frame_path, false);
			num_frame = 1;
			prev_frame = 1;
			num_count = 1;
			completed_file.clear();
			MovieToLedUtils::MtLStates::startExtract();
		}
	}
}

void MovieToLed::draw() {
	ofSetBackgroundColor(0, 0, 0);
	if (!MovieToLedUtils::error_msg.empty()) {
		// エラー
		ofSetBackgroundColor(228, 0, 6);
		ofSetColor(255, 255, 255, 255);
		MovieToLedUtils::Font::Large.drawString(MovieToLedUtils::error_msg, 10, window_height / 2);
		MovieToLedUtils::MtLStates::stop();
		return;
	}
	if (MovieToLedUtils::MtLStates::MainProcess::is_completed) {
		// BINデータ変換終了
		ofSetBackgroundColor(14, 167, 39);
		MovieToLedUtils::Font::Large.drawString("Completed All Product Data", drag_area.position.x, drag_area.position.y + 58);
		MovieToLedUtils::Font::Large.drawString("Press SPACE Key to Continue MtL", drag_area.position.x, drag_area.position.y + 168);
		for (int i = 0; i < static_cast<int>(completed_file.size()); i++) {
			MovieToLedUtils::Font::Tiny.drawString(completed_file[i], drag_area.position.x + 200 * (i / 30), drag_area.position.y + 208 + 24 * (i % 30));
		}
	} else if (MovieToLedUtils::MtLStates::MainProcess::is_converting) {
		// BINデータ変換中
		ofSetBackgroundColor(14, 167, 39);
		MovieToLedUtils::Font::Middle.drawString(Extractor::product_name + " Convert to BIN", drag_area.position.x, drag_area.position.y + 32);
		snprintf(str, 128, "Product ID Range : %d - %d / Devices per Product : %d\r\n", Extractor::start_product_id, Extractor::end_product_id, Extractor::num_device);
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 58);
		std::string device_type = output_data.led_product.getDeviceType() == DeviceType::LINE4 ? "4Line" : "8Line";
		std::string format = output_data.led_product.getDeviceIdFormat() == DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
		MovieToLedUtils::Font::Small.drawString("Device Type : " + device_type + " / ID Format : " + format, drag_area.position.x, drag_area.position.y + 84);
		snprintf(str, 128, "Video : %s", video_file_name.c_str());
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 110);
		snprintf(str, 128, "Frame : %d / %d (%02d:%02d)", num_frame, total_frame, duration_min, duration_sec);
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 136);
		snprintf(str, 32, "Completed BIN File: %llu", completed_file.size());
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 178);
		for (int i = 0; i < static_cast<int>(completed_file.size()); i++) {
			MovieToLedUtils::Font::Tiny.drawString(completed_file[i], drag_area.position.x + 200 * (i / 30), drag_area.position.y + 208 + 24 * (i % 30));
		}
	} else if (MovieToLedUtils::MtLStates::MainProcess::is_extracting) {
		// M5LED生成中
		video_image.draw(0, 0);
		ofSetColor(255, 255, 255, 255);
		MovieToLedUtils::Font::Middle.drawString(Extractor::product_name + " Create M5LED", drag_area.position.x, drag_area.position.y + 32);
		snprintf(str, 128, "Product ID Range : %d - %d / Devices per Product : %d\r\n", Extractor::start_product_id, Extractor::end_product_id, Extractor::num_device);
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 58);
		snprintf(str, 128, "Sound Number : %d", (int)sound_number);
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 84);
		MovieToLedUtils::Font::Small.drawString(loop_playback ? "M5LED Loop Playback : ON" : "M5LED Loop Playback : OFF", drag_area.position.x, drag_area.position.y + 110);
		if (MovieToLedUtils::MtLStates::isOutputBin()) {
			std::string device_type = output_data.led_product.getDeviceType() == DeviceType::LINE4 ? "4Line" : "8Line";
			std::string format = output_data.led_product.getDeviceIdFormat() == DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
			MovieToLedUtils::Font::Small.drawString("BIN : Device Type : " + device_type + " / ID Format : " + format, drag_area.position.x, drag_area.position.y + 136);
		}
		snprintf(str, 128, "Video : %s", video_file_name.c_str());
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 162);
		snprintf(str, 128, "Frame : %d / %d (%02d:%02d)", num_frame, total_frame, duration_min, duration_sec);
		MovieToLedUtils::Font::Small.drawString(str, drag_area.position.x, drag_area.position.y + 188);
		for (int i = 0; i < 32; i++) {
			// ofDrawBitmapString(log_black_msg[i], display_width - 520, display_height - 10 - 15 * i);
			ofDrawBitmapString(log_skip_msg[i], window_width - 340, window_height / 2 + 40 - 15 * i);
		}
	} else {
		// 処理前
		if (video_capture.isOpened()) video_image.draw(0, 0); // 動画ロード完了
		ofSetColor(36, 36, 36, 128);
		ofDrawRectangle(0, 0, window_width, window_height);
		ofSetColor(255, 255, 255);
		ofNoFill();
		ofDrawRectangle(drag_area);
		ofFill();
		ofSetColor(255, 255, 255, 255);
		MovieToLedUtils::Font::Large.drawString("Drag & Drop Here", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 32);
		MovieToLedUtils::Font::Small.drawString("Drag & Drop a Directory (Formation SAVEDATA/csv) to Load New MtL Contents", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 96);
		MovieToLedUtils::Font::Small.drawString("Drag & Drop a MOV or MP4 File to Load New Video", drag_area.x + 32, drag_area.y + drag_area.height / 2 + 128);
		
		MovieToLedUtils::Font::Middle.drawString("MtL Contents", drag_area.x, drag_area.y + drag_area.height + 48);
		if (product_contents.empty()) {
			MovieToLedUtils::Font::Small.drawString("No Contents", drag_area.x, drag_area.y + drag_area.height + 48 + 26);
		} else {
			if (product_config.isExist(product_idx - 1)) {
				MovieToLedUtils::Font::Middle.drawString("Product Setting", product_panel.getPosition().x, drag_area.y + drag_area.height + 48);
			} else {
				MovieToLedUtils::Font::Middle.drawString("Product Setting : New Product", product_panel.getPosition().x, drag_area.y + drag_area.height + 48);
			}
			product_panel.draw();
			product_config.draw();
			device_map_panel.draw();
			for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
				string product_name = product_contents[i].product_name;
				if (!product_contents[i].is_data_generate) ofSetColor(192, 186, 162);
				if (i + 1 == product_idx) ofSetColor(254, 128, 51);
				MovieToLedUtils::Font::Small.drawString(ofToString(i + 1) + ". " + product_name, drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 84 * i);
				if (product_config.isExist(i)) {
					uint16_t num_product = product_contents[i].num_product;
					uint16_t num_device = product_contents[i].num_device;
					uint16_t start_product_id = product_contents[i].start_product_id;
					uint16_t end_product_id = product_contents[i].end_product_id;
					MovieToLedUtils::Font::Tiny.drawString(ofToString(num_product) + " Products x " + ofToString(num_device) + " Devices", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 18 + 84 * i);
					if (product_contents[i].is_data_generate) {
						if (start_product_id == 0 && end_product_id == num_product - 1) {
							MovieToLedUtils::Font::Tiny.drawString("Product ID Range : " + ofToString(start_product_id) + " - " + ofToString(end_product_id) + " : All Product ID", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
						} else {
							MovieToLedUtils::Font::Tiny.drawString("Product ID Range : " + ofToString(start_product_id) + " - " + ofToString(end_product_id), drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
						}
						if (MovieToLedUtils::MtLStates::isOutputBin()) {
							std::string device_type = product_contents[i].device_type == DeviceType::LINE4 ? "4Line" : "8Line";
							std::string format = product_contents[i].id_format == DeviceIdFormat::HEX ? "HEX (ID : 00 - FF)" : "DEC (ID : 00 - 99)";
							MovieToLedUtils::Font::Tiny.drawString("BIN : Device Type : " + device_type + " / ID Format : " + format, drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 54 + 84 * i);
						}
					} else {
						MovieToLedUtils::Font::Tiny.drawString("Data Generation : OFF. No LED Data will be Generated.", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 36 + 84 * i);
					}
				} else {
					MovieToLedUtils::Font::Tiny.drawString("New Product. Enter Product Configuration.", drag_area.x, drag_area.y + drag_area.height + 48 + 26 + 18 + 84 * i);
				}
				if (i + 1 == product_idx || !product_contents[i].is_data_generate) ofSetColor(255, 255, 255);
			}
		}
		if (product_contents.empty() || MovieToLedUtils::MtLStates::Preprocess::config_all_exist) {
			MovieToLedUtils::Font::Middle.drawString("Video", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48);
			snprintf(str, 128, "Name : %s", video_file_name.c_str());
			MovieToLedUtils::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 26);
			snprintf(str, 128, "Duration : %d Frame (%02d:%02d)", total_frame, duration_min, duration_sec);
			MovieToLedUtils::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 52);
			
			MovieToLedUtils::Font::Middle.drawString("Data Setting", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 100);
			snprintf(str, 128, "Sound Number : %d", (int)sound_number);
			MovieToLedUtils::Font::Small.drawString(str, drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 126);
			MovieToLedUtils::Font::Small.drawString(loop_playback ? "M5LED Loop Playback : ON" : "M5LED Loop Playback : OFF", drag_area.x + drag_area.width / 2 + 32, drag_area.y + drag_area.height + 48 + 152);
			// MovieToLedUtils::Font::Tiny.drawString(Extractor::use_led_5V ? "LED V : 5V" : "LED V : 12V", voltage_panel.getPosition().x, voltage_panel.getPosition().y + voltage_panel.getHeight() + 18);
			
			bin_panel.draw();
			data_panel.draw();
			metadata_panel.draw();
			// voltage_panel.draw();
			
			MovieToLedUtils::Font::Middle.drawString("LED Color Gain", drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 100);
			MovieToLedUtils::Font::Small.drawString("Normal LED", drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 26);
			MovieToLedUtils::Font::Small.drawString("Panel LED", drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 104);
			
			param_panel.draw();
			
			// 補正をかけたRGB値を描画するグラフ
			ofSetColor(255, 255, 255, 30);
			ofDrawRectangle(plot_area);
			ofSetColor(255, 255, 255, 96);
			for (int i = 0; i < 7; i++) {
				ofDrawLine(plot_area.getPosition().x + 32 * (i + 1), plot_area.getPosition().y, plot_area.getPosition().x + 32 * (i + 1), plot_area.getPosition().y + plot_area.getHeight());
				ofDrawLine(plot_area.getPosition().x, plot_area.getPosition().y + 32 * (i + 1), plot_area.getPosition().x + plot_area.getWidth(), plot_area.getPosition().y + 32 * (i + 1));
			}
			ofSetColor(0, 153, 204, 255);
			MovieToLedUtils::Font::Tiny.drawString("WHT Normal LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32);
			led_white_curve.draw();
			MovieToLedUtils::Font::Small.drawString("White MAX:" + ofToString((int)led_white_max), drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 52);
			ofSetColor(0, 102, 223, 255);
			MovieToLedUtils::Font::Tiny.drawString("RGB Normal LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 2);
			led_rgb_curve.draw();
			MovieToLedUtils::Font::Small.drawString("Other Color MAX:" + ofToString((int)led_rgb_max), drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 78);
			ofSetColor(254, 145, 0, 255);
			MovieToLedUtils::Font::Tiny.drawString("WHT Panel LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 3);
			panel_white_curve.draw();
			MovieToLedUtils::Font::Small.drawString("White MAX:" + ofToString((int)panel_white_max), drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 130);
			ofSetColor(228, 0, 6, 255);
			MovieToLedUtils::Font::Tiny.drawString("RGB Panel LED", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20 + 32 * 4);
			panel_rgb_curve.draw();
			MovieToLedUtils::Font::Small.drawString("Other Color MAX:" + ofToString((int)panel_rgb_max), drag_area.x + drag_area.width * 2 / 3 + 32, drag_area.y + drag_area.height + 48 + 110 + param_panel.getHeight() + 156);
			ofSetColor(14, 167, 39, 255);
			MovieToLedUtils::Font::Tiny.drawString("Gamma Curve", plot_area.getPosition().x + 4, plot_area.getPosition().y + 20);
			gamma_curve.draw();
			
			if (MovieToLedUtils::MtLStates::MainProcess::is_ready) {
				// 動画ロード完了 and .conf存在
				ofSetColor(255, 255, 255, 255);
				MovieToLedUtils::Font::Large.drawString("Press SPACE Key Start MtL", 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5);
				if (MovieToLedUtils::MtLStates::isOutputBin()) {
					snprintf(str, 128, "Create: %02X_XXX-XXX.M5LED and %05X_XX.BIN File", (int)sound_number, (int)sound_number);
					MovieToLedUtils::Font::Middle.drawString(str, 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5 + 60);
					if (MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists) {
						snprintf(str, 128, "Warnig: %02X_XXX-XXX.M5LED and %05X_XX.BIN File will be Overwritten", (int)sound_number, (int)sound_number);
						MovieToLedUtils::Font::Middle.drawString(str, 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5 + 110);
					}
				} else {
					snprintf(str, 128, "Create: %02X_XXX-XXX.M5LED", (int)sound_number);
					MovieToLedUtils::Font::Middle.drawString(str, 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5 + 60);
					if (MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists) {
						snprintf(str, 128, "Warnig: %02X_XXX-XXX.M5LED will be Overwritten", (int)sound_number);
						MovieToLedUtils::Font::Middle.drawString(str, 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5 + 110);
					}
				}
			} else if (!video_capture.isOpened()) {
				ofSetColor(255, 255, 255, 255);
				MovieToLedUtils::Font::Middle.drawString("Press SHIFT Key Switch Window Size FULL HD or UHD 4K", 10, MovieToLedUtils::DisplaySize::FULL_HD_HEIGHT * 4 / 5);
			}
		}
	}
	// 共通
	ofSetColor(255, 255, 255, 255);
	MovieToLedUtils::Font::Small.drawString("MovieToLED PRO Ver." SOFTWARE_VER, drag_area.position.x, drag_area.position.y - 8);
	MovieToLedUtils::Font::Small.drawString(window_mode, drag_area.position.x + drag_area.width - 96, drag_area.position.y - 8);
	if (MovieToLedUtils::MtLStates::Converter::output_8line_bin && MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
		MovieToLedUtils::Font::Small.drawString("OUTPUT:M5LED & BIN(8Line & 4Line)", drag_area.position.x + MovieToLedUtils::DisplaySize::FULL_HD_WIDTH / 4, drag_area.position.y - 8);
	} else if (MovieToLedUtils::MtLStates::Converter::output_8line_bin && !MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
		MovieToLedUtils::Font::Small.drawString("OUTPUT:M5LED & BIN(8Line)", drag_area.position.x + MovieToLedUtils::DisplaySize::FULL_HD_WIDTH / 4, drag_area.position.y - 8);
	} else if (!MovieToLedUtils::MtLStates::Converter::output_8line_bin && MovieToLedUtils::MtLStates::Converter::output_4line_bin) {
		MovieToLedUtils::Font::Small.drawString("OUTPUT:M5LED & BIN(4Line)", drag_area.position.x + MovieToLedUtils::DisplaySize::FULL_HD_WIDTH / 4, drag_area.position.y - 8);
	} else {
		MovieToLedUtils::Font::Small.drawString("OUTPUT:M5LED", drag_area.position.x + MovieToLedUtils::DisplaySize::FULL_HD_WIDTH / 4, drag_area.position.y - 8);
	}
}

void MovieToLed::update() {
	if (MovieToLedUtils::MtLStates::MainProcess::is_extracting) {
		// M5LED作成
		extract();
	} else if (MovieToLedUtils::MtLStates::MainProcess::is_converting) {
		printf("Convert BIN : %d/%d\r\n", Converter::convert_count, Converter::max_convert_count);
		if (Converter::isFinish()) {
			// すべてのM5LEDのBIN変換が終了
			if (content_index == end_content_index) {
				// 最後のプロダクトのデータ生成が終了
				MovieToLedUtils::MtLStates::onCompleted();
			} else {
				// 次のプロダクトのデータ生成を開始
				updateContentIndex();
				if (content_index < static_cast<int>(product_contents.size())) start();
			}
		} else {
			// M5LEDをBINに変換
			Converter::process();
		}
	}
}

void MovieToLed::onExtracted() {
	// M5LED生成完了後
	MovieToLedUtils::MtLStates::onExtracted();
	if (MovieToLedUtils::MtLStates::MainProcess::is_converting) {
		// BIN変換に移行する場合
		Converter::createBIN(sound_number);
		printf("%s Data Start Convert\r\n", output_data.led_product.getName().c_str());
	} else {
		// BIN変換しない場合は、次のプロダクトのM5LED生成へ
		if (content_index == end_content_index) {
			// 最後のプロダクトのデータ生成が終了
			MovieToLedUtils::MtLStates::onCompleted();
		} else {
			// 次のプロダクトのデータ生成を開始
			updateContentIndex();
			if (content_index < static_cast<int>(product_contents.size())) start();
		}
	}
}

void MovieToLed::updateContentIndex(bool is_init) {
	if (!product_contents.empty()) {
		if (is_init) {
			// データ生成するプロダクトの最初のindexを探索
			for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
				if (product_contents[i].is_data_generate) {
					content_index = i;
					break;
				}
			}
			// 最後のindexを探索
			for (int i = static_cast<int>(product_contents.size() - 1); i >= 0; i--) {
				if (product_contents[i].is_data_generate) {
					end_content_index = i;
					return;
				}
			}
		} else {
			// 次のindexに更新
			int prev_content_idx = content_index;
			for (int i = prev_content_idx + 1; i < static_cast<int>(product_contents.size()); i++) {
				if (product_contents[i].is_data_generate) {
					content_index = i;
					return;
				}
			}
		}
	}
}

void MovieToLed::createDeviceMap() {
	for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
		if (product_config.isExist(i)) {
			device_map.create(product_contents[i]);
		}
	}
}

void MovieToLed::clearMetadata() {
	if (product_contents.empty()) return;
	for (auto content : product_contents) {
		m5led_metadata.clear(output_dir_path + "/M5LED/" + content.product_name);
	}
}

void MovieToLed::soundNumberChanged(unsigned char & sound_num) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		sound_num = Extractor::sound_number;
	} else {
		Extractor::sound_number = sound_num;
		Converter::sound_number = sound_num;
		if (!product_contents.empty()) {
			for (int i = 0; i < static_cast<int>(product_contents.size()); i++) {
				MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists = Extractor::isExistM5LED(product_contents[i], sound_num);
				if (MovieToLedUtils::MtLStates::Preprocess::m5led_already_exists) break;
			}
		}
	}
}

void MovieToLed::playOnceChanged(bool & is_once) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		is_once = !loop_playback;
	} else {
		if (is_once) {
			play_loop = false;
			label_m5led_loop_playback = "NO";
			loop_playback = false;
		} else {
			if (!play_loop) is_once = true;
		}
	}
}

void MovieToLed::playLoopChanged(bool & is_loop) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		is_loop = loop_playback;
	} else {
		if (is_loop) {
			play_once = false;
			label_m5led_loop_playback = "YES";
			loop_playback = true;
		} else {
			if (!play_once) is_loop = true;
		}
	}
}

void MovieToLed::productIdxChanged(uint8_t & idx) {
	if (!product_contents.empty()) {
		if (idx >= 1 && idx <= static_cast<uint8_t>(product_contents.size())) {
			product_id_start.removeListener(this, &MovieToLed::productIdStartChanged);
			product_id_end.removeListener(this, &MovieToLed::productIdEndChanged);
			data_generate.removeListener(this, &MovieToLed::dataGenerateChanged);
			not_data_generate.removeListener(this, &MovieToLed::notDataGenerateChanged);
			label_product_name = product_contents[idx - 1].product_name;
			data_generate = product_contents[idx - 1].is_data_generate;
			not_data_generate = !product_contents[idx - 1].is_data_generate;
			id_format_dec = product_contents[idx - 1].id_format == DeviceIdFormat::DEC;
			id_format_hex = product_contents[idx - 1].id_format == DeviceIdFormat::HEX;
			product_id_start.setMax(product_contents[idx - 1].num_product - 1);
			product_id_end.setMax(product_contents[idx - 1].num_product - 1);
			product_id_start = product_contents[idx - 1].start_product_id;
			product_id_end = product_contents[idx - 1].end_product_id;
			label_data_generate = data_generate ? "YES" : "NO";
			data_generate.addListener(this, &MovieToLed::dataGenerateChanged);
			not_data_generate.addListener(this, &MovieToLed::notDataGenerateChanged);
			label_product_id_range = ofToString(product_id_start) + "-" + ofToString(product_id_end);
			product_id_start.addListener(this, &MovieToLed::productIdStartChanged);
			product_id_end.addListener(this, &MovieToLed::productIdEndChanged);
			product_config.setProduct(idx - 1);
		}
	}
}

void MovieToLed::dataGenerateChanged(bool & is_generate) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			is_generate = product_contents[product_idx - 1].is_data_generate;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (is_generate) {
				not_data_generate = false;
				product_contents[product_idx - 1].is_data_generate = true;
				label_data_generate = "YES";
			} else {
				if (!not_data_generate) is_generate = true;
			}
		}
	}
}

void MovieToLed::notDataGenerateChanged(bool & not_generate) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			not_generate = !product_contents[product_idx - 1].is_data_generate;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (not_generate) {
				data_generate = false;
				product_contents[product_idx - 1].is_data_generate = false;
				label_data_generate = "NO";
			} else {
				if (!data_generate) not_generate = true;
			}
		}
	}
}

void MovieToLed::idFormatDecChanged(bool & is_dec) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			is_dec = product_contents[product_idx - 1].id_format == DeviceIdFormat::DEC;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (is_dec) {
				id_format_hex = false;
				product_contents[product_idx - 1].id_format = DeviceIdFormat::DEC;
				label_id_format = "DEC (00-99)";
			} else {
				if (!id_format_hex) is_dec = true;
			}
		}
	}
}

void MovieToLed::idFormatHexChanged(bool & is_hex) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			is_hex = product_contents[product_idx - 1].id_format == DeviceIdFormat::HEX;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (is_hex) {
				id_format_dec = false;
				product_contents[product_idx - 1].id_format = DeviceIdFormat::HEX;
				label_id_format = "HEX (00-FF)";
			} else {
				if (!id_format_dec) is_hex = true;
			}
		}
	}
}

void MovieToLed::setAllProductId() {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			product_id_start = 0;
			product_id_end = product_contents[product_idx - 1].num_product - 1;
		}
		if (not_data_generate) data_generate = true;
	}
}

void MovieToLed::productIdStartChanged(uint16_t & id_start) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			id_start = product_contents[product_idx - 1].start_product_id;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (product_contents[product_idx - 1].setCreationRange(id_start, product_id_end)) {
				label_product_id_range = ofToString(id_start) + "-" + ofToString(product_id_end);
			} else {
				if (id_start >= product_contents[product_idx - 1].num_product) id_start = 0;
				if (id_start > product_id_end) product_id_end = id_start;
			}
		}
		if (not_data_generate) data_generate = true;
	}
}

void MovieToLed::productIdEndChanged(uint16_t & id_end) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			id_end = product_contents[product_idx - 1].end_product_id;
		}
	} else {
		if (product_idx >= 1 && product_idx <= static_cast<uint8_t>(product_contents.size())) {
			if (product_contents[product_idx - 1].setCreationRange(product_id_start, id_end)) {
				label_product_id_range = ofToString(product_id_start) + "-" + ofToString(id_end);
			} else {
				if (id_end >= product_contents[product_idx - 1].num_product) id_end = 0;
				if (id_end < product_id_start) product_id_start = id_end;
			}
		}
		if (not_data_generate) data_generate = true;
	}
}

void MovieToLed::updatePlot(ofPolyline & curve, unsigned char gain, unsigned char & max) {
	curve.clear();
	for (int i = 0; i < 256; i++) {
		if (i == 0 || i == 255) {
			// 最初と最後を２回描画すると曲線がなめらかになる
			// 補正をかける順番 -> gain(線形補正)をかけてからガンマ補正(非線形補正)をかける
			unsigned char val = i * gain / 100;
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
			if (i == 255) max = gamma_table[val] > 250 ? 250 : gamma_table[val];
		} else {
			unsigned char val = i * gain / 100;
			curve.curveTo(plot_area.getPosition().x + i, plot_area.getPosition().y + plot_area.getHeight() - gamma_table[val]);
		}
	}
}

void MovieToLed::sliderLedWhiteGainChanged(unsigned char & gain) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		gain = led_white_gain;
	} else {
		led_white_gain = gain;
		updatePlot(led_white_curve, gain, led_white_max);
	}
}

void MovieToLed::sliderLedRgbGainChanged(unsigned char & gain) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		gain = led_rgb_gain;
	} else {
		led_rgb_gain = gain;
		updatePlot(led_rgb_curve, gain, led_rgb_max);
	}
}

void MovieToLed::sliderPanelWhiteGainChanged(unsigned char & gain) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		gain = panel_white_gain;
	} else {
		panel_white_gain = gain;
		updatePlot(panel_white_curve, gain, panel_white_max);
	}
}

void MovieToLed::sliderPanelRgbGainChanged(unsigned char & gain) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		gain = panel_rgb_gain;
	} else {
		panel_rgb_gain = gain;
		updatePlot(panel_rgb_curve, gain, panel_rgb_max);
	}
}

void MovieToLed::output8LineBinChanged(bool & output_bin) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		output_bin = MovieToLedUtils::MtLStates::Converter::output_8line_bin;
	} else {
		if (output_bin) {
			not_output_8line_bin = false;
			MovieToLedUtils::MtLStates::Converter::output_8line_bin = output_bin;
			label_output_8line_bin = "YES";
		} else {
			if (!not_output_8line_bin) output_bin = true;
		}
	}
}

void MovieToLed::notOutput8LineBinChanged(bool & not_output_bin) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		not_output_bin = !MovieToLedUtils::MtLStates::Converter::output_8line_bin;
	} else {
		if (not_output_bin) {
			output_8line_bin = false;
			MovieToLedUtils::MtLStates::Converter::output_8line_bin = !not_output_bin;
			label_output_8line_bin = "NO";
		} else {
			if (!output_8line_bin) not_output_bin = true;
		}
	}
}

void MovieToLed::output4LineBinChanged(bool & output_bin) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		output_bin = MovieToLedUtils::MtLStates::Converter::output_4line_bin;
	} else {
		if (output_bin) {
			not_output_4line_bin = false;
			MovieToLedUtils::MtLStates::Converter::output_4line_bin = output_bin;
			label_output_4line_bin = "YES";
		} else {
			if (!not_output_4line_bin) output_bin = true;
		}
	}
}

void MovieToLed::notOutput4LineBinChanged(bool & not_output_bin) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		not_output_bin = !MovieToLedUtils::MtLStates::Converter::output_4line_bin;
	} else {
		if (not_output_bin) {
			output_4line_bin = false;
			MovieToLedUtils::MtLStates::Converter::output_4line_bin = !not_output_bin;
			label_output_4line_bin = "NO";
		} else {
			if (!output_4line_bin) not_output_bin = true;
		}
	}
}

void MovieToLed::use5VChanged(bool & is_use) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		is_use = Extractor::use_led_5V;
	} else {
		if (is_use) {
			use_12V = false;
			label_voltage = "NO (5V)";
			Extractor::use_led_5V = true;
		} else {
			if (!use_12V) is_use = true;
		}
	}
}

void MovieToLed::use12VChanged(bool & is_use) {
	if (MovieToLedUtils::MtLStates::isProcessing()) {
		is_use = !Extractor::use_led_5V;
	} else {
		if (is_use) {
			use_5V = false;
			label_voltage = "YES (12V)";
			Extractor::use_led_5V = false;
		} else {
			if (!use_5V) is_use = true;
		}
	}
}
