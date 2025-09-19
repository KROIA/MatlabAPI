#include "ui/MatlabEmbeddedPlotWidget.h"
#include "MatlabEngine.h"
#include "MatlabArray.h"
#include <QVBoxLayout>
#include <QThread>
#include <QWindow>

namespace MatlabAPI
{
    MatlabEmbeddedPlotWidget::MatlabEmbeddedPlotWidget(const QString& figureName, QWidget* parent)
        : QWidget(parent) 
		, m_figureName(figureName)
    {
        if(!MatlabEngine::isInstantiated())
			MatlabEngine::instantiate();
        m_embeddedWidget = nullptr;
        m_figureHandle = 0;

        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
    }

    MatlabEmbeddedPlotWidget::~MatlabEmbeddedPlotWidget() 
    {
        closeMatlabFigure();
    }

    bool MatlabEmbeddedPlotWidget::createMatlabFigure()
    {
        // Create a MATLAB figure and make it visible
        MatlabEngine::eval(("fig = figure('Name', '"+ m_figureName.toStdString()+"', 'NumberTitle', 'off');").c_str());
        MatlabEngine::eval("set(fig, 'MenuBar', 'none', 'ToolBar', 'figure');");

        // Get figure handle
        MatlabArray* figHandle = MatlabEngine::getVariable("fig");
        if (!figHandle) return false;

        m_figureHandle = (int)*figHandle->getPr();
		MatlabEngine::removeVariable("fig");
   

        // Small delay to let MATLAB create the window
        //QThread::msleep(500);

        // Get the window handle and embed it
        return embedMatlabWindow();
    }

    void MatlabEmbeddedPlotWidget::plotData(
        const std::vector<double>& x, 
        const std::vector<double>& y,
        const QString& title, 
        const QString& xlabel, 
        const QString& ylabel) 
    {

        if (m_figureHandle == 0) 
            return;

        // Set current figure
        QString cmd = QString("figure(%1);").arg(m_figureHandle);
        MatlabEngine::eval(cmd.toStdString().c_str());

        // Send data to MATLAB
        sendVectorToMatlab("x_data", x);
        sendVectorToMatlab("y_data", y);

        // Create plot
        MatlabEngine::eval("plot(x_data, y_data);");
        MatlabEngine::eval("grid on;");

        if (!title.isEmpty()) {
            QString titleCmd = QString("title('%1');").arg(title);
            MatlabEngine::eval(titleCmd.toStdString().c_str());
        }
        if (!xlabel.isEmpty()) {
            QString xlabelCmd = QString("xlabel('%1');").arg(xlabel);
            MatlabEngine::eval(xlabelCmd.toStdString().c_str());
        }
        if (!ylabel.isEmpty()) {
            QString ylabelCmd = QString("ylabel('%1');").arg(ylabel);
            MatlabEngine::eval(ylabelCmd.toStdString().c_str());
        }
    }

    void MatlabEmbeddedPlotWidget::plot3D(
        const std::vector<double>& x, 
        const std::vector<double>& y, 
        const std::vector<double>& z) 
    {
        if (m_figureHandle == 0) 
            return;

        QString cmd = QString("figure(%1);").arg(m_figureHandle);
        MatlabEngine::eval(cmd.toStdString().c_str());

        sendVectorToMatlab("x_data", x);
        sendVectorToMatlab("y_data", y);
        sendVectorToMatlab("z_data", z);

        MatlabEngine::eval("plot3(x_data, y_data, z_data); grid on; rotate3d on;");
    }

