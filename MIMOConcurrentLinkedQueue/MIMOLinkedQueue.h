#pragma once

#include <atomic>
#include <memory>
#include "DefaultFunctionDelete.h"
#include "LockFreeSharedPointer.h"

namespace concurrent::linkstructure {
	struct __LinkElem;
	
	namespace MIMOlinkedQueue {
		class MIMOLinkedQueue
			: public CloneDeleted,
			public LockFreeSharedPointedBase
		{
		public:
			MIMOLinkedQueue(helper);
			virtual ~MIMOLinkedQueue();

			void Enqueue(uintptr_t value);
			bool Dequeue(uintptr_t & value);
		private:
			std::atomic<struct __LinkElem *> head;
			std::atomic<struct __LinkElem *> tail;
			struct __LinkElem* preparedPointer;
		};
	}
}