//
// Created by anonymus-raccoon on 1/15/20.
//

#ifndef THFGAME_VECTOR_HPP
#define THFGAME_VECTOR_HPP

#include <ostream>
#include <cmath>
#include <SFML/System/Vector2.hpp>

namespace Battle
{
	template<typename T>
	class Vector2 {
	public:
		T x;
		T y;

		Vector2<T>()
			: x(0), y(0) {}

		Vector2<T>(T x, T y)
			: x(x), y(y) {}

		template <typename T2>
		Vector2<T>(Vector2<T2> v)
			: x(v.x), y(v.y) {}

		Vector2<T>(sf::Vector2<T> v)
			: x(v.x), y(v.y) {}

		template<typename T2>
		Vector2<T2> to()
		{
			return {
				static_cast<T2>(this->x),
				static_cast<T2>(this->y)
			};
		}

		template <typename T2>
		operator sf::Vector2<T2>()
		{
			return sf::Vector2<T2>(this->x, this->y);
		}

		template<typename T2>
		Vector2<T> &operator+=(const Vector2<T2> &vec)
		{
			this->x += vec.x;
			this->y += vec.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator+(const Vector2<T2> &vec) const
		{
			return Vector2<T>(this->x + vec.x, this->y + vec.y);
		}

		template<typename T2>
		Vector2<T> &operator-=(const Vector2<T2> &vec)
		{
			this->x -= vec.x;
			this->y -= vec.y;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator-(const Vector2<T2> &vec) const
		{
			return Vector2<T>(this->x - vec.x, this->y - vec.y);
		}

		template<typename T2>
		Vector2<T> &operator*=(T2 d)
		{
			this->x *= d;
			this->y *= d;
			return *this;
		}

		template<typename T2>
		Vector2<T> operator*(T2 d) const
		{
			return Vector2<T>(this->x * d, this->y * d);
		}

		template<typename T2>
		T operator*(Vector2<T2> &b) const
		{
			return this->x * b.x + this->y * b.y;
		}

		template<typename T2>
		Vector2<T> operator/=(Vector2<T2> &b)
		{
			this->x /= b.x;
			this->y /= b.y;
			return this;
		}

		template<typename T2>
		Vector2<T> operator/(Vector2<T2> &b) const
		{
			return Vector2<T>(this->x / b.x, this->y / b.y);
		}

		template<typename T2>
		Vector2<T> operator/=(T2 b)
		{
			this->x /= b;
			this->y /= b;
			return this;
		}

		template<typename T2>
		auto operator/(T2 b) const
		{
			return Vector2<decltype(this->x / b)>(this->x / b, this->y / b);
		}

		template<typename T2>
		double distance(const Vector2<T2> &o) const
		{
			return (*this - o).magnitude();
		}

		double magnitude() const
		{
			return (std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2)));
		}

		double angle(const Vector2<T> &o) const
		{
			return (std::atan2(o.y - this->y, o.x - this->x));
		}

		Vector2<T> normalize()
		{
			double mag = this->magnitude();

			this->x /= mag;
			this->y /= mag;
			return *this;
		}

		Vector2<T> normalized() const
		{
			T mag = this->magnitude();

			return Vector2<T>(this->x / mag, this->y / mag);
		}

		void project(const Vector2<T> &point)
		{
			*this = this->projection(point);
		}

		Vector2<float> projection(const Vector2<T> &point) const
		{
			return (point * this) / std::pow(this->magnitude(), 2) * *this;
		}

		void rotate(float angle, const Vector2<T> &center)
		{
			if (angle == 0.f)
				return;

			float c = cos(angle);
			float s = sin(angle);

			this->x = c * (static_cast<float>(x) - center.x) - s * (static_cast<float>(y) - center.y) + center.x;
			this->y = s * (static_cast<float>(x) - center.x) + c * (static_cast<float>(y) - center.y) + center.y;

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
std::ostream &operator<<(std::ostream &s, const Battle::Vector2<T> &v)
{
	s << v.x << " " << v.y;
	return s;
}

template<typename T>
std::istream &operator>>(std::istream &s, Battle::Vector2<T> &v)
{
	s >> v.x >> v.y;
	return s;
}


#endif //THFGAME_VECTOR_HPP
