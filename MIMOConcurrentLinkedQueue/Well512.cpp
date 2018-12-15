#include "Well512.h"

Well512::Well512()
	: index(0)
{
	for (int i = 0; i < state_size; i++)
	{
		state[i] = 0;
	}
}

Well512::Well512(uint32_t defaultState[], int size)
	: index(0)
{
	for (int i = 0; i < state_size; i++)
	{
		state[i] = defaultState[i % size];
	}
}

Well512::result_type Well512::GetNext()
{
	uint32_t a, b, c, d;

	a = state[index];
	c = state[(index + 13) & size_filter];
	b = a ^ c ^ (a << 16) ^ (c << 15);
	c = state[(index + 9) & size_filter];
	c = c ^ (c >> 11);
	a = state[index] = b ^ c;
	d = a ^ ((a << 15) & 0xda442d24U);
	index = (index + shift_size) & size_filter;
	a = state[index];
	state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);

	return state[index];
}