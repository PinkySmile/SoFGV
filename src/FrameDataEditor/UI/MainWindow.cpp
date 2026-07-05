//
// Created by PinkySmile on 05/05/25.
//

#include <TGUI/RendererDefines.hpp>
#include "MainWindow.hpp"

#include <ranges>

#include "ColorPlaneWidget.hpp"
#include "ColorSliderWidget.hpp"
#include "../Operations/DummyOperation.hpp"
#include "../Operations/FlagOperation.hpp"
#include "../Operations/BasicDataOperation.hpp"
#include "../Operations/CleanPropertiesOperation.hpp"
#include "../Operations/EditSpriteOperation.hpp"
#include "../Operations/EditSoundOperation.hpp"
#include "../Operations/ClearBlockOperation.hpp"
#include "../Operations/ClearHitOperation.hpp"
#include "../Operations/CreateMoveOperation.hpp"
#include "../Operations/CreateFrameOperation.hpp"
#include "../Operations/CreateBlockOperation.hpp"
#include "../Operations/CreateBoxOperation.hpp"
#include "../Operations/EditColorOperation.hpp"
#include "../Operations/CreatePaletteOperation.hpp"
#include "../Operations/EditColorsOperation.hpp"
#include "../Operations/FlattenCollisionBoxesOperation.hpp"
#include "../Operations/FlattenPropretiesOperation.hpp"
#include "../Operations/RemovePaletteOperation.hpp"
#include "../Operations/RemoveBoxOperation.hpp"
#include "../Operations/RemoveFrameOperation.hpp"
#include "../Operations/RemoveBlockOperation.hpp"
#include "../Operations/RemoveMoveOperation.hpp"
#include "../Operations/PasteDataOperation.hpp"
#include "../Operations/PasteAnimDataOperation.hpp"
#include "../Operations/PasteBoxDataOperation.hpp"

static ColorConversionCb colorConversions[] = {
	[](unsigned r, unsigned g, unsigned b) {
		return SpiralOfFate::Color(r, g, b, 255);
	},
	[](unsigned h, unsigned s1k, unsigned l1k) { // https://en.wikipedia.org/wiki/HSL_and_HSV#HSL_to_RGB_alternative
		double s = s1k / 1000.;
		double l = l1k / 1000.;
		auto f = [h, s, l](unsigned n){
			double k = std::fmod(n + h / 30., 12);
			double a = s * std::min(l, 1 - l);

			return l - a * std::max(-1., std::min(1., std::min(k - 3, 9 - k)));
		};

		return SpiralOfFate::Color(f(0) * 255, f(8) * 255, f(4) * 255, 255);
	},
	[](unsigned h, unsigned s1k, unsigned v1k) { // https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB_alternative
		double s = s1k / 1000.;
		double v = v1k / 1000.;
		auto f = [h, s, v](unsigned n){
			double k = std::fmod(n + h / 60., 6);

			return v - v * s * std::max(0., std::min(k, std::min(4 - k, 1.)));
		};

		return SpiralOfFate::Color(f(5) * 255, f(3) * 255, f(1) * 255, 255);
	}
};
static ReverseColorConversionCb colorConversionsReverse[] = {
	[](const SpiralOfFate::Color &color) {
		return WidgetColor{color.r, color.g, color.b};
	},
	[](const SpiralOfFate::Color &color) { // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB
		double r = color.r / 255.;
		double g = color.g / 255.;
		double b = color.b / 255.;
		double xmax = std::max(r, std::max(g, b));
		double xmin = std::min(r, std::min(g, b));
		double chroma = xmax - xmin;
		double h;
		double s;
		double v = xmax;
		double l = (xmax + xmin) / 2;

		if (chroma == 0)
			h = 0;
		else if (xmax == r)
			h = 60 * std::fmod((g - b) / (float)chroma, 6);
		else if (xmax == g)
			h = 60 * ((b - r) / (float)chroma + 2);
		else if (xmax == b)
			h = 60 * ((r - g) / (float)chroma + 4);
		else
			assert_not_reached();
		h = std::fmod(h, 360);
		if (l == 0 || l == 1)
			s = 0;
		else
			s = (v - l) / std::min(l, 1 - l);
		return WidgetColor{(unsigned)h, static_cast<unsigned>(1000 * s), static_cast<unsigned>(1000 * l)};
	},
	[](const SpiralOfFate::Color &color) { // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB
		double r = color.r / 255.;
		double g = color.g / 255.;
		double b = color.b / 255.;
		double xmax = std::max(r, std::max(g, b));
		double xmin = std::min(r, std::min(g, b));
		double chroma = xmax - xmin;
		double h;
		double s;
		double v = xmax;

		if (chroma == 0)
			h = 0;
		else if (xmax == r)
			h = 60 * std::fmod((g - b) / (float)chroma, 6);
		else if (xmax == g)
			h = 60 * ((b - r) / (float)chroma + 2);
		else if (xmax == b)
			h = 60 * ((r - g) / (float)chroma + 4);
		else
			assert_not_reached();
		h = std::fmod(h, 360);
		if (v == 0)
			s = 0;
		else
			s = chroma / v;
		return WidgetColor{(unsigned)h, static_cast<unsigned>(1000 * s), static_cast<unsigned>(1000 * v)};
	},
};

static ColorSpaceRanges colorSpaces[] = {
	{std::pair{0U, 255U}, std::pair{0U, 255U}, std::pair{0U, 255U}}, // RGB
	{std::pair{0U, 359U}, std::pair{0U, 1000U},std::pair{0U, 1000U}},// HSL
	{std::pair{0U, 359U}, std::pair{0U, 1000U},std::pair{0U, 1000U}},// HSV
};


template<typename T>
std::string to_string(T value, int)
{
	return std::to_string(value);
}

template<>
std::string to_string(float value, int pres)
{
	char buffer[64];

	sprintf(buffer, "%.*f", pres, value);
	return buffer;
}

std::string to_hex(unsigned long long value)
{
	char buffer[17];

	sprintf(buffer, "%016llX", value);
	return buffer;
}

#define PLACE_HOOK_WIDGET(elem, src) do {                                                                \
        if (!elem)                                                                                       \
                break;                                                                                   \
        elem->onChangeStarted([this]{                                                                    \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->startTransaction();                                                                \
        });                                                                                              \
        elem->onChanged([this](WidgetColor color){                                                       \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->_colorChangeSource = src;                                                          \
                this->updateTransaction([this, color] {                                                  \
                        return new EditColorOperation(                                                   \
                                this->_editor.localize("color.edit"),                                    \
                                this->_palettes[this->_selectedPalette],                                 \
                                this->_selectedPalette,                                                  \
                                this->_selectedColor,                                                    \
                                colorConversions[this->_selectColorMethod](color[0], color[1], color[2]) \
                        );                                                                               \
                });                                                                                      \
        });                                                                                              \
        elem->onChangeEnded([this]{                                                                      \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->commitTransaction();                                                               \
        });                                                                                              \
        this->_containers.emplace(&container);                                                           \
} while (false)
#define PLACE_HOOK_HEXCOLOR(elem) do {                                                                  \
        if (!elem)                                                                                      \
                break;                                                                                  \
        elem->onFocus([this]{                                                                           \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->startTransaction();                                                               \
	}); 	        	        	        	                                        \
        elem->onUnfocus([this]{                                                                         \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->commitTransaction();                                                              \
	}); 	        	        	        	                                        \
        elem->onReturnKeyPress([this]{                                                                  \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->commitTransaction();                                                              \
	        this->startTransaction();                                                               \
	}); 	        	        	        	                                        \
        elem->onTextChange([this](const tgui::String &s){                                               \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
                                                                                                        \
                Color color;                                                                            \
                                                                                                        \
                if (s.size() < 7)                                                                       \
                        return;                                                                         \
                sscanf(s.toStdString().c_str(), "#%02hhX%02hhX%02hhX", &color.r, &color.g, &color.b);   \
                this->_colorChangeSource = 5;                                                           \
                this->updateTransaction([this, color]{                                                  \
                        return new EditColorOperation(                                                  \
                                this->_editor.localize("color.edit"),                                   \
                                this->_palettes[this->_selectedPalette],                                \
                                this->_selectedPalette,                                                 \
                                this->_selectedColor,                                                   \
                                color                                                                   \
                        );                                                                              \
                });                                                                                     \
        });                                                                                             \
        this->_containers.emplace(&container);                                                          \
} while (false)
#define PLACE_HOOK_COLOR_COMPONENT(elem, index) do {                                                                 \
        if (!elem)                                                                                                   \
                break;                                                                                               \
        elem->onFocus([this]{                                                                                        \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->startTransaction();                                                                            \
	}); 	        	        	        	                                                     \
        elem->onUnfocus([this]{                                                                                      \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->commitTransaction();                                                                           \
	}); 	        	        	        	                                                     \
        elem->onReturnKeyPress([this]{                                                                               \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->commitTransaction();                                                                           \
	        this->startTransaction();                                                                            \
	}); 	        	        	        	                                                     \
        elem->onTextChange([this](const tgui::String &s){                                                            \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
                if (s.empty())                                                                                       \
                        return;                                                                                      \
                                                                                                                     \
                auto value = std::stoul(s.toStdString());                                                            \
                auto color = this->_palettes[this->_selectedPalette].colors[this->_selectedColor];                   \
                auto converted = colorConversionsReverse[this->_selectColorMethod](color);                           \
                                                                                                                     \
                converted[index] = value;                                                                            \
                this->_colorChangeSource = index + 2;                                                                \
                this->updateTransaction([this, converted]{                                                           \
                        return new EditColorOperation(                                                               \
                                this->_editor.localize("color.edit"),                                                \
                                this->_palettes[this->_selectedPalette],                                             \
                                this->_selectedPalette,                                                              \
                                this->_selectedColor,                                                                \
                                colorConversions[this->_selectColorMethod](converted[0], converted[1], converted[2]) \
                        );                                                                                           \
                });                                                                                                  \
        });                                                                                                          \
        this->_containers.emplace(&container);                                                                       \
} while (false)
#define PLACE_HOOK_BOX(container, guiId, field, other, name)                                              \
do {                                                                                                      \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                \
                                                                                                          \
        if (!__elem)                                                                                      \
                break;                                                                                    \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                auto box = this->_preview->getSelectedBox();                                              \
                                                                                                          \
                if (box.first == BOXTYPE_NONE || s.empty()) return;                                       \
                try {                                                                                     \
			auto pos = s.find(',');                                                           \
			auto __x = s.substr(1, pos - 1).toStdString();                                    \
			auto __y = s.substr(pos + 1, s.size() - pos - 1).toStdString();                   \
                        decltype(Box::field) __##field{                                                   \
				(decltype(Box::field.x))std::stoi(__x),                                   \
				(decltype(Box::field.y))std::stoi(__y)                                    \
			};                                                                                \
                        decltype(Box::other) __##other = this->_preview->getSelectedBoxRef()->other;      \
                                                                                                          \
                        this->updateTransaction([&]{ return new EditBoxOperation(                         \
                                *this->_object,                                                           \
                                name,                                                                     \
                                box.first, box.second,                                                    \
                                Box{ .pos = __pos, .size = __size }                                       \
                        ); });                                                                            \
                } catch (...) { return; }                                                                 \
        });                                                                                               \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                               \
                Box *box = this->_preview->getSelectedBoxRef();                                           \
                                                                                                          \
                if (!box) return;                                                                         \
                __elem->onTextChange.setEnabled(false);                                                   \
                __elem->setText("(" +                                                                     \
			std::to_string(box->field.x) + "," +                                              \
			std::to_string(box->field.y) +                                                    \
		")");                                                                                     \
                __elem->onTextChange.setEnabled(true);                                                    \
        });                                                                                               \
        this->_containers.emplace(&container);                                                            \
} while (false)
#define PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                                             \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                       \
                                                                                                                 \
        if (!__elem)                                                                                             \
                break;                                                                                           \
        __elem->onFocus([this]{ this->startTransaction(); });                                            \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                         \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); });        \
        __elem->onTextChange([this](const tgui::String &s){                                              \
                if constexpr (noEmpty) if (s.empty()) return;                                                    \
                try {                                                                                            \
                        fromStringPre(s)                                                                         \
                        this->updateTransaction([&]{ return new operation(                                       \
                                *this->_object,                                                                  \
                                name,                                                                            \
                                &FrameData::field,                                                               \
                                fromString(s, decltype(FrameData::field)),                                       \
                                reset                                                                            \
                        ); });                                                                                   \
                } catch (...) { return; }                                                                        \
        });                                                                                                      \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                      \
                auto &data = this->_object->getFrameData();                                                      \
                __elem->onTextChange.setEnabled(false);                                                          \
                __elem->setText(toString(data.field, arg));                                                      \
                __elem->onTextChange.setEnabled(true);                                                           \
        });                                                                                                      \
        this->_containers.emplace(&container);                                                                   \
} while (false)
#define PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg, reset) \
do {                                                                                                                             \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                                       \
                                                                                                                                 \
        if (!__elem)                                                                                                             \
                break;                                                                                                           \
        __elem->onFocus([this]{ this->startTransaction(); });                                                            \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                                         \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); });                        \
        __elem->onTextChange([this](const tgui::String &s){                                                              \
                if (s.empty()) {                                                                                                 \
                        this->updateTransaction([&]{ return new operation(                                                       \
                                *this->_object,                                                                                  \
                                name,                                                                                            \
                                &FrameData::field,                                                                               \
                                std::make_optional<decltype(FrameData::field)::value_type>(),                                    \
                                (reset)                                                                                          \
                        ); });                                                                                                   \
                        return;                                                                                                  \
                }                                                                                                                \
                try {                                                                                                            \
                        fromStringPre(s)                                                                                         \
                        this->updateTransaction([&]{ return new operation(                                                       \
                                *this->_object,                                                                                  \
                                name,                                                                                            \
                                &FrameData::field,                                                                               \
                                std::make_optional<decltype(FrameData::field)::value_type>(                                      \
                                        fromString(s, decltype(FrameData::field)::value_type)                                    \
                                ),                                                                                               \
                                (reset)                                                                                          \
                        ); });                                                                                                   \
                } catch (...) { return; }                                                                                        \
        });                                                                                                                      \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                                      \
                auto &data = this->_object->getFrameData();                                                                      \
                __elem->onTextChange.setEnabled(false);                                                                          \
                if (data.field)                                                                                                  \
                        __elem->setText(toString(*data.field, arg));                                                             \
                else                                                                                                             \
                        __elem->setText("");                                                                                     \
                __elem->onTextChange.setEnabled(true);                                                                           \
        });                                                                                                                      \
        this->_containers.emplace(&container);                                                                                   \
} while (false)

