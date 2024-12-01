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

typedef struct ASTNode {
    char label[256];
    struct ASTNode *children[5];
    int childCount;
} ASTNode;

ASTNode *createNode(const char *label) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    strcpy(node->label, label);
    node->childCount = 0;
    return node;
}

void addChild(ASTNode *parent, ASTNode *child) {
    if (parent->childCount < 5) {
        parent->children[parent->childCount++] = child;
    } else {
        printf("错误：节点的子节点超过5个\n");
        exit(EXIT_FAILURE);
    }
}

void printAST(ASTNode *node, FILE *file) {
    if (node == NULL)
        return;
    fprintf(file, "\"%p\" [label=\"%s\"];\n", node, node->label);
    for (int i = 0; i < node->childCount; i++) {
        fprintf(file, "\"%p\" -> \"%p\";\n", node, node->children[i]);
        printAST(node->children[i], file);
    }
}

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
ASTNode *match(TokenType expected) {
    if (getToken() == expected) {
        if (debug) {
            printf("匹配：%s\n", tokenNames[expected]);
        }
        ASTNode *node = createNode(tokenNames[expected]);
        if (tokens[currentToken].attr[0] != '\0') {
            char label[512]; // 增加缓冲区大小
            snprintf(label, sizeof(label), "%s: %s", tokenNames[expected], tokens[currentToken].attr);
            strcpy(node->label, label);
        }
        currentToken++;
        return node;
    } else {
        printf("语法错误：期望 %d，得到 %d\n", expected, getToken());
        exit(EXIT_FAILURE);
    }
}

// 递归下降解析函数声明
ASTNode *Program();
ASTNode *StmtList();
ASTNode *StmtListPrime();
ASTNode *Stmt();
ASTNode *IfStmt();
ASTNode *IfStmtPrime();
ASTNode *RepeatStmt();
ASTNode *AssignStmt();
ASTNode *ReadStmt();
ASTNode *WriteStmt();
ASTNode *Exp();
ASTNode *ExpPrime();
ASTNode *RelOp();
ASTNode *SimpleExp();
ASTNode *SimpleExpPrime();
ASTNode *AddOp();
ASTNode *Term();
ASTNode *TermPrime();
ASTNode *MulOp();
ASTNode *Factor();

