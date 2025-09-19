#ifdef QT_ENABLED
#include <QApplication>
#endif
#include <iostream>
#include "Matlab.h"

#ifdef QT_WIDGETS_ENABLED
#include <QWidget>
#endif

int main(int argc, char* argv[])
{
#ifdef QT_WIDGETS_ENABLED
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#ifdef QT_ENABLED
	QApplication app(argc, argv);
#endif
	Matlab::Profiler::start();
	Matlab::LibraryInfo::printInfo();
#ifdef QT_WIDGETS_ENABLED
	QWidget* widget = Matlab::LibraryInfo::createInfoWidget();
	if (widget)
		widget->show();
#endif
	int ret = 0;
#ifdef QT_ENABLED
	ret = app.exec();
#endif
	Matlab::Profiler::stop((std::string(Matlab::LibraryInfo::name) + ".prof").c_str());
	return ret;
}