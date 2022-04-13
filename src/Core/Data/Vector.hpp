//
// Created by anonymus-raccoon on 1/15/20.
//

#ifndef THFGAME_VECTOR_HPP
#define THFGAME_VECTOR_HPP

#include <ostream>
#include <cmath>
#include <SFML/System/Vector2.hpp>

namespace SpiralOfFate
{
	template<typename T>
	class Vector2 {
	public:
		T x;
		T y;

		Vector2<T>() noexcept
			: x(0), y(0) {}

		Vector2<T>(T x, T y) noexcept
			: x(x), y(y) {}

		template <typename T2>
		Vector2<T>(Vector2<T2> v) noexcept
			: x(v.x), y(v.y) {}

		Vector2<T>(sf::Vector2<T> v) noexcept
			: x(v.x), y(v.y) {}

		template<typename T2>
		Vector2<T2> to() const noexcept
		{
			return {
				static_cast<T2>(this->x),
				static_cast<T2>(this->y)
			};
		}

		template <typename T2>
		operator sf::Vector2<T2>() const noexcept
		{
			return sf::Vector2<T2>(this->x, this->y);
		}

		template <typename T2>
		bool operator==(const Vector2<T2> &other) const
		{
			return other.x == this->x && this->y == other.y;
		}

		template <typename T2>
		bool operator!=(const Vector2<T2> &other) const
		{
			return other.x != this->x || this->y != other.y;
		}

		template<typename T2>
		Vector2<T> &operator+=(const Vector2<T2> &vec) noexcept
		{
			this->x += vec.x;
			this->y += vec.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator+(const Vector2<T2> &vec) const noexcept
		{
			return Vector2<T>(this->x + vec.x, this->y + vec.y);
		}

		template<typename T2>
		Vector2<T> &operator-=(const Vector2<T2> &vec) noexcept
		{
			this->x -= vec.x;
			this->y -= vec.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator-(const Vector2<T2> &vec) const noexcept
		{
			return Vector2<T>(this->x - vec.x, this->y - vec.y);
		}

		template<typename T2>
		Vector2<T> &operator*=(T2 d) noexcept
		{
			this->x *= d;
			this->y *= d;
			return *this;
		}

		template<typename T2>
		auto operator*(const Vector2<T2> &b) const noexcept
		{
			return this->x * b.x + this->y * b.y;
		}

		template<typename T2>
		Vector2<T> operator*(T2 d) const noexcept
		{
			return Vector2<T>(this->x * d, this->y * d);
		}

		template<typename T2>
		Vector2<T> operator/=(Vector2<T2> &b) noexcept
		{
			this->x /= b.x;
			this->y /= b.y;
			return this;
		}

		template<typename T2>
		Vector2<T> operator/(Vector2<T2> &b) const noexcept
		{
			return Vector2<T>(this->x / b.x, this->y / b.y);
		}

		template<typename T2>
		Vector2<T> operator/=(T2 b) noexcept
		{
			this->x /= b;
			this->y /= b;
			return this;
		}

		template<typename T2>
		auto operator/(T2 b) const noexcept
		{
			return Vector2<decltype(this->x / b)>(this->x / b, this->y / b);
		}

		Vector2<T> operator-() const noexcept
		{
			return Vector2<T>(-this->x, -this->y);
		}

		template<typename T2>
		double distance(const Vector2<T2> &o) const noexcept
		{
			return (*this - o).magnitude();
		}

		double magnitude() const noexcept
		{
			return (std::sqrt(this->x * this->x + this->y * this->y));
		}

		double angle(const Vector2<T> &o) const noexcept
		{
			return (std::atan2(o.y - this->y, o.x - this->x));
		}

		Vector2<float> normal(const Vector2<T> &p2)
		{
			return Vector2<float>{this->y - p2.y, p2.x - this->x}.normalized();
		}

		Vector2<T> normalize() noexcept
		{
			double mag = this->magnitude();

			this->x /= mag;
			this->y /= mag;
			return *this;
		}

		Vector2<T> normalized() const noexcept
		{
			T mag = this->magnitude();

			return Vector2<T>(this->x / mag, this->y / mag);
		}

		void project(const Vector2<T> &point) noexcept
		{
			*this = this->projection(point);
		}

		Vector2<float> projection(const Vector2<T> &point) const noexcept
		{
			return (point * this) / std::pow(this->magnitude(), 2) * *this;
		}

		void rotate(float angle, const Vector2<T> &center) noexcept
		{
			if (angle == 0.f)
				return;

			float c = cos(angle);
			float s = sin(angle);
			auto newX = c * (static_cast<float>(this->x) - center.x) - s * (static_cast<float>(this->y) - center.y) + center.x;

			this->y = s * (static_cast<float>(this->x) - center.x) + c * (static_cast<float>(this->y) - center.y) + center.y;
			this->x = newX;
		}

		Vector2<float> rotation(float angle, const Vector2<T> &center) const noexcept
		{
			Vector2<float> result = *this;

			result.rotate(angle, center);
			return result;
		}
	};

	typedef Vector2<float>              Vector2f;
	typedef Vector2<int>                Vector2i;
	typedef Vector2<unsigned>           Vector2u;
	typedef Vector2<long>               Vector2l;
	typedef Vector2<unsigned long>      Vector2ul;
	typedef Vector2<long long>          Vector2ll;
	typedef Vector2<unsigned long long> Vector2ull;
	typedef Vector2<double>             Vector2d;
	typedef Vector2<bool>               Vector2b;
}

template<typename T>
std::ostream &operator<<(std::ostream &s, const SpiralOfFate::Vector2<T> &v)
{
	s << v.x << " " << v.y;
	return s;
}

template<typename T>
std::istream &operator>>(std::istream &s, SpiralOfFate::Vector2<T> &v)
{
	s >> v.x >> v.y;
	return s;
}


#endif //THFGAME_VECTOR_HPP
