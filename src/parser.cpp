#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "lexer.h"
#include "parser.h"

/* If i ever hit the github push before i fix this, just saying, this is pure shitty code, i will rewrite it later,
with comments and everything, when i get back from programming vacation, please do not look. */

using namespace std;

unordered_map<string, int> getOperatorPrecedenceMap() {
    return {
        {"+", 1},
        {"-", 1},
        {"*", 2},
        {"/", 2},
        {"^", 3},
    };
}

bool isOperator(const Token& token) {
    return token.type == TokenType::OPERATOR;
}

bool isKeyword(const Token& token, const std::string& keyword) {
    return token.type == TokenType::KEYWORD && token.value == keyword;
}

bool isDelimiter(const Token& token, const std::string& delimiter) {
    return token.type == TokenType::DELIMITER && token.value == delimiter;
}

bool isNumber(const Token& token) {
    return token.type == TokenType::NUMBER;
}

bool isVariable(const Token& token) {
    return token.type == TokenType::VARIABLE;
}

Expression* createExpression(ExpressionType type, const std::string& value) {
    Expression* expr = new Expression();
    expr->type = type;
    expr->value = value;
    return expr;
}

Expression* parseExpression(const vector<Token>& tokens, size_t& pos, int precedence) {
    Expression* left = parsePrimaryExpression(tokens, pos);

    while (pos < tokens.size()) {
        Token token = tokens[pos];

        if (isKeyword(token, "if")) {
            pos++;
            Expression* ifStmt = parseIfStatement(tokens, pos);
            return ifStmt;
        } else if (isOperator(token) && token.value == "=") {
            pos++;
            Expression* right = parseExpression(tokens, pos, 0);
            Expression* expr = createExpression(ExpressionType::Assignment, right->value);
            expr->left = left;
            expr->right = right;
            left = expr;
        } else if (isOperator(token)) {
            int opPrecedence = getOperatorPrecedenceMap()[token.value];
            if (opPrecedence < precedence) {
                break;
            }

            pos++;
            Expression* right = parseExpression(tokens, pos, opPrecedence + 1);

            Expression* expr = createExpression(ExpressionType::BinaryOp, token.value);
            expr->left = left;
            expr->right = right;
            left = expr;
        } else {
            break;
        }
    }

    return left;
}


Expression* parsePrimaryExpression(const vector<Token>& tokens, size_t& pos) {
    Token token = tokens[pos];

    if (isOperator(token) && (token.value == "!" || token.value == "-")) {
        pos++;
        Expression* expr = createExpression(ExpressionType::UnaryOp, token.value);
        expr->left = parsePrimaryExpression(tokens, pos);
        return expr;
    } else if (isNumber(token)) {
        pos++;
        return createExpression(ExpressionType::Literal, token.value);
    } else if (isKeyword(token, "if")) {
        pos++;
        return createExpression(ExpressionType::Keyword, token.value);
    } else if (isVariable(token)) {
        pos++;
        return createExpression(ExpressionType::Variable, token.value);
    } else if (isDelimiter(token, "(")) {
        pos++;
        Expression* expr = parseExpression(tokens, pos, 0);
        if (isDelimiter(tokens[pos], ")")) {
            pos++;
        }
        return expr;
    }

    return nullptr;
}

