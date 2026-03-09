#ifndef UI_CONTEXT_HPP
#define UI_CONTEXT_HPP

#include <ofTrueTypeFont.h>
#include <string>

#ifdef TARGET_OSX
	#include <CoreGraphics/CoreGraphics.h>
	#include <GLFW/glfw3.h>
#elif defined(TARGET_WIN32)
	#include <windows.h>
#endif

class UIContext {
public:
	inline const static std::string FONT_REGULAR_PATH = "Font/NotoSansJP-Regular.ttf";
	inline const static std::string FONT_MEDIUM_PATH = "Font/NotoSansJP-Medium.ttf";
	inline const static std::string FONT_SEMIBOLD_PATH = "Font/NotoSansJP-SemiBold.ttf";

	struct Font {
		inline static ofTrueTypeFont Tiny;
		inline static ofTrueTypeFont Small;
		inline static ofTrueTypeFont Middle;
		inline static ofTrueTypeFont Large;
	};

	struct DisplaySize {
		int width;
		int height;
		int left_safe_inset = 0;
		int top_safe_inset = 0;
		int right_safe_inset = 0;
		int bottom_safe_inset = 0;
		DisplaySize(int w, int h) {
			width = w;
			height = h;
		}
	};

	static bool getInternalDisplaySize(int & width, int & height) {
#ifdef TARGET_OSX
		uint32_t count = 0;
		if (CGGetActiveDisplayList(0, nullptr, &count) != kCGErrorSuccess || count == 0) return false;

		std::vector<CGDirectDisplayID> displays(count);
		if (CGGetActiveDisplayList(count, displays.data(), &count) != kCGErrorSuccess) return false;

		for (uint32_t i = 0; i < count; i++) {
			if (CGDisplayIsBuiltin(displays[i])) {
				width = static_cast<int>(CGDisplayPixelsWide(displays[i]));
				height = static_cast<int>(CGDisplayPixelsHigh(displays[i]));
				return true;
			}
		}
		return false;
#elif defined(TARGET_WIN32)
		UINT32 num_paths = 0, num_modes = 0;
		LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &num_paths, &num_modes);
		if (result != ERROR_SUCCESS || num_paths == 0 || num_modes == 0) return false;

		std::vector<DISPLAYCONFIG_PATH_INFO> paths(num_paths);
		std::vector<DISPLAYCONFIG_MODE_INFO> modes(num_modes);
		result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &num_paths, paths.data(), &num_modes, modes.data(), nullptr);
		if (result != ERROR_SUCCESS) return false;

		for (UINT32 i = 0; i < num_paths; i++) {
			if (paths[i].targetInfo.outputTechnology != DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL) continue;
			const LUID adapter_id = paths[i].sourceInfo.adapterId;
			const UINT32 source_id = paths[i].sourceInfo.id;

			for (UINT32 j = 0; j < num_modes; j++) {
				if (modes[j].infoType != DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) continue;
				if (modes[j].adapterId.HighPart != adapter_id.HighPart || modes[j].adapterId.LowPart != adapter_id.LowPart) continue;
				if (modes[j].id != source_id) continue;

				width = static_cast<int>(modes[j].sourceMode.width);
				height = static_cast<int>(modes[j].sourceMode.height);
				return width > 0 && height > 0;
			}
		}
		return false;
