#include <QApplication>
#include "MainFrame.h"

int main(int argc, char* argv[])
{
    try {
        QApplication app(argc, argv);
        MainFrame window;
        window.show();
        return app.exec();
    }
    catch (const std::exception& e) {
        qCritical() << "Exception caught:" << e.what();
    }
    catch (...) {
        qCritical() << "Unknown exception during QApplication init!";
    }
}