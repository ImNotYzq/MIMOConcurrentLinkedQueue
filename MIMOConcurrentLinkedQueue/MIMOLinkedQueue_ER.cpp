#include "MIMOLinkedQueue_ER.h"

#include "LinkElem.h"
#include "HazardPointer.h"

#include "DebugHead.h"

namespace concurrent::linkstructure::MIMOlinkedQueue 
{

	MIMOLinkedQueue_ER::MIMOLinkedQueue_ER(helper)
	{
		preparedPointer = new LinkElem;
		preparedPointer->next = preparedPointer;
		head = tail = preparedPointer;
	}


	MIMOLinkedQueue_ER::~MIMOLinkedQueue_ER()
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

	void MIMOLinkedQueue_ER::Enqueue(uintptr_t value)
	{
		LinkElem* elem = new LinkElem{ 0, nullptr };
		LinkElem* prev;
		LinkElem* next;
		
		//Tail node is like a entrance, every enqueue operation just need to hold one node from tail and set the current node to its 'next' pointer.
		//We do not care about whether the prev node has linked yet. It would be linked to the list one day.
		do
		{
			prev = tail;
		} while (!tail.compare_exchange_weak(prev, elem));

		//I would like to put the value to the prev node, since no other threads might change the value before the current node is linked to it.
		prev->value = value;
		do
		{
			next = prev->next;
		} while (!prev->next.compare_exchange_weak(next, elem));

		//The next pointer become a circle to itself is used as a mark of needing to move the prepared pointer back till the null pointer.
		if (next == prev)
		{
			do {
				next = preparedPointer->next;
				//Keep moving back prepared pointer untill it reaches the null pointer.
				while (next != nullptr) {
					if (next == preparedPointer)
						return;
					preparedPointer = next;
					next = preparedPointer->next;
				}
				//When the prepared pointer cannot move back again, try to put the mark to the current prepared pointer's next pointer.
				//If another thread is tryint to point it to a new node, only one action would succeed.
				//If the current thread failed, then just move on untill another null pointer.
				//If the adding thread failed, it would try to link to the next pointer again and get the mark.
			} while (!preparedPointer->next.compare_exchange_weak(next, preparedPointer));
		}
	}

	bool MIMOLinkedQueue_ER::Dequeue(uintptr_t & value)
	{
		LinkElem * h = nullptr;
		HazardPointer hp;
		
		//Simply take the node out when it is confirmed to be linked.
		do
		{
			h = hp.HoldPointer(head);
			if (h == preparedPointer)
				return false;
		} while (!head.compare_exchange_weak(h, h->next));

		value = h->value;
		HazardPointer::AddDeallocate(h);
		return true;
	}
}