    void MatlabEmbeddedPlotWidget::surf(
        const QString& expression, 
        double xmin, double xmax,
        double ymin, double ymax, 
        int gridSize) 
    {
        if (m_figureHandle == 0) return;

        QString cmd = QString("figure(%1);").arg(m_figureHandle);
        MatlabEngine::eval(cmd.toStdString().c_str());

        // Create meshgrid and surface
        QString meshCmd = QString("[X, Y] = meshgrid(linspace(%1, %2, %3), linspace(%4, %5, %3));")
            .arg(xmin).arg(xmax).arg(gridSize).arg(ymin).arg(ymax);
        MatlabEngine::eval(meshCmd.toStdString().c_str());

        QString surfCmd = QString("Z = %1; surf(X, Y, Z); shading interp;").arg(expression);
        MatlabEngine::eval(surfCmd.toStdString().c_str());

        MatlabEngine::eval("colorbar; rotate3d on;");
    }

    void MatlabEmbeddedPlotWidget::sendVectorToMatlab(const QString& varName, const std::vector<double>& data) 
    {
		MatlabArray* array = new MatlabArray(varName.toStdString(), data);
		MatlabEngine::addVariable(array);
    }

    bool MatlabEmbeddedPlotWidget::embedMatlabWindow()
    {
#ifdef _WIN32
        return embedWindowsHandle();
#elif defined(__linux__)
        return embedX11Window();
#elif defined(__APPLE__)
        return embedMacWindow();
#else
        qDebug() << "Platform not supported for window embedding";
        return false;
#endif
    }

#ifdef _WIN32
    bool MatlabEmbeddedPlotWidget::embedWindowsHandle()
    {
        // Get MATLAB figure window handle
        QString cmd = QString("hwnd = get(%1, 'Number'); disp(hwnd);").arg(m_figureHandle);
        MatlabEngine::eval(cmd.toStdString().c_str());

        // Get all figure windows and find ours
        m_matlabHwnd = getMatlabFigureHwnd();
        //EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));

        if (!m_matlabHwnd) {
            qDebug() << "Could not find MATLAB figure window";
            return false;
        }

        // Create QWindow from HWND and embed it
        QWindow* m_matlabWindow = QWindow::fromWinId(reinterpret_cast<WId>(m_matlabHwnd));
        if (!m_matlabWindow) {
            qDebug() << "Failed to create QWindow from MATLAB HWND";
            return false;
        }

        m_embeddedWidget = QWidget::createWindowContainer(m_matlabWindow, this);
        m_embeddedWidget->setMinimumSize(400, 300);

        // Remove the embedded widget from any existing layout first
        if (m_layout->count() > 0) {
            QLayoutItem* item = m_layout->takeAt(0);
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }

        m_layout->addWidget(m_embeddedWidget);

        // Modify window properties
        SetParent(m_matlabHwnd, reinterpret_cast<HWND>(m_embeddedWidget->winId()));
        SetWindowLong(m_matlabHwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);

        return true;
    }

    HWND MatlabEmbeddedPlotWidget::getMatlabFigureHwnd() {
        if (m_figureHandle == 0) 
            return nullptr;

        // Get the figure's JavaFrame (this is the key!)
        MatlabEngine::eval("drawnow;"); // Ensure figure is drawn

        QString cmd = QString(
            "javaFrame = get(%1, 'JavaFrame'); "
            "if ~isempty(javaFrame), "
            "  hwndVal = javaFrame.fHG2Client.getWindow.getHWnd; "
            "else, "
            "  hwndVal = 0; "
            "end"
        ).arg(m_figureHandle);

        MatlabEngine::eval(cmd.toStdString().c_str());

        // Get the HWND value
        MatlabArray* hwndArray = MatlabEngine::getVariable("hwndVal");
        if (!hwndArray || !hwndArray->isDouble()) {
            if (hwndArray)
                MatlabEngine::removeVariable(hwndArray->getName());
            //return nullptr;
        }
        else
        {
            double hwndValue = *hwndArray->getPr();
            MatlabEngine::removeVariable(hwndArray->getName());

            HWND windowHandle = reinterpret_cast<HWND>(static_cast<uintptr_t>(hwndValue));
            if (windowHandle)
                return windowHandle;
        }

        

        // For newer MATLAB versions, try different approach
        cmd = QString(
            "try, "
            "  fig = %1; "
            "  drawnow; "
            "  warning('off', 'MATLAB:HandleGraphics:ObsoletedProperty:JavaFrame'); "
            "  jFrame = get(fig, 'JavaFrame'); "
            "  jClient = jFrame.fHG2Client; "
            "  hwndVal = jClient.getWindow.getHWnd; "
            "catch, "
            "  hwndVal = 0; "
            "end"
        ).arg(m_figureHandle);

        MatlabEngine::eval(cmd.toStdString().c_str());

        hwndArray = MatlabEngine::getVariable("hwndVal");
        if (!hwndArray || !hwndArray->isDouble()) {
            if (hwndArray)
                MatlabEngine::removeVariable(hwndArray->getName());
            //return nullptr;
        }
        else
        {
            double hwndValue = *hwndArray->getPr();
            MatlabEngine::removeVariable(hwndArray->getName());

            HWND windowHandle = reinterpret_cast<HWND>(static_cast<uintptr_t>(hwndValue));
            if (windowHandle)
                return windowHandle;
        }

        // Method 1: Search by exact title
        HWND windowHandle = FindWindowA(nullptr, m_figureName.toStdString().c_str());
        return windowHandle;
    }