#endif
		return false;
	}

	static bool getWorkAreaInsets(int display_width, int display_height, int & left, int & top, int & right, int & bottom) {
#ifdef TARGET_OSX
		GLFWmonitor * monitor = glfwGetPrimaryMonitor();
		if (!monitor) return false;

		const GLFWvidmode * mode = glfwGetVideoMode(monitor);
		if (!mode || mode->width <= 0 || mode->height <= 0) return false;

		int wx = 0;
		int wy = 0;
		int ww = 0;
		int wh = 0;
		glfwGetMonitorWorkarea(monitor, &wx, &wy, &ww, &wh);

		const float scale_x = static_cast<float>(display_width) / static_cast<float>(mode->width);
		const float scale_y = static_cast<float>(display_height) / static_cast<float>(mode->height);
		left = static_cast<int>(wx * scale_x + 0.5f);
		top = static_cast<int>(wy * scale_y + 0.5f);
		right = static_cast<int>((mode->width - (wx + ww)) * scale_x + 0.5f);
		bottom = static_cast<int>((mode->height - (wy + wh)) * scale_y + 0.5f);
#elif defined(TARGET_WIN32)
		HMONITOR monitor = MonitorFromPoint(POINT { 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

		MONITORINFO mi {};
		mi.cbSize = sizeof(mi);
		if (!GetMonitorInfo(monitor, &mi)) return false;

		const int monitor_width = mi.rcMonitor.right - mi.rcMonitor.left;
		const int monitor_height = mi.rcMonitor.bottom - mi.rcMonitor.top;
		if (monitor_width <= 0 || monitor_height <= 0) return false;

		const int left_raw = mi.rcWork.left - mi.rcMonitor.left;
		const int right_raw = mi.rcMonitor.right - mi.rcWork.right;
		const int top_raw = mi.rcWork.top - mi.rcMonitor.top;
		const int bottom_raw = mi.rcMonitor.bottom - mi.rcWork.bottom;

		const float scale_x = static_cast<float>(display_width) / static_cast<float>(monitor_width);
		const float scale_y = static_cast<float>(display_height) / static_cast<float>(monitor_height);
		left = static_cast<int>(left_raw * scale_x + 0.5f);
		top = static_cast<int>(top_raw * scale_y + 0.5f);
		right = static_cast<int>(right_raw * scale_x + 0.5f);
		bottom = static_cast<int>(bottom_raw * scale_y + 0.5f);
#else
		return false;
#endif
		if (left < 0) left = 0;
		if (top < 0) top = 0;
		if (right < 0) right = 0;
		if (bottom < 0) bottom = 0;
		return true;
	}

	inline static std::string display_mode = "FULL HD";
	inline static DisplaySize display_size = DisplaySize(0, 0);
	inline static DisplaySize pc_display_size = DisplaySize(0, 0);
	inline static const DisplaySize FULL_HD = DisplaySize(1920, 1080);
	inline static const DisplaySize UHD_4K = DisplaySize(3840, 2160);

	static void setDisplaySize(DisplaySize display) {
		display_size = display;
		ofSetWindowShape(display_size.width, display_size.height);
	}

	static void setup() {
		Font::Tiny.load(FONT_REGULAR_PATH, 12);
		Font::Small.load(FONT_REGULAR_PATH, 16);
		Font::Middle.load(FONT_MEDIUM_PATH, 32);
		Font::Large.load(FONT_SEMIBOLD_PATH, 64);

        if (!getInternalDisplaySize(pc_display_size.width, pc_display_size.height)) {
			pc_display_size.width = ofGetScreenWidth();
			pc_display_size.height = ofGetScreenHeight();
		}
		if (getWorkAreaInsets(pc_display_size.width, pc_display_size.height, pc_display_size.left_safe_inset, pc_display_size.top_safe_inset, pc_display_size.right_safe_inset, pc_display_size.bottom_safe_inset)) {
			const int work_width = pc_display_size.width - pc_display_size.left_safe_inset - pc_display_size.right_safe_inset;
			const int work_height = pc_display_size.height - pc_display_size.top_safe_inset - pc_display_size.bottom_safe_inset;
			if (work_width > 0 && work_height > 0) {
				pc_display_size.width = work_width;
				pc_display_size.height = work_height;
			} else {
				pc_display_size.left_safe_inset = 0;
				pc_display_size.top_safe_inset = 0;
				pc_display_size.right_safe_inset = 0;
				pc_display_size.bottom_safe_inset = 0;
			}
		} else {
			pc_display_size.left_safe_inset = 0;
			pc_display_size.top_safe_inset = 0;
			pc_display_size.right_safe_inset = 0;
			pc_display_size.bottom_safe_inset = 0;
		}
        printf("Display Size: W x H = %d x %d\r\n", pc_display_size.width, pc_display_size.height);
		printf("Safe Insets: L=%d T=%d R=%d B=%d\r\n", pc_display_size.left_safe_inset, pc_display_size.top_safe_inset, pc_display_size.right_safe_inset, pc_display_size.bottom_safe_inset);
		
        setDisplaySize(FULL_HD);
		if(pc_display_size.width > display_size.width){
			pc_display_size.width = display_size.width;
		}
		if(pc_display_size.height > display_size.height){
			pc_display_size.height = display_size.height;
		}
        ofSetWindowPosition(pc_display_size.left_safe_inset, pc_display_size.top_safe_inset);
	};
};

#endif
