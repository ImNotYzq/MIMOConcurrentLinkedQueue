#pragma once

#include <memory>
#include "DefaultFunctionDelete.h"
#include "LockFreeSharedPointer.h"

namespace concurrent
{
	class LinkedQueueBase
		: public CloneDeleted,
		public LockFreeSharedPointedBase
	{
	public:
		LinkedQueueBase() = default;
		virtual ~LinkedQueueBase() = default;

		virtual void Enqueue(uintptr_t value) = 0;
		virtual bool Dequeue(uintptr_t & value) = 0;
	};
}