#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Implement a Recursive Descent Parser and Intermediate Code Generator for tiny PL/0.  

#define MAX_SIZE 999999
#define MAX_LINE_LENGTH 1000

// Symbol table
typedef struct  
{ 
    int kind; // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val; // number (ASCII value) 
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} symbol;

// Assembly Code
typedef struct {
    char op[4];
    int l;
    int m;
} AssemblyCode;

// Enum for token values
typedef enum {
    skipsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,  
    multsym = 6,  slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11, 
    leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16, 
    commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,  
    beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, 
    callsym = 27, constsym = 28, varsym = 29, procsym = 30, writesym = 31,  
    readsym = 32, elsesym = 33
} tokenTypes; // talk to professor about 9 before callsym, procsym, and elsesym

// Enum for opcodes
typedef enum {
    LIT = 1, OPR = 2, LOD = 3, STO = 4, CAL = 5, INC = 6, JMP = 7, JPC = 8, SYS = 9
} opCodes;

// Enum for OPR
typedef enum {
    ADD = 1, SUB = 2, MUL = 3, DIV = 4, EQL = 5, NEQ = 6, LSS = 7, LEQ = 8, GTR = 9, GEQ = 10, ODD = 11, NEG = 12
} oprCodes;

// OP Table
char * op_code[] = { "", "LIT", "OPR", "LOD", "STO", "", "INC", "JMP", "JPC", "SYS"};

// Token
char token[MAX_SIZE][11];
int value[MAX_SIZE];
int tokenIndex = 0;

// Lexeme list
char LexemeList[MAX_SIZE];
int LexemeListIndex = 0;

// Symbol table
symbol SymbolTable[MAX_SIZE];
int SymbolTableIndex = 0;

// Assembly Code
AssemblyCode AssemblyCodeList[MAX_SIZE];
int AssemblyCodeListIndex = 0;

// Current token
char CurrentToken[15];
int CurrentTokenValue = 0;
int CurrentIndex = 0;

// Lexeme list functions
int findTokenValue(char * token);
int isNumber(char * token);
int isIdentifier(char * token);
int checkInvalidSymbols(char * token);
int isSymbol(char token);
void addToken(char * tokenize, int tokenValue);

//start parser program
void program();
// Assembly Code/ Symbol Table functions
int symbolTableCheck(char * name);
void program();
// creates assembly code
void emit(int op, int l, int m);
// get token function
void getToken();
//verify constant is properly declared
void constDeclaration();
//verify variable declaration is properly declared
int varDeclaration();
//  ident assignment, expression, begin statment end, if condition-then statement, while condition do statement, read ident, write expression
void statement();
// odd expression or (expression, rel-op, expression
void condition();
//[+ | -] term {(+|-)term}
void expression();
//will output all errors, checking for syntax error
void error(int err);
void term();
void factor();
void block();
void addSymbolTable(int kind, char * name, int val, int level, int addr, int mark);


