#pragma once

#include "UnitTest.h"
#include "ui/PlotMainWindow.h"
#include "MatlabAPI.h"
//#include <QObject>
//#include <QCoreapplication>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>







class TST_QTPlot : public UnitTest::Test
{
	TEST_CLASS(TST_QTPlot)
public:
    TST_QTPlot()
		: Test("TST_QTPlot")
	{
		ADD_TEST(TST_QTPlot::plot1);
		ADD_TEST(TST_QTPlot::test2);

	}

private:
    // Helper function to start the QT event loop until a timer expires
    void startQTEventLoopUntilTimer(int ms)
    {
        QTimer windowCloseTimer;
        QObject::connect(&windowCloseTimer, &QTimer::timeout, [this]() {
            qApp->quit();
            });
        windowCloseTimer.start(ms);
        qApp->exec();
    }

	// Tests
	TEST_FUNCTION(plot1)
	{
		TEST_START;
		PlotMainWindow mainWindow;
		mainWindow.show();
		

        startQTEventLoopUntilTimer(500000);
	}




	TEST_FUNCTION(test2)
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
	}

};

TEST_INSTANTIATE(TST_QTPlot);