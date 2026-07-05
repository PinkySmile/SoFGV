//
// Created by PinkySmile on 07/04/2020.
//

#ifndef SOFGV_UTILS_HPP
#define SOFGV_UTILS_HPP


#include <string>
#include <vector>
#include <filesystem>
#include "Color.hpp"
#if !defined(__ANDROID__) && defined(USE_TGUI)
#include <TGUI/TGUI.hpp>
#ifdef USE_SDL
#include <TGUI/Backend/SDL-Renderer.hpp>
#else
#include <TGUI/Backend/SFML-Graphics.hpp>
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#undef MessageBox
#elif defined(USE_SDL)
#include <SDL2/SDL.h>
#define MB_ICONERROR SDL_MESSAGEBOX_ERROR
#define MB_ICONINFORMATION SDL_MESSAGEBOX_INFORMATION
#define MB_ICONWARNING SDL_MESSAGEBOX_WARNING
#else
#define MB_ICONERROR 0x10
#define MB_ICONINFORMATION 0x20
#define MB_ICONWARNING 0x30
#endif

#define COALESCE(ptr1, ptr2) ((ptr1) ? *(ptr1) : (ptr2))

namespace SpiralOfFate::Utils
{
	namespace Z {
		std::vector<unsigned char> compress(const std::vector<unsigned char> &buffer, int level);
		std::vector<unsigned char> decompress(const std::vector<unsigned char> &buffer);
		int compress(const unsigned char *inBuffer, size_t size, std::vector<unsigned char> &outBuffer, int level);
		int decompress(const unsigned char *inBuffer, size_t size, std::vector<unsigned char> &outBuffer);
		std::string error(int ret);
	}

	template<typename T, size_t s, typename T2, T2 ... ints>
	inline std::array<T, s> __createArray(T *data, std::index_sequence<ints...>)
	{
		return { data[ints]... };
	}

	template<typename T, size_t s>
	inline std::array<T, s> createArray(T *data)
	{
		return __createArray<T, s>(data, std::make_index_sequence<s>{});
	}

	std::string getLocale();

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

	bool isBeingDebugged();
	// FIXME: Forward declare
	void mergeInputs(class InputStruct &inputs1, const class InputStruct &inputs2);

	unsigned char *allocateManually(size_t size);
	void deallocateManually(unsigned char *buffer);

#define TOHEX_ENDIAN_LITTLE false
#define TOHEX_ENDIAN_BIG true
	std::string toHex(const unsigned char *buffer, size_t size, bool endian);

	template<typename T, size_t size = sizeof(T)>
	std::enable_if_t<std::is_integral_v<T>, std::string> toHex(const T &v)
	{
		if constexpr (std::endian::native == std::endian::big)
			return toHex(reinterpret_cast<const unsigned char *>(&v), size, TOHEX_ENDIAN_BIG);
		else if constexpr (std::endian::native == std::endian::little)
			return toHex(reinterpret_cast<const unsigned char *>(&v), size, TOHEX_ENDIAN_LITTLE);
		else
			static_assert(false, "Unknown endianness");
	}

#ifdef USE_TGUI
	void setRenderer(tgui::Container &widget);
	void setRenderer(tgui::Container *widget);
	void setRenderer(const tgui::Gui &widget);

	template<typename T>
	void setRenderer(const std::shared_ptr<T> &widget)
	{
		auto renderer = tgui::Theme::getDefault()->getRendererNoThrow(widget->getWidgetType());

		if (renderer)
			widget->setRenderer(renderer);
	}
	template<typename T>
	void setRenderer(const std::shared_ptr<T> &widget, const std::string &name)
	{
		widget->setRenderer(tgui::Theme::getDefault()->getRenderer(name));
	}

	template<>
	void setRenderer(const tgui::Container::Ptr &widget);

	//! @brief Display a focused window.
	//! @param gui The gui handling the window.
	//! @param width The width of the window.
	//! @param height The height of the window.
	//! @return A pointer to the window
	template<typename T, typename WindowType = tgui::ChildWindow, typename ...Args>
	typename WindowType::Ptr openWindowWithFocus(T &gui, tgui::Layout width, tgui::Layout height, typename WindowType::Ptr window = nullptr, bool closeOut = true, Args ...args)
	{
		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		panel->setUserData(false);
		gui.add(panel);

		if (!window) {
			window = WindowType::create(args...);
			if (!width.isConstant() || width.getValue() != 0)
				window->setSize(width, height);
		} else
			setRenderer(static_cast<tgui::Container::Ptr>(window));
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window);

		window->setFocused(true);

		auto closeWindow = [&gui, window, panel]{
			gui.remove(window);
			gui.remove(panel);
		};

		if (closeOut)
			panel->onClick(closeWindow);
		window->onClose(closeWindow);
		window->onEscapeKeyPress(closeWindow);
		return window;
	}

	//! @brief Display a Windows dialog box.
	//! @details This functions opens a Windows dialog box and return the button clicked by the user.
	//! @param title The title of the window.
	//! @param content The content of the box.
	//! @param variate A bit combination of the window attributes (see Windows MessageBox function for a list of the enums).
	//! @return The button clicked by the user.
	//! @note On Non-Windows systems, it will simulate the Windows dialog box. Only MB_ICONERROR and MB_OK are simulated on those systems.
	template<typename T>
	tgui::MessageBox::Ptr dispMsg(T &gui, const std::string &title, const std::string &content, int variate)
	{
		auto dialog = tgui::MessageBox::create(title, content, { "OK" });

		// TODO: use variate
		(void)variate;
		openWindowWithFocus(gui, 0, 0, dialog);
		dialog->onButtonPress([](const std::weak_ptr<tgui::MessageBox> &d){
			d.lock()->close();
		}, std::weak_ptr(dialog));
		return dialog;
	}

	//! @brief Opens a FileDialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	tgui::FileDialog::Ptr openFileDialog(tgui::Gui &gui, const std::string &title = "Open file", const std::filesystem::path &basePath = ".", bool overWriteWarning = false, bool mustExist = true);

	//! @brief Saves a file dialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	tgui::FileDialog::Ptr saveFileDialog(tgui::Gui &gui, const std::string &title = "Save file", const std::filesystem::path &basePath = ".");

	//! @brief Display a window with a slider.
	//! @param gui The gui handling the window.
	//! @param onFinish Function to call when exiting the window.
	//! @return A pointer to the window
	tgui::ChildWindow::Ptr makeSliderWindow(tgui::Gui &gui, const std::function<void(float value)> &onFinish, float defaultValue = 1, float min = 0, float max = 20, float step = 1);
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
	#define setRenderer(...) __nothing()
	#define getTheme() *(tgui::Theme *)nullptr
	void __nothing();
	void *__nothing2();
	std::filesystem::path __nothing3();
#endif

	template<class... Ts>
	struct overloads : Ts... { using Ts::operator()...; };
}


#endif //SOFGV_UTILS_HPP
