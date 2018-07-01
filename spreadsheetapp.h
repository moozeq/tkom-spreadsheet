#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTableView>
#include <QMainWindow>
#include <QModelIndex>
#include <QLineEdit>
#include <QLabel>
#include <QAction>
#include <QString>
#include <QMessageBox>
#include <QHeaderView>
#include "ui_mainwindow.h"
#include "spreadsheet.h"
#include "delegate.h"
#include "macro.h"
#include "parser.h"

namespace Ui {
class MainWindow;
}

class SpreadsheetApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpreadsheetApp(int rows, int cols, QWidget *parent = 0);
    ~SpreadsheetApp();

signals:
    void parsingCompleted();

private slots:
    void test();
    void showTestDialog(bool passed[]);
    void newSpreadsheet();
    void formulaEdited();
    void showCellInfo(const QModelIndex& index);
    bool parseMacro();

private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    Macro *macro;
    Spreadsheet *spreadsheet;
    Delegate *delegate;
    QLineEdit *cellFormula;
    QLabel *cellLabel;

    int rows;
    int cols;
};
#endif // MAINWINDOW_H
