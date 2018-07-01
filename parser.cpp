#include "parser.h"
#include <QDebug>
#include <QTime>
#include <cmath>

string atomsText[MAXSYM] = {
    "INT", "IF", "ELSE", "BREAK", "WHILE",
    "id", "intconst", "error", "eofs",
    "[", "]", "(", ")", "{", "}",
    ";", ",", "=", "!",
    "+", "-", "*", "/", "%",
    "|", "&", "==", "!=", "<", ">", "<=", ">=",
};

Parser::Parser(Scanner& sc, Spreadsheet* sprsh, bool exp, Macro* mac) : scanner(sc), spreadsheet(sprsh), macro(mac), isExp(exp) {
    Synchronize::p = this;
    multiOp.insert({ mults, divs, mods });
    addOp.insert({ pluss, minuss });
    comparisonOp.insert({ equals, notequals, lesss, mores, lessoreqs, moreoreqs });
    startBlock.insert({ ifs, whiles, ints, idents, lsqbrackets, breaks });
    isBreak = isIf = false;
    if (isExp) {
        nexts();
        expValue = expression(set<SymType>({ semicolons, eofs }));
    }
    else
        start();
}

void Parser::nexts() {
    symbol = scanner.nextSymbol();
}

void Parser::syntaxErrorExp(int atom) {
    if (!canParse)
        return;
    scanner.scanError(-1, "Expected atom: '" + atomsText[atom] + "'");
}

void Parser::syntaxErrorNExp(int atom) {
    if (!canParse)
        return;
    scanner.scanError(-1, "Unexpected atom: '" + atomsText[atom] + "'");
}

void Parser::accept(SymType atom) {
    if (symbol == atom)
        nexts();
    else
        syntaxErrorExp(atom);
}

void Parser::start(void) {
    nexts();
    do {
        body(set<SymType>({ rbrbrackets, semicolons, eofs }));
        if (!canParse)
            break;
        accept(rbrbrackets);
        intsMap.clear();

    } while (symbol != eofs);
    report << scanner.getEndOfReport().rdbuf(); //copy report from scanner
    report << QTime::currentTime().toString("hh:mm:ss").toStdString()
           << " -------------------- End of report --------------------" << endl;
    macro->appendReport(report);
}

void Parser::body(const set<SymType>& fs) {
    accept(lbrbrackets);
    Synchronize s(startBlock, fs);
    while (symbol != rbrbrackets) {
        s.synch(startBlock, fs);
        if (!canParse || isBreak)
            return;
        switch (symbol) {
        case ifs:
            ifStatement(fs);
            if (isBreak || !canParse)
                return;
            accept(rbrbrackets);
            if (symbol == elses) {
                accept(elses);
                if (!isIf) //didnt execute if-block
                    body(fs);
                else
                    skipBlock();
                if (!canParse)
                    return;
                accept(rbrbrackets);
            }
            break;
        case whiles:
            whileStatement(fs);
            if (!canParse)
                return;
            break;
        case ints:
            initStatement(fs);
            if (!canParse)
                return;
            break;
        case idents:
        case lsqbrackets:
            assignStatement(fs);
            if (!canParse)
                return;
            break;
        case breaks:
            accept(breaks);
            accept(semicolons);
            isBreak = true;

            symbol = lbrackets;
            scanner.rewind();
            condition(set<SymType>({ rbrackets }));
            if (!canParse)
                return;
            skipBlock();
            return;
        }
    }

}

void Parser::ifStatement(const set<SymType>& fs) {
    accept(ifs);
    set<SymType> nfs(fs);
    nfs.insert(rbrackets);
    if (condition(nfs) > 0) {
        isIf = true;
        body(fs);
    }
    else {
        isIf = false;
        skipBlock();
    }
}

void Parser::skipBlock() { //skip block of code when condition == false
    int leftBracket = 1;
    int rightBracket = 0;
    accept(lbrbrackets);

    while (leftBracket > rightBracket && symbol != eofs) {
        Synchronize::skipto(set<SymType>({ lbrbrackets, rbrbrackets }));
        if (symbol == lbrbrackets) {
            accept(lbrbrackets);
            ++leftBracket;
        }
        else if (symbol == rbrbrackets) {
            if (rightBracket == leftBracket - 1) //last bracket must be left
                return;
            accept(rbrbrackets);
            ++rightBracket;
        }
        else
            return;
    }
}