int main(int argc, char *argv[]) {

    // Accept file name as command line argument
    char * file_input = argv[1];
    FILE *fp = fopen(file_input, "r"); // Change back to soft file from hard file

    // Check if file exists
    if (fp == NULL) {
        printf("Error opening file");
        exit(0);
    }

    char buffer[MAX_LINE_LENGTH];
    int isComment = 0;

    // Read file into tokenize array
    while(fgets(buffer, 1000, fp) != NULL) {
        // fprintf(stdout, "Current Line: %s\n", buffer);

        int bufferLength = strlen(buffer);
        int commentStartIndex = -1;
        int commentEndIndex = -1;

        if(strstr(buffer, "/*") != NULL || isComment == 1) {
            // Find where the comment starts
            if(isComment == 0) {
                commentStartIndex = 0;
                while(buffer[commentStartIndex] != '/' && buffer[commentStartIndex + 1] != '*') {
                    commentStartIndex++;
                }
            }

            isComment = 1;

            if(strstr(buffer, "*/") != NULL) {
                isComment = 0;

                // Find where the comment ends
                if(commentStartIndex == -1) {
                    commentEndIndex = 0;
                } else {
                    commentEndIndex = commentStartIndex + 2;
                }

                while(buffer[commentEndIndex] != '*' && buffer[commentEndIndex + 1] != '/') {
                    commentEndIndex++;
                }
            } 
        }

        if(isComment == 1) {
            continue;
        }

        // While buffer is not empty
        // Store buffer into tokenize array until a space is reached
        // Print tokenize array and get the token value using function
        int buffer_index = 0;
        if(commentEndIndex != -1 && (commentStartIndex == 0 || commentStartIndex == -1)) {
            buffer_index = commentEndIndex + 2;
        }

        while (buffer[buffer_index] != '\0' && buffer_index < bufferLength)
        {
            char tokenize[MAX_LINE_LENGTH] = {0};
            int tokenize_index = 0;
            int didWhileWork = 0;

            // Store buffer into tokenize array until a space is reached or a symbol is reached
            while(isSymbol(buffer[buffer_index]) == 0 && buffer[buffer_index] != ' ' && buffer[buffer_index] != '\0' && buffer[buffer_index] != '\n' && buffer[buffer_index] != '\t')
            {
                tokenize[tokenize_index] = buffer[buffer_index];
                tokenize_index++;
                buffer_index++;
                didWhileWork = 1;
            }

            // Print tokenize array and get the token value using function

            if(didWhileWork == 1) {
                tokenize[tokenize_index] = '\0';

                int tokenValue = findTokenValue(tokenize);

                if(tokenValue != 0) {
                    // printf("%-15s %-10d\n", tokenize,  tokenValue);

                    // Store token into token array
                    strcpy(token[tokenIndex], tokenize);
                    value[tokenIndex] = tokenValue;
                    tokenIndex++;

                    // Add to lexeme list
                    addToken(tokenize, tokenValue);
                }
            }

            // While buffer has a symbol, print the symbol and get the token value using function
            while(isSymbol(buffer[buffer_index]) == 1)
            {
                char symbol[3] = {0};
                symbol[0] = buffer[buffer_index];
                symbol[1] = '\0';

                if(symbol[0] == ':' && buffer[buffer_index + 1] == '=')
                {
                    symbol[1] = '=';
                    symbol[2] = '\0';
                    buffer_index++;
                } else if(symbol[0] == '<' && buffer[buffer_index + 1] == '=')
                {
                    symbol[1] = '=';
                    symbol[2] = '\0';
                    buffer_index++;
                } else if(symbol[0] == '>' && buffer[buffer_index + 1] == '=')
                {
                    symbol[1] = '=';
                    symbol[2] = '\0';
                    buffer_index++;
                } else if(symbol[0] == '!' && buffer[buffer_index + 1] == '=')
                {
                    symbol[1] = '=';
                    symbol[2] = '\0';
                    buffer_index++;
                } else if(symbol[0] == '<' && buffer[buffer_index + 1] == '>')
                {
                    symbol[1] = '>';
                    symbol[2] = '\0';
                    buffer_index++;
                }

                int tokenValue = findTokenValue(symbol);
                
                if(tokenValue != 0) {
                    // printf("%-15s %-10d\n", symbol,  tokenValue);

                    strcpy(token[tokenIndex], symbol);
                    value[tokenIndex] = tokenValue;
                    tokenIndex++;

                    // Add to lexeme list
                    addToken(symbol, tokenValue);
                }

                buffer_index++;
            }

            // Remove whitespace from buffer
            while (buffer[buffer_index] == ' ' || buffer[buffer_index] == '\t' || buffer[buffer_index] == '\n')
            {
                buffer_index++;
            }

            if(buffer_index >= commentStartIndex && commentEndIndex > buffer_index && commentEndIndex != 0) {
                buffer_index = commentEndIndex+2;
            }
        }

    }

    // Close file
    fclose(fp);

    LexemeList[LexemeListIndex-1] = '\0';

    emit(JMP, 0, 3);

    addSymbolTable(3, "main", 0, 0, 3, 1);

    program();

    printf("Assembly Code: \n");
    printf("%-4s %-4s %-4s %-4s\n", "LINE", "OP", "L", "M");
    for(int i = 0; i < AssemblyCodeListIndex; i++) {
        printf("%-4d %-4s %-4d %-4d\n", i, AssemblyCodeList[i].op, AssemblyCodeList[i].l, AssemblyCodeList[i].m);
    }

    printf("\n");

    printf("Symbol Table: \n");
    printf("%-4s | %-11s | %-5s | %-5s | %-7s | %-4s\n", "KIND", "NAME", "VALUE", "LEVEL", "ADDRESS", "MARK");
    printf("----------------------------------------------------\n");
    for(int i = 0; i < SymbolTableIndex; i++) {
        printf("%4d | %11s | %5d | %5d | %7d | %4d\n", SymbolTable[i].kind, SymbolTable[i].name, SymbolTable[i].val, SymbolTable[i].level, SymbolTable[i].addr, SymbolTable[i].mark);
    }

    return 0;
}

