#ifndef MACRO_H
#define MACRO_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <sstream>

namespace Ui {
class Macro;
}

class Macro : public QDialog
{
    Q_OBJECT

public:
    explicit Macro(QWidget *parent = 0);
    std::stringstream& getMacroText();              //get full macro as stringstream
    void appendReport(std::stringstream& report);   //append report from parsing macro to proper window
    ~Macro();
signals:
    void parse();                                   //signal fired when Ok is clicked
public slots:
    void macrosInfo();                              //info about macro language
private slots:
    void openMacro();                               //open macro from file
    void saveMacro();                               //save macro to file
private:
    std::stringstream macro;                        //full macro as stringstream
    Ui::Macro *ui;
};

#endif // MACRO_H