// 解析 Program -> P → SL
ASTNode *Program() {
    if (debug)
        printf("进入：Program\n");
    ASTNode *node = createNode("Program");
    if (getToken() == IF || getToken() == REPEAT || getToken() == ID || getToken() == READ || getToken() == WRITE) {
        addChild(node, StmtList());
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
    return node;
}

// 解析 StmtList -> SL → S SL'
ASTNode *StmtList() {
    if (debug)
        printf("进入：StmtList\n");
    ASTNode *node = createNode("StmtList");
    addChild(node, Stmt());
    addChild(node, StmtListPrime());
    if (debug)
        printf("离开：StmtList\n");
    return node;
}

// 解析 StmtListPrime -> SL' → ; SL' | ε
ASTNode *StmtListPrime() {
    if (debug)
        printf("进入：StmtListPrime\n");
    ASTNode *node = createNode("StmtListPrime");
    if (getToken() == SEMI) {
        addChild(node, match(SEMI));
        addChild(node, Stmt());
        addChild(node, StmtListPrime());
    } else if (getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == DOLLAR || getToken() == THEN) {
        // SL' -> ε
    } else {
        printf("语法错误：期望分号或结束符号\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：StmtListPrime\n");
    return node;
}

// 解析 Stmt -> S → I | R | A | RD | W
ASTNode *Stmt() {
    if (debug)
        printf("进入：Stmt\n");
    ASTNode *node = createNode("Stmt");
    if (getToken() == IF) {
        addChild(node, IfStmt());
    } else if (getToken() == REPEAT) {
        addChild(node, RepeatStmt());
    } else if (getToken() == ID) {
        addChild(node, AssignStmt());
    } else if (getToken() == READ) {
        addChild(node, ReadStmt());
    } else if (getToken() == WRITE) {
        addChild(node, WriteStmt());
    } else {
        printf("语法错误：无法识别的语句\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：Stmt\n");
    return node;
}

// 解析 IfStmt -> I → if E then SL I'
ASTNode *IfStmt() {
    if (debug)
        printf("进入：IfStmt\n");
    ASTNode *node = createNode("IfStmt");
    addChild(node, match(IF));
    addChild(node, Exp());
    addChild(node, match(THEN));
    addChild(node, StmtList());
    addChild(node, IfStmtPrime());
    if (debug)
        printf("离开：IfStmt\n");
    return node;
}

// 解析 IfStmtPrime -> I' → end | else SL end
ASTNode *IfStmtPrime() {
    if (debug)
        printf("进入：IfStmtPrime\n");
    ASTNode *node = createNode("IfStmtPrime");
    if (getToken() == END) {
        addChild(node, match(END));
    } else if (getToken() == ELSE) {
        addChild(node, match(ELSE));
        addChild(node, StmtList());
        addChild(node, match(END));
    } else {
        printf("语法错误：期望 end 或 else\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：IfStmtPrime\n");
    return node;
}

// 解析 RepeatStmt -> R → repeat SL until E
ASTNode *RepeatStmt() {
    if (debug)
        printf("进入：RepeatStmt\n");
    ASTNode *node = createNode("RepeatStmt");
    addChild(node, match(REPEAT));
    addChild(node, StmtList());
    addChild(node, match(UNTIL));
    addChild(node, Exp());
    if (debug)
        printf("离开：RepeatStmt\n");
    return node;
}

// 解析 AssignStmt -> A → id := E
ASTNode *AssignStmt() {
    if (debug)
        printf("进入：AssignStmt\n");
    ASTNode *node = createNode("AssignStmt");
    addChild(node, match(ID));
    addChild(node, match(ASSIGN));
    addChild(node, Exp());
    if (debug)
        printf("离开：AssignStmt\n");
    return node;
}

// 解析 ReadStmt -> RD → read id
ASTNode *ReadStmt() {
    if (debug)
        printf("进入：ReadStmt\n");
    ASTNode *node = createNode("ReadStmt");
    addChild(node, match(READ));
    addChild(node, match(ID));
    if (debug)
        printf("离开：ReadStmt\n");
    return node;
}

// 解析 WriteStmt -> W → write E
ASTNode *WriteStmt() {
    if (debug)
        printf("进入：WriteStmt\n");
    ASTNode *node = createNode("WriteStmt");
    addChild(node, match(WRITE));
    addChild(node, Exp());
    if (debug)
        printf("离开：WriteStmt\n");
    return node;
}

// 解析 Exp -> E → SE E'
ASTNode *Exp() {
    if (debug)
        printf("进入：Exp\n");
    ASTNode *node = createNode("Exp");
    addChild(node, SimpleExp());
    addChild(node, ExpPrime());
    if (debug)
        printf("离开：Exp\n");
    return node;
}

// 解析 ExpPrime -> E' → RO SE | ε
ASTNode *ExpPrime() {
    if (debug)
        printf("进入：ExpPrime\n");
    ASTNode *node = createNode("ExpPrime");
    if (getToken() == RELOP) {
        addChild(node, RelOp());
        addChild(node, SimpleExp());
    } else if (getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // E' -> ε
    } else {
        printf("语法错误：无法识别的表达式\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：ExpPrime\n");
    return node;
}

// 解析 RelOp -> RO → relop
ASTNode *RelOp() {
    if (debug)
        printf("进入：RelOp\n");
    ASTNode *node = createNode("RelOp");
    addChild(node, match(RELOP));
    if (debug)
        printf("离开：RelOp\n");
    return node;
}

// 解析 SimpleExp -> SE → T SE'
ASTNode *SimpleExp() {
    if (debug)
        printf("进入：SimpleExp\n");
    ASTNode *node = createNode("SimpleExp");
    addChild(node, Term());
    addChild(node, SimpleExpPrime());
    if (debug)
        printf("离开：SimpleExp\n");
    return node;
}

// 解析 SimpleExpPrime -> SE' → AO T SE' | ε
ASTNode *SimpleExpPrime() {
    if (debug)
        printf("进入：SimpleExpPrime\n");
    ASTNode *node = createNode("SimpleExpPrime");
    if (getToken() == PLUS || getToken() == MINUS) {
        addChild(node, AddOp());
        addChild(node, Term());
        addChild(node, SimpleExpPrime());
    } else if (getToken() == RELOP || getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // SE' -> ε
    } else {
        printf("语法错误：无法识别的简单表达式\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：SimpleExpPrime\n");
    return node;
}

// 解析 AddOp -> AO → plus | minus
ASTNode *AddOp() {
    if (debug)
        printf("进入：AddOp\n");
    ASTNode *node = createNode("AddOp");
    if (getToken() == PLUS) {
        addChild(node, match(PLUS));
    } else if (getToken() == MINUS) {
        addChild(node, match(MINUS));
    } else {
        printf("语法错误：期望加法操作符\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：AddOp\n");
    return node;
}

// 解析 Term -> T → F T'
ASTNode *Term() {
    if (debug)
        printf("进入：Term\n");
    ASTNode *node = createNode("Term");
    addChild(node, Factor());
    addChild(node, TermPrime());
    if (debug)
        printf("离开：Term\n");
    return node;
}

// 解析 TermPrime -> T' → MO F T' | ε
ASTNode *TermPrime() {
    if (debug)
        printf("进入：TermPrime\n");
    ASTNode *node = createNode("TermPrime");
    if (getToken() == TIMES || getToken() == OVER) {
        addChild(node, MulOp());
        addChild(node, Factor());
        addChild(node, TermPrime());
    } else if (getToken() == PLUS || getToken() == MINUS || getToken() == RELOP || getToken() == THEN || getToken() == SEMI || getToken() == END || getToken() == ELSE || getToken() == UNTIL || getToken() == RPAREN || getToken() == DOLLAR) {
        // T' -> ε
    } else {
        printf("语法错误：无法识别的项\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：TermPrime\n");
    return node;
}

// 解析 MulOp -> MO → times | over
ASTNode *MulOp() {
    if (debug)
        printf("进入：MulOp\n");
    ASTNode *node = createNode("MulOp");
    if (getToken() == TIMES) {
        addChild(node, match(TIMES));
    } else if (getToken() == OVER) {
        addChild(node, match(OVER));
    } else {
        printf("语法错误：期望乘法操作符\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：MulOp\n");
    return node;
}

// 解析 Factor -> F → (E) | id | num
ASTNode *Factor() {
    if (debug)
        printf("进入：Factor\n");
    ASTNode *node = createNode("Factor");
    if (getToken() == LPAREN) {
        addChild(node, match(LPAREN));
        addChild(node, Exp());
        addChild(node, match(RPAREN));
    } else if (getToken() == ID) {
        addChild(node, match(ID));
    } else if (getToken() == NUM) {
        addChild(node, match(NUM));
    } else {
        printf("语法错误：无法识别的因子\n");
        exit(EXIT_FAILURE);
    }
    if (debug)
        printf("离开：Factor\n");
    return node;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        printf("用法：%s <词法分析器输出文件> [-d] [-AST ast.dot]\n", argv[0]);
        return 1;
    }

    char *astFilePath = NULL;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debug = 1;
        } else if (strcmp(argv[i], "-AST") == 0 && i + 1 < argc) {
            astFilePath = argv[++i];
        }
    }

    readTokens(argv[1]);
    ASTNode *root = Program();

    if (astFilePath) {
        FILE *astFile = fopen(astFilePath, "w");
        if (!astFile) {
            perror("无法打开AST输出文件");
            return 1;
        }
        fprintf(astFile, "digraph AST {\n");
        printAST(root, astFile);
        fprintf(astFile, "}\n");
        fclose(astFile);
    }

    return 0;
}