int findTokenValue(char * token) {

    // If token is a reserved word, return the token value
    // Else return 3 (numbersym) if token is a number and 2 (identsym) if token is an identifier
    // Emit error if token is not a reserved word, number more than 5 digits, or identifier more than 11 characters
    if(token[0] == 's' && token[1] == 'k' && token[2] == 'i' && token[3] == 'p' && token[4] == '\0') {return skipsym;}
    if(token[0] == '+' && token[1] == '\0') {return plussym;}
    if(token[0] == '-' && token[1] == '\0') {return minussym;}
    if(token[0] == '*' && token[1] == '\0') {return multsym;}
    if(token[0] == '/' && token[1] == '\0') {return slashsym;}
    if(token[0] == 'o' && token[1] == 'd' && token[2] == 'd' && token[3] == '\0') {return oddsym;}
    if(token[0] == '=' && token[1] == '\0') {return eqlsym;}
    if(token[0] == '<' && token[1] == '>' && token[2] == '\0') {return neqsym;}
    if(token[0] == '<' && token[1] == '\0') {return lessym;}
    if(token[0] == '<' && token[1] == '=' && token[2] == '\0') {return leqsym;}
    if(token[0] == '>' && token[1] == '\0') {return gtrsym;}
    if(token[0] == '>' && token[1] == '=' && token[2] == '\0') {return geqsym;}
    if(token[0] == '(' && token[1] == '\0') {return lparentsym;}
    if(token[0] == ')' && token[1] == '\0') {return rparentsym;}
    if(token[0] == ',' && token[1] == '\0') {return commasym;}
    if(token[0] == ';' && token[1] == '\0') {return semicolonsym;}
    if(token[0] == '.' && token[1] == '\0') {return periodsym;}
    if(token[0] == ':' && token[1] == '=' && token[2] == '\0') {return becomessym;}
    if(token[0] == 'b' && token[1] == 'e' && token[2] == 'g' && token[3] == 'i' && token[4] == 'n' && token[5] == '\0') {return beginsym;}
    if(token[0] == 'e' && token[1] == 'n' && token[2] == 'd' && token[3] == '\0') {return endsym;}
    if(token[0] == 'i' && token[1] == 'f' && token[2] == '\0') {return ifsym;}
    if(token[0] == 't' && token[1] == 'h' && token[2] == 'e' && token[3] == 'n' && token[4] == '\0') {return thensym;}
    if(token[0] == 'w' && token[1] == 'h' && token[2] == 'i' && token[3] == 'l' && token[4] == 'e' && token[5] == '\0') {return whilesym;}
    if(token[0] == 'd' && token[1] == 'o' && token[2] == '\0') {return dosym;}
    if(token[0] == 'c' && token[1] == 'a' && token[2] == 'l' && token[3] == 'l' && token[4] == '\0') {return callsym;}
    if(token[0] == 'c' && token[1] == 'o' && token[2] == 'n' && token[3] == 's' && token[4] == 't' && token[5] == '\0') {return constsym;}
    if(token[0] == 'v' && token[1] == 'a' && token[2] == 'r' && token[3] == '\0') {return varsym;}
    if(token[0] == 'p' && token[1] == 'r' && token[2] == 'o' && token[3] == 'c' && token[4] == 'e' && token[5] == 'd' && token[6] == 'u' && token[7] == 'r' && token[8] == 'e' && token[9] == '\0') {return procsym;}
    if(token[0] == 'w' && token[1] == 'r' && token[2] == 'i' && token[3] == 't' && token[4] == 'e' && token[5] == '\0') {return writesym;}
    if(token[0] == 'r' && token[1] == 'e' && token[2] == 'a' && token[3] == 'd' && token[4] == '\0') {return readsym;}
    if(token[0] == 'e' && token[1] == 'l' && token[2] == 's' && token[3] == 'e' && token[4] == '\0') {return elsesym;}
    if(isNumber(token) == 1) {return numbersym;}
    if(isIdentifier(token) == 1) {return identsym;}

    return 0;
}

