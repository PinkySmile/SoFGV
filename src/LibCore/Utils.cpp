//
// Created by PinkySmile on 07/04/2020.
//

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#endif
#ifdef __linux__
#include <sys/ptrace.h>
#endif
#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include <regex>
#include <codecvt>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <numeric>
#include <fstream>
#include <cmath>
#include "Resources/Game.hpp"
#include "Utils.hpp"
#ifdef HAS_NETWORK
#include <zlib.h>
#endif

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#ifdef HAS_NETWORK
namespace SpiralOfFate::Utils::Z
{
	static constexpr long int CHUNK = {16384};

	int compress(const unsigned char *inBuffer, size_t size, std::vector<unsigned char> &outBuffer, int level)
	{
		int ret;
		unsigned have;
		z_stream strm;
		unsigned char out[CHUNK];

		outBuffer.clear();
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		ret = deflateInit(&strm, level);
		if (ret != Z_OK)
			return ret;

		strm.avail_in = size;
		strm.next_in = inBuffer;
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, Z_FINISH);    /* anyone error value */
			assert_exp(ret != Z_STREAM_ERROR);
			have = CHUNK - strm.avail_out;
			outBuffer.insert(outBuffer.end(), out, out + have);
		} while (strm.avail_out == 0);
		assert_exp(strm.avail_in == 0);
		assert_exp(ret == Z_STREAM_END);
		deflateEnd(&strm);
		return Z_OK;
	}

	int decompress(const unsigned char *inBuffer, size_t size, std::vector<unsigned char> &outBuffer)
	{
		int ret;
		unsigned have;
		z_stream strm;
		unsigned char out[CHUNK];

		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit(&strm);
		if (ret != Z_OK)
			return ret;

		strm.avail_in = size;
		strm.next_in = inBuffer;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert_exp(ret != Z_STREAM_ERROR);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
				// FALLTHROUGH
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&strm);
				return ret;
			default:
				have = CHUNK - strm.avail_out;
				outBuffer.insert(outBuffer.end(), out, out + have);
			}
		} while (strm.avail_out == 0);
		assert_exp(ret == Z_STREAM_END);

		inflateEnd(&strm);
		return Z_OK;
	}

	std::vector<unsigned char> compress(const std::vector<unsigned char> &buffer, int level)
	{
		std::vector<unsigned char> result;
		int ret = compress(buffer.data(), buffer.size(), result, level);

		if (ret != Z_OK)
			throw std::runtime_error(error(ret));
		return result;
	}

	std::vector<unsigned char> decompress(const std::vector<unsigned char> &buffer)
	{
		std::vector<unsigned char> result;
		int ret = decompress(buffer.data(), buffer.size(), result);

		if (ret != Z_OK)
			throw std::runtime_error(error(ret));
		return result;
	}

	std::string error(int ret)
	{
		switch (ret) {
		case Z_ERRNO:
			if (ferror(stdin))
				return "Error reading from stdin.";
			else if (ferror(stdout))
				return "Error writing ro stdout.";
			return "Errno error";
		case Z_STREAM_ERROR:
			return "Invalid compression level.";
		case Z_DATA_ERROR:
			return "Empty data, invalid or incomplete.";
		case Z_MEM_ERROR:
			return "No memory.";
		case Z_VERSION_ERROR:
			return "zlib version is incompatible.";
		default:
			return "Unknown error " + std::to_string(ret);
		}
	}
}
#endif

