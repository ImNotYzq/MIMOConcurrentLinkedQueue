#include <iostream>
#include "MIMOLinkedQueueTest.h"
#include <memory>

#include "DebugHead.h"

using namespace std;

int Entrance()
{
	int tc, ic;
	auto testObj = test::MIMOLinkedQueueTest::PointerType::Create();

	shared_ptr<int> p;

	while (true)
	{
		do {
			cin.clear();
			cin >> tc >> ic;
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (!cin);

		if (tc == 0 || ic == 0)
			break;

		testObj->SetThreadCount(tc);
		testObj->SetInsertCount(ic);
		testObj->RunTest();
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
