//
// Created by PinkySmile on 17/12/22.
//

#ifndef SOFGV_RANDOMWRAPPER_HPP
#define SOFGV_RANDOMWRAPPER_HPP


#include <random>

namespace SpiralOfFate
{
	class RandomWrapper {
	public:
		struct SerializedWrapper {
			uint64_t seed = 0;
			uint64_t invoke_count = 0;
		};
		static_assert(sizeof(SerializedWrapper) == 16);

		SerializedWrapper ser;
		std::mt19937 rng;

		typedef std::mt19937::result_type result_type;

		RandomWrapper() = default;
		RandomWrapper(uint64_t _seed);
		RandomWrapper(const SerializedWrapper &ser);
		RandomWrapper(std::istream &in);
		void discard(unsigned long long z);
		void rollback(unsigned long long z);
		void seed(uint64_t _seed);
		result_type operator()();

		static constexpr result_type min() {
			return std::mt19937::min();
		}

		static constexpr result_type max() {
			return std::mt19937::max();
		}
	};
}


#endif //SOFGV_RANDOMWRAPPER_HPP
