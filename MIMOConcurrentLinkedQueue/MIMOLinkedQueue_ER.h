#pragma once

#include <atomic>
#include "LinkedQueueBase.h"

namespace concurrent::linkstructure {
	struct __LinkElem;
	
	namespace MIMOlinkedQueue {
		class MIMOLinkedQueue_ER
			: public LinkedQueueBase
		{
		public:
			MIMOLinkedQueue_ER(helper);
			virtual ~MIMOLinkedQueue_ER();

			virtual void Enqueue(uintptr_t value) override;
			virtual bool Dequeue(uintptr_t & value) override;
		private:
			std::atomic<struct __LinkElem *> head;
			std::atomic<struct __LinkElem *> tail;
			struct __LinkElem* volatile preparedPointer;
		};
	}
}