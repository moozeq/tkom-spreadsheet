#include "spreadsheet.h"
#include <QDebug>
#include <QSet>
#include <QMessageBox>

Spreadsheet::Spreadsheet(int initRows = MAXCELLS, int initCols = MAXCELLS, QObject *parent = nullptr)
    : QAbstractTableModel(parent)
{
    initRows < MAXCELLS ? rows = initRows : rows = MAXCELLS;
    initCols < MAXCELLS ? cols = initCols : cols = MAXCELLS;
}

int Spreadsheet::rowCount(const QModelIndex&) const
{
    return rows;
}

int Spreadsheet::columnCount(const QModelIndex&) const
{
    return cols;
}

QVariant Spreadsheet::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.column() < 0 || index.row() > rowCount() || index.column() > columnCount())
        return 0;
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (!cellsData[index.row()][index.column()].checkIfShowing())
            return "";
        if (cellsData[index.row()][index.column()].checkIfString())
            return cellsData[index.row()][index.column()].getFormula();
        else
            return cellsData[index.row()][index.column()].getValue();
    case FormulaRole:
        return cellsData[index.row()][index.column()].getFormula();
    case ValueRole:
        return cellsData[index.row()][index.column()].getValue();
    }
    return QVariant();
}

bool Spreadsheet::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.row() < 0 || index.column() < 0 || index.row() > rowCount() || index.column() > columnCount())
        return false;
    switch (role) {
    case Qt::EditRole:
        setCurrentCell(index);
        cellsData[index.row()][index.column()].setItemFormula(value.toString(), this);
        emit dataChanged(QModelIndex(), QModelIndex());
        break;
    case ValueRole:
        cellsData[index.row()][index.column()].setItemValue(value.toInt(), this);
        break;
    case DepCellAddRole:
        cellsData[index.row()][index.column()].addDepCell(value.toString());
        break;
    case DepCellDelRole:
        cellsData[index.row()][index.column()].clearDepCells();
        break;
    }
    return true;
}

Qt::ItemFlags Spreadsheet::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

void Spreadsheet::setFormula(const QString &formula) //set formula in current selected cell
{
    cellsData[currentCell.row()][currentCell.column()].setItemFormula(formula, this);
}

void Spreadsheet::reevaluateAllCells() {
    for (int i = 0; i < rowCount(); ++i) {
        for (int j = 0; j < columnCount(); ++j) {
            if (!cellsData[i][j].getDepCells().isEmpty()) {
                setCurrentCell(index(i, j));
                reevaluateCells();
            }
        }
    }
}

void Spreadsheet::showCyclicDepDialog() {
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText("Cyclic dependencies, cannot reevaluate cells");
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle("Cyclic dependencies");
    msgBox->setModal(true);
    msgBox->show();
}

void Spreadsheet::reevaluateCells()
{
    QString currCellId = QString::number(currentCell.row()) + ';' + QString::number(currentCell.column());
    QVector<QString> checkedCells;
    checkedCells.push_back(currCellId);
    if (cellsData[currentCell.row()][currentCell.column()].checkIfCellDep(currCellId)) { //check if cyclic to itself
        cellsData[currentCell.row()][currentCell.column()].delDepCell(currCellId);
        cellsData[currentCell.row()][currentCell.column()].setNotVisible();
        showCyclicDepDialog();
        return;
    }
    QVector<QString> depCells = cellsData[currentCell.row()][currentCell.column()].getDepCells();
    for (QString cell: depCells) { //check who should be informed about changes
        QStringList rowCol = cell.split(';');
        int nextRow = rowCol[0].toInt();
        int nextCol = rowCol[1].toInt();
        setCurrentCell(index(nextRow, nextCol));
        cellsData[nextRow][nextCol].evaluate(this);
        reevaluateRecursive(checkedCells, nextRow, nextCol);
    }
}

void Spreadsheet::reevaluateRecursive(QVector<QString>& checkedCells, int row, int col) {
    QString currCellId = QString::number(row) + ';' + QString::number(col);
    if (checkedCells.indexOf(currCellId) < 0)
        checkedCells.push_back(currCellId);
    else
        return;
    int cyclicCell = cellsData[row][col].checkIfCellDep(checkedCells);
    if (cyclicCell >= 0) { //check for cyclic dependencies
        for (int i = 0; i < rowCount(); ++i) {
            for (int j = 0; j < columnCount(); ++j) {
                cellsData[i][j].delDepCell(checkedCells[cyclicCell]);
            }
        }
        QStringList rowCol = checkedCells[cyclicCell].split(';');
        int badCellRow = rowCol[0].toInt();
        int badCellCol = rowCol[1].toInt();
        cellsData[badCellRow][badCellCol].setNotVisible();
        showCyclicDepDialog();
        return;
    }
    for (QString cell: cellsData[row][col].getDepCells()) {
        QStringList rowCol = cell.split(';');
        int nextRow = rowCol[0].toInt();
        int nextCol = rowCol[1].toInt();
        setCurrentCell(index(nextRow, nextCol));
        cellsData[nextRow][nextCol].evaluate(this);
        reevaluateRecursive(checkedCells, nextRow, nextCol);
    }
}
