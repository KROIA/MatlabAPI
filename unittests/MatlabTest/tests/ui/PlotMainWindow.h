#pragma once
#include "Matlab.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>


using namespace Matlab;
class PlotMainWindow : public QMainWindow {
    Q_OBJECT

private:
    MatlabEmbeddedPlotWidget* plotWidget;
    QPushButton* plotButton;
    QPushButton* plot3DButton;
    QPushButton* surfButton;
    QPushButton* clearButton;

public:
    PlotMainWindow() {
        // Create central widget with layout
        QWidget* centralWidget = new QWidget();
        setCentralWidget(centralWidget);

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        // Create buttons
        QHBoxLayout* buttonLayout = new QHBoxLayout();

        plotButton = new QPushButton("2D Plot");
        plot3DButton = new QPushButton("3D Plot");
        surfButton = new QPushButton("Surface Plot");
        clearButton = new QPushButton("Clear");

        buttonLayout->addWidget(plotButton);
        buttonLayout->addWidget(plot3DButton);
        buttonLayout->addWidget(surfButton);
        buttonLayout->addWidget(clearButton);
        buttonLayout->addStretch();

        // Create plot widget
        plotWidget = new MatlabEmbeddedPlotWidget("Test Plot");
        plotWidget->setMinimumSize(600, 400);
 

        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(plotWidget, 1); // Give it stretch factor

        // Connect buttons
        connect(plotButton, &QPushButton::clicked, this, &PlotMainWindow::create2DPlot);
        connect(plot3DButton, &QPushButton::clicked, this, &PlotMainWindow::create3DPlot);
        connect(surfButton, &QPushButton::clicked, this, &PlotMainWindow::createSurfacePlot);
        connect(clearButton, &QPushButton::clicked, this, &PlotMainWindow::clearPlot);

        setWindowTitle("Embedded MATLAB Plot");
        resize(800, 600);
    }

private slots:
    void create2DPlot() {
        std::vector<double> x, y;
        for (int i = 0; i < 1000; i++) {
            double t = i * 0.01;
            x.push_back(t);
            y.push_back(sin(t) * exp(-t / 5) + 0.1 * sin(10 * t));
        }
        plotWidget->plotData(x, y, "Damped Oscillation", "Time (s)", "Amplitude");
    }

    void create3DPlot() {
        std::vector<double> x, y, z;
        for (int i = 0; i < 1000; i++) {
            double t = i * 0.02;
            x.push_back(sin(t));
            y.push_back(cos(t));
            z.push_back(t);
        }
        plotWidget->plot3D(x, y, z);
    }

    void createSurfacePlot() {
        plotWidget->surf("sin(sqrt(X.^2 + Y.^2))./sqrt(X.^2 + Y.^2)", -8, 8, -8, 8, 50);
    }

    void clearPlot() {
        // Clear the current plot
        // Note: You might want to implement this in the plot widget
    }
};