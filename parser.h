#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "macro.h"
#include "spreadsheet.h"
#include <set>
#include <unordered_map>
#include <vector>

enum ParserErrors { //errors raised by parser
    IDNOTUNIQUE = 4, IDUNDEFINED, LASTPARSERERROR
};

enum CodeErrors { //errors raised also by parser when executing
    DIVIDEBYZERO, OUTOFRANGE
};

class Synchronize
{
    friend class Parser;
    const set<SymType> &f;
    void synch(const set<SymType>& starterset, const set<SymType>& fset);

public:
    static Parser *p;
    Synchronize(const set<SymType>& starterset, const set<SymType>& fset);
    ~Synchronize();
    static void skipto(const set<SymType> &sset);
};

struct INTStuct {           //struct for INT variables
    string id;
    int value;
};

struct CONSTINTStruct {     //struct for CONST INT
    int value;
};

struct CELLStruct {         //struct for CELLS variables
    int row, col;
    int value;
};

class Parser
{
    Spreadsheet* spreadsheet;   //spreadsheet data model
    friend class Synchronize;
    Scanner& scanner;
    Macro* macro;               //currently executing macro
    SymType symbol;             //next symbol
    bool canParse;              //true if next symbol can be parsed
    bool isBreak;               //true when BREAK occured
    bool isIf;                  //true when IF body was executed so ELSE body wont be
    bool isExp;                 //true when macro parsing for cell formula which started with '='
    stringstream report;        //report from parsing macro
    int expValue;               //expression value used as value when cell has right formula
    std::vector<CELLStruct> cellsDep; //cells used in formula, after parsing will be informed to have this cell in their dependencies

    std::set<SymType> startBlock, comparisonOp, multiOp, addOp; //sets w/ symbol which can be accepted in certain situations
    std::unordered_map<string, INTStuct> intsMap;               //hash map for INT variables identifier

    void nexts();                       //go to next symbol
    void syntaxErrorExp(int atom);      //raised when expected symbol not found
    void syntaxErrorNExp(int atom);     //raised when unexpected symbol
    void accept(SymType atom);          //accept symbol from scanner
    void skipBlock();                   //skip whole block of code when condition false or BREAK

    void body(const set<SymType>&);
    void ifStatement(const set<SymType>&);
    void whileStatement(const set<SymType>&);
    void initStatement(const set<SymType>&);
    void assignStatement(const set<SymType>&);
    int expression(const set<SymType>&);
    int multiExp(const set<SymType>&);
    int primaryExp(const set<SymType>&);
    int parentExp(const set<SymType>&);
    int condition(const set<SymType>&);
    int andCond(const set<SymType>&);
    int comparisonCond(const set<SymType>&);
    int primaryCond(const set<SymType>&);
    CELLStruct cell(const set<SymType>&);

public:
    Parser(Scanner& sc, Spreadsheet* sprsh, bool exp = false, Macro* mac = nullptr);
    int getExpValue() const {return expValue;}
    std::vector<CELLStruct>& getCellsDep() {return cellsDep;}     //get cells which need to be inform about them used in formula
    bool parsed() const {return canParse;}                              //check if parsing successfully completetd
    void start(void);                                                   //start parsing macro
    void semanticError(int encode);                                     //raise semantic error
    void codeError(int encode);                                         //raise error in code while executing
    stringstream& getReport() { return report; }                        //get report about parsing as stringstream
};


#endif // PARSER_H
