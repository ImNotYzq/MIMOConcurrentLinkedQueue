#include <iostream>
#include "MIMOLinkedQueueTest.h"
#include <memory>

#include "DebugHead.h"

using namespace std;

int Entrance()
{
	auto testObj = test::MIMOLinkedQueueTest::PointerType::Create();

	while (testObj->RunTest() != test::TestBase::RunTestStatus::COMPLETED)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	return 0;
}

int main() {
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int result = Entrance();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	cout << "Press Enter to continue" << endl;
	cin.get();
#endif
	return result;
}
