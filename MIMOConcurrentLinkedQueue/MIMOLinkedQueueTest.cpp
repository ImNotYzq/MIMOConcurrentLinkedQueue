#include "MIMOLinkedQueueTest.h"

#include <thread>
#include "Well512.h"

#include "DebugHead.h"

namespace test {
	MIMOLinkedQueueTest::MIMOLinkedQueueTest(helper h)
		: threadCount(0),
		insertCount(0)
	{
	}

	MIMOLinkedQueueTest::~MIMOLinkedQueueTest()
	{
	}

	void MIMOLinkedQueueTest::SetThreadCount(int count)
	{
		threadCount = count;
	}

	void MIMOLinkedQueueTest::SetInsertCount(uintptr_t count)
	{
		insertCount = count;
	}

	void MIMOLinkedQueueTest::StartRun()
	{
		auto p = concurrent::LockFreeSharedPointer<LinkedQueue>::Create();

		valueFlagResource = 0;
		valueFlagResult = 0;
		runningThreadCount = threadCount;
		timeCost = 0;
		for (int i = 0; i < threadCount; i++)
		{
			std::thread(ThreadFunc, GetThisPointer<MIMOLinkedQueueTest>(), p, insertCount).detach();
		}
	}
	void MIMOLinkedQueueTest::ThreadFunc(PointerType testObject, concurrent::LockFreeSharedPointer<LinkedQueue> h, uintptr_t insertCount)
	{
		Well512 well512;
		uint32_t temp;
		uintptr_t v;

		auto start = std::chrono::high_resolution_clock::now();

		for (uintptr_t i = 0; i < insertCount; i++)
		{
			temp = i;
			testObject->valueFlagResource ^= temp;
			h->Enqueue(static_cast<size_t>(temp));
			while (h->Dequeue(v))
			{
				for (int i = 0; i < 10; i++)
				{
					well512.GetNext();
				}
				testObject->valueFlagResult ^= v;
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto cost = (end - start).count();
		if (testObject->timeCost < cost)
			testObject->timeCost = cost;

		if (--(testObject->runningThreadCount) == 0)
		{
			testObject->CompleteNotice(TestResult{ testObject->valueFlagResource == testObject->valueFlagResult, testObject->timeCost / (double)std::chrono::high_resolution_clock::period::den });
		}
	}
}