// Function to check if the token is a number is under 5 digits long
// If a number is greater than 5 digits long, it is an error
int isNumber(char *token) {
    int i = 0;
    int length = strlen(token);

    // Check if the token is a number
    while(i < length) {
        if(token[i] < '0' || token[i] > '9') {
            return 0;
        }
        i++;
    }

    // Check if the length of the token is greater than 5
    if(length > 5) {
        // printf("Error: Number is greater than 5 digits long\n");
        return 0;
    }  

    // Return 1 if the token is a valid number
    return 1;
}

// Function to check if the token is a word and less than 11 characters long
// If a word is greater than 11 characters long, it is an error
int isIdentifier(char *token) {
    int i = 0;
    int length = strlen(token);

    // Check if the length of the token is greater than 11
    if(length > 11) {
        // printf("Error: Word is greater than 11 characters long\n");
        return 0;
    }

    // Check if the first character of the token is a letter
    if(token[0] > '0' && token[0] < '9') {
        // printf("Error: Identifier must start with a letter\n");
        return 0;
    }

    // Check if the token has any symbols, if it does, then return 0
    if(checkInvalidSymbols(token)) {
        return 0;
    }

    while(i < length) {
        // Check if the token is a letter
        if(token[i] < 'a' || token[i] > 'z') {
            return 0;
        }
        i++;
    }

    // Return 1 if the token is a valid identifier
    return 1;
}

// Function to check if the current token has any invalid symbols
int checkInvalidSymbols(char * token) {
    int found = 0;
    int length = strlen(token);

    // Check if the token has any invalid symbols
    for(int i = 0; i < length; i++) {
        if(token[i] == '`' || token[i] == '~' || token[i] == '!' || token[i] == '@' || 
        token[i] == '#' || token[i] == '$' || token[i] == '%' || token[i] == '^' || 
        token[i] == '&' || token[i] == '_' || token[i] == '{' || token[i] == '}' || 
        token[i] == '[' || token[i] == ']' || token[i] == '|' || token[i] == '\\' || 
        token[i] == '\'' || token[i] == '"' || token[i] == '?') {
            // If the token has an invalid symbol, then print an error message and return 1
            // fprintf(stdout, "Error: Invalid symbol found\n");
            found = 1;
        }
    }

    // Return 0 if there are no invalid symbols
    if(found) {
        return 1;
    }

    return 0;
}

// Function to check if the token is a symbol
int isSymbol(char token) {
    // Check if the token is a symbol
    if(token == '+' || token == '-' || token == '*' || token == '/' || token == '(' || token == ')' || token == '=' || 
    token == ',' || token == '.' || token == '<' || token == '>' || token == ';' || token == ':' || token == '!' || 
    token == '&' || token == '|' || token == '~' || token == '#' || token == '%' || token == '^' || token == '_' || 
    token == '{' || token == '}' || token == '[' || token == ']' || token == '\\' || token == '\'' || token == '"' ||
    token == '?' || token == '`' || token == '@' || token == '$') {
        return 1;
    }

    return 0;
}

