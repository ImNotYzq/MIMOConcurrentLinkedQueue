#pragma once

#include <atomic>

namespace concurrent
{
	namespace linkstructure 
	{
		//The node of lock-free linked list.
		typedef struct __LinkElem
		{
			uintptr_t value;
			std::atomic<struct __LinkElem *> next;
		}LinkElem;
	}
}