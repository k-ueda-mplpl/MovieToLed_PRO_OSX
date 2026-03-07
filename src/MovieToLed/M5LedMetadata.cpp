#include "M5LedMetadata.hpp"
#include "MovieToLedFileUtils.hpp"
#include "MovieToLedUtils.hpp"
#include <ofFileUtils.h>
#include <vector>

M5LedMetadata::M5LedMetadata() {
	for (int i = 0; i <= 10; i++) {
		metadata[i].sound_number = i;
		metadata[i].duration = 0;
		metadata[i].play_mode = 0xff;
	}
}

M5LedMetadata::~M5LedMetadata() {
}

void M5LedMetadata::set(uint8_t sound_num, uint16_t duration, uint8_t play_mode) {
	if ((sound_num >= 0 && sound_num <= 10)) {
		metadata[sound_num].sound_number = sound_num;
		metadata[sound_num].duration = duration < 0xf000 ? duration & 0xffff : 0;
		metadata[sound_num].play_mode = play_mode == 0 || play_mode == 1 ? play_mode : 0xff;
	}
}

void M5LedMetadata::write(std::string dir_path, uint8_t sound_num) {
	std::string file_path = dir_path + "/" + MovieToLedUtils::FilePaths::METADATA_FILE;
	ofFile file(file_path);
	std::vector<std::string> lines;
	if (file.exists()) {
		// ファイルがある->read
		ofBuffer buff = file.readToBuffer();
		lines = ofSplitString(buff.getText(), "\n");
	} else {
		// ファイルがない->create
		MovieToLedFileUtils::createFile(file_path, false);
	}
	while (lines.size() < 11) {
		char col[15];
		snprintf(col, 15, "%d,0,NO DATA,", static_cast<int>(lines.size()));
		lines.push_back(col);
	}

	if (sound_num >= 0 && sound_num <= 10) {
		char new_dat[17];
		if (metadata[sound_num].play_mode == 0) {
			snprintf(new_dat, 17, "%d,%d,NORMAL,", metadata[sound_num].sound_number, metadata[sound_num].duration);
		} else if (metadata[sound_num].play_mode == 1) {
			snprintf(new_dat, 17, "%d,%d,LOOP,", metadata[sound_num].sound_number, metadata[sound_num].duration);
		} else if (metadata[sound_num].play_mode == 0xff) {
			snprintf(new_dat, 17, "%d,%d,NO DATA,", metadata[sound_num].sound_number, metadata[sound_num].duration);
		}
		lines[sound_num] = new_dat;
	}

	file.open(file_path, ofFile::WriteOnly);
	for (const std::string & line : lines) {
		file << line << std::endl;
	}
	file.close();
}

void M5LedMetadata::clear(std::string dir_path) {
	for (int sound_num = 0; sound_num <= 10; sound_num++) {
		set(sound_num, 0, 0xff);
	}
	std::string file_path = dir_path + "/" + MovieToLedUtils::FilePaths::METADATA_FILE;
	ofFile file(file_path);
	std::vector<std::string> lines;
	if (!file.exists()) MovieToLedFileUtils::createFile(file_path, false);
	while (lines.size() < 11) {
		char col[15];
		snprintf(col, 15, "%d,0,NO DATA,", static_cast<int>(lines.size()));
		lines.push_back(col);
	}
	file.open(file_path, ofFile::WriteOnly);
	for (const std::string & line : lines) {
		file << line << std::endl;
	}
	file.close();
}
