#pragma once
#include "Matlab_base.h"
#include <QWidget>
#include <QString>

#ifdef _WIN32
#include <windows.h>
//#include <QWinHost>
#elif defined(__linux__)
// Untested for Linux
#include <QX11Info>
#include <X11/Xlib.h>
#elif defined(__APPLE__)
// Untested for macOS
#include <QMacCocoaViewContainer>
#endif

class QVBoxLayout;

namespace Matlab
{
    class MATLAB_API MatlabEmbeddedPlotWidget : public QWidget {
        Q_OBJECT
    public:
        MatlabEmbeddedPlotWidget(const QString &figureName, QWidget* parent = nullptr);
        ~MatlabEmbeddedPlotWidget();

        bool createMatlabFigure();

        void plotData(
            const std::vector<double>& x, 
            const std::vector<double>& y,
            const QString& title = "", 
            const QString& xlabel = "", 
            const QString& ylabel = "");

        void plot3D(
            const std::vector<double>& x, 
            const std::vector<double>& y, 
            const std::vector<double>& z);

        void surf(
            const QString& expression, 
            double xmin = -5, double xmax = 5,
            double ymin = -5, double ymax = 5, 
            int gridSize = 50);

    private:

        void sendVectorToMatlab(const QString& varName, const std::vector<double>& data);
        bool embedMatlabWindow();

#ifdef _WIN32
        bool embedWindowsHandle();
        HWND getMatlabFigureHwnd();

        //static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

#elif defined(__linux__)
        bool embedX11Window();

#elif defined(__APPLE__)
        bool embedMacWindow();
#endif

        void closeMatlabFigure();

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;


    private:
        QWidget* m_embeddedWidget;
        QVBoxLayout* m_layout;
        int m_figureHandle;
		QString m_figureName;

#ifdef _WIN32
        HWND m_matlabHwnd;
#elif defined(__linux__)
        Window m_matlabWindow;
#elif defined(__APPLE__)
        void* m_matlabView;  // NSView*
#endif
    };
}