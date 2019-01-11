#pragma once

#include<chrono>
#include<random>
#include "TestBase.h"
#include "LockFreeSharedPointer.h"
#include "LinkedQueueBase.h"

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
		virtual ~MIMOLinkedQueueTest();
	protected:
		virtual void StartRun() override;
		virtual bool CheckInputValues() override;
	private:
		using LinkedQueue = concurrent::LinkedQueueBase;

		uint32_t testTargetType;
		uint32_t threadCount;
		uint32_t insertCount;
		std::atomic<uintptr_t> valueFlagResult;
		std::atomic<uintptr_t> valueFlagResource;
		std::atomic<long> runningThreadCount;
		std::atomic<std::chrono::high_resolution_clock::rep> timeCost;

		static void ThreadFunc(PointerType testObject, concurrent::LockFreeSharedPointer<LinkedQueue> h, unsigned int seed, uintptr_t insertCount);
	};
}