// Create a function to add the token to the lexeme list
void addToken(char * tokenize, int tokenValue) {
    // Add to lexeme list
    if(tokenValue == 2 || tokenValue == 3) {
        int tokenLength = strlen(tokenize);

        LexemeList[LexemeListIndex] = tokenValue + '0';
        LexemeListIndex++;

        LexemeList[LexemeListIndex] = ' ';
        LexemeListIndex++;

        for(int i = 0; i < tokenLength; i++) {
            LexemeList[LexemeListIndex] = tokenize[i];
            LexemeListIndex++;
        }

    } else if (tokenValue < 10) {
        LexemeList[LexemeListIndex] = tokenValue + '0';
        LexemeListIndex++; 
    } else {
        char tokenValueChar[2];
        int i = 1;

        // Converting the token value to a char
        while(tokenValue > 0)
        {
            int digit = tokenValue % 10;
            tokenValue = tokenValue / 10;
            char value = digit + '0';

            tokenValueChar[i] = value;
            i--;
        }

        // Adding the token value
        LexemeList[LexemeListIndex] = tokenValueChar[0];
        LexemeListIndex++;

        LexemeList[LexemeListIndex] = tokenValueChar[1];
        LexemeListIndex++;
    }

    LexemeList[LexemeListIndex] = ' ';
    LexemeListIndex++;
}

// SYMBOLTABLECHECK (string)
//  linear search through symbol table looking at name
//  return index if found, -1 if not
int symbolTableCheck(char * name) {
    int i = 0;

    while(i < SymbolTableIndex) {
        if(strcmp(name, SymbolTable[i].name) == 0) {
            return i;
        }
        i++;
    }

    return -1;
}

// Create emit function
void emit(int op, int l, int m) {
    AssemblyCodeList[AssemblyCodeListIndex].op[0] = op_code[op][0];
    AssemblyCodeList[AssemblyCodeListIndex].op[1] = op_code[op][1];
    AssemblyCodeList[AssemblyCodeListIndex].op[2] = op_code[op][2];
    AssemblyCodeList[AssemblyCodeListIndex].op[3] = '\0';
    
    AssemblyCodeList[AssemblyCodeListIndex].l = l;
    AssemblyCodeList[AssemblyCodeListIndex].m = m;

    AssemblyCodeListIndex++;
}

// Create get token function
void getToken() {
    if(CurrentIndex < tokenIndex) {
        strcpy(CurrentToken, token[CurrentIndex]);
        CurrentTokenValue = value[CurrentIndex];
        CurrentIndex++;
    }
}

void addSymbolTable(int kind, char * name, int val, int level, int addr, int mark) {
    SymbolTable[SymbolTableIndex].kind = kind;
    strcpy(SymbolTable[SymbolTableIndex].name, name);
    SymbolTable[SymbolTableIndex].val = val;
    SymbolTable[SymbolTableIndex].level = level;
    SymbolTable[SymbolTableIndex].addr = addr;
    SymbolTable[SymbolTableIndex].mark = mark;

    SymbolTableIndex++;
}

void program() {
    getToken();
    block();

    if(CurrentTokenValue != periodsym) {
        fprintf(stdout, "Error: Period expected\n");
        exit(0);
    }

    emit(SYS, 0, 3);
}

void block() {
    constDeclaration();
    int numVars = varDeclaration();
    emit(INC, 0, numVars + 3);
    statement();
}


void constDeclaration() {
    if(CurrentTokenValue == constsym) {
        do {
            getToken();
            if(CurrentTokenValue != identsym) {
                fprintf(stdout, "Error: Identifier expected\n");
                exit(0);
            }

            if(symbolTableCheck(CurrentToken) != -1) {
                fprintf(stdout, "Error: Identifier already declared\n");
                exit(0);
            }

            char * name = CurrentToken;
            getToken();
            if(CurrentTokenValue != eqlsym) {
                fprintf(stdout, "Error: = expected\n");
                exit(0);
            }

            getToken();
            if(CurrentTokenValue != numbersym) {
                fprintf(stdout, "Error: Number expected\n");
                exit(0);
            }

            addSymbolTable(1, name, atoi(CurrentToken), 0, 0, 1);
            getToken();
        } while(CurrentTokenValue == commasym);

        if(CurrentTokenValue != semicolonsym) {
            fprintf(stdout, "Error: Semicolon expected\n");
            exit(0);
        }

        getToken();
    }
}


