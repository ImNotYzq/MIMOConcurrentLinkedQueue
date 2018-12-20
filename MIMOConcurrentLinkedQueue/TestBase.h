#pragma once

#include <functional>
#include <atomic>
#include <iostream>

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
		enum RunTestStatus
		{
			SUCCEED,
			STILL_RUNNING,
			COMPLETED
		};
	public:
		class RequiredInputElemBase;
	public:
		template<class T>
		class RequiredInputElem;
	public:
		TestBase();
		virtual ~TestBase() noexcept(false);

		bool isNowRunning();
		RunTestStatus RunTest();
	protected:
		std::vector<RequiredInputElemBase*> inputElems;

		virtual void StartRun() = 0;
		virtual bool CheckInputValues() = 0;
		virtual void OnComplete(TestResult& result);

		void CompleteNotice(TestResult result);
	private:
		std::atomic<bool> isTestStillRunning;
	public:
		class RequiredInputElemBase
		{
		public:
			virtual const type_info& GetTypeInfo() = 0;
			virtual bool InputElem(std::istream& stream, std::string errorMessage) = 0;
			std::string GetDescription();
		protected:
			RequiredInputElemBase(const std::string description);
			virtual ~RequiredInputElemBase();
		private:
			std::string description;
		};
	public:
		template<class T>
		class RequiredInputElem final
			: public RequiredInputElemBase
		{
		public:
			RequiredInputElem(const std::string description, std::function<bool(T, std::string&)> check = nullptr)
				: RequiredInputElemBase(description),
				checkFunc(check)
			{
			}

			virtual ~RequiredInputElem()
			{
			}

			virtual bool InputElem(std::istream& stream, std::string errorMessage) override
			{
				stream.clear();
				stream >> value;
				stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				if (!stream)
				{
					errorMessage = "Please input the data needed correctly.";
					return false;
				}
				if (checkFunc != nullptr && !checkFunc(value, errorMessage))
				{
					return false;
				}
				return true;
			}

			T GetValue()
			{
				return value;
			}

			virtual const type_info& GetTypeInfo() override
			{
				return typeid(T);
			}
		private:
			T value;
			std::function<bool(T, std::string&)> checkFunc;
		};
	};
}