#define NO_FROMSTRING_PRE(s)

#define STRING_FROM_STRING(s, _) s.toStdString()
#define STRING_TO_STRING(s, _) s

#define NUMBER_FROM_STRING(s, type) static_cast<type>(std::stof(s.toStdString()))
#define NUMBER_TO_STRING(s, p) to_string(s, p)

#define NUMBER_RAD_FROM_STRING(s, type) static_cast<type>(std::stof(s.toStdString()) * M_PI / 180)
#define NUMBER_RAD_TO_STRING(s, p) to_string(s * 180 / M_PI, p)

#define NUMFLAGS_FROM_STRING(s, _) { (unsigned int)std::stoul(s.toStdString(), nullptr, 16) }
#define NUMFLAGS_TO_STRING(s, _) to_hex(s.flags)

#define VECTOR_FROM_STRING_PRE(s)                                       \
        auto pos = s.find(',');                                         \
        auto __x = s.substr(1, pos - 1).toStdString();                  \
        auto __y = s.substr(pos + 1, s.size() - pos - 1).toStdString();
#define VECTOR_FROM_STRING(s, type) {       \
        (decltype(type::x))std::stof(__x), \
        (decltype(type::y))std::stof(__y)  \
}
#define VECTOR_TO_STRING(s, p) "(" + \
	to_string((s).x, p) + "," +  \
	to_string((s).y, p) +        \
")"

#define SNAP_FROM_STRING_PRE(s)                          \
        auto pos = s.find(',');                          \
        auto s2 = s.substr(pos + 1, s.size() - pos - 2); \
        auto pos2 = s2.find(',');                        \
        auto __x = s.substr(1, pos - 1).toStdString();   \
        auto __y = s2.substr(0, pos2 - 1).toStdString(); \
        auto __r = s2.substr(pos2 + 1).toStdString();
#define SNAP_FROM_STRING(s, type) {                                                           \
        { (decltype(type::first.x))std::stod(__x), (decltype(type::first.y))std::stod(__y) }, \
        (decltype(type::second))std::stof(__r) * (float)M_PI / 180.f                          \
}
#define SNAP_TO_STRING(s, _) "(" +                     \
	to_string((s).first.x, 2) + "," +              \
	to_string((s).first.y, 2) + "," +              \
	to_string((int)((s).second * 180 / M_PI), 0) + \
")"

#define RECT_FROM_STRING_PRE(s)                                     \
        auto __pos = s.find(',');                                   \
        auto __x = s.substr(1, __pos - 1).toStdString();            \
	                                                            \
        auto __remainder = s.substr(__pos + 1);                     \
        auto __pos2 = __remainder.find(',');                        \
        auto __y = __remainder.substr(0, __pos2 + 1).toStdString(); \
	                                                            \
        auto __remainder2 = __remainder.substr(__pos2 + 1);         \
        auto __pos3 = __remainder2.find(',');                       \
        auto __w = __remainder2.substr(0, __pos3 + 1).toStdString();\
        auto __h = __remainder2.substr(__pos3 + 1, __remainder2.size() - __pos3 - 1).toStdString();
#define RECT_FROM_STRING(s, type) {                                                       \
        {(decltype(type::pos.x))std::stof(__x),  (decltype(type::pos.y))std::stof(__y) }, \
        {(decltype(type::size.x))std::stof(__w), (decltype(type::size.y))std::stof(__h) } \
}
#define RECT_TO_STRING(s, p) "(" +     \
        to_string(s.pos.x, p) + "," +  \
        to_string(s.pos.y, p) + "," +  \
        to_string(s.size.x, p) + "," + \
        to_string(s.size.y, p) +       \
")"

#define PLACE_HOOK_STRING(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, STRING_TO_STRING, NO_FROMSTRING_PRE, STRING_FROM_STRING, _, reset, false)
#define PLACE_HOOK_NUMBER(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset, true)
#define PLACE_HOOK_NUMBER_DEG(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_RAD_TO_STRING, NO_FROMSTRING_PRE, NUMBER_RAD_FROM_STRING, precision, reset, true)
#define PLACE_HOOK_OPTIONAL_NUMBER(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset)
#define PLACE_HOOK_NUMFLAGS(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMFLAGS_TO_STRING, NO_FROMSTRING_PRE, NUMFLAGS_FROM_STRING, _, reset, true)
#define PLACE_HOOK_VECTOR(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision, reset, true)
#define PLACE_HOOK_OPTIONAL_VECTOR(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision, reset)
#define PLACE_HOOK_OPTIONAL_SNAP(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, SNAP_TO_STRING, SNAP_FROM_STRING_PRE, SNAP_FROM_STRING, _, reset)
#define PLACE_HOOK_RECT(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, RECT_TO_STRING, RECT_FROM_STRING_PRE, RECT_FROM_STRING, 0, reset, true)
#define PLACE_HOOK_FLAG(container, guiId, field, index, name, reset)               \
do {                                                                               \
        auto __elem = container.get<tgui::CheckBox>(guiId);                        \
                                                                                   \
        if (!__elem)                                                               \
                break;                                                             \
        __elem->onChange([this, index](bool b){                            \
                this->applyOperation(new FlagOperation(                            \
                        *this->_object,                                            \
                        name,                                                      \
                        &FrameData::field,                                         \
                        index, b, (reset)                                          \
                ));                                                                \
                this->_rePopulateFrameData();                                      \
        });                                                                        \
        this->_updateFrameElements[&container].emplace_back([__elem, this, index]{ \
                auto &data = this->_object->getFrameData();                        \
                __elem->onChange.setEnabled(false);                                \
                __elem->setChecked(data.field.flags & (1ULL << index));            \
                __elem->onChange.setEnabled(true);                                 \
        });                                                                        \
        this->_containers.emplace(&container);                                     \
} while (false)


TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleColorFocused, tgui::Color::Black)
TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleBarColorFocused, tgui::Color::White)
TGUI_RENDERER_PROPERTY_TEXTURE(SpiralOfFate::MainWindow::Renderer, TextureTitleBarFocused)
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, CloseButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MaximizeButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MinimizeButtonFocused, "ChildWindowButton")

