// tokens.h
#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    IF,
    THEN,
    ELSE,
    END,
    REPEAT,
    UNTIL,
    READ,
    WRITE,
    ID,
    NUM,
    ASSIGN,
    RELOP,
    PLUS,
    MINUS,
    TIMES,
    OVER,
    LPAREN,
    RPAREN,
    SEMI,
    DELIMETER,
    COMMENT,
    DOLLAR
} TokenType;

const char *tokenNames[] = {
    "IF", "THEN", "ELSE", "END", "REPEAT", "UNTIL", "READ", "WRITE", "ID", "NUM", "ASSIGN", "RELOP", "PLUS", "MINUS", "TIMES", "OVER", "LPAREN", "RPAREN", "SEMI", "DELIMETER", "COMMENT", "DOLLAR"};

#endif // TOKENS_H