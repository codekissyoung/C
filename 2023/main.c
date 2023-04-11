#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "tree_node.h"

#define MAX_TOKEN_TEXT_LENGTH 256
#define INITIAL_TOKEN_LIST_CAPACITY 30

typedef enum {
    TokenType_Identifier,
    TokenType_IntLiteral,
    TokenType_Plus,
    TokenType_Minus,
    TokenType_Star,
    TokenType_Slash,
    TokenType_SemiColon,
    TokenType_LeftParen,
    TokenType_RightParen,
    TokenType_Assignment,
    TokenType_Greater,
    TokenType_GreaterEqual,
    TokenType_LessEqual,
    TokenType_Less,
    TokenType_NotEqual,
    TokenType_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[MAX_TOKEN_TEXT_LENGTH];
} Token;

typedef struct {
    int count;
    int capacity;
    Token **tokens;
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

void add_token_to_list(TokenList *token_list, Token *token) {
    if (token_list->count >= token_list->capacity) {
        token_list->capacity *= 2;
        token_list->tokens = (Token **)realloc(token_list->tokens, token_list->capacity * sizeof(Token *));
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
    TokenList *token_list = create_token_list();
    int index = 0;
    int length = strlen(code);

    while (index < length) {
        char c = code[index];

        if (is_whitespace(c)) {
            index++;
            continue;
        }

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
            default:
                printf("Unexpected character: %c\n", c);
                exit(EXIT_FAILURE);
        }
    }

    add_token_to_list(token_list, create_token(TokenType_EOF, "EOF"));
    return token_list;
}

void print_tokens(const TokenList *token_list) {
    for (int i = 0; i < token_list->count; i++) {
        Token *token = token_list->tokens[i];
        printf("类型: %d, 文本: %s\n", token->type, token->text);
    }
}

int main() {
    const char *code = "int age = 5;";
    TokenList *token_list = tokenize(code);
    printf("解析: %s\n", code);
    print_tokens(token_list);
    destroy_token_list(token_list);
    return 0;
}
