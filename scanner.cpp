#include "scanner.h"
#include <QDebug>
#include <QTextEdit>
#include <QTime>

Scanner::KeyRec Scanner::KeywordTable[NKEYS] = {
{"INT", ints},
{"IF", ifs},
{"ELSE", elses},
{"BREAK", breaks},
{"WHILE", whiles}
};

Scanner::Scanner(stringstream& macro) {
    inFile << macro.rdbuf();
    outFile << QTime::currentTime().toString("hh:mm:ss").toStdString()
            << " -------------------- Start of report --------------------" << endl;
    errorsTotal = 0;
    len = 0;
    position.token = position.line = position.exact = 0;
    prevPosition.token = prevPosition.line = prevPosition.exact = 0;
    nextChar();
    prevPosition.token = position.token;
    prevPosition.line = position.line;
    prevPosition.exact = position.exact - 1;
}

stringstream& Scanner::getEndOfReport() {
    outFile << endl << "Errors detected: " << errorsTotal << endl;
    return outFile;
}

void Scanner::nextChar() {
    c = inFile.get();
    prevPosition = position;
    ++position.exact;
    if (c == '\n') {
        prevPosition = position;
        ++position.line;
        position.exact = position.token = 0;
    }
    //qDebug() << position.line << position.token << position.exact;
}

unsigned Scanner::hash(string str, unsigned len) {
    unsigned h = 0;
    for (unsigned i = 0; i < len; ++i)
        h += str[i] + str[0];
    h /= len;
    h %= 3 * NKEYS;
    switch (h) { //mapping
    case 1: return 0;
    case 9: return 1;
    case 8: return 2;
    case 2: return 3;
    case 12: return 4;
    }
    return h;
}

SymType Scanner::nextSymbol() {
    string errorValue;
    prevPosition = position;
    ++position.token;
    do {
        while (isspace(c))
            nextChar();
        if (c == EOF)
            return eofs;
    } while (isspace(c));

    len = 1;
    if (isalpha(c)) { //ID or keyword
        unsigned h;
        bool tooLongId = false;
        identifier = "";
        do {
            if (len > MAXLEN) {
                tooLongId = true;
                errorValue.push_back(c);
            }
            identifier.push_back(c);
            ++len;
            nextChar();
        } while (isalnum(c));

        if (tooLongId) //ID is too long but need to eat it all
            return scanError(ID2LONG, errorValue);

        identifier[len - 1] = '\0';
        h = hash(identifier, len - 1);
        if (h >= NKEYS) //not in table = not a keyword
            return idents;
        if (strcmp(KeywordTable[h].word.c_str(), identifier.c_str()) == 0)
            return KeywordTable[h].token;
        else
            return idents;
    }
    else if (isdigit(c)) { //const int
        long long int l = 0;
        bool big = false;
        do {
            l = l * 10 + (c - '0');
            ++len;
            if (len > MAXLEN || l > INT_MAX) { //overflow by length of digits or > INT_MAX
                big = true;
                errorValue.push_back(c);
            }
            nextChar();
        } while (isdigit(c));
        if (isalpha(c)) { //e.g. 120329dssd is wrong
            while (isalpha(c)) {
                errorValue.push_back(c);
                nextChar();
            }
            return scanError(WRONGINT, errorValue);
        }
        if (big)
            return scanError(INTCONST2BIG, errorValue);
        intConst = (int)l;
        return intconsts;
    }
    else {
        switch (c) {
        case '[': nextChar(); return lsqbrackets;
        case ']': nextChar(); return rsqbrackets;
        case '(': nextChar(); return lbrackets;
        case ')': nextChar(); return rbrackets;
        case '{': nextChar(); return lbrbrackets;
        case '}': nextChar(); return rbrbrackets;
        case ';': nextChar(); return semicolons;
        case ',': nextChar(); return commas;
        case '=': nextChar();
            if (c == '=') {
                nextChar();
                return equals;
            }
            return assigns;
        case '!': nextChar();
            if (c == '=') {
                nextChar();
                return notequals;
            }
            else
                return nots;
        case '+': nextChar(); return pluss;
        case '-': nextChar(); return minuss;
        case '*': nextChar(); return mults;
        case '/': nextChar(); return divs;
        case '%': nextChar(); return mods;
        case '|': nextChar(); return ors;
        case '&': nextChar(); return ands;
        case '<': nextChar();
            if (c == '=') {
                nextChar();
                return lessoreqs;
            }
            else
                return lesss;
        case '>': nextChar();
            if (c == '=') {
                nextChar();
                return moreoreqs;
            }
            else
                return mores;
        default:
            errorValue.push_back(c);
            nextChar();
            return scanError(WRONGCHAR, errorValue);
        }
    }
}

void Scanner::savePosition() {
    unsigned unsignedPosition = inFile.tellg();
    unsignedPosition--; //save position 1 char before
    loopsFilePos.push_back(unsignedPosition);
    loopsPos.push_back(position);
}

void Scanner::rewind() {
    inFile.seekg(loopsFilePos.back(), ios_base::beg);
    position = loopsPos.back();
}

void Scanner::deleteLastPosition() {
    loopsFilePos.pop_back();
    loopsPos.pop_back();
}

SymType Scanner::scanError(int errorCode, string errorPrompt) {
    string errorPrompts[] =
    {
        "wrong const integer",		//errorCode = 0 LEXER
        "integer too big",			//errorCode = 1
        "unrecognized char",		//errorCode = 2
        "id too long",				//errorCode = 3

        "identifier not unique",	//errorCode = 4 PARSER
        "variable not defined",		//errorCode = 5
    };
    int line = prevPosition.line;
    int token = prevPosition.token;
    int exact = prevPosition.exact - len; //because of '/r', exact spot right after symbol

    if (line == 0) //in first line (0,0) is (0,1) because of first ++atom
        --token;

    if (errorCode < 0)
        outFile << "\t\t" << errorPrompt;
    else {
        outFile << "\t\t" << errorPrompts[errorCode];
        if (errorCode < PARSERFIRSTERROR)
            outFile << " '" << errorPrompt << "'";
        else
            outFile << " '" << identifier << "'";
    }

    outFile
        << "\n\t\t\tline: " << line + 1
        << "\n\t\t\ttoken: " << token + 1
        << "\n\t\t\texact spot: " << exact << endl;

    ++errorsTotal;
    return errors;
}
