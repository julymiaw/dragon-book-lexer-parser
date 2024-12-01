// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

#define MAX_TOKENS 1024

int debug = 0; // 调试开关，0 表示关闭，1 表示开启

typedef struct {
    TokenType token;
    char attr[256];
} Token;

Token tokens[MAX_TOKENS];
int tokenIndex = 0;
int currentToken = 0;

// 从词法分析器的输出文件中读取 tokens
void readTokens(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("无法打开词法分析器输出文件");
        exit(EXIT_FAILURE);
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // 去掉行末的换行符
        line[strcspn(line, "\n")] = 0;

        char tokenName[256];
        char attr[256];
        TokenType tokenType;

        // 检查是否包含制表符，判断是否有属性
        char *tabPos = strchr(line, '\t');
        if (tabPos) {
            // 有属性
            *tabPos = 0; // 将制表符替换为字符串结束符，以分割字符串
            strcpy(tokenName, line);
            strcpy(attr, tabPos + 1);
        } else {
            // 无属性
            strcpy(tokenName, line);
            attr[0] = '\0'; // 属性为空字符串
        }

        // 将 tokenName 转换为 TokenType
        if (strcmp(tokenName, "IF") == 0)
            tokenType = IF;
        else if (strcmp(tokenName, "THEN") == 0)
            tokenType = THEN;
        else if (strcmp(tokenName, "ELSE") == 0)
            tokenType = ELSE;
        else if (strcmp(tokenName, "END") == 0)
            tokenType = END;
        else if (strcmp(tokenName, "REPEAT") == 0)
            tokenType = REPEAT;
        else if (strcmp(tokenName, "UNTIL") == 0)
            tokenType = UNTIL;
        else if (strcmp(tokenName, "READ") == 0)
            tokenType = READ;
        else if (strcmp(tokenName, "WRITE") == 0)
            tokenType = WRITE;
        else if (strcmp(tokenName, "ID") == 0)
            tokenType = ID;
        else if (strcmp(tokenName, "NUM") == 0)
            tokenType = NUM;
        else if (strcmp(tokenName, "ASSIGN") == 0)
            tokenType = ASSIGN;
        else if (strcmp(tokenName, "RELOP") == 0)
            tokenType = RELOP;
        else if (strcmp(tokenName, "PLUS") == 0)
            tokenType = PLUS;
        else if (strcmp(tokenName, "MINUS") == 0)
            tokenType = MINUS;
        else if (strcmp(tokenName, "TIMES") == 0)
            tokenType = TIMES;
        else if (strcmp(tokenName, "OVER") == 0)
            tokenType = OVER;
        else if (strcmp(tokenName, "LPAREN") == 0)
            tokenType = LPAREN;
        else if (strcmp(tokenName, "RPAREN") == 0)
            tokenType = RPAREN;
        else if (strcmp(tokenName, "SEMI") == 0)
            tokenType = SEMI;
        else if (strcmp(tokenName, "DOLLAR") == 0)
            tokenType = DOLLAR;
        else {
            printf("未知的词法单元：%s\n", tokenName);
            exit(EXIT_FAILURE);
        }

        tokens[tokenIndex].token = tokenType;
        strcpy(tokens[tokenIndex].attr, attr);
        tokenIndex++;

        // 如果遇到 DOLLAR token，停止读取
        if (tokenType == DOLLAR) {
            break;
        }
    }
    fclose(file);
}

// 获取当前词法单元的类型
TokenType getToken() {
    return tokens[currentToken].token;
}

// 匹配并消耗期望的词法单元
void match(TokenType expected) {
    if (getToken() == expected) {
        if (debug) {
            printf("匹配：%s\n", tokenNames[expected]);
        }
        currentToken++;
    } else {
        printf("语法错误：期望 %d，得到 %d\n", expected, getToken());
        exit(EXIT_FAILURE);
    }
}

// 递归下降解析函数声明
void Program();
void StmtList();
void StmtListPrime();
void Stmt();
void IfStmt();
void IfStmtPrime();
void RepeatStmt();
void AssignStmt();
void ReadStmt();
void WriteStmt();
void Exp();
void ExpPrime();
void RelOp();
void SimpleExp();
void SimpleExpPrime();
void AddOp();
void Term();
void TermPrime();
void MulOp();
void Factor();

