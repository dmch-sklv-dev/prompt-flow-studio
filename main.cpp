#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("PromptFlowStudio");
    app.setApplicationDisplayName("PromptFlow Studio");
    app.setOrganizationName("University");

    // Modern clean fusion style
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow window;
    window.show();

    return app.exec();
}
