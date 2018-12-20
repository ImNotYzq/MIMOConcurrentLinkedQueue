#pragma once

#include <atomic>
#include "LinkedQueueBase.h"

namespace concurrent::linkstructure {
	struct __LinkElem;

	namespace MIMOlinkedQueue {
		class MIMOLinkedQueue_ORI
			: public LinkedQueueBase
		{
		public:
			MIMOLinkedQueue_ORI(helper);
			virtual ~MIMOLinkedQueue_ORI();

			virtual void Enqueue(uintptr_t value) override;
			virtual bool Dequeue(uintptr_t & value) override;
		private:
			std::atomic<struct __LinkElem *> head;
			std::atomic<struct __LinkElem *> tail;
		};
	}
}