namespace SpiralOfFate::Utils
{
	static const std::unordered_map<std::string, std::string> _icons{
		{"folder", "assets/icons/folder.png"     },
		{".rar",   "assets/icons/archive.png"    },
		{".zip",   "assets/icons/archive.png"    },
		{".7z",    "assets/icons/archive.png"    },
		{".tgz",   "assets/icons/archive.png"    },
		{".gz",    "assets/icons/archive.png"    },
		{".xz",    "assets/icons/archive.png"    },
		{".htm",   "assets/icons/webFile.png"    },
		{".html",  "assets/icons/webFile.png"    },
		{".css",   "assets/icons/webFile.png"    },
		{".mimp",  "assets/icons/mimpFile.png"   },
		{".iso",   "assets/icons/discFile.png"   },
		{".txt",   "assets/icons/textFile.png"   },
		{".doc",   "assets/icons/textFile.png"   },
		{".mid",   "assets/icons/midiFile.png"   },
		{".midi",  "assets/icons/midiFile.png"   },
		{".class", "assets/icons/javaFile.png"   },
		{".jar",   "assets/icons/javaFile.png"   },
		{".ttf",   "assets/icons/fontsFile.png"  },
		{".otf",   "assets/icons/fontsFile.png"  },
		{".wav",   "assets/icons/musicFile.png"  },
		{".ogg",   "assets/icons/musicFile.png"  },
		{".mp3",   "assets/icons/musicFile.png"  },
		{".png",   "assets/icons/imageFile.png"  },
		{".jpg",   "assets/icons/imageFile.png"  },
		{".jpeg",  "assets/icons/imageFile.png"  },
		{".bmp",   "assets/icons/imageFile.png"  },
		{".gif",   "assets/icons/imageFile.png"  },
		{".exe",   "assets/icons/binaryFile.png" },
		{".o",     "assets/icons/binaryFile.png" },
		{".out",   "assets/icons/binaryFile.png" },
		{".a",     "assets/icons/binaryFile.png" },
		{".gch",   "assets/icons/binaryFile.png" },
		{".dll",   "assets/icons/binaryFile.png" },
		{".so",    "assets/icons/binaryFile.png" },
		{".xml",   "assets/icons/configFile.png" },
		{".ini",   "assets/icons/configFile.png" },
		{".json",  "assets/icons/configFile.png" },
		{".md",    "assets/icons/configFile.png" },
		{".c",     "assets/icons/sourceFile.png" },
		{".h",     "assets/icons/sourceFile.png" },
		{".hpp",   "assets/icons/sourceFile.png" },
		{".cpp",   "assets/icons/sourceFile.png" },
		{".lua",   "assets/icons/sourceFile.png" },
		{".java",  "assets/icons/sourceFile.png" },
		{".py",    "assets/icons/sourceFile.png" },
		{".asm",   "assets/icons/sourceFile.png" },
		{".php",   "assets/icons/sourceFile.png" },
		{".js",    "assets/icons/sourceFile.png" },
		{".sh",    "assets/icons/shellScript.png"},
		{".run",   "assets/icons/shellScript.png"},
		{".bat",   "assets/icons/shellScript.png"},
		{"",       "assets/icons/unknownFile.png"},
	};

	std::string getLastExceptionName()
	{
#ifdef __GNUG__
		int status;
		char *value;
		std::string name;

		auto val = abi::__cxa_current_exception_type();

		if (!val)
			return "No exception";

		value = abi::__cxa_demangle(val->name(), nullptr, nullptr, &status);
		name = value;
		free(value);
		return name;
#else
		return "Unknown exception";
#endif
	}

#ifdef USE_TGUI
	tgui::FileDialog::Ptr openFileDialog(tgui::Gui &gui, const std::string &title, const std::filesystem::path &basePath, bool overWriteWarning, bool mustExist)
	{
		auto dialog = tgui::FileDialog::create(title, overWriteWarning ? "Save" : "Open", overWriteWarning);

		dialog->setFileMustExist(mustExist);
		dialog->setPath(tgui::Filesystem::Path(absolute(basePath)));
		openWindowWithFocus(gui, 0, 0, dialog);
		return dialog;
	}

	tgui::FileDialog::Ptr saveFileDialog(tgui::Gui &gui, const std::string &title, const std::filesystem::path &basePath)
	{
		return openFileDialog(gui, title, basePath, true, false);
	}
#else
#ifdef USE_SDL
	int dispMsg(const std::string &title, const std::string &content, int variate, Screen *win)
	{
		SDL_ShowSimpleMessageBox(variate, title.c_str(), content.c_str(), win ? win->getSDLWindow() : nullptr);
		return 0;
	}
#endif
	void __nothing() {};
	void *__nothing2() { return nullptr; };
	std::filesystem::path __nothing3() { return "Not implemented"; }
#endif

#ifdef USE_TGUI
	void setRenderer(tgui::Container *widget)
	{
		setRenderer(*widget);
	}

	void setRenderer(tgui::Container &widget)
	{
		auto renderer = tgui::Theme::getDefault()->getRendererNoThrow(widget.getWidgetType());

		if (renderer)
			widget.setRenderer(renderer);
		for (auto &w : widget.getWidgets()) {
			try {
				if (!w->getUserData<bool>())
					continue;
			} catch (std::bad_any_cast &) {}
			if (auto c = w->cast<tgui::Container>())
				Utils::setRenderer(c);
			else
				Utils::setRenderer(w);
		}
	}

	void setRenderer(const tgui::Gui &widget)
	{
		for (auto &w : widget.getWidgets()) {
			try {
				if (!w->getUserData<bool>())
					continue;
			} catch (std::bad_any_cast &) {}
			if (auto c = w->cast<tgui::Container>())
				Utils::setRenderer(c);
			else
				Utils::setRenderer(w);
		}
	}

