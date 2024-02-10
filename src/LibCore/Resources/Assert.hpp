//
// Created by PinkySmile on 10/02/24.
//

#ifndef SOFGV_ASSERT_HPP
#define SOFGV_ASSERT_HPP


#include <string>

#ifdef __GNUC__
#define FCT_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FCT_NAME __FUNCSIG__
#else
#define FCT_NAME __func__
#endif

#ifdef _DEBUG
#define checked_cast(s, t, a) auto s = dynamic_cast<t *>(a); my_assert(s)
#else
#define checked_cast(s, t, a) auto s = reinterpret_cast<t *>(a)
#endif

#define my_assert(_Expression)                                                                 \
	do {                                                                                   \
		if ((_Expression));                                                            \
		else                                                                           \
			throw _AssertionFailedException(                                       \
				"Debug Assertion " + std::string(#_Expression) +               \
				" failed in " + __FILE__ +                                     \
				" at line " + std::to_string(__LINE__) +                       \
				" in " + FCT_NAME                                              \
			);                                                                     \
	} while (0)
#define my_assert2(_Expression, msg)                                                           \
	do {                                                                                   \
		if ((_Expression));                                                            \
		else                                                                           \
			throw AssertionFailedException(#_Expression, msg);                     \
	} while (0)

#define my_assert_eq(_Expression, _Expression2)                                                \
	do {                                                                                   \
 		auto a = (_Expression);                                                        \
		auto b = (_Expression2);                                                       \
		                                                                               \
		if (a != b)                                                                    \
			throw AssertionFailedException(                                        \
				std::string(#_Expression) + " == " + #_Expression2,            \
				std::to_string(a) + " != " + std::to_string(b)                 \
			);                                                                     \
	} while (0)

#define AssertionFailedException(expr, msg) _AssertionFailedException( \
	"Debug Assertion " + std::string(expr) +                       \
	" failed in " + __FILE__ +                                     \
	" at line " + std::to_string(__LINE__) +                       \
	" in " + FCT_NAME + ": " + msg                                 \
)

class _AssertionFailedException : public std::exception {
private:
	std::string _msg;

public:
	_AssertionFailedException(const std::string &&msg);
	const char *what() const noexcept override;
};


#endif //SOFGV_ASSERT_HPP
