#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QVector>

class Spreadsheet;

class Item
{
public:
    Item(const QString initFormula = "");
    void setItemFormula(const QString &newFormula, Spreadsheet *spreadsheet);           //set formula and reevaluate if needed
    int getValue() const {return value;}
    QString getFormula() const {return formula;}
    bool checkIfString() const {return isString;}                                       //check if its string
    bool checkIfShowing() const {return isShowing;}                                     //check if the cell's showing
    void setNotVisible() {isShowing = false;}                                           //set cell to not be visible when error
    bool evaluate(Spreadsheet *spreadsheet);                                            //evaluate value based on formula
    void setItemValue(const int newValue, Spreadsheet *spreadsheet);                    //set formula and value to int, if cell was w/ formula need to clear dependencies
    void addDepCell(QString cellDep);                                                   //add dependency, which cell should be informed if this cell changed
    void delDepCell(QString cellDep);                                                   //delete dependency
    void clearDepCells() {depCells.clear();}                                            //delete all dependencies
    bool checkIfCellDep(QString cell) const {return depCells.indexOf(cell) >= 0;}       //check if cell w/ id = 'row;col' should be informed by this cell
    int checkIfCellDep(QVector<QString> &setOfCells);                                   //check if at least one cell from vector is in dependencies
    QVector<QString>& getDepCells() {return depCells;}                                  //get dependencies
private:
    QString formula;                //formula which can be evaluated in cell - can be also string or just integer
    QVector<QString> depCells;      //cells which should be informed when this cell is changed
    int value;                      //value evaluated from formula or get from macro
    bool isString;
    bool isShowing;
};

#endif // ITEM_H
