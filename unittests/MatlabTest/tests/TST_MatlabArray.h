#pragma once

#include "UnitTest.h"
#include "MatlabAPI.h"
//#include <QObject>
//#include <QCoreapplication>




using namespace MatlabAPI;
class TST_MatlabArray : public UnitTest::Test
{
	TEST_CLASS(TST_MatlabArray)
	
public:
	TST_MatlabArray()
		: Test("TST_MatlabArray")
	{
		ADD_TEST(TST_MatlabArray::scalar);
		ADD_TEST(TST_MatlabArray::vector);
		ADD_TEST(TST_MatlabArray::printVariables);

		

	}

private:
	const double scalarValue = 42.0;

	// Tests
	TEST_FUNCTION(scalar)
	{
		TEST_START;
		MatlabArray *scalar = new MatlabArray("scalar", scalarValue);
		TEST_MESSAGE("Scalar value: " +std::to_string(scalar->getScalar()));
		TEST_ASSERT(scalar->getScalar() == scalarValue);

		TEST_ASSERT(MatlabEngine::addVariable(scalar) == true);
	}




	TEST_FUNCTION(vector)
	{
		TEST_START;

		// Create vector
		std::vector<double> vec_data = { 1.0, 2.0, 3.0, 4.0, 5.0 };
		MatlabArray *vector = new MatlabArray("vec_data", vec_data);
		TEST_MESSAGE("Vector size: " +std::to_string(vector->getNumberOfElements()));

		std::vector<double> convertedBack = vector->getDoubleVector();
		TEST_ASSERT(convertedBack == vec_data);

		TEST_ASSERT(MatlabEngine::addVariable(vector) == true);

		// Multiply in MATLAB
		int evalResult = MatlabEngine::eval("vec_data = vec_data * scalar;");
		TEST_ASSERT(evalResult == 0);
		std::vector<double> expected;
		for (double v : vec_data) 
			expected.push_back(v * scalarValue);

		TEST_ASSERT(MatlabEngine::getVariable("vec_data")->getDoubleVector() == expected);
	}


	TEST_FUNCTION(printVariables)
	{
		TEST_START;

		auto vars = MatlabEngine::listVariables();
		for (const auto& name : vars)
		{
			auto var = MatlabEngine::getVariable(name);
			TEST_MESSAGE("Variable: " + var->toString());
		}
	}
};

TEST_INSTANTIATE(TST_MatlabArray);