Expression* parseIfStatement(const std::vector<Token>& tokens, size_t& pos) {
    Token token = tokens[pos];
    if (token.type != TokenType::KEYWORD || token.value != "if") {
        return nullptr; // Если это не if, возвращаем null
    }

    pos++; // Пропускаем "if"
    
    // Проверяем "("
    if (tokens[pos].type != TokenType::DELIMITER || tokens[pos].value != "(") {
        throw std::runtime_error("Expected '(' after 'if'");
    }

    pos++; // Пропускаем "("

    // Парсим условие (например, переменная a > b)
    Expression* condition = parseExpression(tokens, pos, 0);
    
    // Проверка закрывающей скобки ")"
    if (tokens[pos].type != TokenType::DELIMITER || tokens[pos].value != ")") {
        throw std::runtime_error("Expected ')' after condition");
    }

    pos++; // Пропускаем ")"
    
    // Проверяем на открывающую фигурную скобку "{"
    if (tokens[pos].type != TokenType::DELIMITER || tokens[pos].value != "{") {
        throw std::runtime_error("Expected '{' for block after 'if' condition");
    }

    pos++; // Пропускаем "{"
    
    // Парсим блок кода внутри if
    std::vector<Expression*> ifBlock = parse(tokens); // Здесь вызываем парсер для тела if
    
    // Проверка на "else"
    std::vector<Expression*> elseBlock;
    if (tokens[pos].type == TokenType::KEYWORD && tokens[pos].value == "else") {
        pos++; // Пропускаем "else"
        
        // Проверка на открывающую фигурную скобку для блока else
        if (tokens[pos].type != TokenType::DELIMITER || tokens[pos].value != "{") {
            throw std::runtime_error("Expected '{' after 'else'");
        }

        pos++; // Пропускаем "{"
        
        // Парсим блок кода внутри else
        elseBlock = parse(tokens); // Парсим блок для else
    }

    // Возвращаем объект IfStatementExpression с условием и блоками if и else
    return new IfStatementExpression(condition, ifBlock, elseBlock);
}

Program parseProgram(const vector<Token>& tokens) {
    Program program;
    size_t pos = 0;

    while (pos < tokens.size()) {
        Expression* expr = parseExpression(tokens, pos, 0);
        program.expressions.push_back(expr);

        if (pos < tokens.size() && isDelimiter(tokens[pos], ";")) {
            pos++;
        } else {
            break;
        }
    }

    return program;
}

vector<Expression*> parse(const vector<Token>& tokens) {
    vector<Expression*> expressions;
    size_t pos = 0;

    while (pos < tokens.size()) {
        Expression* expr = parseExpression(tokens, pos, 0);
        expressions.push_back(expr);

        if (pos < tokens.size() && isDelimiter(tokens[pos], ";")) {
            pos++;
        } else {
            break;
        }
    }

    return expressions;
}

void printExpression(Expression* expr, const std::string& indent) {
    if (expr == nullptr) return;
    switch (expr->type) {
        case ExpressionType::Literal:
            std::cout << indent << "Literal: " << expr->value << std::endl;
            break;
        case ExpressionType::Variable:
            std::cout << indent << "Variable: " << expr->value << std::endl;
            break;
        case ExpressionType::BinaryOp:
            std::cout << indent << "BinaryOp: " << expr->value << std::endl;
            printExpression(expr->left, indent + "   ");
            printExpression(expr->right, indent + "   ");
            break;
        case ExpressionType::Assignment:
            std::cout << indent << "Assignment: =" << std::endl;
            printExpression(expr->left, indent + "   ");
            printExpression(expr->right, indent + "   ");
            break;
        case ExpressionType::UnaryOp:
            std::cout << indent << "UnaryOp: " << expr->value << std::endl;
            printExpression(expr->left, indent + "   ");
            break;
        case ExpressionType::Keyword:
            std::cout << indent << "Keyword: " << expr->value << std::endl;
            break;
        case ExpressionType::IfStatement: {
            auto ifStmt = static_cast<IfStatementExpression*>(expr);
            std::cout << indent << "IfStatement" << std::endl;
            std::cout << indent << "   Condition:" << std::endl;
            printExpression(ifStmt->condition, indent + "   ");
            std::cout << indent << "   If block:" << std::endl;
            for (auto& subExpr : ifStmt->ifBlock) {
                printExpression(subExpr, indent + "   ");
            }
            if (!ifStmt->elseBlock.empty()) {
                std::cout << indent << "   Else block:" << std::endl;
                for (auto& subExpr : ifStmt->elseBlock) {
                    printExpression(subExpr, indent + "   ");
                }
            }
            break;
        }
        default:
            std::cout << indent << "Unknown expression type" << std::endl;
            break;
    }
}

void printProgram(const Program& program) {
    cout << "Program:" << endl;
    for (auto expr : program.expressions) {
        printExpression(expr, "|--");
        cout << endl;
    }
}

