#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKEN_TEXT_LENGTH 512
#define INITIAL_TOKEN_LIST_CAPACITY 30

typedef enum {
    TokenType_Identifier,   // 表示标识符（如变量名和函数名）
    TokenType_IntLiteral,   // 表示整数字面量（如 42）
    TokenType_Plus,         // 表示加号运算符（+）
    TokenType_Minus,        // 表示减号运算符（-）
    TokenType_Star,         // 表示乘号运算符（*）
    TokenType_Slash,        // 表示除号运算符（/）
    TokenType_Comment_Slash,// 表示注释（// ..... \n）
    TokenType_SemiColon,    // 表示分号（;）
    TokenType_LeftParen,    // 表示左括号（(）
    TokenType_RightParen,   // 表示右括号（)）
    TokenType_Assignment,   // 表示赋值运算符（=）
    TokenType_Greater,      // 表示大于号（>）
    TokenType_GreaterEqual, // 表示大于等于号（>=）
    TokenType_LessEqual,    // 表示小于等于号（<=）
    TokenType_Less,         // 表示小于号（<）
    TokenType_NotEqual,     // 表示不等于号（!=）
    TokenType_EOF,          // 表示文件结束或输入结束
    TokenType_DoubleQuote,  // 添加双引号类型
    TokenType_SingleQuote,  // 添加单引号类型
} TokenType;

typedef struct {
    TokenType type;                   // 表示 Token 的类型（如运算符、字面量等）
    char text[MAX_TOKEN_TEXT_LENGTH]; // 存储 Token 的文本内容
} Token;

typedef Token *TokenPtr;

typedef struct {
    int count;                      // 当前 TokenList 中的 Token 数量
    int capacity;                   // TokenList 的容量，即最多可容纳的 Token 数量
    TokenPtr *tokens;               // 指向 TokenPtr 类型的指针
} TokenList;

Token *create_token(TokenType type, const char *text) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    strncpy(token->text, text, MAX_TOKEN_TEXT_LENGTH);
    token->text[MAX_TOKEN_TEXT_LENGTH - 1] = '\0';
    return token;
}

void destroy_token(Token *token) {
    if (token != NULL) {
        free(token);
    }
}

TokenList *create_token_list() {
    TokenList *token_list = (TokenList *)malloc(sizeof(TokenList));
    token_list->count = 0;
    token_list->capacity = INITIAL_TOKEN_LIST_CAPACITY;
    token_list->tokens = (Token **)malloc(token_list->capacity * sizeof(Token *));
    return token_list;
}

void destroy_token_list(TokenList *token_list) {
    if (token_list != NULL) {
        for (int i = 0; i < token_list->count; i++) {
            destroy_token(token_list->tokens[i]);
        }
        if(token_list->tokens != NULL)
            free(token_list->tokens);
        if(token_list != NULL)
            free(token_list);
    }
}

// 将 token 添加到 token_list 中
void add_token_to_list(TokenList *token_list, Token *token) {
    // 如果 token_list 中的 Token 数量已达到容量上限，需要扩大容量
    if (token_list->count >= token_list->capacity) {
        token_list->capacity *= 2;
        // 使用 realloc 调整 tokens 的内存大小以适应新的容量
        TokenPtr *new_tokens = (TokenPtr *)realloc(token_list->tokens, token_list->capacity * sizeof(TokenPtr));
        // 注意：
        // 内存泄漏：如果realloc失败并返回NULL，原始指针将不会被释放。
        // 为了避免内存泄漏，我们需要在使用realloc之前保存原始指针的副本，以便在realloc失败时释放原始内存
        // 数据丢失：如果realloc分配了一个新的内存块并将数据从旧内存块复制到新内存块，那么在分配过程中，指向旧内存块的其他指针将变得无效
        // 这可能导致数据丢失或程序崩溃。为了避免这个问题，确保在realloc之后更新所有指向原始内存块的指针
        if (new_tokens == NULL) {
            // 当realloc失败时，释放原始内存并处理错误，例如退出程序或返回错误代码
            free(token_list->tokens);
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        } else {
            token_list->tokens = new_tokens;
        }
    }
    token_list->tokens[token_list->count++] = token;
}

