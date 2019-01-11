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
		uint32_t n = counter;
		while (n != 0 && !counter.compare_exchange_weak(n, n + 1));
		return n != 0;
	}

	bool LockFreeSharedPointedBase::CheckRemoveCounter()
	{
		uint32_t n = counter;
		while (!counter.compare_exchange_weak(n, n - 1));
		return n <= 1;
	}
}