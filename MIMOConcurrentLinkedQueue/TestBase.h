#pragma once

#include <functional>
#include <atomic>

namespace test
{
	struct TestResult
	{
		bool succeed;
		double time_s_cost;
	};

	class TestBase
	{
	public:
		TestBase();
		virtual ~TestBase() noexcept(false);

		bool isNowRunning();
		bool RunTest();
	protected:
		virtual void StartRun() = 0;
		virtual void OnComplete(TestResult& result);

		void CompleteNotice(TestResult result);
	private:
		std::atomic<bool> isTestStillRunning;
	};
}