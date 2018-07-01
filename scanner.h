#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#define MAXLEN 15 //max length of ID or digits in INT

using namespace std;

//extern enum ParserErrors;

enum ScanErrors { //erros which are raised by scanner
    WRONGINT, INTCONST2BIG, WRONGCHAR, ID2LONG, PARSERFIRSTERROR = 4
};

enum SymType //available symbols in macro
{
    ints, ifs, elses, breaks, whiles, NKEYS, MAXKEY = whiles,
    idents, intconsts, errors, eofs,

    STARTCHARS = eofs,
    lsqbrackets, rsqbrackets, lbrackets, rbrackets, lbrbrackets, rbrbrackets,
    semicolons, commas, assigns, nots,
    pluss, minuss, mults, divs, mods,
    ors, ands, equals, notequals, lesss, mores, lessoreqs, moreoreqs,
    STOPCHARS = moreoreqs,

    MAXSYM
};

struct TextPos  //position in macro
{
    int line;   //which line in macro
    int token;  //which token in line
    int exact;  //which exact position in line (including whitespace)
};

class Scanner
{
    struct KeyRec {
        string word;    //symbol special word
        SymType token;  //symbol enum type
    };

    stringstream inFile;    //macro as stringstream
    stringstream outFile;   //report from parsing macro
    int errorsTotal;

    char c;                 //last scanned char
    int intConst;           //last scanned const integer
    int len;                //length of last scanned identifier
    string identifier;      //last scanned identifier
    TextPos position;       //current position in macro
    TextPos prevPosition;   //previous position in macro
    static KeyRec KeywordTable[NKEYS];      //table with all special words (e.g. WHILE, INT)
    std::vector<unsigned> loopsFilePos;     //vector w/ exact positions of WHILE <condition> in macro, need for BREAK
    std::vector<TextPos> loopsPos;          //vector w/ positions of WHILE <condition> to restore previous, need for errors

    void nextChar(); //get next char
    static unsigned hash(string str, unsigned len); //hash for special words recognizing

public:
    stringstream& getInFile() {return inFile;}      //get macro code
    Scanner(stringstream& macro);
    SymType scanError(int errorCode, string errorValue);    //errors written to inFile report
    stringstream& getEndOfReport();                         //end of report in case of error

    SymType nextSymbol();                   //get next symbol
    int intConstSym() { return intConst; }  //get last const integer value
    string idSym() { return identifier; }   //get last identifier
    void savePosition();                    //save current position in macro
    void deleteLastPosition();              //delete last saved position in macro
    void rewind();                          //rewind to last saved position in macro
    std::streamoff getCurrentPosition() { return inFile.tellg(); } //get current position in macro
};


#endif // SCANNER_H
