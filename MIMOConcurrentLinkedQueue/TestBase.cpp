#include "TestBase.h"

#include <iostream>

#include "DebugHead.h"

namespace test
{
	TestBase::TestBase()
		: isTestStillRunning(false)
	{
	}


	TestBase::~TestBase() noexcept(false)
	{
		if(isTestStillRunning)
		{
			throw std::exception("Test object is destructed before complete!\nCheck your test class");
		}
	}

	bool TestBase::isNowRunning()
	{
		return isTestStillRunning;
	}

	bool TestBase::RunTest()
	{
		bool threadRunning;
		do {
			threadRunning = isTestStillRunning;
			if (threadRunning)
				return false;
		} while (!isTestStillRunning.compare_exchange_weak(threadRunning, true));

		StartRun();

		return true;
	}

	void TestBase::OnComplete(TestResult& result)
	{
		std::cout << "Testing " << (result.succeed ? "Succeed" : "Failed") << std::endl;
		std::cout << "Time Cost : " << result.time_s_cost << "s" << std::endl;
	}

	void TestBase::CompleteNotice(TestResult result)
	{
		isTestStillRunning = false;
		OnComplete(result);
	}
}