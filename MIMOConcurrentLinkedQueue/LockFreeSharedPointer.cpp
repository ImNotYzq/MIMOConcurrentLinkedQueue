#include "LockFreeSharedPointer.h"

#include "DebugHead.h"

namespace concurrent {
	LockFreeSharedPointedBase::LockFreeSharedPointedBase()
		: counter(1)
	{
	}

	LockFreeSharedPointedBase::~LockFreeSharedPointedBase()
	{
	}

	bool LockFreeSharedPointedBase::CheckAddCounter()
	{
		uint32_t n;
		do
		{
			n = counter;
			if (n == 0) 
			{
				return false;
			}
		} while (!counter.compare_exchange_weak(n, n + 1));

		return true;
	}

	bool LockFreeSharedPointedBase::CheckRemoveCounter()
	{
		uint32_t n;
		do
		{
			n = counter;
		} while (!counter.compare_exchange_weak(n, n - 1));

		return n <= 1;
	}
}