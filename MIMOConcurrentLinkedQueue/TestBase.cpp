#include "TestBase.h"

#include <iostream>
#include <string>

#include "DebugHead.h"

namespace test
{
	TestBase::TestBase()
		: isTestStillRunning(false)
	{
	}


	TestBase::~TestBase()
	{
	}

	bool TestBase::isNowRunning()
	{
		return isTestStillRunning;
	}

	TestBase::RunTestStatus TestBase::RunTest()
	{
		bool threadRunning = isTestStillRunning;
		while (!threadRunning && !isTestStillRunning.compare_exchange_weak(threadRunning, true));
		if(threadRunning)
			return RunTestStatus::STILL_RUNNING;

		std::string errorMessage;
		for (auto iter = inputElems.begin(); iter != inputElems.end(); iter++)
		{
			std::cout << (*iter)->GetDescription() << std::endl;
			while (!(*iter)->InputElem(std::cin, errorMessage))
			{
				std::cout << errorMessage << std::endl;
			}
		}

		if (!CheckInputValues())
		{
			isTestStillRunning = false;
			return RunTestStatus::COMPLETED;
		}

		StartRun();

		return RunTestStatus::SUCCEED;
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

	std::string TestBase::RequiredInputElemBase::GetDescription()
	{
		return description;
	}

	TestBase::RequiredInputElemBase::RequiredInputElemBase(std::string description)
		: description(description)
	{
	}

	TestBase::RequiredInputElemBase::~RequiredInputElemBase()
	{
	}
}