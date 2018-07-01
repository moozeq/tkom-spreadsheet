#include "spreadsheetapp.h"

SpreadsheetApp::SpreadsheetApp(int rows, int cols, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tableView(new QTableView(this)),
    delegate(new Delegate()),
    cellFormula(new QLineEdit()),
    cellLabel(new QLabel()),
    macro(new Macro())
{
    this->rows = rows;
    this->cols = cols;
    ui->setupUi(this);
    cellLabel->setMinimumSize(80, 0);
    ui->toolBar->addWidget(cellLabel);
    ui->toolBar->addWidget(cellFormula);

    setCentralWidget(tableView);
    spreadsheet = new Spreadsheet(rows, cols, this);
    tableView->setModel(spreadsheet);
    tableView->setItemDelegate(delegate);
    setWindowTitle("Spreadsheet");

    connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(showCellInfo(QModelIndex))); //focus changed
    connect(tableView, SIGNAL(activated(QModelIndex)), this, SLOT(showCellInfo(QModelIndex))); //enter pressed when editing cell

    connect(cellFormula, SIGNAL(editingFinished()), this, SLOT(formulaEdited())); //formula edited

    connect(ui->actionMacro, SIGNAL(triggered()), macro, SLOT(show())); //show macro window
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newSpreadsheet())); //create new spreadsheet
    connect(ui->actionTest, SIGNAL(triggered(bool)), this, SLOT(test()));
    connect(macro, SIGNAL(parse()), this, SLOT(parseMacro())); //parse macro invoked
}

SpreadsheetApp::~SpreadsheetApp()
{
    delete ui;
    delete tableView;
    delete macro;
    delete spreadsheet;
    delete delegate;
    delete cellFormula;
    delete cellLabel;
}

void SpreadsheetApp::test()
{
    bool tests[4];
    int testNum = 0;
    spreadsheet->setData(spreadsheet->index(0, 0), 10);
    spreadsheet->setData(spreadsheet->index(0, 1), 20);
    spreadsheet->setData(spreadsheet->index(0, 2), "=[1,1] + [1,2]");
    tests[testNum++] = spreadsheet->data(spreadsheet->index(0, 2), ValueRole) == 30;

    spreadsheet->setData(spreadsheet->index(0, 3), "=[1,3]");
    tests[testNum++] = spreadsheet->data(spreadsheet->index(0, 3), ValueRole) == 30;

    spreadsheet->setData(spreadsheet->index(0, 1), 40);
    tests[testNum++] = spreadsheet->data(spreadsheet->index(0, 3), ValueRole) == 50 && spreadsheet->data(spreadsheet->index(0, 2), ValueRole) == 50;

    spreadsheet->setData(spreadsheet->index(0, 4), "=SUM([1,1] - [1,4])");
    tests[testNum++] = spreadsheet->data(spreadsheet->index(0, 4), ValueRole) == 150;

    showTestDialog(tests);
}

void SpreadsheetApp::showTestDialog(bool tests[])
{
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setIcon(QMessageBox::Information);
    QString prompt;
    for (int i = 0; i < 4; ++i) {
        prompt.append("Test " + QString::number(i + 1));
        tests[i] ? prompt.append(" passed\n") : prompt.append(" failed\n");
    }
    msgBox->setText(prompt);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle("Tests");
    msgBox->setModal(true);
    msgBox->show();
}

bool SpreadsheetApp::parseMacro()
{
    Scanner *scan = new Scanner(macro->getMacroText());
    Parser *parser = new Parser(*scan, spreadsheet, false, macro);
    spreadsheet->reevaluateAllCells();
    tableView->viewport()->update();
    delete scan;
    delete parser;
    return true;
}

void SpreadsheetApp::newSpreadsheet()
{
    delete spreadsheet;
    spreadsheet = new Spreadsheet(rows, cols, this);
    tableView->setModel(spreadsheet);
    cellLabel->setText("");
    cellFormula->setText("");
    connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(showCellInfo(QModelIndex))); //focus changed
    connect(tableView, SIGNAL(activated(QModelIndex)), this, SLOT(showCellInfo(QModelIndex))); //enter pressed when editing cell

    connect(cellFormula, SIGNAL(editingFinished()), this, SLOT(formulaEdited())); //formula edited

}

void SpreadsheetApp::showCellInfo(const QModelIndex &index)
{
    spreadsheet->setCurrentCell(index);
    QString label = "Cell [" + QString::number(index.row() + 1) + "," + QString::number(index.column() + 1) + "]";
    cellLabel->setText(label);
    cellFormula->setText(spreadsheet->data(index, FormulaRole).toString());
}

void SpreadsheetApp::formulaEdited()
{
    spreadsheet->setFormula(cellFormula->text());
    spreadsheet->reevaluateCells();
    tableView->viewport()->update();
}
