#include "MIMOLinkedQueueTest.h"

#include <thread>
#include <random>
#include "MIMOLinkedQueue_ER.h"
#include "MIMOLinkedQueue_ORI.h"

#include "DebugHead.h"

using namespace concurrent;
using namespace std::chrono;

namespace test {

	MIMOLinkedQueueTest::MIMOLinkedQueueTest(helper h)
	{
		inputElems.push_back(new RequiredInputElem<uint32_t>(
			"Please Input the type of Concurrent Multi-in-Multi-out Linked Queue:\n1 as the origin one, 2 as the exclusive-refresh one",
			[](uint32_t value, std::string& errormessage)->bool 
		{
			if (value < 0 || value > 2) 
			{
				errormessage = "The type must be represented by 1 or 2. And you can type 0 to end the program";
				return false;
			}
			return true;
		}));
		inputElems.push_back(new RequiredInputElem<uint32_t>(
			"Please Input the count of threads you need the test program to run simultaneously"
			));
		inputElems.push_back(new RequiredInputElem<uint32_t>(
			"Please Input the count of enqueue action you need each thread to perform"
			));
	}

	MIMOLinkedQueueTest::~MIMOLinkedQueueTest()
	{
	}

	void MIMOLinkedQueueTest::StartRun()
	{
		LockFreeSharedPointer<LinkedQueue> p;
		switch (testTargetType)
		{
		case 1:
			p = LockFreeSharedPointer<linkstructure::MIMOlinkedQueue::MIMOLinkedQueue_ORI>::Create();
			break;
		case 2:
			p = LockFreeSharedPointer<linkstructure::MIMOlinkedQueue::MIMOLinkedQueue_ER>::Create();
			break;
		default:
			throw std::exception("Error Condition.");
			break;
		}

		valueFlagResource = 0;
		valueFlagResult = 0;
		runningThreadCount = threadCount;
		timeCost = 0;
		std::random_device rd;
		std::vector<unsigned int> seeds(threadCount);
		for (uint32_t i = 0; i < threadCount; i++)
		{
			seeds[i] = rd();
		}
		for (uint32_t i = 0; i < threadCount; i++)
		{
			std::thread(ThreadFunc, GetThisPointer<MIMOLinkedQueueTest>(), p, seeds[i], insertCount).detach();
		}
	}

	bool MIMOLinkedQueueTest::CheckInputValues()
	{
		if (inputElems[0]->GetTypeInfo() != typeid(uint32_t) || inputElems[1]->GetTypeInfo() != typeid(uint32_t) || inputElems[2]->GetTypeInfo() != typeid(uint32_t))
		{
			throw std::exception("Type Error");
			return false;
		}
		testTargetType = static_cast<RequiredInputElem<uint32_t>*>(inputElems[0])->GetValue();
		threadCount = static_cast<RequiredInputElem<uint32_t>*>(inputElems[1])->GetValue();
		insertCount = static_cast<RequiredInputElem<uint32_t>*>(inputElems[2])->GetValue();
		if (testTargetType == 0 || threadCount == 0 || insertCount == 0)
		{
			return false;
		}
		return true;
	}

	void MIMOLinkedQueueTest::ThreadFunc(PointerType testObject, concurrent::LockFreeSharedPointer<LinkedQueue> h, unsigned int seed, uintptr_t insertCount)
	{
		uintptr_t temp;
		uintptr_t v;
		std::default_random_engine re(seed);
		std::uniform_int_distribution uid(0, 100000000);

		auto start = high_resolution_clock::now();

		for (uintptr_t i = 0; i < insertCount; i++)
		{
			temp = uid(re);
			testObject->valueFlagResource ^= temp;
			h->Enqueue(static_cast<size_t>(temp));
			while (h->Dequeue(v))
			{
				testObject->valueFlagResult ^= v;
			}
		}

		auto end = high_resolution_clock::now();
		auto cost = (end - start).count();
		if (testObject->timeCost < cost)
			testObject->timeCost = cost;

		if (--(testObject->runningThreadCount) == 0)
		{
			testObject->CompleteNotice(TestResult{ testObject->valueFlagResource == testObject->valueFlagResult, testObject->timeCost / (double)high_resolution_clock::period::den });
		}
	}
}