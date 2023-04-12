#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKEN_TEXT_LENGTH 256
#define INITIAL_TOKEN_LIST_CAPACITY 30

typedef enum {
    TokenType_Identifier,   // 表示标识符（如变量名和函数名）
    TokenType_IntLiteral,   // 表示整数字面量（如 42）
    TokenType_Plus,         // 表示加号运算符（+）
    TokenType_Minus,        // 表示减号运算符（-）
    TokenType_Star,         // 表示乘号运算符（*）
    TokenType_Slash,        // 表示除号运算符（/）
    TokenType_SemiColon,    // 表示分号（;）
    TokenType_LeftParen,    // 表示左括号（(）
    TokenType_RightParen,   // 表示右括号（)）
    TokenType_Assignment,   // 表示赋值运算符（=）
    TokenType_Greater,      // 表示大于号（>）
    TokenType_GreaterEqual, // 表示大于等于号（>=）
    TokenType_LessEqual,    // 表示小于等于号（<=）
    TokenType_Less,         // 表示小于号（<）
    TokenType_NotEqual,     // 表示不等于号（!=）
    TokenType_EOF,           // 表示文件结束或输入结束
    TokenType_DoubleQuote, // 添加双引号类型
    TokenType_SingleQuote, // 添加单引号类型
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
                add_token_to_list(token_list, create_token(TokenType_Slash, "/"));
                index++;
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

int expression(TokenList *token_list, int *index);
int term(TokenList *token_list, int *index);
int factor(TokenList *token_list, int *index);

int factor(TokenList *token_list, int *index) {
    Token *t = token_list->tokens[*index];
    int value;

    if (t->type == TokenType_IntLiteral) {
        value = atoi(t->text);
        (*index)++;
    } else if (t->type == TokenType_LeftParen) {
        (*index)++;
        value = expression(token_list, index);
        if (token_list->tokens[*index]->type == TokenType_RightParen) {
            (*index)++;
        } else {
            printf("Mismatched parenthesis\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Unexpected token type: %d\n", t->type);
        exit(EXIT_FAILURE);
    }

    return value;
}

int term(TokenList *token_list, int *index) {
    int left = factor(token_list, index);

    while (token_list->tokens[*index]->type == TokenType_Star || token_list->tokens[*index]->type == TokenType_Slash) {
        Token *op = token_list->tokens[*index];
        (*index)++;
        int right = factor(token_list, index);
        if (op->type == TokenType_Star) {
            left *= right;
        } else {
            left /= right;
        }
    }

    return left;
}

int expression(TokenList *token_list, int *index) {
    int left = term(token_list, index);

    while (token_list->tokens[*index]->type == TokenType_Plus || token_list->tokens[*index]->type == TokenType_Minus) {
        Token *op = token_list->tokens[*index];
        (*index)++;
        int right = term(token_list, index);
        if (op->type == TokenType_Plus) {
            left += right;
        } else {
            left -= right;
        }
    }

    return left;
}

int main() {
    const char *expr = "1 + 2 * (3 + 4) - 5";
    TokenList *token_list = tokenize(expr);
    printf("source code :\n%s\n", expr);
    print_tokens(token_list);

    int index = 0;
    int result = expression(token_list, &index);
    printf("Expression result: %d\n", result);

    destroy_token_list(token_list);
    return 0;
}
