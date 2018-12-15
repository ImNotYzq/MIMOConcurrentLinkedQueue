#pragma once

#include <cstdint>
#include <limits>

class Well512
{
public:
	using result_type = uint32_t;

	static result_type min() { return 0; }
	static result_type max() { return std::numeric_limits<uint32_t>::max(); }

	static const uint32_t state_size = 16;

	Well512();
	Well512(uint32_t defaultState[], int size);
	result_type GetNext();

private:
	static const uint32_t shift_size = state_size - 1;
	static const uint32_t size_filter = state_size - 1;

	uint32_t index;
	uint32_t state[state_size];
};