    /*BOOL CALLBACK MatlabEmbeddedPlotWidget::EnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        MatlabEmbeddedPlotWidget* self = reinterpret_cast<MatlabEmbeddedPlotWidget*>(lParam);

        char className[256];
        GetClassName(hwnd, className, sizeof(className));

        // MATLAB figure windows have this class name
        if (strcmp(className, "MATLABMainFrame") == 0 ||
            strcmp(className, "HG_FigureFrame") == 0) {

            char windowTitle[256];
            GetWindowText(hwnd, windowTitle, sizeof(windowTitle));

            // Check if this is our figure
            if (strstr(windowTitle, m_figureName.) != nullptr) {
                self->m_matlabHwnd = hwnd;
                return FALSE; // Stop enumeration
            }
        }
        return TRUE; // Continue enumeration
    }*/

#elif defined(__linux__)
    bool MatlabEmbeddedPlotWidget::embedX11Window() 
    {
        // Get X11 window ID from MATLAB
        engEvalString(matlabEngine, "pause(0.5);"); // Wait for window creation

        // This is more complex on Linux and requires additional X11 programming
        // You would need to find the X11 Window ID of the MATLAB figure

        qDebug() << "Linux X11 embedding not fully implemented in this example";
        return false;
    }

#elif defined(__APPLE__)
    bool MatlabEmbeddedPlotWidget::embedMacWindow()
    {
        // macOS implementation would use Cocoa APIs
        // This requires Objective-C++ code to get NSView from MATLAB figure

        qDebug() << "macOS embedding not fully implemented in this example";
        return false;
    }
#endif

    void MatlabEmbeddedPlotWidget::closeMatlabFigure()
    {
        if (m_figureHandle != 0) {
            QString cmd = QString("close(%1);").arg(m_figureHandle);
            MatlabEngine::eval(cmd.toStdString().c_str());
            m_figureHandle = 0;
        }
    }


    void MatlabEmbeddedPlotWidget::resizeEvent(QResizeEvent* event)
    {
        QWidget::resizeEvent(event);

#ifdef _WIN32
        if (m_matlabHwnd && m_embeddedWidget) {
            // Resize the embedded MATLAB window
            QRect rect = m_embeddedWidget->geometry();
            SetWindowPos(m_matlabHwnd, nullptr, 0, 0, rect.width(), rect.height(),
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
#endif
    }

    void MatlabEmbeddedPlotWidget::showEvent(QShowEvent* event)
    {
        QWidget::showEvent(event);

        // Create MATLAB figure when widget becomes visible
        if (m_figureHandle == 0) {
            QTimer::singleShot(100, this, &MatlabEmbeddedPlotWidget::createMatlabFigure);
        }
    }
}