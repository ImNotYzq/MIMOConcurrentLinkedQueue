#include "HazardPointer.h"

#include "DebugHead.h"

namespace concurrent {
	
	//HazardPointersCenter Start
	InnerHazardPointer::HazardPointersCenter::HazardPointersCenter(helper h)
		: count(0),
		head(nullptr),
		hazardCacheMtx(),
		cache()
	{
	}

	InnerHazardPointer::HazardPointersCenter::~HazardPointersCenter()
	{
	}

	InnerHazardPointer * InnerHazardPointer::HazardPointersCenter::RequireInnerHazardPointer()
	{
		InnerHazardPointer * hazardPointer = new InnerHazardPointer();

		std::unique_lock<std::shared_mutex> lock(innerPointerRegistMtx);

		hazardPointer->next = head;
		head = hazardPointer;
		count++;

		return hazardPointer;
	}

	void InnerHazardPointer::HazardPointersCenter::ReleaseInnerHazardPointer(InnerHazardPointer * hazardPointer)
	{
		if (hazardPointer == nullptr)
			return;

		std::unique_lock<std::shared_mutex> lock(innerPointerRegistMtx);

		if (head == hazardPointer) {
			head = hazardPointer->next;
			hazardPointer->next = nullptr;
			count--;
		}
		else {
			for (InnerHazardPointer * p = head; p->next != nullptr; p = p->next) {
				if (p->next == hazardPointer) {
					p->next = hazardPointer->next;
					hazardPointer->next = nullptr;
					count--;
					break;
				}
			}
		}

		delete hazardPointer;
	}

	void InnerHazardPointer::HazardPointersCenter::TransferHazardCache(const HazardCache & from)
	{
		std::lock_guard<std::mutex> guard(hazardCacheMtx);
		cache.MergeCache(from);
	}

	bool InnerHazardPointer::HazardPointersCenter::TryStartComparing()
	{
		return innerPointerRegistMtx.try_lock_shared();
	}

	void InnerHazardPointer::HazardPointersCenter::ComparingComplete()
	{
		innerPointerRegistMtx.unlock_shared();
	}

	bool InnerHazardPointer::HazardPointersCenter::IsPointerMightUsing(void * pointer)
	{
		if (pointer == nullptr)
			return false;
		for (InnerHazardPointer * p = head; p != nullptr; p = p->next) {
			if (p->pointer == pointer) {
				return true;
			}
		}
		return false;
	}

	int InnerHazardPointer::HazardPointersCenter::GetCount()
	{
		return count;
	}
	//HazardPointersCenter End

	//HazardPointersManager Start
	InnerHazardPointer::HazardPointersManager::HazardPointersManager(helper h)
		: totalCount(0),
		freeHead(nullptr),
		cache(),
		centerInstance(HazardPointersCenter::GetInstance())
	{
	}

	InnerHazardPointer::HazardPointersManager::~HazardPointersManager()
	{
		InnerHazardPointer * next;
		while (freeHead != nullptr)
		{
			next = freeHead->threadFreeNext;
			centerInstance.ReleaseInnerHazardPointer(freeHead);
			totalCount--;
			freeHead = next;
		}
	}

	int InnerHazardPointer::HazardPointersManager::GetTotalCount()
	{
		return totalCount;
	}

	InnerHazardPointer * InnerHazardPointer::HazardPointersManager::RequireHazardPointer()
	{
		InnerHazardPointer * result;
		if (freeHead != nullptr)
		{
			result = freeHead;
			freeHead = freeHead->threadFreeNext;
		}
		else
		{
			result = centerInstance.RequireInnerHazardPointer();
		}
		return result;
	}

	void InnerHazardPointer::HazardPointersManager::ReleaseHazardPointer(InnerHazardPointer * hazardPointer)
	{
		hazardPointer->threadFreeNext = freeHead;
		freeHead = hazardPointer;
	}
	void InnerHazardPointer::HazardPointersManager::AddHazardCacheElem(void * p, HazardCache::destructFunc func)
	{
		cache.AddToCache(p, func);
	}
	//HazardPointersManager End

	//HazardPointer Start
	HazardPointer::HazardPointer()
	{
		InnerManager& innerManager = InnerManager::GetInstance();
		hp = innerManager.RequireHazardPointer();
	}

	HazardPointer::~HazardPointer()
	{
		hp->pointer = nullptr;
		InnerManager::GetInstance().ReleaseHazardPointer(hp);
	}

	void * & HazardPointer::operator*()
	{
		return hp->pointer;
	}

	int HazardPointer::GetTotalPointerCount() 
	{
		return InnerCenter::GetInstance().GetCount();
	}

	int HazardPointer::GetThreadHoldingCount()
	{
		return InnerManager::GetInstance().GetTotalCount();
	}
	//HazardPointer End

	//HazardCache Start
	InnerHazardPointer::HazardCache::HazardCache()
		: count(0),
		minRemoveCount(8),
		minRemoveCountUpdate(1),
		head(),
		rear(&head)
	{
	}

	InnerHazardPointer::HazardCache::~HazardCache()
	{
		TryRemove(false);
		if (count > 0)
		{
			HazardPointersCenter::GetInstance().TransferHazardCache(*this);
		}
		count = 0;
	}

	int InnerHazardPointer::HazardCache::GetCount()
	{
		return count;
	}

	void InnerHazardPointer::HazardCache::AddToCache(void * p, destructFunc func)
	{
		HazardCacheElem * elem = new HazardCacheElem;
		elem->BindDeallocate(p, func);
		elem->next = nullptr;

		rear->next = elem;

		rear = elem;
		count++;

		TryRemove();
	}

	void InnerHazardPointer::HazardCache::MergeCache(const HazardCache & from)
	{
		TryRemove();

		rear->next = from.head.next;
		rear = from.rear;
		count += from.count;
	}

	void InnerHazardPointer::HazardCache::TryRemove(bool checkMinRemoveCount)
	{
		HazardPointersCenter& instance = HazardPointersCenter::GetInstance();

		if (checkMinRemoveCount) {
			minRemoveCount = instance.GetCount() << 1;
			if(count < minRemoveCount)
				return;
		}

		if (!instance.TryStartComparing())
			return;
		
		for(HazardCacheElem * prev = &head, * elem = prev->next; elem != nullptr; elem = prev->next)
		{
			if(!instance.IsPointerMightUsing(elem->GetPointer()))
			{
				prev->next = elem->next;
				if (elem == rear)
					rear = prev;
				delete elem;
				count--;
			}
			else
			{
				prev = elem;
			}
		}

		instance.ComparingComplete();
	}

	InnerHazardPointer::HazardCache::HazardCacheElem::HazardCacheElem()
		: next(nullptr),
		pointer(nullptr),
		deallocateFunc(nullptr)
	{
	}

	InnerHazardPointer::HazardCache::HazardCacheElem::~HazardCacheElem()
	{
		BindDeallocate();
	}

	void InnerHazardPointer::HazardCache::HazardCacheElem::BindDeallocate(void * p, destructFunc func)
	{
		if(pointer != nullptr && deallocateFunc != nullptr)
		{
			deallocateFunc(pointer);
		}
		pointer = p;
		deallocateFunc = func;
	}
	void * InnerHazardPointer::HazardCache::HazardCacheElem::GetPointer()
	{
		return pointer;
	}
	//HazardCache End
}