// 解析 Program -> P → SL
void Program() {
    if (debug)
        printf("进入：Program\n");
    if (getToken() == IF || getToken() == REPEAT || getToken() == ID || getToken() == READ || getToken() == WRITE) {
        StmtList();
        if (getToken() == DOLLAR) {
            if (debug)
                printf("匹配：DOLLAR\n");
            printf("语法分析成功！\n");
        } else {
            printf("语法错误：程序末尾缺少 $ 符号\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("语法错误：无法识别的起始符号\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：Program\n");
}

// 解析 StmtList -> SL → S SL'
void StmtList() {
    if (debug)
        printf("进入：StmtList\n");
    Stmt();
    StmtListPrime();
    if (debug)
        printf("离开：StmtList\n");
}

// 解析 StmtListPrime -> SL' → ; SL' | ε
void StmtListPrime() {
    if (debug)
        printf("进入：StmtListPrime\n");
    if (getToken() == SEMI) {
        match(SEMI);
        Stmt();
        StmtListPrime();
    } else if (getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == DOLLAR || getToken() == THEN) {
        // SL' -> ε
    } else {
        printf("语法错误：期望分号或结束符号\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：StmtListPrime\n");
}

// 解析 Stmt -> S → I | R | A | RD | W
void Stmt() {
    if (debug)
        printf("进入：Stmt\n");
    if (getToken() == IF) {
        IfStmt();
    } else if (getToken() == REPEAT) {
        RepeatStmt();
    } else if (getToken() == ID) {
        AssignStmt();
    } else if (getToken() == READ) {
        ReadStmt();
    } else if (getToken() == WRITE) {
        WriteStmt();
    } else {
        printf("语法错误：无法识别的语句\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：Stmt\n");
}

// 解析 IfStmt -> I → if E then SL I'
void IfStmt() {
    if (debug)
        printf("进入：IfStmt\n");
    match(IF);
    Exp();
    match(THEN);
    StmtList();
    IfStmtPrime();
    if (debug)
        printf("离开：IfStmt\n");
}

// 解析 IfStmtPrime -> I' → end | else SL end
void IfStmtPrime() {
    if (debug)
        printf("进入：IfStmtPrime\n");
    if (getToken() == END) {
        match(END);
    } else if (getToken() == ELSE) {
        match(ELSE);
        StmtList();
        match(END);
    } else {
        printf("语法错误：期望 end 或 else\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：IfStmtPrime\n");
}

// 解析 RepeatStmt -> R → repeat SL until E
void RepeatStmt() {
    if (debug)
        printf("进入：RepeatStmt\n");
    match(REPEAT);
    StmtList();
    match(UNTIL);
    Exp();
    if (debug)
        printf("离开：RepeatStmt\n");
}

// 解析 AssignStmt -> A → id := E
void AssignStmt() {
    if (debug)
        printf("进入：AssignStmt\n");
    match(ID);
    match(ASSIGN);
    Exp();
    if (debug)
        printf("离开：AssignStmt\n");
}

// 解析 ReadStmt -> RD → read id
void ReadStmt() {
    if (debug)
        printf("进入：ReadStmt\n");
    match(READ);
    match(ID);
    if (debug)
        printf("离开：ReadStmt\n");
}

// 解析 WriteStmt -> W → write E
void WriteStmt() {
    if (debug)
        printf("进入：WriteStmt\n");
    match(WRITE);
    Exp();
    if (debug)
        printf("离开：WriteStmt\n");
}

// 解析 Exp -> E → SE E'
void Exp() {
    if (debug)
        printf("进入：Exp\n");
    SimpleExp();
    ExpPrime();
    if (debug)
        printf("离开：Exp\n");
}

// 解析 ExpPrime -> E' → RO SE | ε
void ExpPrime() {
    if (debug)
        printf("进入：ExpPrime\n");
    if (getToken() == RELOP) {
        RelOp();
        SimpleExp();
    } else if (getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // E' -> ε
    } else {
        printf("语法错误：无法识别的表达式\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：ExpPrime\n");
}

// 解析 RelOp -> RO → relop
void RelOp() {
    if (debug)
        printf("进入：RelOp\n");
    match(RELOP);
    if (debug)
        printf("离开：RelOp\n");
}

// 解析 SimpleExp -> SE → T SE'
void SimpleExp() {
    if (debug)
        printf("进入：SimpleExp\n");
    Term();
    SimpleExpPrime();
    if (debug)
        printf("离开：SimpleExp\n");
}

// 解析 SimpleExpPrime -> SE' → AO T SE' | ε
void SimpleExpPrime() {
    if (debug)
        printf("进入：SimpleExpPrime\n");
    if (getToken() == PLUS || getToken() == MINUS) {
        AddOp();
        Term();
        SimpleExpPrime();
    } else if (getToken() == RELOP || getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // SE' -> ε
    } else {
        printf("语法错误：无法识别的简单表达式\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：SimpleExpPrime\n");
}

// 解析 AddOp -> AO → plus | minus
void AddOp() {
    if (debug)
        printf("进入：AddOp\n");
    if (getToken() == PLUS) {
        match(PLUS);
    } else if (getToken() == MINUS) {
        match(MINUS);
    } else {
        printf("语法错误：期望加法操作符\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：AddOp\n");
}

// 解析 Term -> T → F T'
void Term() {
    if (debug)
        printf("进入：Term\n");
    Factor();
    TermPrime();
    if (debug)
        printf("离开：Term\n");
}

// 解析 TermPrime -> T' → MO F T' | ε
void TermPrime() {
    if (debug)
        printf("进入：TermPrime\n");
    if (getToken() == TIMES || getToken() == OVER) {
        MulOp();
        Factor();
        TermPrime();
    } else if (getToken() == PLUS || getToken() == MINUS || getToken() == RELOP || getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // T' -> ε
    } else {
        printf("语法错误：无法识别的项\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：TermPrime\n");
}

// 解析 MulOp -> MO → times | over
void MulOp() {
    if (debug)
        printf("进入：MulOp\n");
    if (getToken() == TIMES) {
        match(TIMES);
    } else if (getToken() == OVER) {
        match(OVER);
    } else {
        printf("语法错误：期望乘法操作符\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：MulOp\n");
}

// 解析 Factor -> F → (E) | id | num
void Factor() {
    if (debug)
        printf("进入：Factor\n");
    if (getToken() == LPAREN) {
        match(LPAREN);
        Exp();
        match(RPAREN);
    } else if (getToken() == ID) {
        match(ID);
    } else if (getToken() == NUM) {
        match(NUM);
    } else {
        printf("语法错误：无法识别的因子\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：Factor\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        printf("用法：%s <词法分析器输出文件> [-d]\n", argv[0]);
        return 1;
    }

    if (argc == 3 && strcmp(argv[2], "-d") == 0) {
        debug = 1;
    }

    readTokens(argv[1]);
    Program();

    return 0;
}