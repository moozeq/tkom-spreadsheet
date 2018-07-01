#include <QtWidgets/QApplication>
#include "spreadsheetapp.h"
#include "QDebug"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SpreadsheetApp mainWindow(30, 30);
    mainWindow.show();
    return app.exec();
}
