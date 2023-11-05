//
// Created by PinkySmile on 07/04/2020.
//

#ifndef SOFGV_UTILS_HPP
#define SOFGV_UTILS_HPP


#include <string>
#include <vector>
#include <filesystem>
#ifndef __ANDROID__
#include <TGUI/TGUI.hpp>
#endif

#ifndef _WIN32
#ifdef USE_SDL
#include <SDL2/SDL.h>
#define MB_ICONERROR SDL_MESSAGEBOX_ERROR
#define MB_ICONINFORMATION SDL_MESSAGEBOX_INFORMATION
#define MB_ICONWARNING SDL_MESSAGEBOX_WARNING
#else
#define MB_ICONERROR 0x10
#define MB_ICONINFORMATION 0x20
#define MB_ICONWARNING 0x30
#endif
#else
#include <windows.h>
#endif


namespace SpiralOfFate::Utils
{
	struct HSLColor {
		unsigned char h;
		unsigned char s;
		unsigned char l;
	};

	//! @brief Get the last Exception Name
	//! @details Return the last type of Exception name
	//! @return std::string The last Exception name
	std::string getLastExceptionName();

	//! @brief clean the . and .. in relative paths
	//! @param path filesystem path to clean
	//! @return std::string Converted Path in String
	std::string cleanPath(const std::string &path);

	template<typename T>
	std::wstring utf8ToUtf16(const std::basic_string<T> &str, wchar_t replacement = L'?')
	{
		std::wstring result;
		size_t i = 0;

		static_assert(sizeof(T) == 1);
		result.reserve(str.size());
		while (i < str.size()) {
			if ((signed char)str[i] >= 0) {
				result += str[i];
				i++;
				continue;
			}
			if ((str[i] & 0b11100000) == 0b11000000) {
				if (i + 1 >= str.size() || (str[i + 1] & 0b11000000) != 0b10000000) {
					result += replacement;
					i += 2;
					continue;
				}

				int c = ((str[i] & 0b00011111) << 6) | (str[i + 1] & 0b00111111);

				result += (wchar_t)c;
				i += 2;
				continue;
			}
			if ((str[i] & 0b11110000) == 0b11100000) {
				if (i + 2 >= str.size() || (str[i + 1] & 0b11000000) != 0b10000000 || (str[i + 2] & 0b11000000) != 0b10000000) {
					result += replacement;
					i += 3;
					continue;
				}

				unsigned short c = ((str[i] & 0b00001111) << 12) | ((str[i + 1] & 0b00111111) << 6) | (str[i + 2] & 0b00111111);

				if (c > 0xD7FF && c < 0xE000) {
					result += replacement;
					i += 3;
					continue;
				}
				result += (wchar_t)c;
				i += 3;
				continue;
			}
			if ((str[i] & 0b11111000) == 0b11110000) {
				if (
					i + 3 >= str.size() ||
					(str[i + 1] & 0b11000000) != 0b10000000 ||
					(str[i + 2] & 0b11000000) != 0b10000000 ||
					(str[i + 3] & 0b11000000) != 0b10000000
					) {
					result += replacement;
					i += 4;
					continue;
				}

				unsigned short c =
					((str[i] & 0b00001111) << 18) |
					((str[i + 1] & 0b00111111) << 12) |
					((str[i + 2] & 0b00111111) << 6) |
					(str[i + 3] & 0b00111111);

				result += (wchar_t)(0xD800 | ((c >> 10) & 0b1111111111));
				result += (wchar_t)(0xDC00 | (c & 0b1111111111));
			}
			result += replacement;
			i++;
		}
		return result;
	}

#ifdef USE_TGUI
	//! @brief Display a Windows dialog box.
	//! @details This functions opens a Windows dialog box and return the button clicked by the user.
	//! @param title The title of the window.
	//! @param content The content of the box.
	//! @param variate A bit combination of the window attributes (see Windows MessageBox function for a list of the enums).
	//! @return The button clicked by the user.
	//! @note On Non-Windows systems, it will simulate the Windows dialog box. Only MB_ICONERROR and MB_OK are simulated on those systems.
	int	dispMsg(const std::string &title, const std::string &content, int variate, Screen *win = nullptr);

	//! @brief Opens a FileDialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	std::filesystem::path openFileDialog(const std::string &title = "Open file", const std::string &basePath = ".", const std::vector<std::pair<std::string, std::string>> &patterns = {}, bool overWriteWarning = false, bool mustExist = true);

	//! @brief Saves a file dialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	std::filesystem::path saveFileDialog(const std::string &title = "Save file", const std::string &basePath = ".", const std::vector<std::pair<std::string, std::string>> &patterns = {});

	//! @brief Display a focused window.
	//! @param gui The gui handling the window.
	//! @param width The width of the window.
	//! @param height The height of the window.
	//! @return A pointer to the window
	tgui::ChildWindow::Ptr openWindowWithFocus(tgui::Gui &gui, tgui::Layout width, tgui::Layout height);

	//! @brief Display a window with a slider.
	//! @param gui The gui handling the window.
	//! @param onFinish Function to call when exiting the window.
	//! @return A pointer to the window
	tgui::ChildWindow::Ptr makeSliderWindow(tgui::Gui &gui, const std::function<void(float value)> &onFinish, float defaultValue = 1, float min = 0, float max = 20, float step = 1);

	//! @brief Display a window with a Color picker.
	//! @param gui The gui handling the window.
	//! @param onFinish Function to call when exiting the window.
	//! @param startColor The color to initialize the picker.
	//! @return A pointer to the window
	tgui::ChildWindow::Ptr makeColorPickWindow(tgui::Gui &gui, const std::function<void(sf::Color color)> &onFinish, sf::Color startColor);
	tgui::Theme &getTheme();
	void setRenderer(const tgui::Widget::Ptr &widget);
#else
	#define openWindowWithFocus(...) __nothing2()
	#define makeSliderWindow(...) __nothing2()
	#define makeColorPickWindow(...) __nothing2()
	#define openFileDialog(...) __nothing3()
	#define saveFileDialog(...) __nothing3()
#ifdef USE_SDL
	int dispMsg(const std::string &title, const std::string &content, int variate, Screen *win = nullptr);
#else
	#define dispMsg(...) __nothing()
#endif
	#define setRendered(...) __nothing()
	#define getTheme() *(tgui::Theme *)nullptr
	void __nothing();
	void *__nothing2();
	std::filesystem::path __nothing3();
#endif

	HSLColor RGBtoHSL(const sf::Color &color);
	sf::Color HSLtoRGB(const HSLColor &color);
}


#endif //SOFGV_UTILS_HPP