void Parser::whileStatement(const set<SymType>& fs) {
    isBreak = false;
    accept(whiles);
    scanner.savePosition();
    while (condition(fs) > 0) {
        body(fs);
        if (!canParse)
            return;
        if (isBreak) {
            accept(rbrbrackets);
            break;
        }
        symbol = lbrackets;
        scanner.rewind();
    }
    scanner.deleteLastPosition();
    if (!isBreak) { //need to skip if not break
        skipBlock();
        accept(rbrbrackets);
    }
    isBreak = false;
}

void Parser::initStatement(const set<SymType>& fs) {
    INTStuct newVar;
    accept(ints);
    accept(idents);
    if (intsMap.find(scanner.idSym()) != intsMap.end()) { //found this id
        semanticError(IDNOTUNIQUE);
        canParse = false;
        return;
    }
    else {
        newVar.id = scanner.idSym();
        newVar.value = 0;
    }
    if (symbol == assigns) {
        accept(assigns);
        set<SymType> nfs(fs);
        nfs.insert(semicolons);
        newVar.value = expression(nfs);
        if (!canParse)
            return;
    }
    accept(semicolons);
    intsMap.insert(std::make_pair(newVar.id, newVar));
}

void Parser::assignStatement(const set<SymType>& fs) {
    if (symbol == idents) { //id1 = ...
        accept(idents);
        string indentId = scanner.idSym(); //save name of variable
        if (!(intsMap.find(indentId) != intsMap.end())) { //not found id
            semanticError(IDUNDEFINED);
            canParse = false;
            return;
        }
        else {
            accept(assigns);
            set<SymType> secnfs(fs);
            secnfs.insert(semicolons);
            intsMap.find(indentId)->second.value = expression(secnfs);
            if (!canParse)
                return;
        }
    }
    else { //cell = ...
        // here is interface to write to cell
        CELLStruct newCell;
        set<SymType> nfs(fs);
        nfs.insert(assigns);
        newCell = cell(nfs);
        if (!canParse)
            return;
        accept(assigns);
        set<SymType> secnfs(fs);
        secnfs.insert(semicolons);
        newCell.value = expression(secnfs);
        if (!canParse)
            return;
        if (newCell.row <= 0 || newCell.col <= 0 || newCell.row > spreadsheet->rowCount() || newCell.col > spreadsheet->columnCount()) {
            codeError(OUTOFRANGE);
            canParse = false;
            return;
        }
        QModelIndex ind = spreadsheet->index(newCell.row - 1, newCell.col - 1);
        spreadsheet->setCurrentCell(ind);
        spreadsheet->setData(ind, newCell.value, ValueRole);
    }
    accept(semicolons);
}

int Parser::expression(const set<SymType>& fs) {
    int returnValue = 0;
    set<SymType> nfs(addOp);
    nfs.insert(fs.begin(), fs.end());
    returnValue = multiExp(nfs);
    if (!canParse)
        return 0;
    while (addOp.find(symbol) != addOp.end()) {
        if (symbol == pluss) {
            accept(symbol);
            returnValue += multiExp(nfs);
        }
        else {
            accept(symbol);
            returnValue -= multiExp(nfs);
        }

    }
    return returnValue;
}

int Parser::multiExp(const set<SymType>& fs) {
    int returnValue = 0;
    int tempValue = 0;
    set<SymType> nfs(multiOp);
    nfs.insert(fs.begin(), fs.end());
    returnValue = primaryExp(nfs);
    if (!canParse)
        return 0;
    while (multiOp.find(symbol) != multiOp.end()) {
        switch (symbol) {
        case mults:
            accept(symbol);
            returnValue *= primaryExp(nfs);
            if (!canParse)
                return 0;
            break;
        case divs:
            accept(symbol);
            tempValue = primaryExp(nfs);
            if (!canParse)
                return 0;
            if (tempValue == 0) {
                codeError(DIVIDEBYZERO);
                canParse = false;
                return 0;
            }
            else
                returnValue /= tempValue;
            break;
        case mods:
            accept(symbol);
            returnValue %= primaryExp(nfs);
            if (!canParse)
                return 0;
            break;
        }
    }
    return returnValue;
}

