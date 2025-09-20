#pragma once

#include "UnitTest.h"
#include "MatlabAPI.h"
//#include <QObject>
//#include <QCoreapplication>




using namespace MatlabAPI;
class TST_Matrix : public UnitTest::Test
{
	TEST_CLASS(TST_Matrix)
public:
	TST_Matrix()
		: Test("TST_Matrix")
	{
		ADD_TEST(TST_Matrix::matmul);
		ADD_TEST(TST_Matrix::matlabInterface);
		//ADD_TEST(TST_Matrix::test2);

	}

private:

	// Tests
	TEST_FUNCTION(matmul)
	{
		TEST_START;
		Matrix m1({ {1, 2, 3},
					{4, 5, 6} });
		Matrix m2({ {2},
					{2},
					{2} });

		Matrix m3 = m1 * m2;

		TEST_MESSAGE("m1 * m2 = \n"+m3.toString());

		int a = 0;
		TEST_MESSAGE("is a == 0?");
		TEST_ASSERT(a == 0);
	}

	TEST_FUNCTION(matlabInterface)
	{
		TEST_START;
		//MatlabEngine::eval("m1=[1,2,3;4,5,6];");
		//MatlabEngine::eval("m2=[2;2;2];");
		Matrix m1({ {1, 2, 3},
					{4, 5, 6} });
		Matrix m2({ {2},
					{2},
					{2} });
		MatlabEngine::addVariable(m1.toMatlabArray("m1"));
		MatlabEngine::addVariable(m2.toMatlabArray("m2"));
		MatlabEngine::eval("m3= m1*m2;");
		

		Matrix m1Compare(MatlabEngine::getVariable("m1"));
		Matrix m2Compare(MatlabEngine::getVariable("m2"));
		Matrix m3Compare(MatlabEngine::getVariable("m3"));
		

		Matrix m3 = m1 * m2;
		TEST_ASSERT(m1 == m1Compare);
		TEST_ASSERT(m2 == m2Compare);
		TEST_ASSERT(m3 == m3Compare);
		TEST_MESSAGE("m3 = \n" + m3.toString());
		TEST_MESSAGE("m3Compare = \n" + m3Compare.toString());


		int a = 0;
		TEST_MESSAGE("is a == 0?");
		TEST_ASSERT(a == 0);
	}




	/*TEST_FUNCTION(test2)
	{
		TEST_START;

		int a = 0;
		TEST_ASSERT_M(a == 0, "is a == 0?");

		int b = 0;
		if (b != 0)
		{
			TEST_FAIL("b is not 0");
		}

		// fails if a != b
		TEST_COMPARE(a, b);
	}*/

};

TEST_INSTANTIATE(TST_Matrix);