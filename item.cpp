#include "item.h"
#include "spreadsheetapp.h"
#include <QDebug>
#include "parser.h"
#include <sstream>

Item::Item(const QString initFormula)
{
    formula = initFormula;
    value = 0;
    isShowing = false;
}

int Item::checkIfCellDep(QVector<QString>& setOfCells) {
    for (int i = 0; i < setOfCells.size(); ++i) {
        if (checkIfCellDep(setOfCells[i]))
            return i;
    }
    return -1;
}

void Item::setItemValue(const int newValue, Spreadsheet *spreadsheet) {
    if (formula[0] == '=') { //it was formula so it needs to clear all previous dependencies
        QString cellId = QString::number(spreadsheet->getCurrentCell().row()) + ';' + QString::number(spreadsheet->getCurrentCell().column());
        for (int i = 0; i < spreadsheet->rowCount(); ++i) {
            for (int j = 0; j < spreadsheet->columnCount(); ++j) {
                QModelIndex ind = spreadsheet->index(i, j);
                spreadsheet->setData(ind, cellId, DepCellDelRole);
            }
        }
    }
    formula = QString::number(newValue);
    value = newValue;
    isString = false;
    isShowing = true;
}

void Item::setItemFormula(const QString &newFormula, Spreadsheet *spreadsheet)
{
    if (newFormula == "") {
        value = 0;
        isString = false;
        isShowing = false;
    }
    formula = newFormula;
    if (evaluate(spreadsheet)) //reset value
        isShowing = true;
    if (!depCells.isEmpty())
        spreadsheet->reevaluateCells();
}

void Item::delDepCell(QString cellDep) {
    if (depCells.indexOf(cellDep) >= 0)
        depCells.remove(depCells.indexOf(cellDep));
}

void Item::addDepCell(QString cellDep) {
    if (depCells.indexOf(cellDep) < 0)
        depCells.push_back(cellDep);
}

bool Item::evaluate(Spreadsheet *spreadsheet)
{
    if (formula[0] == '=') {
        stringstream mac;
        QString formulaEq(formula);
        formulaEq[0] = ' ';
        mac << formulaEq.toStdString() << ";";
        Scanner *scan = new Scanner(mac);
        Parser *parser = new Parser(*scan, spreadsheet, true);
        if (parser->parsed()) {
            QString currentCellId = QString::number(spreadsheet->getCurrentCell().row()) + ';' + QString::number(spreadsheet->getCurrentCell().column());
            value = parser->getExpValue();
            for (CELLStruct cell: parser->getCellsDep()) {
                QModelIndex ind = spreadsheet->index(cell.row - 1, cell.col - 1);
                spreadsheet->setData(ind, currentCellId, DepCellAddRole);
            }
            delete scan;
            delete parser;
            isString = false;
            return true;
        }
        else {
            delete scan;
            delete parser;
            isString = true;
            return true;
        }
    }
    else {
        bool flag;
        value = formula.toInt(&flag);
        isString = false;
        if (!flag)
            isString = true;
        return true;
    }
}
