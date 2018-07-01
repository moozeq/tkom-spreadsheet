#include "macro.h"
#include "ui_macro.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QPushButton>
#include <string>

Macro::Macro(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Macro)
{
    ui->setupUi(this);

    QPushButton *okButton = new QPushButton(tr("&Ok"));
    QPushButton *saveButton = new QPushButton(tr("&Save"));
    QPushButton *openButton = new QPushButton(tr("&Open"));

    okButton->setDefault(true);
    ui->buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    ui->buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    ui->buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);

    connect(openButton, SIGNAL(clicked()), this, SLOT(openMacro()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveMacro()));
    connect(okButton, SIGNAL(clicked()), this, SIGNAL(parse()));
    connect(ui->buttonBox, SIGNAL(helpRequested()), this, SLOT(macrosInfo()));
}

std::stringstream& Macro::getMacroText()
{
    macro << ui->textEdit->toPlainText().toStdString();
    return macro;
}

void Macro::appendReport(std::stringstream& report) {
    QString rep = QString::fromStdString(report.str());
    ui->errorsTextEdit->append(rep);
}

Macro::~Macro()
{
    delete ui;
}

void Macro::macrosInfo()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText("Macro language:\n"
                    "\n\tINT <identifier> = <expression>\n"
                    "\n\tIF (<condition>)\n\t\t{<body>}\n\tELSE\n\t\t{<body>}\n"
                    "\n\tWHILE (<condition>)\n\t\t{<body>}\n"
                    "\nAssignment to expression in variable declaration be skipped"
                    "\nELSE after IF condition body is optional"
                    "\nWHILE loop can be skipped by BREAK"
                    "\n\nEach line of code should be terminated by semicols ';'"
                    "\nEach block of code should be started by '{' and end by '}'"
                    "\nBetween blocks of code internal variables are not visible"
                    "\n\nCells in spreadsheet are available by: [<row>, <col>]"
                    "\nThere are available for write/read value from/to them"
                    "\nThere is available simple sum option by formula in cell:\n"
                    "\n\t'=SUM( [<row>, <col>] - [<row>, <col>] )'");
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle("Macros information");
    msgBox->setModal(false);
    msgBox->show();
}


void Macro::openMacro()
{
    QString macroFilename = QFileDialog::getOpenFileName(this);
    QFile file(macroFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    ui->textEdit->clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        ui->textEdit->append(line);
    }
}

void Macro::saveMacro()
{
    QString filename = QFileDialog::getSaveFileName(this);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
}
