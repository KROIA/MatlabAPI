#include <iostream>
#include "MatlabAPI.h"
#include <iostream>
#include <QApplication>
#include "tests.h"


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	MatlabAPI::LibraryInfo::printInfo();
	Log::UI::createConsoleView(Log::UI::nativeConsoleView);
	Log::UI::getConsoleView<Log::UI::NativeConsoleView>()->show();

	//std::vector<std::u16string> names = matlab::engine::findMATLAB();
	//auto matlabPtr = matlab::engine::connectMATLAB(u"MySession");

	if (!MatlabEngine::instantiate(u"MySession"))
	{
		if (!MatlabEngine::instantiate())
		{
			Logger::logError("Could not start MATLAB engine");
		}
	}

	std::cout << "Running "<< UnitTest::Test::getTests().size() << " tests...\n";
	UnitTest::Test::TestResults results;
	UnitTest::Test::runAllTests(results);
	UnitTest::Test::printResults(results);

	MatlabAPI::MatlabEngine::terminate();
	return results.getSuccess();
}