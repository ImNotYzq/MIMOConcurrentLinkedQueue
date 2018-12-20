#include "MIMOLinkedQueue_ORI.h"



#include "LinkElem.h"
#include "HazardPointer.h"

#include "DebugHead.h"

namespace concurrent::linkstructure::MIMOlinkedQueue
{

	MIMOLinkedQueue_ORI::MIMOLinkedQueue_ORI(helper)
	{
		auto elem = new LinkElem;
		elem->next = nullptr;
		head = tail = elem;
	}


	MIMOLinkedQueue_ORI::~MIMOLinkedQueue_ORI()
	{
		LinkElem * prev = head;
		LinkElem * next;
		for (; prev != nullptr;)
		{
			next = prev->next;
			if (next == prev)
			{
				next = nullptr;
			}
			delete prev;
			prev = next;
		}
	}

	void MIMOLinkedQueue_ORI::Enqueue(uintptr_t value)
	{
		LinkElem* elem = new LinkElem{ value, nullptr };
		static int counter = 0;

		LinkElem * t;
		LinkElem * next;
		HazardPointer hp[2];
		for (;;)
		{
			t = hp[0].HoldPointer(tail);
			next = t->next;
			if (next != nullptr)
			{
				tail.compare_exchange_weak(t, next);
				continue;
			}

			if (t->next.compare_exchange_weak(next, elem))
				break;
		}
		tail.compare_exchange_strong(t, elem);
	}

	bool MIMOLinkedQueue_ORI::Dequeue(uintptr_t & value)
	{
		LinkElem *h;
		LinkElem *t;
		LinkElem *next;
		HazardPointer hp[2];

		for (;;)
		{
			h = hp[0].HoldPointer(head);
			t = tail;
			if (h == head)
			{
				next = hp[1].HoldPointer(h->next);
				if (h == t)
				{
					if (next == nullptr)
						return false;
					tail.compare_exchange_weak(t, next);
				}
				else
				{
					if (head.compare_exchange_weak(h, next))
					{
						value = next->value;
						break;
					}
				}
			}
		}
		HazardPointer::AddDeallocate(h);
		return true;
	}
}