int Parser::primaryExp(const set<SymType>& fs) {
    int returnValue = 0;
    bool isNot = false;
    if (symbol == nots) {
        isNot = true;
        accept(nots);
    }

    Synchronize s(set<SymType>({ lbrackets, idents, lsqbrackets, intconsts, minuss }), fs);
    if (!canParse)
        return 0;

    switch (symbol) {
    case lbrackets:
        returnValue = parentExp(fs);
        if (!canParse)
            return 0;
        break;
    case idents:
        accept(idents);
        if (!(intsMap.find(scanner.idSym()) != intsMap.end())) { //not found id
            if (isExp && scanner.idSym() == "SUM") { //can be SUM in cell
                CELLStruct cellFrom, cellTo;
                accept(lbrackets);
                cellFrom = cell(set<SymType>({minuss}));
                if (!canParse)
                    return 0;
                Synchronize::skipto(set<SymType>({minuss}));
                accept(minuss);
                cellTo = cell(set<SymType>({rbrackets}));
                if (!canParse)
                    return 0;
                accept(rbrackets);
                if (cellFrom.row == cellTo.row && cellFrom.col == cellTo.col) //'SUM([1,1] - [1,1])' same as '= [1,1]'
                    return cellFrom.value;

                if (cellFrom.row == cellTo.row) {
                    int minCol = min(cellFrom.col, cellTo.col);
                    int maxCol = max(cellFrom.col, cellTo.col) + 1;

                    for (int i = minCol; i < maxCol; ++i) {
                        CELLStruct newCell;
                        newCell.row = cellFrom.row;
                        newCell.col = i;
                        QModelIndex ind = spreadsheet->index(newCell.row - 1, newCell.col - 1);
                        newCell.value = ind.data(ValueRole).toInt();
                        returnValue += newCell.value;

                        if (i != minCol && i != maxCol) //if not 1st or last cell need to add dependencies
                            cellsDep.push_back(newCell);
                    }
                }
                else if (cellFrom.col == cellTo.col) {
                    int minRow = min(cellFrom.row, cellTo.row);
                    int maxRow = max(cellFrom.row, cellTo.row) + 1;

                    for (int i = minRow; i < maxRow; ++i) {
                        CELLStruct newCell;
                        newCell.row = i;
                        newCell.col = cellFrom.col;
                        QModelIndex ind = spreadsheet->index(newCell.row - 1, newCell.col - 1);
                        newCell.value = ind.data(ValueRole).toInt();
                        returnValue += newCell.value;

                        if (i != minRow && i != maxRow) //if not 1st or last cell need to add dependencies
                            cellsDep.push_back(newCell);
                    }
                }
                else {
                    semanticError(IDUNDEFINED);
                    canParse = false;
                }
            }
            else {
                semanticError(IDUNDEFINED);
                canParse = false;
            }
        }
        else
            returnValue = intsMap.find(scanner.idSym())->second.value;
        if (!canParse)
            return 0;
        break;
    case lsqbrackets:
        returnValue = cell(fs).value;
        if (!canParse)
            return 0;
        break;
    case intconsts:
        accept(intconsts);
        returnValue = scanner.intConstSym();
        if (!canParse)
            return 0;
        break;
    }

    if (isNot)
        return !returnValue;
    else
        return returnValue;
}

int Parser::parentExp(const set<SymType>& fs) {
    int returnValue = 0;
    accept(lbrackets);
    set<SymType> nfs(fs);
    nfs.insert(rbrackets);
    returnValue = expression(nfs);
    if (!canParse)
        return 0;
    accept(rbrackets);
    return returnValue;
}

