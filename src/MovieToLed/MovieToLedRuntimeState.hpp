#ifndef MOVIETOLED_RUNTIME_STATE_HPP
#define MOVIETOLED_RUNTIME_STATE_HPP

#include <cstdint>
#include <string>

class MovieToLedRuntimeState {
public:
	enum RuntimeState {
		WAITING,
		READY,
		EXTRACTING,
		CONVERTING,
		COMPLETED,
	};

	inline static RuntimeState runtime_state = RuntimeState::WAITING;

	static bool isWaiting() {
		return runtime_state == RuntimeState::WAITING;
	};

	static bool isReady() {
		return runtime_state == RuntimeState::READY;
	}

	static bool isExtracting() {
		return runtime_state == RuntimeState::EXTRACTING;
	}

	static bool isConverting() {
		return runtime_state == RuntimeState::CONVERTING;
	}

	static bool isCompleted() {
		return runtime_state == RuntimeState::COMPLETED;
	}

	inline static uint16_t process_count = 0;
	inline static bool config_all_exist = false;
	inline static bool m5led_already_exists = false;
	
	inline static std::string error_msg;
};

#endif
