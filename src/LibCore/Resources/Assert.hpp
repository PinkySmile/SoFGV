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
#define checked_cast(s, t, a) auto s = dynamic_cast<t *>(a); assert_exp(s)
#else
#define checked_cast(s, t, a) auto s = reinterpret_cast<t *>(a)
#endif

#define assert_exp(_Expression)                                                                \
	do {                                                                                   \
		if ((_Expression));                                                            \
		else                                                                           \
			throw AssertionFailedException(#_Expression);                          \
	} while (0)
#define assert_msg(_Expression, msg)                                                           \
	do {                                                                                   \
		if ((_Expression));                                                            \
		else                                                                           \
			throw AssertionFailedExceptionMsg(#_Expression, msg);                  \
	} while (0)
#define assert_eq(_Expression, _Expression2)                                                   \
	do {                                                                                   \
 		auto __a = (_Expression);                                                      \
		auto __b = (_Expression2);                                                     \
		                                                                               \
		if (__a != __b)                                                                \
			throw AssertionFailedExceptionMsg(                                     \
				std::string(#_Expression) + " == " + #_Expression2,            \
				std::to_string(__a) + " != " + std::to_string(__b)             \
			);                                                                     \
	} while (0)

#define assert_not_reached() throw AssertionFailedException("\"not reached\"")
#define assert_not_reached_msg(msg) throw AssertionFailedExceptionMsg("\"not reached\"", msg)

#define AssertionFailedException(expr) _AssertionFailedException( \
	"Debug Assertion " + std::string(expr) +                  \
	" failed in " + __FILE__ +                                \
	" at line " + std::to_string(__LINE__) +                  \
	" in " + FCT_NAME                                         \
)
#define AssertionFailedExceptionMsg(expr, msg) _AssertionFailedException( \
	"Debug Assertion " + std::string(expr) +                          \
	" failed in " + __FILE__ +                                        \
	" at line " + std::to_string(__LINE__) +                          \
	" in " + FCT_NAME + ": " + msg                                    \
)

class _AssertionFailedException : public std::exception {
private:
	std::string _msg;

public:
	_AssertionFailedException(const std::string &&msg);
	const char *what() const noexcept override;
};


#endif //SOFGV_ASSERT_HPP
