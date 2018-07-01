#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QAbstractTableModel>
#include <QTableView>
#include <QMainWindow>
#include <QString>
#include "item.h"

const int MAXCELLS = 64;

enum Roles {FormulaRole = Qt::UserRole, ValueRole, DepCellAddRole, DepCellDelRole};

class Spreadsheet : public QAbstractTableModel
{
    Q_OBJECT
public:
    Spreadsheet(int initRows, int initCols, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void setCurrentCell(const QModelIndex &index) {currentCell = index;}    //saved currently used cell
    const QModelIndex& getCurrentCell() {return currentCell;}               //get currently used cell
    void setFormula(const QString &formula);    //set formula to currently used cell
    void reevaluateCells();                     //reevaluate cells after change
    void reevaluateAllCells();                  //reeavaluate all cells after macro parsed
    void reevaluateRecursive(QVector<QString> &checkedCells, int row, int col); //recursivly reevaluate all dependent cells
signals:
    void editCompleted(const QString &);    //signal emited when edition of formula in cell completed
private:
    void showCyclicDepDialog();
    Item cellsData[MAXCELLS][MAXCELLS];     //all cells data
    QModelIndex currentCell;                //currently used cell index
    int rows;
    int cols;
};

#endif // SPREADSHEET_H