int varDeclaration() {
    int numVars = 0;

    if(CurrentTokenValue == varsym) {
        do {
            numVars++;
            getToken();
            if(CurrentTokenValue != identsym) {
                fprintf(stdout, "Error: Identifier expected\n");
                exit(0);
            }

            if(symbolTableCheck(CurrentToken) != -1) {
                fprintf(stdout, "Error: Identifier already declared\n");
                exit(0);
            }

            addSymbolTable(2, CurrentToken, 0, 0, numVars + 2, 1);
            getToken();
        } while(CurrentTokenValue == commasym);

        if(CurrentTokenValue != semicolonsym) {
            fprintf(stdout, "Error: Semicolon expected\n");
            exit(0);
        }

        getToken();
    }

    return numVars;
}

void statement() {
    if(CurrentTokenValue == identsym) {
        int symIdx = symbolTableCheck(CurrentToken);
        if(symIdx == -1) {
            fprintf(stdout, "Error: Identifier not declared\n");
            exit(0);
        }

        if(SymbolTable[symIdx].kind != 2) {
            fprintf(stdout, "Error: Identifier must be a variable\n");
            exit(0);
        }

        getToken();
        if(CurrentTokenValue != becomessym) {
            error(1);
            fprintf(stdout, "Error: = expected\n");
            exit(0);
        }

        getToken();
        expression();
        emit(STO, 0, SymbolTable[symIdx].addr);
        return;
    }

    if(CurrentTokenValue == beginsym) {
        do {
            getToken();
            statement();
        } while(CurrentTokenValue == semicolonsym);

        if(CurrentTokenValue != endsym) {
            //end expected
            error(5);
            exit(0);
        }

        getToken();
        return;
    }

    if(CurrentTokenValue == ifsym) {
        getToken();
        condition();
        int jpcIdx = AssemblyCodeListIndex;
        emit(JPC, 0, 0);
        if(CurrentTokenValue != thensym) {
            //Then expected
            error(10);
            exit(0);
        }

        getToken();
        statement();
        AssemblyCodeList[jpcIdx].m = AssemblyCodeListIndex;
        return;
    }

    if(CurrentTokenValue == whilesym) {
        getToken();
        int loopIdx = AssemblyCodeListIndex;
        condition();
        if(CurrentTokenValue != dosym) {
            //do expected
            error(11);
            exit(0);
        }

        getToken();
        int jpcIdx = AssemblyCodeListIndex;
        emit(JPC, 0, 0);
        statement();
        emit(JMP, 0, loopIdx);
        AssemblyCodeList[jpcIdx].m = AssemblyCodeListIndex;
        return;
    }

    if(CurrentTokenValue == readsym) {
        getToken();
        if(CurrentTokenValue != identsym) {
            //expected identifier
            error(2);
            exit(0);
        }

        int symIdx = symbolTableCheck(CurrentToken);
        if(symIdx == -1) {
            //undeclared identifier
            error(8);
            exit(0);
        }

        if(SymbolTable[symIdx].kind != 2) {
            //must be a variable
            error(2);
            exit(0);
        }

        getToken();
        emit(SYS, 0, 2);
        emit(STO, 0, SymbolTable[symIdx].addr);

        return;
    }

    if(CurrentTokenValue == writesym) {
        getToken();
        expression();
        emit(SYS, 0, 1);

        return;
    }
}


