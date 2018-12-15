#pragma once

#include<chrono>
#include "TestBase.h"
#include "LockFreeSharedPointer.h"
#include "MIMOLinkedQueue.h"

namespace test 
{
	class MIMOLinkedQueueTest
		: public TestBase,
		public concurrent::LockFreeSharedPointedBase
	{
	public:
		using PointerType = concurrent::LockFreeSharedPointer<MIMOLinkedQueueTest>;
	public:
		MIMOLinkedQueueTest(helper h);
		~MIMOLinkedQueueTest();

		void SetThreadCount(int count);
		void SetInsertCount(uintptr_t count);
	protected:
		virtual void StartRun() override;
	private:
		using LinkedQueue = concurrent::linkstructure::MIMOlinkedQueue::MIMOLinkedQueue;

		int threadCount;
		uintptr_t insertCount;
		std::atomic<uintptr_t> valueFlagResult;
		std::atomic<uintptr_t> valueFlagResource;
		std::atomic<long> runningThreadCount;
		std::atomic<std::chrono::high_resolution_clock::rep> timeCost;

		static void ThreadFunc(PointerType testObject, concurrent::LockFreeSharedPointer<LinkedQueue> h, uintptr_t insertCount);
	};
}