//
// Created by PinkySmile on 17/12/22.
//

#include <istream>
#include "RandomWrapper.hpp"

namespace SpiralOfFate
{
	RandomWrapper::RandomWrapper(uint64_t _seed) :
		ser{_seed, 0},
		rng(_seed)
	{
	}

	RandomWrapper::RandomWrapper(const RandomWrapper::SerializedWrapper &ser) :
		ser{ser},
		rng(ser.seed)
	{
		this->rng.discard(this->ser.invoke_count);
	}

	RandomWrapper::RandomWrapper(std::istream &in)
	{
		in.read(reinterpret_cast<char *>(&this->ser.seed), sizeof(this->ser.seed));
		in.read(reinterpret_cast<char *>(&this->ser.invoke_count), sizeof(this->ser.invoke_count));
		this->rng.seed(this->ser.seed);
		this->rng.discard(this->ser.invoke_count);
	}

	void RandomWrapper::discard(unsigned long long z)
	{
		this->rng.discard(z);
		this->ser.invoke_count += z;
	}

	void RandomWrapper::rollback(unsigned long long int z)
	{
		this->ser.invoke_count = z;
		this->rng = std::mt19937(this->ser.seed);
		this->rng.discard(this->ser.invoke_count);
	}

	RandomWrapper::result_type RandomWrapper::operator()()
	{
		++this->ser.invoke_count;
		return this->rng();
	}

	void RandomWrapper::seed(uint64_t _seed)
	{
		this->ser.seed = _seed;
		this->ser.invoke_count = 0;
		this->rng.seed(this->ser.seed);
	}
}