Token *get_token_from_list(TokenList *token_list, int index) {
    if (index < 0 || index >= token_list->count) {
        return NULL;
    }
    return token_list->tokens[index];
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_identifier_start(char c) {
    return is_alpha(c) || c == '_';
}

bool is_identifier_part(char c) {
    return is_alpha(c) || is_digit(c) || c == '_';
}

TokenList *tokenize(const char *code) {
    // 创建一个TokenList
    TokenList *token_list = create_token_list();
    int index = 0;
    int length = strlen(code);

    // 遍历输入的代码字符串
    while (index < length) {
        char c = code[index];

        // 跳过空白字符
        if (is_whitespace(c)) {
            index++;
            continue;
        }

        // 处理整数字面量
        if (is_digit(c)) {
            int start = index;
            while (index < length && is_digit(code[index])) {
                index++;
            }
            char buffer[MAX_TOKEN_TEXT_LENGTH];
            strncpy(buffer, code + start, index - start);
            buffer[index - start] = '\0';
            add_token_to_list(token_list, create_token(TokenType_IntLiteral, buffer));
            continue;
        }

        // 处理标识符
        if (is_identifier_start(c)) {
            int start = index;
            while (index < length && is_identifier_part(code[index])) {
                index++;
            }
            char buffer[MAX_TOKEN_TEXT_LENGTH];
            strncpy(buffer, code + start, index - start);
            buffer[index - start] = '\0';
            add_token_to_list(token_list, create_token(TokenType_Identifier, buffer));
            continue;
        }

        // 处理其他符号
        switch (c) {
            case '+':
                add_token_to_list(token_list, create_token(TokenType_Plus, "+"));
                index++;
                break;
            case '-':
                add_token_to_list(token_list, create_token(TokenType_Minus, "-"));
                index++;
                break;
            case '*':
                add_token_to_list(token_list, create_token(TokenType_Star, "*"));
                index++;
                break;
            case '/':
                // 可能是 // .... \n 这样的注释
                if(code[index+1] == '/'){
                    int start = index;
                    while (start < length && code[index] != '\n'){
                        index++;
                    }
                    char buffer[MAX_TOKEN_TEXT_LENGTH];
                    strncpy(buffer,code + start,index - start);
                    buffer[index -start] = '\0';
                    add_token_to_list(token_list, create_token(TokenType_Comment_Slash,buffer));
                }else{
                    add_token_to_list(token_list, create_token(TokenType_Slash, "/"));
                    index++;
                }
                break;
            case ';':
                add_token_to_list(token_list, create_token(TokenType_SemiColon, ";"));
                index++;
                break;
            case '(':
                add_token_to_list(token_list, create_token(TokenType_LeftParen, "("));
                index++;
                break;
            case ')':
                add_token_to_list(token_list, create_token(TokenType_RightParen, ")"));
                index++;
                break;
            case '=':
                if (index + 1 < length && code[index + 1] == '=') {
                    add_token_to_list(token_list, create_token(TokenType_Assignment, "=="));
                    index += 2;
                } else {
                    add_token_to_list(token_list, create_token(TokenType_Assignment, "="));
                    index++;
                }
                break;
            case '>':
                if (index + 1 < length && code[index + 1] == '=') {
                    add_token_to_list(token_list, create_token(TokenType_GreaterEqual, ">="));
                    index += 2;
                } else {
                    add_token_to_list(token_list, create_token(TokenType_Greater, ">"));
                    index++;
                }
                break;
            case '<':
                if (index + 1 < length && code[index + 1] == '=') {
                    add_token_to_list(token_list, create_token(TokenType_LessEqual, "<="));
                    index += 2;
                } else {
                    add_token_to_list(token_list, create_token(TokenType_Less, "<"));
                    index++;
                }
                break;
            case '!':
                if (index + 1 < length && code[index + 1] == '=') {
                    add_token_to_list(token_list, create_token(TokenType_NotEqual, "!="));
                    index += 2;
                } else {
                    printf("Unexpected character: %c\n", c);
                    exit(EXIT_FAILURE);
                }
                break;
            case '"':
                {
                    int start = index + 1;
                    index++;
                    // 在解析字符串时，它会检查反斜杠，并跳过紧跟在反斜杠后面的双引号或单引号
                    // 这样，你可以在字符串中使用\"和\'，而不会导致字符串提前结束
                    while (index < length && (code[index] != '"' || code[index - 1] == '\\')) {
                        index++;
                    }
                    if (index == length) {
                        printf("未关闭的双引号\n");
                        exit(EXIT_FAILURE);
                    }
                    char buffer[MAX_TOKEN_TEXT_LENGTH];
                    int buf_index = 0;
                    for (int i = start; i < index; i++) {
                        if (code[i] == '\\' && (i + 1 < index) && code[i + 1] == '"') {
                            continue;
                        }
                        buffer[buf_index++] = code[i];
                    }
                    buffer[buf_index] = '\0';
                    add_token_to_list(token_list, create_token(TokenType_DoubleQuote, buffer));
                    index++;
                }
                break;

            case '\'':
                {
                    int start = index + 1;
                    index++;
                    // 在解析字符串时，它会检查反斜杠，并跳过紧跟在反斜杠后面的双引号或单引号
                    // 这样，你可以在字符串中使用\"和\'，而不会导致字符串提前结束
                    while (index < length && (code[index] != '\'' || code[index - 1] == '\\')) {
                        index++;
                    }
                    if (index == length) {
                        printf("未关闭的单引号\n");
                        exit(EXIT_FAILURE);
                    }
                    char buffer[MAX_TOKEN_TEXT_LENGTH];
                    int buf_index = 0;
                    for (int i = start; i < index; i++) {
                        if (code[i] == '\\' && (i + 1 < index) && code[i + 1] == '\'') {
                            continue;
                        }
                        buffer[buf_index++] = code[i];
                    }
                    buffer[buf_index] = '\0';
                    add_token_to_list(token_list, create_token(TokenType_SingleQuote, buffer));
                    index++;
                }
                break;

            default:
                printf("Unexpected character: %c\n", c);
                exit(EXIT_FAILURE);
        }
    }

    // 在末尾添加EOF（文件结束）Token
    add_token_to_list(token_list, create_token(TokenType_EOF, "EOF"));
    return token_list;
}

void print_tokens(const TokenList *token_list) {
    printf("tokens :\n");
    for (int i = 0; i < token_list->count; i++) {
        Token *token = get_token_from_list(token_list, i);
        printf("type : %d, context : %s\n", token->type, token->text);
    }
}

typedef enum {
    NODE_TYPE_LITERAL,
    NODE_TYPE_BINARY_OPERATOR
} NodeType; // 节点的类型

typedef struct Node {
    NodeType type;
} Node;

typedef struct {
    Node base;
    int value;
} LiteralNode;

typedef struct {
    Node base;
    Node *left;
    Node *right;
    TokenType operator;
} BinaryOperatorNode;

Node *factor(TokenList *token_list, int *token_index);
Node *expression(TokenList *token_list, int *index);
Node *term(TokenList *token_list, int *index);

// 定义 factor 函数，接收 TokenList 和 token_index 指针作为参数
Node *factor(TokenList *token_list, int *token_index) {
    // 获取当前 token
    Token *token = get_token_from_list(token_list, *token_index);
    // 跳过注释节点
    while (token->type == TokenType_Comment_Slash){
        (*token_index)++;
        token = get_token_from_list(token_list, *token_index);
    }

    // 定义一个 Node 指针变量，用于存储处理后的节点
    Node *node = NULL;
    // 如果当前 token 是整数字面量
    if (token->type == TokenType_IntLiteral) {
        // 移动到下一个 token
        (*token_index)++;
        // 创建一个 LiteralNode 节点，用于存储整数值
        LiteralNode *literal_node = (LiteralNode *)malloc(sizeof(LiteralNode));
        // 设置节点类型为字面量
        literal_node->base.type = NODE_TYPE_LITERAL;
        // 将 token 文本转换为整数值并存储在节点中
        literal_node->value = atoi(token->text);
        // 将创建的 LiteralNode 节点转换为 Node 类型
        node = (Node *)literal_node;
    }
    // 如果当前 token 是左括号
    else if (token->type == TokenType_LeftParen) {
        // 跳过左括号，移动到下一个 token
        (*token_index)++;
        // 处理括号内的表达式，并获取对应的节点
        node = expression(token_list, token_index);
        // 获取下一个 token
        token = get_token_from_list(token_list, *token_index);
        // 如果下一个 token 不是右括号，说明括号不匹配
        if (token->type != TokenType_RightParen) {
            printf("Mismatched parenthesis\n");
            exit(EXIT_FAILURE);
        }
        // 跳过右括号，移动到下一个 token
        (*token_index)++;
    }
    // 如果遇到意外的 token 类型
    else {
        printf("Unexpected token in factor: %d\n", token->type);
        exit(EXIT_FAILURE);
    }
    // 返回处理后的节点
    return node;
}

// 定义 term 函数，接收 TokenList 和 token_index 指针作为参数
Node *term(TokenList *token_list, int *token_index) {
    // 首先解析一个 factor（数字或括号内的表达式）
    Node *left = factor(token_list, token_index);

    // 获取当前 token
    Token *token = get_token_from_list(token_list, *token_index);

    // 跳过注释节点
    while (token->type == TokenType_Comment_Slash){
        (*token_index)++;
        token = get_token_from_list(token_list, *token_index);
    }

    // 如果当前 token 是乘法或除法操作符，继续循环
    while (token->type == TokenType_Star || token->type == TokenType_Slash) {
        // 移动到下一个 token
        (*token_index)++;

        // 解析下一个 factor
        Node *right = factor(token_list, token_index);

        // 创建一个新的二元操作符节点用于存储乘法或除法操作
        BinaryOperatorNode *node = (BinaryOperatorNode *)malloc(sizeof(BinaryOperatorNode));

        // 设置节点类型为二元操作符
        node->base.type = NODE_TYPE_BINARY_OPERATOR;

        // 设置节点的左子树和右子树
        node->left = left;
        node->right = right;

        // 设置节点的操作符
        node->operator = token->type;

        // 更新 left 节点，以便在下一次循环中处理连续的乘法和除法操作
        left = (Node *)node;

        // 获取下一个 token
        token = get_token_from_list(token_list, *token_index);
    }

    // 返回处理后的节点
    return left;
}

// EBNF 描述算法
// expression ::= term (additive_op term)*
// term ::= factor (multiplicative_op factor)*
// factor ::= integer | '(' expression ')'
// additive_op ::= '+' | '-'
// multiplicative_op ::= '*' | '/'
// integer ::= digit+
//            digit ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
Node *expression(TokenList *token_list, int *token_index) {
    // 首先调用 term 函数解析乘法和除法表达式
    Node *left = term(token_list, token_index);
    // 获取当前 token
    Token *token = get_token_from_list(token_list, *token_index);

    // 跳过注释节点
    while (token->type == TokenType_Comment_Slash){
        (*token_index)++;
        token = get_token_from_list(token_list, *token_index);
    }

    // 当 token 类型为 TokenType_Plus 或 TokenType_Minus 时，表示有加法或减法运算
    while (token->type == TokenType_Plus || token->type == TokenType_Minus) {
        // token_index 自增
        (*token_index)++;
        // 解析右侧的 term
        Node *right = term(token_list, token_index);
        // 创建一个 BinaryOperatorNode 结构体存储二元操作符信息
        BinaryOperatorNode *node = (BinaryOperatorNode *)malloc(sizeof(BinaryOperatorNode));
        node->base.type = NODE_TYPE_BINARY_OPERATOR;
        node->left = left;
        node->right = right;
        node->operator = token->type;
        // 将新创建的二元操作节点作为左侧节点，继续进行下一次循环，处理连续的加法和减法运算
        left = (Node *)node;

        // 获取下一个 token
        token = get_token_from_list(token_list, *token_index);
    }
    // 返回构建好的 AST 节点
    return left;
}

int evaluate_ast(Node *node) {
    if (node == NULL) {
        return 0;
    }

    switch (node->type) {
        case NODE_TYPE_LITERAL: {
            LiteralNode *literal_node = (LiteralNode *)node;
            return literal_node->value;
        }
        case NODE_TYPE_BINARY_OPERATOR: {
            BinaryOperatorNode *binary_operator_node = (BinaryOperatorNode *)node;
            int left_value = evaluate_ast(binary_operator_node->left);
            int right_value = evaluate_ast(binary_operator_node->right);

            switch (binary_operator_node->operator) {
                case TokenType_Plus:
                    return left_value + right_value;
                case TokenType_Minus:
                    return left_value - right_value;
                case TokenType_Star:
                    return left_value * right_value;
                case TokenType_Slash:
                    return left_value / right_value;
                default:
                    printf("Unexpected operator: %d\n", binary_operator_node->operator);
                    exit(EXIT_FAILURE);
            }
        }
        default:
            printf("Unexpected node type: %d\n", node->type);
            exit(EXIT_FAILURE);
    }
}

void print_ast(const Node *node, int indent) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (node->type) {
        case NODE_TYPE_LITERAL: {
            const LiteralNode *literal_node = (const LiteralNode *)node;
            printf("Num: %d\n", literal_node->value);
            break;
        }
        case NODE_TYPE_BINARY_OPERATOR: {
            const BinaryOperatorNode *binary_operator_node = (const BinaryOperatorNode *)node;
            printf("Opr: ");
            switch (binary_operator_node->operator) {
                case TokenType_Plus:
                    printf("+\n");
                    break;
                case TokenType_Minus:
                    printf("-\n");
                    break;
                case TokenType_Star:
                    printf("*\n");
                    break;
                case TokenType_Slash:
                    printf("/\n");
                    break;
                default:
                    printf("Unexpected operator: %d\n", binary_operator_node->operator);
                    exit(EXIT_FAILURE);
            }
            print_ast(binary_operator_node->left, indent + 1);
            print_ast(binary_operator_node->right, indent + 1);
            break;
        }
        default:
            printf("Unexpected node type: %d\n", node->type);
            exit(EXIT_FAILURE);
    }
}

void free_ast(Node *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_TYPE_LITERAL: {
            LiteralNode *literal_node = (LiteralNode *)node;
            free(literal_node);
            break;
        }
        case NODE_TYPE_BINARY_OPERATOR: {
            BinaryOperatorNode *binary_operator_node = (BinaryOperatorNode *)node;
            free_ast(binary_operator_node->left);
            free_ast(binary_operator_node->right);
            free(binary_operator_node);
            break;
        }
        default:
            printf("Unexpected node type: %d\n", node->type);
            exit(EXIT_FAILURE);
    }
}

int main() {
    const char *expr = "// compute 中文 expr\n// author by link\n1 + 2 * (3 + 4) - 5;";
    TokenList *token_list = tokenize(expr);
    printf("source code :\n%s\n", expr);
//    print_tokens(token_list);

    int token_index = 0;
    Node *ast = expression(token_list, &token_index);
    printf("AST:\n");
    print_ast(ast, 0);

    int result = evaluate_ast(ast);
    printf("The result is: %d\n", result);

    destroy_token_list(token_list);
    free_ast(ast);

    return 0;
}