SpiralOfFate::MainWindow::MainWindow(const std::filesystem::path &frameDataPath, FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_editor(editor),
	_path(frameDataPath),
	_pathBak(frameDataPath),
	_chrPath(frameDataPath.parent_path()),
	_character(frameDataPath.parent_path().filename().string()),
	_object(new EditableObject(frameDataPath))
{
	this->_preview = std::make_shared<PreviewWidget>(std::ref(editor), std::ref(*this), *this->_object);
	this->_preview->setPosition(0, 0);
	this->_preview->setSize("&.w", "&.h");
	this->_preview->onBoxSelect([this]{
		this->_editor.setCanDelBoxes(true);
		this->_rePopulateFrameData();
	});
	this->_preview->onBoxUnselect([this]{
		this->_editor.setCanDelBoxes(false);
		this->_rePopulateFrameData();
	});
	this->_pathBak += ".bak";

	std::ifstream chrStream{this->_chrPath / "chr.json"};

	if (chrStream.fail())
		throw std::invalid_argument("Failed to open " + (this->_chrPath / "chr.json").string() + " for reading: " + strerror(errno));
	chrStream >> this->_characterData;

	auto palettes = this->_characterData["palettes"].get<std::vector<std::string>>();

	this->m_renderer = aurora::makeCopied<Renderer>();
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/animationWindow.gui");
	if (!palettes.empty()) {
		std::array<unsigned char, 768> paletteData;

		for (auto &ppath : palettes) {
			std::ifstream paletteStream{this->_chrPath / ppath};

			this->_palettes.emplace_back();

			auto &pal = this->_palettes.back();

			pal.path = ppath;
			if (paletteStream.fail()) {
				game->logger.error("Failed to open " + (this->_chrPath / ppath).string() + " for reading: " + strerror(errno));
				continue;
			}
			paletteStream.read(reinterpret_cast<char *>(paletteData.data()), paletteData.size());
			if (paletteStream.fail()) {
				game->logger.error("Failed to read " + (this->_chrPath / ppath).string() + ".");
				continue;
			}
			for (size_t i = 0 ; i < 251; i++) {
				pal.colors[i].r = paletteData[i * 3];
				pal.colors[i].g = paletteData[i * 3 + 1];
				pal.colors[i].b = paletteData[i * 3 + 2];
				pal.colors[i].a = i == 0 ? 0 : 255;
			}

			pal.colors[251] = game->typeColors[TYPECOLOR_NEUTRAL];
			pal.colors[252] = game->typeColors[TYPECOLOR_NON_TYPED];
			pal.colors[253] = game->typeColors[TYPECOLOR_MATTER];
			pal.colors[254] = game->typeColors[TYPECOLOR_SPIRIT];
			pal.colors[255] = game->typeColors[TYPECOLOR_VOID];
		}
		this->_preview->setPalette(&this->_palettes.front().colors);
	} else
		Utils::dispMsg(*this, this->_editor.localize("message_box.title.no_pal"), this->_editor.localize("message_box.no_pal"), MB_ICONWARNING);

	auto panel = this->get<tgui::Panel>("AnimationPanel");
	auto showBoxes = panel->get<tgui::BitmapButton>("ShowBoxes");
	auto displace = panel->get<tgui::BitmapButton>("Displace");
	auto ctrlPanel = this->get<tgui::Panel>("ControlPanel");

	ctrlPanel->loadWidgetsFromFile("assets/gui/editor/character/framedata.gui");
	this->_localizeWidgets(*ctrlPanel, true);
	Utils::setRenderer(this);

	this->setSize("min(1200, &.w - 20)", "min(625, &.h - 40)");
	this->setPosition(10, 30);
	this->setTitleButtons(TitleButton::Minimize | TitleButton::Maximize | TitleButton::Close);
	this->setTitle(frameDataPath.string());
	this->setResizable();
	this->setCloseBehavior(CloseBehavior::None);
	this->onClose([this]{
		// TODO: Check if it needs to be saved
		std::filesystem::remove(this->_pathBak);
		this->m_parent->remove(this->shared_from_this());
		this->onRealClose.emit(this);
	});

	panel->add(this->_preview);
	this->_preview->moveToBack();

	showBoxes->onClick([this](std::weak_ptr<tgui::BitmapButton> This){
		this->_preview->displayBoxes = !this->_preview->displayBoxes;
		This.lock()->setImage(tgui::Texture("assets/gui/editor/" + std::string(this->_preview->displayBoxes ? "" : "no") + "boxes.png"));
	}, std::weak_ptr(showBoxes));
	displace->onClick([this](std::weak_ptr<tgui::BitmapButton> This){
		this->_preview->displaceObject = !this->_preview->displaceObject;
		This.lock()->setImage(tgui::Texture("assets/gui/editor/" + std::string(this->_preview->displaceObject ? "" : "no") + "dispose.png"));
	}, std::weak_ptr(displace));

	this->onFocus([this]{
		this->m_titleText.setColor(this->_titleColorFocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorFocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});
	this->onUnfocus([this]{
		this->m_titleText.setColor(this->_titleColorUnfocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorUnfocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});

	this->_placeUIHooks(*this);
	this->_populateData(*this);
}

bool SpiralOfFate::MainWindow::isModified() const noexcept
{
	return
		this->_operationIndex != this->_operationSaved ||
		(this->_pendingTransaction && this->_pendingTransaction->hasModification());
}

bool SpiralOfFate::MainWindow::hasUndoData() const noexcept
{
	return this->_operationIndex || (this->_pendingTransaction && this->_pendingTransaction->hasModification());
}

bool SpiralOfFate::MainWindow::hasRedoData() const noexcept
{
	return this->_operationQueue.size() != this->_operationIndex;
}

void SpiralOfFate::MainWindow::refreshMenuItems() const
{
	auto curr = this->_object->_moves.find(this->_object->_action);
	assert_exp(curr != this->_object->_moves.end());

	auto &mov = this->_object->_moves;
	auto &act = curr->second;
	auto &blk = act[this->_object->_actionBlock];

	this->_editor.setHasRedo(this->hasRedoData());
	this->_editor.setHasUndo(this->hasUndoData());
	this->_editor.setCanDelBoxes(this->_preview->getSelectedBox().first != BOXTYPE_NONE);
	this->_editor.setCanDelAction(mov.size() > 1);
	this->_editor.setCanDelBlock(act.size() > 1);
	this->_editor.setCanDelFrame(blk.size() > 1);
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastAction(curr != this->_object->_moves.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_moves.end());
}

void SpiralOfFate::MainWindow::redo()
{
	assert_exp(this->_pendingTransaction || this->_operationIndex < this->_operationQueue.size());
	if (this->_pendingTransaction) {
		this->commitTransaction();
		return;
	}

	auto wasModified = this->isModified();

	this->_operationQueue[this->_operationIndex]->apply();
	this->_operationIndex++;
	this->_requireReload = true;
	if (!wasModified)
		this->setTitle(this->_path.string() + "*");
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(this->hasRedoData());
	this->autoSave();
}

void SpiralOfFate::MainWindow::undo()
{
	assert_exp(this->_pendingTransaction || this->_operationIndex > 0);
	if (this->_pendingTransaction) {
		this->cancelTransaction();
		return;
	}
	this->_operationIndex--;
	this->_operationQueue[this->_operationIndex]->undo();
	this->_requireReload = true;
	if (!this->isModified())
		this->setTitle(this->_path.string());
	this->_editor.setHasUndo(this->hasUndoData());
	this->_editor.setHasRedo(true);
	this->autoSave();
}

void SpiralOfFate::MainWindow::copyFrame()
{
	sf::Clipboard::setString(this->_object->getFrameData().toJson().dump(4));
}

void SpiralOfFate::MainWindow::pasteFrame()
{
	auto &data = this->_object->getFrameData();
	auto oldPal = data.__paletteData;
	auto oldPalette = data.__palette;
	auto oldFolder = data.__folder;

	try {
		auto anim = SpiralOfFate::FrameData(
			nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
			this->_object->_folder
		);

		anim.__paletteData = oldPal;
		anim.__palette = oldPalette;
		anim.__folder = oldFolder;
		this->applyOperation(new PasteDataOperation(
			*this->_object,
			this->_editor.localize("operation.paste"),
			anim
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::pasteBoxData()
{
	auto &data = this->_object->getFrameData();
	auto oldPal = data.__paletteData;
	auto oldPalette = data.__palette;
	auto oldFolder = data.__folder;

	try {
		auto anim = SpiralOfFate::FrameData(
			nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
			this->_object->_folder
		);

		anim.__paletteData = oldPal;
		anim.__palette = oldPalette;
		anim.__folder = oldFolder;
		this->applyOperation(new PasteBoxDataOperation(
			*this->_object,
			this->_editor.localize("operation.paste_box"),
			anim
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::pasteAnimData()
{
	auto &data = this->_object->getFrameData();
	auto oldPal = data.__paletteData;
	auto oldPalette = data.__palette;
	auto oldFolder = data.__folder;

	try {
		auto anim = SpiralOfFate::FrameData(
			nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
			this->_object->_folder
		);

		anim.__paletteData = oldPal;
		anim.__palette = oldPalette;
		anim.__folder = oldFolder;
		this->applyOperation(new PasteAnimDataOperation(
			*this->_object,
			this->_editor.localize("operation.paste_anim"),
			anim
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::startTransaction(SpiralOfFate::Operation *operation)
{
	assert_exp(!this->_pendingTransaction);

	auto wasModified = this->isModified();

	if (!operation)
		operation = new DummyOperation();
	this->_pendingTransaction.reset(operation);
	this->_pendingTransaction->apply();
	if (!wasModified && this->_pendingTransaction->hasModification())
		this->setTitle(this->_path.string() + "*");
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::updateTransaction(const std::function<Operation *()> &operation)
{
	assert_exp(this->_pendingTransaction);

	auto wasModified = this->isModified();

	this->_pendingTransaction->undo();
	this->_pendingTransaction.reset(operation());
	this->_pendingTransaction->apply();
	if (!wasModified && this->_pendingTransaction->hasModification())
		this->setTitle(this->_path.string() + "*");
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::cancelTransaction()
{
	assert_exp(this->_pendingTransaction);
	this->_pendingTransaction->undo();
	this->_pendingTransaction.reset();
	if (!this->isModified())
		this->setTitle(this->_path.string());
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::commitTransaction()
{
	assert_exp(this->_pendingTransaction);
	if (!this->_pendingTransaction->hasModification())
		return this->_pendingTransaction.reset();

	auto wasModified = this->isModified();

	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(nullptr);
	this->_operationQueue.back().swap(this->_pendingTransaction);
	this->_requireReload = true;
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();
	if (!wasModified)
		this->setTitle(this->_path.string() + "*");
	this->autoSave();
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
}

void SpiralOfFate::MainWindow::applyOperation(Operation *operation)
{
	if (!operation->hasModification()) {
		delete operation;
		return;
	}

	auto wasModified = this->isModified();

	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(operation);
	this->_operationQueue.back()->apply();
	this->_requireReload = true;
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();
	if (wasModified)
		this->setTitle(this->_path.string() + "*");
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
	this->autoSave();
}

void SpiralOfFate::MainWindow::save(const std::filesystem::path &path)
{
	this->setPath(path);
	this->save();
}

void SpiralOfFate::MainWindow::setPath(const std::filesystem::path &path)
{
	std::error_code err;
	std::filesystem::path pathBak = path;

	pathBak += ".bak";
	std::filesystem::rename(this->_pathBak, pathBak, err);
	if (err)
		// FIXME: strerror only works on Linux (err.message()?)
		game->logger.error(this->_editor.localize("error.rename", this->_pathBak.string(), pathBak.string(), strerror(errno)));
	this->_path = path;
	this->_pathBak = pathBak;
	for (auto &pal : this->_palettes)
		pal.modified = true;
}

void SpiralOfFate::MainWindow::save()
{
	nlohmann::json j = nlohmann::json::array();

	if (this->_pendingTransaction)
		this->commitTransaction();
	for (auto &[key, value] : this->_object->_moves)
		j.push_back({
			{"action", key},
			{"framedata", value}
		});

	std::ofstream stream{this->_path};

	if (stream.fail()) {
		auto err = this->_editor.localize("error.open", this->_path.string(), strerror(errno));

		// FIXME: strerror only works on Linux (err.message()?)
		// TODO: Somehow return the message box so the caller can open the save as dialog when OK is clicked
		SpiralOfFate::Utils::dispMsg(game->gui, this->_editor.localize("message_box.title.save_err"), err, MB_ICONERROR);
		game->logger.error(err);
		return;
	}
	stream << j.dump(2);

	for (auto &pal : this->_palettes) {
		auto p = this->_chrPath / pal.path;
		std::ofstream palStream{p};

		if (palStream.fail()) {
			auto err = this->_editor.localize("error.open", p.string(), strerror(errno));

			// FIXME: strerror only works on Linux (err.message()?)
			SpiralOfFate::Utils::dispMsg(game->gui, this->_editor.localize("message_box.title.save_err"), err, MB_ICONERROR);
			game->logger.error(err);
			continue;
		}

		std::array<unsigned char, 768> paletteData;

		for (size_t i = 0 ; i < 256; i++) {
			paletteData[i * 3]     = pal.colors[i].r;
			paletteData[i * 3 + 1] = pal.colors[i].g;
			paletteData[i * 3 + 2] = pal.colors[i].b;
		}
		palStream.write(reinterpret_cast<char *>(paletteData.data()), paletteData.size());
	}

	auto p = this->_chrPath / "chr.json";
	std::ofstream chrStream{p};

	this->_characterData["palettes"] = nlohmann::json::array();
	for (auto &pal : this->_palettes)
		this->_characterData["palettes"].push_back(pal.path);
	if (!chrStream.fail())
		chrStream << this->_characterData.dump(2);
	else {
		auto err = this->_editor.localize("error.open", p.string(), strerror(errno));

		// FIXME: strerror only works on Linux (err.message()?)
		// FIXME: Isn't it confusing that we show an error box, but still somewhat ignore the error and proceed?
		SpiralOfFate::Utils::dispMsg(game->gui, this->_editor.localize("message_box.title.save_err"), err, MB_ICONERROR);
		game->logger.error(err);
	}

	this->_operationSaved = this->_operationIndex;
	this->setTitle(this->_path.string());
}

void SpiralOfFate::MainWindow::autoSave()
{
	nlohmann::json j = nlohmann::json::array();

	for (auto &[key, value] : this->_object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{this->_pathBak};

	if (stream.fail()) {
		SpiralOfFate::Utils::dispMsg(game->gui, "Saving failed", this->_pathBak.string() + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(2);
}

std::string SpiralOfFate::MainWindow::_localizeActionName(unsigned int id)
{
	if (this->_editor.hasLocalization("action." + this->_character + "." + std::to_string(id)))
		return this->_editor.localize("action." + this->_character + "." + std::to_string(id));
	else if (this->_editor.hasLocalization("action.generic." + std::to_string(id)))
		return this->_editor.localize("action.generic." + std::to_string(id));
	return Character::actionToString(id);
}

SpiralOfFate::LocalizedContainer<tgui::ChildWindow>::Ptr SpiralOfFate::MainWindow::_createPopup(const std::string &path)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = std::make_shared<LocalizedContainer<tgui::ChildWindow>>(this->_editor);
	tgui::Vector2f size = {0, 0};

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	contentPanel->setSize(100, 100);

	auto closePopup = [this](std::weak_ptr<tgui::Panel> outsidePanel, std::weak_ptr<tgui::ChildWindow> contentPanel){
		auto content = contentPanel.lock();

		this->remove(outsidePanel.lock());
		this->remove(content);
		this->_updateFrameElements.erase(&*content);
		this->_containers.erase(&*content);
	};
	auto data = this->_object->getFrameData();

	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 10);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 10);
	}
	size.y += contentPanel->getSize().y - contentPanel->getInnerSize().y;
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	contentPanel->onClose(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	return contentPanel;
}

void SpiralOfFate::MainWindow::_createGenericPopup(const std::string &path)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = std::make_shared<LocalizedContainer<tgui::ScrollablePanel>>(this->_editor);
	tgui::Vector2f size = {0, 0};

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);

	auto closePopup = [this](std::weak_ptr<tgui::Panel> outsidePanel, std::weak_ptr<tgui::ScrollablePanel> contentPanel){
		auto content = contentPanel.lock();

		this->remove(outsidePanel.lock());
		this->remove(content);
		this->_updateFrameElements.erase(&*content);
		this->_containers.erase(&*content);
	};
	auto data = this->_object->getFrameData();

	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 10);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 10);
	}
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	this->_placeUIHooks(*contentPanel);
	this->_populateData(*contentPanel);
}

void SpiralOfFate::MainWindow::_createMoveListPopup(const std::function<void(unsigned)> &onConfirm, bool showNotAdded)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::Panel::create({500, "&.h - 100"});
	auto movesPanel = tgui::ScrollablePanel::create({"&.w", "&.h - 42"});
	auto search = tgui::EditBox::create();
	auto scroll = 0;
	unsigned i = 0;
	std::set<unsigned> moves;
	std::unordered_map<unsigned, std::string> movesNames;
	std::unordered_map<unsigned, std::string> movesNamesLower;

	for (const auto &moveId: this->_object->_moves | std::views::keys)
		moves.insert(moveId);
	if (showNotAdded) {
		for (const auto &moveId: SpiralOfFate::actionNames | std::views::keys)
			moves.insert(moveId);
		for (const auto &local: this->_editor.getLocalizationData() | std::views::keys) {
			try {
				if (local.rfind("action.generic.", 0) == 0)
					moves.insert(std::stoul(local.substr(strlen("action.generic."))));
				else if (local.rfind("action." + this->_character + ".", 0) == 0)
					moves.insert(std::stoul(local.substr(strlen("action..") + this->_character.size())));
			} catch (std::exception &e) {
				Utils::dispMsg(game->gui, "Error", local + ": " + e.what(), 0);
			}
		}
	}

	search->setPosition({10, 10});
	search->setSize({"&.w - 20", 22});
	search->setDefaultText(this->_editor.localize("popup.search"));
	contentPanel->add(search);
	movesPanel->setPosition(0, 42);
	movesPanel->getRenderer()->setBorders({0, 1, 0, 0});
	contentPanel->add(movesPanel);

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	outsidePanel->setUserData(false);
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	for (auto moveId : moves) {
		movesNames[moveId] = this->_localizeActionName(moveId);
		movesNamesLower[moveId].resize(movesNames[moveId].size());
		// TODO: Handle non-ascii characters
		std::ranges::transform(movesNames[moveId], movesNamesLower[moveId].begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		if (moveId == this->_object->_action)
			scroll = i * 25 + 20;
		i++;
	}

	auto closePopup = [this](const std::weak_ptr<tgui::Panel> &outsidePanel_w, const std::weak_ptr<tgui::Panel> &contentPanel_w){
		this->remove(outsidePanel_w.lock());
		this->remove(contentPanel_w.lock());
	};
	auto refresh = [moves, movesNames, movesNamesLower, this, onConfirm, closePopup](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::ScrollablePanel> &movesPanel_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		std::string query = search_w.lock()->getText().toStdString();
		auto movesPanel_p = movesPanel_w.lock();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		movesPanel_p->removeAllWidgets();
		for (auto moveId : moves) {
			if (movesNamesLower.at(moveId).find(query) == std::string::npos)
				continue;

			const auto &name = movesNames.at(moveId);
			auto label = tgui::Label::create(std::to_string(moveId));
			auto button = tgui::Button::create(name);

			label->setPosition(10, index * 25 + 5);
			button->setPosition(50, index * 25 + 3);
			button->setSize(410, 20);
			Utils::setRenderer(button);
			Utils::setRenderer(label);
			if (moveId == this->_object->_action) {
				button->getRenderer()->setTextColor(tgui::Color::Green);
				button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
				button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
			} else if (!this->_object->_moves.contains(moveId)) {
				button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x00, 0x00});
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0x40, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x00, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x00, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x20, 0x20});
			} else if (name.rfind("Action #", 0) == 0) {
				button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00});
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0xA0, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x50, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x40, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x60, 0x20});
			}

			button->onClick(onConfirm, moveId);
			button->onClick(closePopup, outsidePanel_w, contentPanel_w);

			movesPanel_p->add(label);
			movesPanel_p->add(button);
			index++;
		}

		auto label = tgui::Label::create("");

		label->setPosition(10, index * 25);
		label->setSize(100, 5);
		label->setTextSize(1);
		movesPanel_p->add(label);
	};
	auto validate = [moves, movesNamesLower, onConfirm, closePopup](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		unsigned lastIndex = 0;
		unsigned move;
		std::string query = search_w.lock()->getText().toStdString();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		for (auto moveId : moves) {
			auto &name = movesNamesLower.at(moveId);

			if (name == query) {
				move = moveId;
				lastIndex = 1;
				break;
			}
			if (name.find(query) != std::string::npos) {
				if (lastIndex)
					lastIndex = UINT32_MAX;
				else
					lastIndex = index + 1;
				move = moveId;
			}
			index++;
		}
		if (lastIndex == UINT32_MAX || lastIndex == 0)
			return;
		onConfirm(move);
		closePopup(outsidePanel_w, contentPanel_w);
	};

	refresh(std::weak_ptr(search), std::weak_ptr(movesPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onTextChange(refresh, std::weak_ptr(search), std::weak_ptr(movesPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onReturnKeyPress(validate, std::weak_ptr(search), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	scroll -= movesPanel->getSize().y / 2;
	if (scroll > 0)
		movesPanel->getVerticalScrollbar()->setValue(scroll);
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->setFocused(true);
}

void SpiralOfFate::MainWindow::_placeUIHooks(tgui::Container &container)
{
	auto clearHit = container.get<tgui::Button>("ClearHit");
	auto clearBlock = container.get<tgui::Button>("ClearBlock");
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto prevAction = container.get<tgui::Button>("PrevAction");
	auto nextAction = container.get<tgui::Button>("NextAction");
	auto play = container.get<tgui::Button>("Play");
	auto step = container.get<tgui::Button>("Step");
	auto frame = container.get<tgui::Slider>("Frame");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto aFlags = container.get<tgui::Button>("AFlagsButton");
	auto dFlags = container.get<tgui::Button>("DFlagsButton");
	auto hitEdit = container.get<tgui::Button>("HitEdit");
	auto blockEdit = container.get<tgui::Button>("BlockEdit");
	auto generalEdit = container.get<tgui::Button>("GeneralEdit");
	auto ctrl = container.get<tgui::Tabs>("ControlTabs");
	auto ctrlPanel = container.get<tgui::Panel>("ControlPanel");
	auto boxes = container.get<tgui::Panel>("SelectedBoxPanel");
	auto boxLabel = container.get<tgui::Label>("SelectedBoxName");
	auto plane = container.get<ColorPlaneWidget>("ColorPlane");
	auto slider = container.get<ColorSliderWidget>("ColorSlider");
	auto comp1 = container.get<tgui::EditBox>("ColorComponent1");
	auto comp2 = container.get<tgui::EditBox>("ColorComponent2");
	auto comp3 = container.get<tgui::EditBox>("ColorComponent3");
	auto hexcode = container.get<tgui::EditBox>("RGBColor");
	auto addPal = container.get<tgui::Button>("AddPalette");
	auto removePal = container.get<tgui::Button>("RemovePalette");
	auto preview = container.get<tgui::Label>("ColorPreview");
	auto paletteList = container.get<tgui::ComboBox>("PaletteList");
	auto colorPanel = container.get<tgui::ScrollablePanel>("AllColorPanel");
	auto mode = container.get<tgui::Tabs>("ColorModes");
	auto boundsButton = container.get<tgui::Button>("BoundsButton");
	tgui::Container &sidePanel = ctrlPanel ? *ctrlPanel : container;

	if (mode)
		mode->onTabSelect([this, &sidePanel](std::weak_ptr<tgui::Tabs> This){
			this->_selectColorMethod = This.lock()->getSelectedIndex();
			this->_reinitSidePanel(sidePanel);
			this->_rePopulateColorData();
		}, std::weak_ptr(mode));
	if (colorPanel) {
		float stepX = (colorPanel->getSize().x - 24) / 8.f;
		float stepY = stepX - 8;
		float extra = this->getSize().y - this->getInnerSize().y;

		colorPanel->setSize("&.w - 20", tgui::Layout("(&.h - y) - 10 - " + std::to_string(extra)));
		colorPanel->removeAllWidgets();
		for (size_t i = 0; i < 256; i++) {
			auto button = tgui::Button::create();

			button->setSize({24, 16});
			button->setPosition({(i % 8) * stepX, (i / 8) * stepY});
			colorPanel->add(button, "Color" + std::to_string(i));

			button->onClick([this, i, &container]{
				this->_selectedColor = i;
				this->_preview->setSelectedColor(i);
				this->_populateColorData(container);
			});
			button->onMouseEnter([this, i]{
				this->_object->_paletteIndex = i;
				this->_object->_needGenerate = true;
			});
			button->onMouseLeave([this]{
				if (this->_selectedColor == 0)
					this->_object->_paletteIndex = -1;
				else {
					this->_object->_paletteIndex = this->_selectedColor;
					this->_object->_needGenerate = true;
				}
			});
		}
	}
	this->_preview->onColorSelect.disconnectAll();
	this->_preview->onColorSelect([this, &container](unsigned colorIndex){
		this->_selectedColor = colorIndex;
		this->_populateColorData(container);
	});
	if (paletteList)
		paletteList->onItemSelect([this, &container](unsigned index){
			if (this->_palettes.empty())
				return;
			this->_selectedPalette = index;
			this->_populateColorData(container);
			this->_preview->setPalette(&this->_palettes[this->_selectedPalette].colors);
		});

	PLACE_HOOK_WIDGET(plane, 0);
	PLACE_HOOK_WIDGET(slider, 1);
	PLACE_HOOK_COLOR_COMPONENT(comp1, 0);
	PLACE_HOOK_COLOR_COMPONENT(comp2, 1);
	PLACE_HOOK_COLOR_COMPONENT(comp3, 2);
	PLACE_HOOK_HEXCOLOR(hexcode);

	if (addPal)
		addPal->onClick([this]{
			auto window = this->_createPopup("assets/gui/editor/palette/add.gui");
			auto create = window->get<tgui::Button>("CreatePalette");
			auto cancel = window->get<tgui::Button>("CancelPalette");
			auto name = window->get<tgui::EditBox>("PaletteName");
			auto list = window->get<tgui::ComboBox>("PaletteList");

			for (auto &pal : this->_palettes)
				list->addItem(pal.path);
			if (this->_palettes.empty())
				list->setEnabled(false);
			list->setSelectedItemByIndex(this->_selectedPalette);
			create->setEnabled(false);

			name->onTextChange([this](std::weak_ptr<tgui::Button> ptr, const tgui::String &s){
				auto button = ptr.lock();

				if (s.empty()) {
					button->setEnabled(false);
					return;
				}
				for (auto &pal : this->_palettes)
					if (pal.path == s.toStdString()) {
						button->setEnabled(false);
						return;
					}
				button->setEnabled(true);
			}, std::weak_ptr(create));
			create->onClick([this, name, list](std::weak_ptr<LocalizedContainer<tgui::ChildWindow>> ptr){
				std::array<Color, 256> colors;

				if (this->_palettes.empty()) {
					colors.fill(Color::Black);
					colors[0] = Color::Transparent;
					colors[251] = game->typeColors[TYPECOLOR_NEUTRAL];
					colors[252] = game->typeColors[TYPECOLOR_NON_TYPED];
					colors[253] = game->typeColors[TYPECOLOR_MATTER];
					colors[254] = game->typeColors[TYPECOLOR_SPIRIT];
					colors[255] = game->typeColors[TYPECOLOR_VOID];
				} else
					colors = this->_palettes[list->getSelectedItemIndex()].colors;
				this->applyOperation(new CreatePaletteOperation(
					this->_palettes,
					this->_editor.localize("palette.create_action"),
					{ .path = name->getText().toStdString(), .colors = colors, .modified = true },
					this->_selectedPalette
				));
				ptr.lock()->close();
			}, std::weak_ptr(window));
			cancel->onClick([](std::weak_ptr<LocalizedContainer<tgui::ChildWindow>> ptr){
				ptr.lock()->close();
			}, std::weak_ptr(window));
		});
	if (removePal)
		removePal->onClick([this]{
			this->applyOperation(new RemovePaletteOperation(
				this->_palettes,
				this->_editor.localize("palette.remove_action"),
				this->_selectedPalette
			));
		});

	if (ctrl)
		ctrl->onTabSelect([this, &sidePanel](std::weak_ptr<tgui::Tabs> This){
			auto selected = This.lock()->getSelectedIndex();

			if (selected == this->_showingPalette)
				return;

			this->_planes.clear();
			this->_sliders.clear();
			this->_showingPalette = selected;
			sidePanel.loadWidgetsFromFile(
				selected == 0 ?
				"assets/gui/editor/character/framedata.gui" :
				"assets/gui/editor/palette/palette.gui"
			);
			this->_initSidePanel(sidePanel);
			this->_localizeWidgets(sidePanel, true);
			Utils::setRenderer(&sidePanel);
			this->_placeUIHooks(sidePanel);
			this->_populateData(sidePanel);
			this->_preview->showingPalette = this->_showingPalette;
		}, std::weak_ptr(ctrl));
	if (action)
		action->onReturnOrUnfocus([this](std::weak_ptr<tgui::EditBox> This, const tgui::String &s){
			if (std::to_string(this->_object->_action) == s)
				return;
			if (s.empty()) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}

			unsigned action = std::stoul(s.toStdString());

			if (this->_object->_moves.count(action) == 0) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}
			this->_object->_action = action;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_object->_animationCtr = 0;
			this->_requireReload = true;
		}, std::weak_ptr(action));
	if (clearHit)
		clearHit->onClick([this]{
			this->applyOperation(new ClearHitOperation(*this->_object, this->_editor));
		});
	if (clearBlock)
		clearBlock->onClick([this]{
			this->applyOperation(new ClearBlockOperation(*this->_object, this->_editor));
		});
	if (aFlags)
		// TODO: Change flags label if a character or subobject
		aFlags->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/attackFlags.gui");
	if (dFlags)
		// TODO: Change flags label if a character or subobject
		dFlags->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/defenseFlags.gui");
	if (hitEdit)
		// TODO: Change fields label if a character or subobject
		hitEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/hitProperties.gui");
	if (blockEdit)
		// TODO: Change fields label if a character or subobject
		blockEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/blockProperties.gui");
	if (generalEdit)
		// TODO: Change fields label if a character or subobject
		generalEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/generalProperties.gui");
	if (actionSelect)
		actionSelect->onClick(&MainWindow::navGoTo, this);
	if (prevAction)
		prevAction->onClick(&MainWindow::navToPrevAction, this);
	if (nextAction)
		nextAction->onClick(&MainWindow::navToNextAction, this);
	if (play)
		play->onPress([this]{
			this->_paused = false;
		});
	if (step)
		step->onPress([this]{
			this->_paused = false;
			this->tick();
			this->_paused = true;
		});
	if (frame)
		frame->onValueChange([this](float value){
			auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
			this->_editor.setCanCopyLast(this->_object->_animation > 0);
			this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
			this->_editor.setHasLastFrame(this->_object->_animation > 0);
			this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
		});
	if (frameSpin)
		frameSpin->onValueChange([this](float value){
			auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
			this->_editor.setCanCopyLast(this->_object->_animation > 0);
			this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
			this->_editor.setHasLastFrame(this->_object->_animation > 0);
			this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
		});
	if (blockSpin)
		blockSpin->onValueChange([this](float value){
			auto &act = this->_object->_moves[this->_object->_action];
			auto &blk = act[value];

			this->_object->_actionBlock = value;
			this->_object->_animation = 0;
			this->_requireReload = true;
			this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
			this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
			this->_editor.setHasLastFrame(false);
			this->_editor.setHasNextFrame(blk.size() != 1);
			this->_editor.setCanCopyLast(false);
			this->_editor.setCanCopyNext(blk.size() != 1);
		});
	if (boundsButton)
		boundsButton->onClick([] {
			// TODO: Not implemented
			Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
		});

	PLACE_HOOK_STRING(container,   "Sprite",   spritePath,    this->_editor.localize("animation.sprite"),   EditSpriteOperation, false);
	PLACE_HOOK_NUMBER(container,   "Duration", duration,      this->_editor.localize("animation.duration"), BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,   "Offset",   offset,        this->_editor.localize("animation.offset"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,   "Scale",    scale,         this->_editor.localize("animation.scale"),    BasicDataOperation, false, 2);
	PLACE_HOOK_RECT(container,     "Bounds",   textureBounds, this->_editor.localize("animation.bounds"),   BasicDataOperation, false);
	PLACE_HOOK_NUMFLAGS(container, "AFlags",   oFlag,         this->_editor.localize("animation.aflags"),   BasicDataOperation, false);
	PLACE_HOOK_NUMFLAGS(container, "DFlags",   dFlag,         this->_editor.localize("animation.dflags"),   BasicDataOperation, false);

	PLACE_HOOK_NUMBER(container,          "PGen",      particleGenerator, this->_editor.localize("animation.general.partgenerator"), BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "SubObj",    subObjectSpawn,    this->_editor.localize("animation.general.subobj"),        BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "Marker",    specialMarker,     this->_editor.localize("animation.general.marker"),        BasicDataOperation, false, 0);
	PLACE_HOOK_OPTIONAL_NUMBER(container, "Tier",      priority,          this->_editor.localize("animation.general.tier"),          BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,          "MoveSpeed", speed,             this->_editor.localize("animation.general.speed"),         BasicDataOperation, true,  2);
	PLACE_HOOK_OPTIONAL_VECTOR(container, "Gravity",   gravity,           this->_editor.localize("animation.general.gravity"),       BasicDataOperation, true,  2);
	PLACE_HOOK_STRING(container,          "Sound",     soundPath,         this->_editor.localize("animation.general.sound"),         EditSoundOperation, false);
	PLACE_HOOK_NUMBER(container,          "FadeTime",  fadeTime,          this->_editor.localize("animation.general.fadetime"),      BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "ManaGain",  manaGain,          this->_editor.localize("animation.general.managain"),      BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "ManaCost",  manaCost,          this->_editor.localize("animation.general.manacost"),      BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER_DEG(container,      "Rotation",  rotation,          this->_editor.localize("animation.general.rotation"),      BasicDataOperation, true, 2);

	PLACE_HOOK_NUMBER(container,        "PHitStop",  hitPlayerHitStop,  this->_editor.localize("animation.hit.pstop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "OHitStop",  hitOpponentHitStop,this->_editor.localize("animation.hit.ostop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,        "HitSpeed",  hitSpeed,          this->_editor.localize("animation.hit.speed"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,        "CHitSpeed", counterHitSpeed,   this->_editor.localize("animation.hit.chspeed"), BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "NLimit",    neutralLimit,      this->_editor.localize("animation.hit.nlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "SLimit",    spiritLimit,       this->_editor.localize("animation.hit.slimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "MLimit",    matterLimit,       this->_editor.localize("animation.hit.mlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "VLimit",    voidLimit,         this->_editor.localize("animation.hit.vlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "Rate",      prorate,           this->_editor.localize("animation.hit.rate"),    BasicDataOperation, false, 2);
	PLACE_HOOK_NUMBER(container,        "MinRate",   minProrate,        this->_editor.localize("animation.hit.minrate"), BasicDataOperation, false, 2);
	PLACE_HOOK_NUMBER(container,        "Untech",    untech,            this->_editor.localize("animation.hit.untech"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "Damage",    damage,            this->_editor.localize("animation.hit.damage"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "HitStun",   hitStun,           this->_editor.localize("animation.hit.stun"),    BasicDataOperation, false, 0);
	PLACE_HOOK_STRING(container,        "HitSound",  hitSoundPath,      this->_editor.localize("animation.hit.sound"),   EditSoundOperation, false);
	PLACE_HOOK_OPTIONAL_SNAP(container, "HitSnap",   snap,              this->_editor.localize("animation.hit.snap"),    BasicDataOperation, false);

	PLACE_HOOK_NUMBER(container, "PBlockStop", blockPlayerHitStop,  this->_editor.localize("animation.block.pstop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "OBlockStop", blockOpponentHitStop,this->_editor.localize("animation.block.ostop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "PPush",      pushBack,            this->_editor.localize("animation.block.ppush"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "OPush",      pushBlock,           this->_editor.localize("animation.block.opush"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "GuardDmg",   guardDmg,            this->_editor.localize("animation.block.guarddmg"),BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "Chip",       chipDamage,          this->_editor.localize("animation.block.chip"),    BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "BlockStun",  blockStun,           this->_editor.localize("animation.block.stun"),    BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "WBlockStun", wrongBlockStun,      this->_editor.localize("animation.block.wstun"),   BasicDataOperation, false, 0);

	PLACE_HOOK_BOX(container, "SelectedBoxPos",  pos,  size, this->_editor.localize("animation.selectedbox.pos"));
	PLACE_HOOK_BOX(container, "SelectedBoxSize", size, pos,  this->_editor.localize("animation.selectedbox.size"));

	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "aFlag" + std::to_string(i), oFlag, i, this->_editor.localize("animation.aflags.flag" + std::to_string(i)), false);
	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "dFlag" + std::to_string(i), dFlag, i, this->_editor.localize("animation.dflags.flag" + std::to_string(i)), i == 8 || i == 13 || i == 21);
	if (boxes) {
		this->_updateFrameElements[&container].emplace_back([boxes, this]{
			bool hasBox = this->_preview->getSelectedBox().first != BOXTYPE_NONE;

			boxes->setVisible(hasBox);
			if (hasBox) boxes->setSize({"&.w", 90});
			else  boxes->setSize({0, 0});
			for (auto &fct : this->_updateFrameElements[&*boxes])
				fct();
		});
		this->_containers.emplace(&container);
	}
	if (boxLabel) {
		this->_updateFrameElements[&container].emplace_back([boxLabel, this] {
			auto box = this->_preview->getSelectedBox();

			switch (box.first) {
			case BOXTYPE_HITBOX:
				boxLabel->setText(this->_editor.localize("animation.selectedbox.hitbox", std::to_string(box.second)));
				break;
			case BOXTYPE_HURTBOX:
				boxLabel->setText(this->_editor.localize("animation.selectedbox.hurtbox", std::to_string(box.second)));
				break;
			case BOXTYPE_COLLISIONBOX:
				boxLabel->setText(this->_editor.localize("animation.selectedbox.collision"));
				break;
			default:
				boxLabel->setText("");
			}
		});
		this->_containers.emplace(&container);
	}
}


void SpiralOfFate::MainWindow::newFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->_editor.localize("operation.create_frame"),
		this->_object->_animation,
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newEndFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->_editor.localize("operation.create_frame_end"),
		this->_object->_moves[this->_object->_action][this->_object->_actionBlock].size(),
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newAnimationBlock()
{
	this->applyOperation(new CreateBlockOperation(
		*this->_object,
		this->_editor.localize("operation.create_block"),
		this->_object->_actionBlock + 1,
		{ this->_object->getFrameData() }
	));
}

void SpiralOfFate::MainWindow::newAction()
{
	auto window = this->_createPopup("assets/gui/editor/character/actionCreate.gui");
	auto idBox = window->get<tgui::EditBox>("ActionID");
	auto action = window->get<tgui::Button>("Action");
	auto create = window->get<tgui::Button>("Create");
	auto actionW = std::weak_ptr(action);
	auto createW = std::weak_ptr(create);
	auto idBoxW  = std::weak_ptr(idBox);
	auto windowW = std::weak_ptr(window);

	window->setTitle(this->_editor.localize("message_box.title.create_action"));
	create->setEnabled(false);
	idBox->onTextChange([createW, actionW, idBoxW, this](const tgui::String &t){
		if (t.empty()) {
			actionW.lock()->setText("");
			createW.lock()->setEnabled(false);
		} else {
			actionW.lock()->setText(this->_localizeActionName(std::stoul(t.toStdString())));
			createW.lock()->setEnabled(true);
		}
	});
	action->onClick(&MainWindow::_createMoveListPopup, this, [idBoxW](unsigned move){
		idBoxW.lock()->setText(std::to_string(move));
	}, true);
	create->onClick([windowW, idBox, this]{
		auto &data = this->_object->getFrameData();
		auto action = std::stoul(idBox->getText().toStdString());

		if (!this->_object->_moves.contains(action)) {
			this->applyOperation(new CreateMoveOperation(
				*this->_object,
				this->_editor.localize("operation.create_move"),
				action, {{data}}
			));
			windowW.lock()->close();
			this->_rePopulateData();
		} else
			Utils::dispMsg(game->gui, this->_editor.localize("message_box.title.already_exists"), this->_editor.localize("message_box.action_exists"), MB_ICONERROR);
	});
}

void SpiralOfFate::MainWindow::newHurtBox()
{
	this->applyOperation(new CreateBoxOperation(
		*this->_object,
		this->_editor.localize("operation.create_hurtbox"),
		true
	));
}

void SpiralOfFate::MainWindow::newHitBox()
{
	this->applyOperation(new CreateBoxOperation(
		*this->_object,
		this->_editor.localize("operation.create_hitbox"),
		false
	));
}

void SpiralOfFate::MainWindow::removeFrame()
{
	if (this->_object->_moves[this->_object->_action][this->_object->_actionBlock].size() == 1)
		return;
	this->applyOperation(new RemoveFrameOperation(
		*this->_object,
		this->_editor.localize("operation.remove_frame")
	));
}

void SpiralOfFate::MainWindow::removeAnimationBlock()
{
	assert_exp(this->_object->_moves[this->_object->_action].size() > 1);
	this->applyOperation(new RemoveBlockOperation(
		*this->_object,
		this->_editor.localize("operation.remove_block"),
		this->_object->_actionBlock
	));
}

void SpiralOfFate::MainWindow::removeAction()
{
	assert_exp(this->_object->_moves.size() > 1);
	this->applyOperation(new RemoveMoveOperation(
		*this->_object,
		this->_editor.localize("operation.remove_action"),
		this->_object->_action
	));
}

void SpiralOfFate::MainWindow::removeBox()
{
	auto b = this->_preview->getSelectedBox();

	assert_exp(b.first != BOXTYPE_NONE);
	this->applyOperation(new RemoveBoxOperation(
		*this->_object,
		this->_editor.localize("operation.remove_box"),
		b.first, b.second,
		[this](BoxType t, unsigned i) { this->_preview->setSelectedBox(t, i); }
	));
}

void SpiralOfFate::MainWindow::copyBoxesFromLastFrame()
{
	auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation > 0);
	this->applyOperation(new PasteBoxDataOperation(
		*this->_object,
		this->_editor.localize("operation.copy_box_last"),
		blk[this->_object->_animation - 1]
	));
}

void SpiralOfFate::MainWindow::copyBoxesFromNextFrame()
{
	auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation < blk.size() - 1);
	this->applyOperation(new PasteBoxDataOperation(
		*this->_object,
		this->_editor.localize("operation.copy_box_last"),
		blk[this->_object->_animation + 1]
	));
}

void SpiralOfFate::MainWindow::flattenThisMoveCollisionBoxes()
{
	auto &f = this->_object->_moves[this->_object->_action][this->_object->_actionBlock][this->_object->_animation];

	this->applyOperation(new FlattenCollisionBoxesOperation(
		*this->_object,
		this->_editor.localize("operation.flatten_collision"),
		f.collisionBox ? std::optional(*f.collisionBox) : std::optional<Box>()
	));
}

void SpiralOfFate::MainWindow::flattenThisMoveProperties()
{
	this->applyOperation(new FlattenPropretiesOperation(
		*this->_object,
		this->_editor.localize("operation.flatten_properties"),
		this->_object->_moves[this->_object->_action][this->_object->_actionBlock][this->_object->_animation]
	));
}

void SpiralOfFate::MainWindow::cleanThisCharacterProperties()
{
	this->applyOperation(new CleanPropertiesOperation(
		*this->_object,
		this->_editor.localize("operation.flatten_properties")
	));
}

void SpiralOfFate::MainWindow::reloadTextures()
{
	game->textureMgr.reloadEverything();
}

void SpiralOfFate::MainWindow::invertColors()
{
	auto &pal = this->_palettes[this->_selectedPalette];
	std::array<Color, 256> palette = pal.colors;

	for (size_t i = 1; i < 251; i++) {
		palette[i].r = ~palette[i].r;
		palette[i].g = ~palette[i].g;
		palette[i].b = ~palette[i].b;
	}
	this->applyOperation(new EditColorsOperation(
		this->_editor.localize("operation.invert_colors"),
		pal, this->_selectedPalette,
		palette
	));
}

void SpiralOfFate::MainWindow::reversePalette()
{
	auto &pal = this->_palettes[this->_selectedPalette];
	std::array<Color, 256> palette;

	palette[0] = pal.colors[0];
	palette[251] = pal.colors[251];
	palette[252] = pal.colors[252];
	palette[253] = pal.colors[253];
	palette[254] = pal.colors[254];
	palette[255] = pal.colors[255];
	for (size_t i = 1; i < 251; i++)
		palette[i] = pal.colors[255 - i];
	this->applyOperation(new EditColorsOperation(
		this->_editor.localize("operation.reverse_palette"),
		pal, this->_selectedPalette,
		palette
	));
}

void SpiralOfFate::MainWindow::_rePopulateData()
{
	for (auto key : this->_containers)
		this->_populateData(*key);
	this->_object->resetState();
	this->_preview->frameChanged();
}

void SpiralOfFate::MainWindow::_rePopulateFrameData()
{
	for (auto key: this->_containers)
		this->_populateFrameData(*key);
}

void SpiralOfFate::MainWindow::_rePopulateColorData()
{
	for (auto key: this->_containers)
		this->_populateColorData(*key);
}

void SpiralOfFate::MainWindow::_populateData(const tgui::Container &container)
{
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto prevAction = container.get<tgui::Button>("PrevAction");
	auto nextAction = container.get<tgui::Button>("NextAction");
	auto block = container.get<tgui::Label>("BlockLabel");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");
	auto ctrl = container.get<tgui::Tabs>("ControlTabs");

	if (action)
		action->setText(std::to_string(this->_object->_action));
	if (actionSelect)
		actionSelect->setText(this->_localizeActionName(this->_object->_action) + " (" + std::to_string(this->_object->_action) + ")");
	if (prevAction) {
		auto it = this->_object->_moves.find(this->_object->_action);

		if (it != this->_object->_moves.begin()) {
			auto prev = std::prev(it)->first;

			prevAction->setText(this->_localizeActionName(prev) + " (" + std::to_string(prev) + ")");
			prevAction->setEnabled(true);
		} else {
			prevAction->setText(this->_editor.localize("animation.no_prev_action"));
			prevAction->setEnabled(false);
		}
	}
	if (nextAction) {
		auto it = this->_object->_moves.find(this->_object->_action);
		assert_exp(it != this->_object->_moves.end());
		auto next = std::next(it);

		if (next != this->_object->_moves.end()) {
			nextAction->setText(this->_localizeActionName(next->first) + " (" + std::to_string(next->first) + ")");
			nextAction->setEnabled(true);
		} else {
			nextAction->setText(this->_editor.localize("animation.no_next_action"));
			nextAction->setEnabled(false);
		}
	}
	if (block)
		block->setText(this->_editor.localize(
			block->getUserData<std::string>(),
			std::to_string(this->_object->_actionBlock),
			std::to_string(this->_object->_moves.at(this->_object->_action).size() - 1)
		));
	if (blockSpin) {
		blockSpin->onValueChange.setEnabled(false);
		blockSpin->setMinimum(0);
		blockSpin->setMaximum(this->_object->_moves.at(this->_object->_action).size() - 1);
		blockSpin->setValue(this->_object->_actionBlock);
		blockSpin->onValueChange.setEnabled(true);
	}
	if (frameSpin) {
		frameSpin->onValueChange.setEnabled(false);
		frameSpin->setMinimum(0);
		frameSpin->setMaximum(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frameSpin->onValueChange.setEnabled(true);
	}
	if (frame) {
		frame->onValueChange.setEnabled(false);
		frame->setMinimum(0);
		frame->setMaximum(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frame->onValueChange.setEnabled(true);
	}
	if (ctrl) {
		ctrl->onTabSelect.setEnabled(false);
		ctrl->select(this->_showingPalette);
		ctrl->onTabSelect.setEnabled(true);
	}

	this->_populateFrameData(container);
	this->_populateColorData(container);
}

void SpiralOfFate::MainWindow::_populateColorData(const tgui::Container &container)
{
	if (auto removePal = container.get<tgui::Button>("RemovePalette"))
		removePal->setEnabled(this->_palettes.size() > 1);

	if (this->_palettes.empty())
		return;

	auto mode = container.get<tgui::Tabs>("ColorModes");
	auto plane = container.get<ColorPlaneWidget>("ColorPlane");
	auto slider = container.get<ColorSliderWidget>("ColorSlider");
	auto comp1 = container.get<tgui::EditBox>("ColorComponent1");
	auto comp2 = container.get<tgui::EditBox>("ColorComponent2");
	auto comp3 = container.get<tgui::EditBox>("ColorComponent3");
	auto hexcode = container.get<tgui::EditBox>("RGBColor");
	auto paletteList = container.get<tgui::ComboBox>("PaletteList");
	auto preview = container.get<tgui::Label>("ColorPreview");
	auto colorIndex = container.get<tgui::Label>("ColorIndex");
	auto colorPanel = container.get<tgui::ScrollablePanel>("AllColorPanel");
	auto comp1lab = container.get<tgui::Label>("ColorComponentName1");
	auto comp2lab = container.get<tgui::Label>("ColorComponentName2");
	auto comp3lab = container.get<tgui::Label>("ColorComponentName3");
	auto color = this->_palettes[this->_selectedPalette].colors[this->_selectedColor];
	auto colorConv = colorConversionsReverse[this->_selectColorMethod](color);

	if (plane && this->_colorChangeSource == 0)
		colorConv = plane->getColor();
	if (slider && this->_colorChangeSource == 1)
		colorConv = slider->getColor();
	if (comp1 && this->_colorChangeSource == 2)
		colorConv[0] = std::stoul(comp1->getText().toStdString());
	if (comp2 && this->_colorChangeSource == 3)
		colorConv[1] = std::stoul(comp2->getText().toStdString());
	if (comp3 && this->_colorChangeSource == 4)
		colorConv[2] = std::stoul(comp3->getText().toStdString());
	if (mode) {
		mode->onTabSelect.setEnabled(false);
		mode->removeAll();
		for (size_t i = 0; i < std::size(colorConversions); i++)
			mode->add(this->_editor.localize("color.type" + std::to_string(i) + ".name"), this->_selectColorMethod == i);
		mode->onTabSelect.setEnabled(true);
	}
	if (colorPanel) {
		float extra = this->getSize().y - this->getInnerSize().y;
		auto lay = colorPanel->getSizeLayout();
		auto data = colorPanel->getUserData<tgui::String>();

		colorPanel->setSize(lay.x, tgui::Layout("max(" + data + " - " + std::to_string(extra) + ")"));
		for (size_t i = 0; i < 256; i++) {
			auto button = colorPanel->get<tgui::Button>("Color" + std::to_string(i));
			auto render = button->getRenderer();
			auto ccolor = this->_palettes[this->_selectedPalette].colors[i];

			render->setBorders({1, 1, 1, 1});
			render->setBorderColor(tgui::Color{0, 0, 0});
			render->setBorderColorHover(tgui::Color{100, 100, 100});
			render->setBorderColorDisabled(tgui::Color{200, 200, 200});
			render->setBorderColorFocused(tgui::Color{0, 0, 255});
			render->setBorderColorDown(tgui::Color{0, 0, 0});
			render->setBorderColorDownDisabled(tgui::Color{0, 0, 0});
			render->setBorderColorDownFocused(tgui::Color{0, 0, 0});
			render->setBorderColorDownHover(tgui::Color{0, 0, 0});

			render->setBackgroundColor(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorHover(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDisabled(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorFocused(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDown(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownDisabled(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownFocused(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownHover(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});

			render->setTexture(tgui::Texture{});
			render->setTextureHover(tgui::Texture{});
			render->setTextureDisabled(tgui::Texture{});
			render->setTextureFocused(tgui::Texture{});
			render->setTextureDown(tgui::Texture{});
			render->setTextureDownDisabled(tgui::Texture{});
			render->setTextureDownFocused(tgui::Texture{});
			render->setTextureDownHover(tgui::Texture{});
		}
	}
	if (colorIndex)
		colorIndex->setText("#" + std::to_string(this->_selectedColor));
	if (paletteList) {
		paletteList->onItemSelect.setEnabled(false);
		paletteList->removeAllItems();
		for (auto &s : this->_palettes)
			paletteList->addItem(s.path);
		paletteList->setSelectedItemByIndex(this->_selectedPalette);
		paletteList->onItemSelect.setEnabled(true);
	}
	if (preview) {
		auto r = preview->getRenderer();

		r->setBorders({1, 1, 1, 1});
		r->setBackgroundColor(sf::Color(color));
	}
	if (plane && this->_colorChangeSource != 0) {
		plane->onChanged.setEnabled(false);
		plane->setColor(colorConv);
		plane->onChanged.setEnabled(true);
	}
	if (slider && this->_colorChangeSource != 1) {
		slider->onChanged.setEnabled(false);
		slider->setColor(colorConv);
		slider->onChanged.setEnabled(true);
	}
	if (comp1 && this->_colorChangeSource != 2) {
		comp1->onTextChange.setEnabled(false);
		comp1->setText(std::to_string(colorConv[0]));
		comp1->onTextChange.setEnabled(true);
	}
	if (comp2 && this->_colorChangeSource != 3) {
		comp2->onTextChange.setEnabled(false);
		comp2->setText(std::to_string(colorConv[1]));
		comp2->onTextChange.setEnabled(true);
	}
	if (comp3 && this->_colorChangeSource != 4) {
		comp3->onTextChange.setEnabled(false);
		comp3->setText(std::to_string(colorConv[2]));
		comp3->onTextChange.setEnabled(true);
	}
	if (comp1lab && this->_colorChangeSource == 255)
		comp1lab->setText(this->_editor.localize("color.type" + std::to_string(this->_selectColorMethod) + ".component1"));
	if (comp2lab && this->_colorChangeSource == 255)
		comp2lab->setText(this->_editor.localize("color.type" + std::to_string(this->_selectColorMethod) + ".component2"));
	if (comp3lab && this->_colorChangeSource == 255)
		comp3lab->setText(this->_editor.localize("color.type" + std::to_string(this->_selectColorMethod) + ".component3"));
	if (hexcode && this->_colorChangeSource != 5) {
		char buffer[8];

		sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
		hexcode->onTextChange.setEnabled(false);
		hexcode->setText(buffer);
		hexcode->onTextChange.setEnabled(true);
	}
}

void SpiralOfFate::MainWindow::_populateFrameData(const tgui::Container &container)
{
	for (const auto &fct : this->_updateFrameElements[&container])
		fct();

	auto frameLabel = container.get<tgui::Label>("FrameLabel");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");

	if (frameLabel)
		frameLabel->setText(this->_editor.localize(
			frameLabel->getUserData<std::string>(),
			std::to_string(this->_object->_animation),
			std::to_string(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1)
		));
	if (frame) {
		frame->onValueChange.setEnabled(false);
		frame->setValue(this->_object->_animation);
		frame->onValueChange.setEnabled(true);
	}
	if (frameSpin) {
		frameSpin->onValueChange.setEnabled(false);
		frameSpin->setValue(this->_object->_animation);
		frameSpin->onValueChange.setEnabled(true);
	}
}

void SpiralOfFate::MainWindow::tick()
{
	auto animation = this->_object->_animation;

	if (this->_requireReload) {
		this->_rePopulateData();
		this->_preview->invalidatePalette();
		this->refreshMenuItems();
		this->_requireReload = false;
	}
	this->_timer++;
	if (this->_timer % 2 == 0) {
		if (this->_colorChangeSource != 255) {
			for (auto container : this->_containers)
				this->_populateColorData(*container);
			this->_colorChangeSource = 255;
			this->_preview->invalidatePalette();
		}
	}
	if (!this->_paused)
		this->_object->update();
	if (animation != this->_object->_animation) {
		this->_preview->frameChanged();
		for (auto key : this->_containers)
			this->_populateFrameData(*key);

		auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

		this->_editor.setCanCopyLast(this->_object->_animation > 0);
		this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
		this->_editor.setHasLastFrame(this->_object->_animation > 0);
		this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
	}
}

void SpiralOfFate::MainWindow::rendererChanged(const tgui::String &property)
{
	if (property == U"TitleColor") {
		this->_titleColorUnfocusedCached = this->getSharedRenderer()->getTitleColor();
		if (!this->isFocused())
			this->m_titleText.setColor(this->_titleColorUnfocusedCached);
	} else if (property == U"TitleColorFocused") {
		this->_titleColorFocusedCached = this->getSharedRenderer()->getTitleColorFocused();
		if (this->isFocused())
			this->m_titleText.setColor(this->_titleColorFocusedCached);

	} else if (property == U"TitleBarColor") {
		this->_titleBarColorUnfocusedCached = this->getSharedRenderer()->getTitleBarColor();
		if (!this->isFocused())
			this->m_titleBarColorCached = this->_titleBarColorUnfocusedCached;
	} else if (property == U"TitleBarColorFocused") {
		this->_titleBarColorFocusedCached = this->getSharedRenderer()->getTitleBarColorFocused();
		if (this->isFocused())
			this->m_titleBarColorCached = this->_titleBarColorFocusedCached;

	} else if (property == U"CloseButton") {
		this->_closeButtonRendererUnFocusedCached = this->getSharedRenderer()->getCloseButton();
		this->_updateTitleButtons();
	} else if (property == U"MaximizeButton") {
		this->_maximizeButtonRendererUnFocusedCached = this->getSharedRenderer()->getMaximizeButton();
		this->_updateTitleButtons();
	} else if (property == U"MinimizeButton") {
		this->_minimizeButtonRendererUnFocusedCached = this->getSharedRenderer()->getMinimizeButton();
		this->_updateTitleButtons();

	} else if (property == U"CloseButtonFocused") {
		this->_closeButtonRendererFocusedCached = this->getSharedRenderer()->getCloseButtonFocused();
		this->_updateTitleButtons();
	} else if (property == U"MaximizeButtonFocused") {
		this->_maximizeButtonRendererFocusedCached = this->getSharedRenderer()->getMaximizeButtonFocused();
		this->_updateTitleButtons();
	} else if (property == U"MinimizeButtonFocused") {
		this->_minimizeButtonRendererFocusedCached = this->getSharedRenderer()->getMinimizeButtonFocused();
		this->_updateTitleButtons();

	} else if (property == U"TextureTitleBar" || property == U"TextureTitleBarFocused") {
		this->_updateTextureTitleBar();
	} else
		tgui::ChildWindow::rendererChanged(property);
}

void SpiralOfFate::MainWindow::_updateTitleButtons()
{
	std::shared_ptr<tgui::RendererData> closeButtonRenderer;
	std::shared_ptr<tgui::RendererData> maximizeButtonRenderer;
	std::shared_ptr<tgui::RendererData> minimizeButtonRenderer;

	if (this->isFocused()) {
		closeButtonRenderer = this->_closeButtonRendererFocusedCached;
		maximizeButtonRenderer = this->_maximizeButtonRendererFocusedCached;
		minimizeButtonRenderer = this->_minimizeButtonRendererFocusedCached;
	} else {
		closeButtonRenderer = this->_closeButtonRendererUnFocusedCached;
		maximizeButtonRenderer = this->_maximizeButtonRendererUnFocusedCached;
		minimizeButtonRenderer = this->_minimizeButtonRendererUnFocusedCached;
	}

	if (!closeButtonRenderer || (closeButtonRenderer->propertyValuePairs.empty() && !closeButtonRenderer->connectedTheme))
		closeButtonRenderer = tgui::Theme::getDefault()->getRendererNoThrow("Button");
	if (!maximizeButtonRenderer || (maximizeButtonRenderer->propertyValuePairs.empty() && !maximizeButtonRenderer->connectedTheme))
		maximizeButtonRenderer = closeButtonRenderer;
	if (!minimizeButtonRenderer || (minimizeButtonRenderer->propertyValuePairs.empty() && !minimizeButtonRenderer->connectedTheme))
		minimizeButtonRenderer = closeButtonRenderer;
	this->m_closeButton->setRenderer(closeButtonRenderer);
	this->m_maximizeButton->setRenderer(maximizeButtonRenderer);
	this->m_minimizeButton->setRenderer(minimizeButtonRenderer);
	this->updateTitleBarHeight();
}

void SpiralOfFate::MainWindow::_updateTextureTitleBar()
{
	const float oldTitleBarHeight = this->m_titleBarHeightCached;

	if (this->isFocused())
		this->m_spriteTitleBar.setTexture(getSharedRenderer()->getTextureTitleBarFocused());
	else
		this->m_spriteTitleBar.setTexture(getSharedRenderer()->getTextureTitleBar());

	this->m_titleBarHeightCached = getSharedRenderer()->getTitleBarHeight();
	this->updateTitleBarHeight();
	if (oldTitleBarHeight != this->m_titleBarHeightCached) {
		if (this->m_decorationLayoutY && (this->m_decorationLayoutY == this->m_size.y.getRightOperand()))
			this->m_decorationLayoutY->replaceValue(
				this->m_bordersCached.getTop() +
				this->m_bordersCached.getBottom() +
				this->m_titleBarHeightCached +
				this->m_borderBelowTitleBarCached
			);

		// If the title bar changes in height then the inner size will also change
		this->recalculateBoundSizeLayouts();
	}
}

SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getSharedRenderer()
{
	return aurora::downcast<Renderer *>(Widget::getSharedRenderer());
}

const SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getSharedRenderer() const
{
	return aurora::downcast<const Renderer *>(Widget::getSharedRenderer());
}

SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getRenderer()
{
	return aurora::downcast<Renderer *>(Widget::getRenderer());
}

void SpiralOfFate::MainWindow::keyPressed(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Escape) {
		if (this->_pendingTransaction) {
			this->cancelTransaction();
			this->_requireReload = true;
			// FIXME: Special case for when you are dragging a box.
			//        Right now it will start a transaction that will never be removed
			//        and starting a new one will crash because of the assertion
			this->startTransaction();
		}
	} else
		ChildWindow::keyPressed(event);
}

bool SpiralOfFate::MainWindow::canHandleKeyPress(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Escape)
		return true;
	return ChildWindow::canHandleKeyPress(event);
}

void SpiralOfFate::MainWindow::navToNextFrame()
{
	auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation < blk.size() - 1);
	this->_object->_animation++;

	this->_paused = true;
	this->_object->resetState();
	this->_preview->frameChanged();
	this->_rePopulateFrameData();
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
}

void SpiralOfFate::MainWindow::navToPrevFrame()
{
	assert_exp(this->_object->_animation > 0);
	this->_object->_animation--;

	auto &blk = this->_object->_moves[this->_object->_action][this->_object->_actionBlock];

	this->_paused = true;
	this->_object->resetState();
	this->_preview->frameChanged();
	this->_rePopulateFrameData();
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
}

void SpiralOfFate::MainWindow::navToNextBlock()
{
	auto &act = this->_object->_moves[this->_object->_action];

	assert_exp(this->_object->_actionBlock < act.size() - 1);
	this->_object->_actionBlock++;

	auto &blk = act[this->_object->_actionBlock];

	this->_object->_animation = 0;
	this->_requireReload = true;
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToPrevBlock()
{
	assert_exp(this->_object->_actionBlock > 0);
	this->_object->_actionBlock--;

	auto &act = this->_object->_moves[this->_object->_action];
	auto &blk = act[this->_object->_actionBlock];

	this->_object->_animation = 0;
	this->_requireReload = true;
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToNextAction()
{
	auto curr = this->_object->_moves.find(this->_object->_action);
	assert_exp(curr != this->_object->_moves.end());
	auto it = std::next(curr);
	assert_exp(it != this->_object->_moves.end());
	auto move = it->first;
	auto &act = this->_object->_moves[move];
	auto &blk = act.front();

	curr = this->_object->_moves.find(move);
	assert_exp(curr != this->_object->_moves.end());

	this->_object->_action = move;
	this->_object->_actionBlock = 0;
	this->_object->_animation = 0;
	this->_object->_animationCtr = 0;
	this->_requireReload = true;
	this->_editor.setHasLastAction(curr != this->_object->_moves.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_moves.end());
	this->_editor.setHasLastBlock(false);
	this->_editor.setHasNextBlock(act.size() != 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToPrevAction()
{
	auto it = this->_object->_moves.find(this->_object->_action);
	assert_exp(it != this->_object->_moves.begin());
	auto move = std::prev(it)->first;
	auto &act = this->_object->_moves[move];
	auto &blk = act.front();

	auto curr = this->_object->_moves.find(move);
	assert_exp(curr != this->_object->_moves.end());

	this->_object->_action = move;
	this->_object->_actionBlock = 0;
	this->_object->_animation = 0;
	this->_object->_animationCtr = 0;
	this->_requireReload = true;
	this->_editor.setHasLastAction(curr != this->_object->_moves.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_moves.end());
	this->_editor.setHasLastBlock(false);
	this->_editor.setHasNextBlock(act.size() != 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navGoTo()
{
	this->_createMoveListPopup([this](unsigned move){
		auto curr = this->_object->_moves.find(move);
		assert_exp(curr != this->_object->_moves.end());

		auto &act = this->_object->_moves[move];
		auto &blk = act.front();

		this->_object->_action = move;
		this->_object->_actionBlock = 0;
		this->_object->_animation = 0;
		this->_object->_animationCtr = 0;
		this->_requireReload = true;
		this->_editor.setHasLastAction(curr != this->_object->_moves.begin());
		this->_editor.setHasNextAction(std::next(curr) != this->_object->_moves.end());
		this->_editor.setHasLastBlock(false);
		this->_editor.setHasNextBlock(act.size() != 1);
		this->_editor.setHasLastFrame(false);
		this->_editor.setHasNextFrame(blk.size() != 1);
		this->_editor.setCanCopyLast(false);
		this->_editor.setCanCopyNext(blk.size() != 1);
	}, false);
}

void SpiralOfFate::MainWindow::_reinitSidePanel(tgui::Container &panel)
{
	if (!this->_showingPalette)
		return;

	auto rect = panel.get<tgui::Panel>("ColorSpaceRectPanel");

	if (rect) {
		auto colorPlane = rect->get<ColorPlaneWidget>("ColorPlane");
		auto colorSlider = rect->get<ColorSliderWidget>("ColorSlider");

		colorPlane->setColorSpace(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod]);
		colorSlider->setColorSpace(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod]);
	}
}

void SpiralOfFate::MainWindow::_initSidePanel(tgui::Container &panel)
{
	if (!this->_showingPalette)
		return;

	auto rect = panel.get<tgui::Panel>("ColorSpaceRectPanel");

	if (rect) {
		rect->removeAllWidgets();

		auto colorPlane = ColorPlaneWidget::create(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod], {0, 1}, {"&.w - 30", "&.w - 30"});
		auto colorSlider= ColorSliderWidget::create(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod], 2, true, {"20", "&.w - 30"});

		rect->add(colorPlane, "ColorPlane");
		rect->add(colorSlider, "ColorSlider");
		colorPlane->setPosition(0, 0);
		colorSlider->setPosition("ColorPlane.x + ColorPlane.w", 0);
		this->_planes.push_back(colorPlane);
		this->_sliders.push_back(colorSlider);
	}
}

void SpiralOfFate::MainWindow::refreshInterface()
{
	this->_populateData(*this);
}

void SpiralOfFate::MainWindow::mouseMovedAbsolute(tgui::Vector2f pos)
{
	for (auto &widget : this->_planes)
		widget->mouseMovedAbs(pos - widget->getAbsolutePosition());
	for (auto &widget : this->_sliders)
		widget->mouseMovedAbs(pos - widget->getAbsolutePosition());
}
