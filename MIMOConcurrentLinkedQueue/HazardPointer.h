#pragma once

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include "DefaultFunctionDelete.h"
#include "Singleton.h"

namespace concurrent 
{
	class HazardPointer;

	//The real structure that holds using pointers
	class InnerHazardPointer final
		: public CloneDeleted
	{
		friend class HazardPointer;
	public:
		void * pointer = nullptr;
	private:
		InnerHazardPointer * next = nullptr;
		InnerHazardPointer * threadFreeNext = nullptr;

		InnerHazardPointer() = default;
		~InnerHazardPointer() = default;
	private:
		//A cache for the objects that might be released and need to be checked before.
		class HazardCache final
			: public CloneDeleted
		{
		private:
			static const int StableCountUpdatePeriod = 1000;
			static const int UnstableCountUpdatePeriod = 8;
		public:
			typedef void(*destructFunc)(void * a);
		private:
			//The elem that holds the pointer to the object might be released and the release method.
			class HazardCacheElem final
				: public CloneDeleted
			{
			public:
				HazardCacheElem * next;

				HazardCacheElem();
				~HazardCacheElem();

				void BindDeallocate(void * p = nullptr, destructFunc func = nullptr);
				void * GetPointer();
			private:
				void * pointer;
				destructFunc deallocateFunc;
			};
		public:
			HazardCache();
			~HazardCache();

			int GetCount();
			void AddToCache(void * p, destructFunc func);
			void MergeCache(const HazardCache & from);
		private:
			int count;
			int minRemoveCount;
			int minRemoveCountUpdate;
			HazardCacheElem head;
			HazardCacheElem * rear;

			void TryRemove(bool checkMinRemoveCount = true);
		};
	private:
		//Global Singleton that hold a linked list in which all of the hazard pointers are linked in.
		class HazardPointersCenter final
			: public Singleton<HazardPointersCenter>
		{
		public:
			HazardPointersCenter(helper h);
			~HazardPointersCenter();

			bool TryStartComparing();
			void ComparingComplete();

			bool IsPointerMightUsing(void * pointer);
			int GetCount();

			InnerHazardPointer * RequireInnerHazardPointer();
			void ReleaseInnerHazardPointer(InnerHazardPointer * hazardPointer);

			void TransferHazardCache(const HazardCache & from);
		private:
			std::atomic<int> count;
			InnerHazardPointer * head;
			std::mutex hazardCacheMtx;
			HazardCache cache;
			std::shared_mutex innerPointerRegistMtx;
		};
	private:
		//Thread Local Singleton that holds unused hazard pointers and cache for the objects that might be released.
		class HazardPointersManager final
			: public Singleton<HazardPointersManager, SingletonType::ThreadLazy>
		{
		public:
			HazardPointersManager(helper h);
			~HazardPointersManager();

			int GetTotalCount();

			InnerHazardPointer * RequireHazardPointer();
			void ReleaseHazardPointer(InnerHazardPointer * hazardPointer);
			void AddHazardCacheElem(void * p, HazardCache::destructFunc func);
		private:
			int totalCount;
			InnerHazardPointer * freeHead;
			HazardCache cache;
			HazardPointersCenter & centerInstance;
		};
	};

	//This class is similar to a guard, only hold the true hazard pointer structure when needed.
	class HazardPointer final
		: public CloneDeleted,
		public NewDeleted
	{
	private:
		using InnerCenter = InnerHazardPointer::HazardPointersCenter;
		using InnerManager = InnerHazardPointer::HazardPointersManager;
		using InnerCache = InnerHazardPointer::HazardCache;
	public:
		static int GetTotalPointerCount();
		static int GetThreadHoldingCount();
		template<class T>
		static void AddDeallocate(T * p, InnerCache::destructFunc func = nullptr)
		{
			InnerManager::GetInstance().AddHazardCacheElem(p, func != nullptr ? func : &DeallocateFunc<T>);
		}
	public:
		HazardPointer();
		~HazardPointer();

		void * & operator*();
		template<class T>
		T * HoldPointer(const std::atomic<T *> & from)
		{
			T * temp;
			for(;;)
			{
				temp = from;
				hp->pointer = temp;
				if (temp == from)
					return temp;
			}
		}
	private:
		InnerHazardPointer * hp;

		template<class T>
		static void DeallocateFunc(void * p)
		{
			delete static_cast<T *>(p);
		}
	};
}