int Parser::condition(const set<SymType>& fs) {
    bool right = false;
    int returnValue = 0;
    accept(lbrackets);
    set<SymType> nfs(fs);
    nfs.insert({ ors, rbrackets });
    returnValue = andCond(nfs);
    if (!canParse)
        return 0;
    if (returnValue > 0)
        returnValue = 1;
    while (symbol == ors) {
        accept(ors);
        if (comparisonCond(nfs) && !right) { //one cond is true
            right = true;
            returnValue = 1;
        }
    }
    accept(rbrackets);
    return returnValue;
}

int Parser::andCond(const set<SymType>& fs) {
    bool wrong = false;
    int returnValue = 0;
    set<SymType> nfs(fs);
    nfs.insert(ands);
    returnValue = comparisonCond(nfs);
    if (!canParse)
        return 0;
    while (symbol == ands) {
        accept(ands);
        if (!comparisonCond(nfs) && !wrong) { //one cond is wrong
            wrong = true;
            returnValue = 0;
        }
    }
    return returnValue;
}

int Parser::comparisonCond(const set<SymType>& fs) {
    int firstValue = 0;
    int secValue = 0;
    int currentSymbol;

    set<SymType> nfs(comparisonOp);
    nfs.insert(fs.begin(), fs.end());
    firstValue = primaryCond(nfs);
    if (!canParse)
        return 0;
    if (comparisonOp.find(symbol) != comparisonOp.end()) { //2 operators
        currentSymbol = symbol;
        accept(symbol);
        secValue = primaryCond(fs);
        if (!canParse)
            return 0;
        switch (currentSymbol) {
        case equals:
            return firstValue == secValue;
        case notequals:
            return firstValue != secValue;
        case lesss:
            return firstValue < secValue;
        case mores:
            return firstValue > secValue;
        case lessoreqs:
            return firstValue <= secValue;
        case moreoreqs:
            return firstValue >= secValue;
        }
    }
    if (firstValue > 0) //one operator
        return 1;
    else
        return 0;
}

int Parser::primaryCond(const set<SymType>& fs) {
    if (symbol == nots)
        accept(nots);
    return expression(fs);
}

CELLStruct Parser::cell(const set<SymType>& fs) {
    CELLStruct newCell;
    accept(lsqbrackets);
    set<SymType> nfs(fs);
    nfs.insert(commas);
    newCell.row = expression(nfs);
    if (!canParse)
        return newCell;
    accept(commas);
    set<SymType> secnfs(fs);
    secnfs.insert(rsqbrackets);
    newCell.col = expression(secnfs);
    if (!canParse)
        return newCell;
    accept(rsqbrackets);

    if (newCell.row > spreadsheet->rowCount() || newCell.col > spreadsheet->columnCount() || newCell.row <= 0 || newCell.col <= 0) {
        codeError(OUTOFRANGE);
        canParse = false;
        return newCell;
    }
    QModelIndex ind = spreadsheet->index(newCell.row - 1, newCell.col - 1);
    newCell.value = ind.data(ValueRole).toInt();
    //interface to spreadsheet which get value from cell
    cellsDep.push_back(newCell);
    return newCell;
}


void Parser::semanticError(int encode) {
    scanner.scanError(encode, "");
}

void Parser::codeError(int encode) {
    string errorPrompts[] =
    {
        "cannot divide by 0",		//errorCode = 0
        "index out of range"        //errorCode = 1
    };
    scanner.scanError(-1, errorPrompts[encode]);
}


Parser* Synchronize::p = 0;

void Synchronize::synch(const set<SymType>& starterset, const set<SymType>& fset) {
    if (!(starterset.find(p->symbol) != starterset.end())) {
        p->syntaxErrorNExp(p->symbol);
        set<SymType> newSet(starterset);
        newSet.insert(fset.begin(), fset.end());
        skipto(newSet);
    }
    p->canParse = (starterset.find(p->symbol) != starterset.end());
}

Synchronize::Synchronize(const set<SymType>& starterset, const set<SymType>& fset) : f(fset) {
    synch(starterset, fset);
}
Synchronize::~Synchronize() {
    if (!(f.find(p->symbol) != f.end())) {
        p->syntaxErrorNExp(p->symbol);
        skipto(f);
    }
}

void Synchronize::skipto(const set<SymType> &sset) {
    while (!(sset.find(p->symbol) != sset.end()))
        p->nexts();
}