	template<>
	void setRenderer(const tgui::Container::Ptr &widget)
	{
		Utils::setRenderer(*widget);
	}

	tgui::ChildWindow::Ptr makeColorPickWindow(tgui::Gui &gui, const std::function<void(sf::Color color)> &onFinish, sf::Color startColor)
	{
		auto dialog = tgui::ColorPicker::create("Pick color", startColor);
		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		gui.add(panel);
		dialog->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(dialog);

		dialog->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, dialog, panel, tabUsageEnabled]{
			gui.remove(dialog);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};

		panel->onClick(closeWindow);
		dialog->onClose(closeWindow);
		dialog->onEscapeKeyPress(closeWindow);
		dialog->onOkPress(onFinish);
		return dialog;
	}

	tgui::ChildWindow::Ptr makeSliderWindow(tgui::Gui &gui, const std::function<void(float value)> &onFinish, float defaultValue, float min, float max, float step)
	{
		auto window = openWindowWithFocus(gui, 260, 80);
		window->loadWidgetsFromFile("assets/gui/slider.gui");

		auto slider = window->get<tgui::Slider>("Slider");
		auto txt = window->get<tgui::Label>("Nb");
		auto sliderCallback = [slider, txt]{
			std::stringstream s;

			s << slider->getValue();
			txt->setText(s.str());
		};
		std::stringstream s;

		slider->setMaximum(min);
		slider->setMaximum(max);
		slider->setStep(step);
		slider->setValue(defaultValue);
		slider->onValueChange(sliderCallback);

		s << slider->getValue();
		txt->setText(s.str());

		window->get<tgui::Button>("Cancel")->onClick([window]{
			window->close();
		});
		window->get<tgui::Button>("Ok")->onClick([onFinish, slider, window]{
			if (onFinish)
				onFinish(slider->getValue());
			window->close();
		});
		return window;
	}

	std::string getLocale()
	{
#ifdef _WIN32
		char locale[LOCALE_NAME_MAX_LENGTH] = {0};
		wchar_t localw[LOCALE_NAME_MAX_LENGTH] = {0};

		GetUserDefaultLocaleName(localw, sizeof(localw));
		for (int i = 0; localw[i]; i++)
			locale[i] = localw[i];

		auto end = strchr(locale, '-');

		if (end)
			*end = 0;
		return locale;
#else
		char locale[1024] = {0};
		char *lang = getenv("LANG");

		if (!lang)
			strcpy(locale, "en_EN");
		else
			strncpy(locale, lang, sizeof(locale) - 1);

		auto end = strchr(locale, '_');

		if (end)
			*end = 0;
		return locale;
#endif
	}
#endif

	// https://forum.juce.com/t/detecting-if-a-process-is-being-run-under-a-debugger/2098
	bool isBeingDebugged()
	{
	#ifdef _WIN32
		return IsDebuggerPresent() == TRUE;
	#elif defined(__linux__)
		static bool isCheckedAlready = false;
		static bool underDebugger = false;

		if (!isCheckedAlready) {
			if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
				underDebugger = true;
			else
				ptrace(PTRACE_DETACH, 0, 1, 0);
			isCheckedAlready = true;
		}
		return underDebugger;
	#else
		return false;
	#endif
	}

	void mergeInputs(InputStruct &inputs1, const InputStruct &inputs2)
	{
		if (std::abs(inputs1.horizontalAxis) < std::abs(inputs2.horizontalAxis))
			inputs1.horizontalAxis = inputs2.horizontalAxis;
		if (std::abs(inputs1.verticalAxis) < std::abs(inputs2.verticalAxis))
			inputs1.verticalAxis = inputs2.verticalAxis;
		for (size_t i = 2; i < sizeof(inputs1) / sizeof(int); i++)
			((int *)&inputs1)[i] = std::max(((int *)&inputs1)[i], ((int *)&inputs2)[i]);
	}

	unsigned char *allocateManually(size_t size)
	{
		return new unsigned char[size];
	}
	void deallocateManually(unsigned char *buffer)
	{
		delete[] buffer;
	}

	std::string toHex(const unsigned char *buffer, size_t size, bool endian)
	{
		char digits[] = "0123456789ABCDEF";
		std::string result;

		result.resize(size * 2);
		for (size_t i = 0; i < size; i++) {
			size_t index = endian == TOHEX_ENDIAN_BIG ? i : size - i - 1;
			unsigned char c = buffer[index];

			result[i * 2 + 0] = digits[c >> 4];
			result[i * 2 + 1] = digits[c & 0xF];
		}
		return result;
	}
}
