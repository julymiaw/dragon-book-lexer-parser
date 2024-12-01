#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "tokens.h"

void printToken(TokenType token, const char *attr) {
    printf("%s\t%s\n", tokenNames[token], attr);
}

typedef struct {
    char name[256];
    int id;
} Symbol;

Symbol symbolTable[256];
int symbolCount = 0;

int lookupSymbol(const char *name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return symbolTable[i].id;
        }
    }
    return -1;
}

int addSymbol(const char *name) {
    strcpy(symbolTable[symbolCount].name, name);
    symbolTable[symbolCount].id = symbolCount + 1;
    return symbolTable[symbolCount++].id;
}

void processFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    int bufferIndex = 0;
    int c;
    char annotations[1024] = "";
    int annotationIndex = 0;

    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) {
            continue;
        }

        if (isalpha(c)) {
            bufferIndex = 0;
            while (isalpha(c) || isdigit(c)) {
                buffer[bufferIndex++] = c;
                c = fgetc(file);
            }
            buffer[bufferIndex] = '\0';
            ungetc(c, file);

            if (strcmp(buffer, "read") == 0) {
                printToken(READ, "");
            } else if (strcmp(buffer, "if") == 0) {
                printToken(IF, "");
            } else if (strcmp(buffer, "then") == 0) {
                printToken(THEN, "");
            } else if (strcmp(buffer, "repeat") == 0) {
                printToken(REPEAT, "");
            } else if (strcmp(buffer, "until") == 0) {
                printToken(UNTIL, "");
            } else if (strcmp(buffer, "write") == 0) {
                printToken(WRITE, "");
            } else if (strcmp(buffer, "end") == 0) {
                printToken(END, "");
            } else if (strcmp(buffer, "else") == 0) {
                printToken(ELSE, "");
            } else {
                int id = lookupSymbol(buffer);
                if (id == -1) {
                    id = addSymbol(buffer);
                }
                char idStr[10];
                sprintf(idStr, "%d", id);
                printToken(ID, idStr);
            }
        } else if (isdigit(c)) {
            bufferIndex = 0;
            while (isdigit(c)) {
                buffer[bufferIndex++] = c;
                c = fgetc(file);
            }
            if (c == '.') {
                buffer[bufferIndex++] = c;
                c = fgetc(file);
                while (isdigit(c)) {
                    buffer[bufferIndex++] = c;
                    c = fgetc(file);
                }
            }
            buffer[bufferIndex] = '\0';
            ungetc(c, file);
            double num = atof(buffer);
            sprintf(buffer, "%.6f", num);
            printToken(NUM, buffer);
        } else if (c == '/') {
            c = fgetc(file);
            if (c == '/') {
                // Single line comment
                bufferIndex = 0;
                buffer[bufferIndex++] = '/';
                buffer[bufferIndex++] = '/';
                while ((c = fgetc(file)) != '\n' && c != EOF) {
                    buffer[bufferIndex++] = c;
                }
                buffer[bufferIndex] = '\0';
                strcat(annotations, buffer);
                strcat(annotations, "\n");
            } else if (c == '*') {
                // Multi-line comment
                bufferIndex = 0;
                buffer[bufferIndex++] = '/';
                buffer[bufferIndex++] = '*';
                while (1) {
                    c = fgetc(file);
                    if (c == EOF) {
                        break;
                    }
                    buffer[bufferIndex++] = c;
                    if (c == '*') {
                        c = fgetc(file);
                        if (c == '/') {
                            buffer[bufferIndex++] = '/';
                            break;
                        } else {
                            ungetc(c, file);
                        }
                    }
                }
                buffer[bufferIndex] = '\0';
                strcat(annotations, buffer);
                strcat(annotations, "\n");
            } else {
                ungetc(c, file);
                printToken(OVER, "/");
            }
        } else {
            switch (c) {
            case ';':
                printToken(SEMI, ";");
                break;
            case ':':
                if ((c = fgetc(file)) == '=') {
                    printToken(ASSIGN, ":=");
                } else {
                    ungetc(c, file);
                }
                break;
            case '<':
                if ((c = fgetc(file)) == '=' || c == '>') {
                    buffer[0] = '<';
                    buffer[1] = c;
                    buffer[2] = '\0';
                    printToken(RELOP, buffer);
                } else {
                    ungetc(c, file);
                    printToken(RELOP, "<");
                }
                break;
            case '>':
                if ((c = fgetc(file)) == '=') {
                    printToken(RELOP, ">=");
                } else {
                    ungetc(c, file);
                    printToken(RELOP, ">");
                }
                break;
            case '=':
                printToken(RELOP, "=");
                break;
            case '+':
                printToken(PLUS, "+");
                break;
            case '-':
                printToken(MINUS, "-");
                break;
            case '*':
                printToken(TIMES, "*");
                break;
            case '(':
                printToken(LPAREN, "(");
                break;
            case ')':
                printToken(RPAREN, ")");
                break;
            default:
                break;
            }
        }
    }

    fclose(file);

    // 输出 DOLLAR 符号，表示输入结束
    printToken(DOLLAR, "$");

    // Print annotations
    printf("Annotations :\n%s", annotations);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    processFile(argv[1]);

    return EXIT_SUCCESS;
}