void condition() {
    if(CurrentTokenValue == oddsym) {
        getToken();
        expression();
        emit(OPR, 0, ODD);
    } else {
        expression();
        if(CurrentTokenValue == eqlsym) {
            getToken();
            expression();
            emit(OPR, 0, EQL);
        } else if(CurrentTokenValue == neqsym) {
            getToken();
            expression();
            emit(OPR, 0, NEQ);
        } else if(CurrentTokenValue == lessym) {
            getToken();
            expression();
            emit(OPR, 0, LSS);
        } else if(CurrentTokenValue == leqsym) {
            getToken();
            expression();
            emit(OPR, 0, LEQ);
        } else if(CurrentTokenValue == gtrsym) {
            getToken();
            expression();
            emit(OPR, 0, GTR);
        } else if(CurrentTokenValue == geqsym) {
            getToken();
            expression();
            emit(OPR, 0, GEQ);
        } else {
            //relational operator
            error(9);
            exit(0);
        }
    }
}


void expression() {
    if(CurrentTokenValue == minussym) {
        getToken();
        term();
        emit(OPR, 0, NEG);

        while(CurrentTokenValue == plussym || CurrentTokenValue == minussym) {
            if(CurrentTokenValue == plussym) {
                getToken();
                term();
                emit(OPR, 0, ADD);
            } else {
                getToken();
                term();
                emit(OPR, 0, SUB);
            }
        }
    } else {
        if(CurrentTokenValue == plussym) {
            getToken();
        }

        term();

        while(CurrentTokenValue == plussym || CurrentTokenValue == minussym) {
            if(CurrentTokenValue == plussym) {
                getToken();
                term();
                emit(OPR, 0, ADD);
            } else {
                getToken();
                term();
                emit(OPR, 0, SUB);
            }
        }
    }
}


void term() {
    factor();

    while(CurrentTokenValue == multsym || CurrentTokenValue == slashsym) {
        if(CurrentTokenValue == multsym) {
            getToken();
            factor();
            emit(OPR, 0, MUL);
        } else {
            getToken();
            factor();
            emit(OPR, 0, DIV);
        }
    }
}


void factor() {
    if(CurrentTokenValue == identsym) {
        int symIdx = symbolTableCheck(CurrentToken);

        if(symIdx == -1) {
            fprintf(stdout, "Error: Identifier is not declared");
            exit(0);
        }

        if(SymbolTable[symIdx].kind == 1) {
            emit(LIT, 0, SymbolTable[symIdx].val);
        } else {
            emit(LOD, 0, SymbolTable[symIdx].addr);
        }

        getToken();
    } else if(CurrentTokenValue == numbersym) {
        emit(LIT, 0, atoi(CurrentToken));
        getToken();
    } else if(CurrentTokenValue == lparentsym) {
        getToken();
        expression();

        if(CurrentTokenValue != rparentsym) {
            fprintf(stdout, "Error: Right parenthesis expected\n");
            exit(0);
        }

        getToken();
    } else {
        fprintf(stdout, "Error: Identifier, number, or left parenthesis expected\n");
        exit(0);
    }
}


// //will output all errors, checking for syntax error
void error(int err){
	switch(err){
		case 1: 
			printf("Error: constants must be assigned with =\n");
			break;
		case 2:
			printf("Error: const, var, and read keywords must be followed by identifier\n");
			break;
		case 3:
			printf("Error: constant and variable declarations must be followed by a semicolon\n");
			break;
		case 4:
			printf("Error: constants must be assigned an integer value\n");
			break;
		case 5:
			printf("Error: begin must be followed by end\n");
			break;
		case 6:
			printf("Error: right parenthesis must follow left parenthesis\n");
			break;
		case 7:
			printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
			break;
		case 8:
			printf("Error: undeclared identifier\n");
			break;
		case 9:
			printf("Error: condition must contain comparison operator\n");
			break;
		case 10:
			printf("Error: if must be followed by then\n");
			break;
		case 11:
			printf("Error: while must be followed by do\n");
			break;
		case 12:
			printf("Error: program must end with period\n");
			break;
		case 13:
			printf("Error: only variable values may be altered\n");
			break;
        case 14:
			printf("Error: assignment statements must use :=\n");
			break;
        case 15:
			printf("Error: symbol name has already been declared\n");
			break;
		default:
			printf("Invalid choice\n");
			break;
	}
}


