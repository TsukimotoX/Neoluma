#include "../nodes.hpp"
#include "parser.hpp"
#include "../token.hpp"
#include "../../../HelperFunctions.hpp"
#include "ASTBuilder.hpp"
#include <iostream>
#include <unordered_set>
#include <print>
#include <typeinfo>

// ==== Print the parser output ====
void Parser::printModule(int indentation) {
    parseModule();
    if (!moduleSource) {
        std::cerr << "[Neoluma/Parser]["<< __func__ << "] No module to print.\n";
        return;
    }

    std::println("{}", moduleSource->toString(indentation));
}

// ==== Main parsing ====
void Parser::parseModule() {
    auto moduleNode = ASTBuilder::createModule(moduleName);
    auto dn = getDelimeterNames();

    while (!isAtEnd()) {
        if (curToken().type == TokenType::Delimeter && curToken().value == dn[Delimeters::Semicolon]) { next(); continue; }

        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse statement in module {} (L{}:{}, char '{}')", __func__, moduleName, curToken().line, curToken().column, curToken().value);
            break;
        }

        moduleNode->body.push_back(std::move(stmt));
    }

    moduleSource = std::move(moduleNode);
}

// ==== Statement parsing ====
MemoryPtr<ASTNode> Parser::parseStatement() {
    while (match(TokenType::Delimeter) && isNextLine()) { // Skips newlines in case they ever appear
        next();
    }

    Token token = curToken();
    auto km = getKeywordMap();
    auto dm = getDelimeterMap();
    auto dn = getDelimeterNames();
    auto om = getOperatorMap();

    std::vector<MemoryPtr<CallExpressionNode>> decorators = {};

    if (match(TokenType::Preprocessor)) return parsePreprocessor();
    if (match(TokenType::Decorator)) decorators = parseDecoratorCalls();

    auto modifiers = parseModifiers();

    if (match(TokenType::Identifier) && (om[lookupNext().value] == Operators::Nullable || dm[lookupNext().value] == Delimeters::Colon))
        return parseDeclaration();

    token = curToken();
    // === Control Flow Keywords ===
    if (token.type == TokenType::Keyword) {
        if (km[token.value] == Keywords::If) return parseIf();
        if (km[token.value] == Keywords::Switch) return parseSwitch();
        if (km[token.value] == Keywords::Try) return parseTryCatch();
        if (km[token.value] == Keywords::For) return parseFor();
        if (km[token.value] == Keywords::While) return parseWhile();
        if (km[token.value] == Keywords::Return) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected expression after 'return' (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            return ASTBuilder::createReturnStatement(std::move(expr));
        }
        if (km[token.value] == Keywords::Throw) {
            next();
            MemoryPtr<ASTNode> expr = parseExpression();
            if (!expr) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected expression after 'throw' (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            return ASTBuilder::createThrowStatement(std::move(expr));
        }
        if (km[token.value] == Keywords::Break) return ASTBuilder::createBreakStatement();
        if (km[token.value] == Keywords::Continue) return ASTBuilder::createContinueStatement();

        // for modifier affected structures
        if (km[token.value] == Keywords::Function) return parseFunction(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Class) return parseClass(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Enum) return parseEnum(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Interface) return parseInterface(std::move(decorators), std::move(modifiers));
        if (km[token.value] == Keywords::Decorator) return parseDecorator(std::move(decorators), std::move(modifiers));
        if (!modifiers.empty()) { std::println(std::cerr, "[Neoluma/Parser][{}] Unexpected modifier before {} (L{}:{})", __func__, token.value, token.line, token.column); }
    }

    // === Block ===
    if (match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        return parseBlock();
    }

    // === Fallback ===
    return parseExpression(); // Default to expression statement
}

// ==== Expression parsing ====
MemoryPtr<ASTNode> Parser::parseExpression() {
    Token token = curToken();
    auto om = getOperatorMap();
    auto dm = getDelimeterMap();

    // Unary operations
    if (om.find(token.value) != om.end()) {
        Operators op = om[token.value];
        if (op == Operators::LogicalNot || op == Operators::Subtract) {
            next();
            return parseUnary(token.value);
        }
    }

    // Assignment
    if (match(TokenType::Identifier) && isAssignableAhead(0)) {
        return parseAssignment();
    }

    // Fallback to binary
    return parseBinary(0);
}

MemoryPtr<ASTNode> Parser::parseBinary(int prevPredecence) {
    MemoryPtr<ASTNode> left = parsePrimary();
    if (!left) return nullptr;
    //std::println("Parsed left: {}", left->toString());

    while (true) {
        Token token = curToken();
        int predecence = getOperatorPrecedence(token.value);
        if (token.type != TokenType::Operator || predecence < prevPredecence) break;
        std::string op = token.value;
        next();
        //std::println("Current operator: {}", op);

        MemoryPtr<ASTNode> right = parseBinary(predecence+1);
        if (!right) return nullptr;

        //std::println("Parsed right: {}", right->toString());

        left = ASTBuilder::createBinaryOperation(std::move(left), op, std::move(right));
    }

    //std::println("Result: {}", left->toString());
    return left;
}

MemoryPtr<UnaryOperationNode> Parser::parseUnary(const std::string& op) {
    MemoryPtr<ASTNode> operand = parsePrimary();
    if (!operand) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Missing operand after unary operator: {} (L{}:{})", __func__, op, curToken().line, curToken().column);
        return nullptr;
    }
    return ASTBuilder::createUnaryOperation(op, std::move(operand));
}

MemoryPtr<ASTNode> Parser::parsePrimary() {
    Token token = curToken();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    // Parenthesis, lambdas, arrays, sets, dicts
    if (token.type == TokenType::Delimeter) {
        // Parenthesis / lambdas
        if (token.value == dn[Delimeters::LeftParen]) {
            next();
            std::vector<MemoryPtr<ASTNode>> exprs;
            while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                MemoryPtr<ASTNode> expr = parseExpression();
                if (!expr) {
                    std::println(std::cerr, "[Neoluma/Parser][{}] Expected expression after '(' in lambda expression (L{}:{})", __func__, token.line, token.column);
                    return nullptr;
                }
                exprs.push_back(std::move(expr));
                if (curToken().type == TokenType::Delimeter && isNextLine()) next();
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after expressions in lambda expression (L{}:{})", __func__, token.line, token.column);
                return nullptr;
            }
            next();
            // Check for lambda expressions here
            if (match(TokenType::Operator, on[Operators::AssignmentArrow])) {
                next();
                auto block = parseBlock();
                if (!block) {
                    std::println(std::cerr, "[Neoluma/Parser][{}] Could not find a block of code after assignment arrow. (L{}:{})", __func__, token.line, token.column);
                    return nullptr;
                }
                return ASTBuilder::createLambda(std::move(exprs), std::move(block));
            }
            return std::move(exprs[0]);
        }
        // Arrays
        if (token.value == dn[Delimeters::LeftBracket]) {
            next();
            std::vector<MemoryPtr<ASTNode>> e;

            while (!match(TokenType::Delimeter, dn[Delimeters::RightBracket])) {
                e.push_back(parseExpression());
                if (curToken().type == TokenType::Delimeter && isNextLine()) next(); // To allow multiline expressions of arrays. I think this would be absolutely neat sugar for everybody.
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
            }
            next();
            return ASTBuilder::createArray(std::move(e)/*, typeHint miss*/);
        }
        // Sets / dicts
        if (token.value == dn[Delimeters::LeftBraces]) {
            next();
            bool isDict = (lookupNext().type == TokenType::Delimeter && lookupNext().value == dn[Delimeters::Colon]);

            if (isDict) {
                std::vector<std::pair<MemoryPtr<ASTNode>, MemoryPtr<ASTNode>>> e;
                while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                    auto key = parseExpression();
                    next();
                    if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
                        std::println(std::cerr, "[Neoluma/Parser][{}] Could not find colon after key '{}' (L{}:{})", __func__, key->value, token.line, token.column);
                        return nullptr;
                    }
                    next();
                    auto val = parseExpression();
                    e.push_back({std::move(key), std::move(val)});
                    if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                    else break;
                }
                return ASTBuilder::createDict(std::move(e)/*, typeHint miss*/);
            }

            std::vector<MemoryPtr<ASTNode>> e;
            while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                e.push_back(parseExpression());
                next(); // todo: check if needed later
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            next();
            return ASTBuilder::createSet(std::move(e)/*, typeHint miss*/);
        }
        if (isNextLine()) {
            next();
            //std::println("skip newline L{}:{}", token.line, token.column);
            return nullptr;
        }
    }
    // Data type + Booleans
    else if ((token.type == TokenType::Number || token.type == TokenType::String)
    || (token.type == TokenType::Identifier && (token.value == "true" || token.value == "false"))) {
        next();
        return ASTBuilder::createLiteral(token.value);
    }
    // Null
    else if (token.type == TokenType::Null) {
        next();
        return ASTBuilder::createLiteral("null");
    }

    // Identifier/variable or function call
    else if (token.type == TokenType::Identifier) {
        Token id = next();
        MemoryPtr<ASTNode> node;
        //std::println("{}", id.value);

        // If function call
        if (match(TokenType::Delimeter, dn[Delimeters::LeftParen])) {
            next();

            //std::println("Assuming it's a function call");
            std::vector<MemoryPtr<ASTNode>> args;

            while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                auto arg = parseExpression();
                if (arg) args.push_back(std::move(arg));
                //std::println("Parsed {}", args.back()->value);

                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            //std::println("I closed the args with ')', current token: {}", curToken().value);
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after arguments (L{}:{})", __func__, token.line, token.column);
                return nullptr;
            }
            next();

            auto callee = ASTBuilder::createVariable(id.value);
            node = ASTBuilder::createCallExpression(std::move(callee), std::move(args));
        } else {
            // else identifier
            node = ASTBuilder::createVariable(id.value);
        }

        while (match(TokenType::Delimeter, dn[Delimeters::Dot])) {
            next();
            MemoryPtr<ASTNode> parent = std::move(node);

            if (!match(TokenType::Identifier)) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected identifier after '.' (L{}:{})", __func__, token.line, token.column);
                return nullptr;
            }
            MemoryPtr<ASTNode> member = parsePrimary();
            if (member->type != ASTNodeType::Variable && member->type != ASTNodeType::CallExpression) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected variable or function call after '.' (L{}:{})", __func__, token.line, token.column);
                return nullptr;
            }
            node = ASTBuilder::createMemberAccess(std::move(parent), std::move(member));
        }
        return node;
    }

    std::println(std::cerr, "[Neoluma/Parser][{}] Unexpected token in primary expression: {} (L{}:{})", __func__, token.value, token.line, token.column);
    return nullptr;
}

MemoryPtr<DeclarationNode> Parser::parseDeclaration() {
    auto om = getOperatorMap();
    auto on = getOperatorNames();
    auto dn = getDelimeterNames();

    Token token = curToken();
    if (token.type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected identifier for assignment (L{}:{})", __func__, token.line, token.column);
        return nullptr;
    }
    MemoryPtr<VariableNode> var = ASTBuilder::createVariable(token.value);
    next();

    bool isNullable = false;
    if (match(TokenType::Operator, on[Operators::Nullable])) { isNullable = true; next(); }

    std::string rawType = "None";
    if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
        std::println("[Neoluma/Parser][{}] Expected ':' after variable name in declaration (L{}:{})", __func__, token.line, token.column);
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected type after ':' in variable declaration (L{}:{})", __func__, token.line, token.column);
        return nullptr;
    }

    rawType = curToken().value;
    next();
    MemoryPtr<ASTNode> value = nullptr;
    if (match(TokenType::Operator, on[Operators::Assign])) {
        next();

        value = parseExpression();
        if (!value) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse expression after variable assignment (L{}:{})", __func__, token.line, token.column);
            return nullptr;
        }
    }

    return ASTBuilder::createDeclaration(std::move(var), rawType, std::move(value), isNullable);
}

MemoryPtr<AssignmentNode> Parser::parseAssignment() {
    auto om = getOperatorMap();
    auto on = getOperatorNames();

    MemoryPtr<ASTNode> var = parsePrimary();
    if (!var) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse left-hand side of assignment (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    if (var->type != ASTNodeType::Variable && var->type != ASTNodeType::MemberAccess) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Left-hand side of assignment must be a variable or member access (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    Token token = curToken();
    if (!match(TokenType::Operator) && !isAssignmentOperator(token.value)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected assignment operator (=, +=, -=, *=, etc.) after identifier (L{}:{})", __func__, token.line, token.column);
        return nullptr;
    }
    std::string op = token.value;
    next();

    MemoryPtr<ASTNode> value = parseExpression();
    if (!value) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse expression after assignment operator (L{}:{})", __func__, token.line, token.column);
        return nullptr;
    }

    return ASTBuilder::createAssignment(std::move(var), op, std::move(value));
}

MemoryPtr<ReturnStatementNode> Parser::parseReturn(){
    next();

    if (curToken().type == TokenType::Delimeter && isNextLine()) {
        return ASTBuilder::createReturnStatement(nullptr);
    }

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        return nullptr;
    }

    if (curToken().type != TokenType::Delimeter || !isNextLine()) {
        return nullptr;
    }
    next();

    return ASTBuilder::createReturnStatement(std::move(expr));
}

// ==== Control flow ====
MemoryPtr<IfNode> Parser::parseIf() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after 'if' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next(); // consume '('
    if (match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Empty condition in 'if' (L{}:{})", __func__, curToken().line, curToken().column);
        next(); // consume ')' because empty
        return nullptr;
    }

    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse condition expression in 'if' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after condition in 'if' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> ifBlock = parseBlockorStatement();
    MemoryPtr<ASTNode> elseBlock = nullptr;

    auto km = getKeywordNames();
    if (match(TokenType::Keyword, km[Keywords::Else])) {
        next();
        elseBlock = parseBlockorStatement();
    }

    return ASTBuilder::createIf(std::move(condition), std::move(ifBlock), std::move(elseBlock));
}

MemoryPtr<SwitchNode> Parser::parseSwitch() {
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after 'switch' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> expr = parseExpression();
    if (!expr) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse expression in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after expression in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '{{' to start 'switch' body (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    while (match(TokenType::Delimeter, dm[Delimeters::Semicolon])) next();

    std::vector<MemoryPtr<CaseNode>> cases;
    MemoryPtr<SCDefaultNode> defaultCase = nullptr;

    while (!match(TokenType::Delimeter, dm[Delimeters::RightBraces])) {
        Token tok = curToken();

        if (match(TokenType::Keyword, km[Keywords::Case])) {
            next(); // consume 'case'
            MemoryPtr<ASTNode> condition = parseExpression();
            if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected ':' after 'case' condition in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next(); // consume ':'
            auto body = parseBlockorStatement();
            if (!body) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected body after 'case' in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            while (match(TokenType::Delimeter, dm[Delimeters::Semicolon])) next();
            cases.push_back(ASTBuilder::createCase(std::move(condition), std::move(body)));
        }
        else if (match(TokenType::Keyword, km[Keywords::Default])) {
            next();
            if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected ':' after 'default' in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next();
            auto body = parseBlockorStatement();
            if (!body) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected body after 'default' in 'switch' statement (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            while (match(TokenType::Delimeter, dm[Delimeters::Semicolon])) next();
            defaultCase = ASTBuilder::createDefaultCase(std::move(body));
        }
        else {
            std::println(std::cerr, "[Neoluma/Parser][{}] Expected 'case' or 'default' in 'switch' statement but found '{}' (L{}:{})", __func__, tok.value, tok.line, tok.column);
            return nullptr;
        }
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '}}' to close 'switch' body (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    return ASTBuilder::createSwitch(std::move(expr), std::move(cases), std::move(defaultCase));
}

MemoryPtr<TryCatchNode> Parser::parseTryCatch() {
    next();
    auto km = getKeywordNames();
    auto dn = getDelimeterNames();

    auto tryBlock = parseBlock();
    if (!tryBlock) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected 'try' block (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    if (!match(TokenType::Keyword, km[Keywords::Catch])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected 'catch' after 'try' block (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    if (!match(TokenType::Delimeter, dn[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after 'catch' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected an exception variable after 'catch' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    auto varName = curToken().value;
    auto exception = ASTBuilder::createVariable(varName);
    next();

    if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after exception variable in 'catch' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    auto catchBlock = parseBlock();
    if (!catchBlock) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected 'catch' block (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    return ASTBuilder::createTryCatch(std::move(tryBlock), std::move(exception), std::move(catchBlock));
}

MemoryPtr<ForLoopNode> Parser::parseFor() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after 'for' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    if (curToken().type != TokenType::Identifier) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected variable name in 'for' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    std::string varName = curToken().value;
    auto varNode = ASTBuilder::createVariable(varName);
    next();

    if (!match(TokenType::Delimeter, dm[Delimeters::Colon])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ':' after variable in 'for' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    MemoryPtr<ASTNode> iterable = parseExpression();
    if (!iterable) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected iterable expression in 'for' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after iterable in 'for' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    auto body = parseBlock();
    if (!body) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected body in 'for' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    return ASTBuilder::createForLoop(std::move(varNode), std::move(iterable), std::move(body));
}

MemoryPtr<WhileLoopNode> Parser::parseWhile() {
    next();
    auto dm = getDelimeterNames();

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after 'while' (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    MemoryPtr<ASTNode> condition = parseExpression();
    if (!condition) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse condition expression in 'while' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after condition in 'while' statement (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    auto body = parseBlock();
    if (!body) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected body in 'while' loop (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    return ASTBuilder::createWhileLoop(std::move(condition), std::move(body));
}

// ==== Declarations ====
MemoryPtr<FunctionNode> Parser::parseFunction(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected function name (L{}:{})", __func__, nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string funcName = nameToken.value;
    next();

    if (!match(TokenType::Delimeter, dn[Delimeters::LeftParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after function name (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    std::vector<MemoryPtr<ParameterNode>> params;
    while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
        Token paramName = curToken();
        if (paramName.type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Expected parameter name (L{}:{})", __func__, paramName.line, paramName.column);
            return nullptr;
        }
        next();
        std::string type = "None";
        if (match(TokenType::Delimeter, dn[Delimeters::Colon])) {
            next();
            if (curToken().type != TokenType::Identifier) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected parameter type after ':' (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            type = curToken().value;
            next();
        }

        MemoryPtr<ASTNode> defaultValue = nullptr;
        if (match(TokenType::Operator, on[Operators::Assign])) {
            next();
            defaultValue = parseExpression();
            if (!defaultValue) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected default value expression after '=' (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
        }
        params.push_back(ASTBuilder::createParameter(paramName.value, type, std::move(defaultValue)));

        if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
        else break;
    }

    if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after function parameters (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    MemoryPtr<VariableNode> returnType = nullptr;
    if (match(TokenType::Operator, on[Operators::TypeArrow])) {
        next();
        if (curToken().type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Expected return type after '->' (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        returnType = ASTBuilder::createVariable(curToken().value);
        next();
    }

    MemoryPtr<BlockNode> body = parseBlock();
    if (!body) return nullptr;

    return ASTBuilder::createFunction(funcName, std::move(params), std::move(returnType), std::move(body), std::move(decorators), std::move(modifiers));
}

MemoryPtr<ClassNode> Parser::parseClass(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    // TODO: Add constructor parsing, it will be neat
    next();
    auto dm = getDelimeterNames();
    auto km = getKeywordNames();
    auto on = getOperatorNames();

    Token nameToken = curToken();
    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected class name (L{}:{})", __func__, nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string className = nameToken.value;
    next();

    // Checking if class is inherited
    MemoryPtr<VariableNode> super = nullptr;
    if (match(TokenType::Operator, on[Operators::InheritanceArrow])) {
        next();
        super = ASTBuilder::createVariable(curToken().value);
        next();
    }

    if (!match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '{{' to start class body (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    while (match(TokenType::Delimeter, dm[Delimeters::Semicolon])) next();

    std::vector<MemoryPtr<DeclarationNode>> fields;
    std::vector<MemoryPtr<FunctionNode>> methods;
    MemoryPtr<FunctionNode> constructor = nullptr;

    while (!match(TokenType::Delimeter, dm[Delimeters::RightBraces])) {
        Token token = curToken();

        std::vector<MemoryPtr<CallExpressionNode>> funcdec = {};
        if (match(TokenType::Decorator)) funcdec = parseDecoratorCalls();
        auto modifs = parseModifiers();

        if (match(TokenType::Keyword, km[Keywords::Function])) {
            auto method = parseFunction(std::move(funcdec), std::move(modifs));
            if (method) methods.push_back(std::move(method));
        } else if (match(TokenType::Identifier, className)) {
            // Moving it back by one to make sure it doesn't screw up
            pos--;;
            constructor = parseFunction(std::move(funcdec), std::move(modifs));
            if (!constructor) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse constructor (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
        }
        else if (token.type == TokenType::Identifier && (lookupNext().type == TokenType::Delimeter && lookupNext().value == dm[Delimeters::Colon])) {
            MemoryPtr<DeclarationNode> decl = parseDeclaration();
            fields.push_back(std::move(decl));
            if (isNextLine()) next();
        }
        else if (isNextLine()) next();
        else {
            std::println(std::cerr, "[Neoluma/Parser][{}] Unexpected token in class body (L{}:{})", __func__, curToken().line, curToken().column);
            break;
        }
    }

    next();
    return ASTBuilder::createClass(className, std::move(constructor), std::move(super), std::move(fields), std::move(methods), std::move(decorators), std::move(modifiers));
}

MemoryPtr<BlockNode> Parser::parseBlock() {
    auto dn = getDelimeterNames();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '{{' to start block (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (match(TokenType::Delimeter, dn[Delimeters::Semicolon])) next();

    std::vector<MemoryPtr<ASTNode>> block = {};

    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        MemoryPtr<ASTNode> stmt = parseStatement();
        if (!stmt) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse statement in block (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        block.push_back(std::move(stmt));
        if (curToken().type == TokenType::Delimeter && isNextLine()) next();
    }
    if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected '}}' to end block (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }

    next();
    return ASTBuilder::createBlock(std::move(block));
}

MemoryPtr<ASTNode> Parser::parseDecorator(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers, bool isCall) {
    auto dm = getDelimeterNames();
    Token nameToken = curToken();

    if (!isCall) {
        next();
        if (!match(TokenType::Identifier)) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Expected decorator name (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        std::string name = curToken().value;
        next();

        std::vector<MemoryPtr<ParameterNode>> params;
        if (match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
            next();
            while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
                if (!match(TokenType::Identifier)) {
                    std::println(std::cerr, "[Neoluma/Parser][{}] Expected parameter name in decorator (L{}:{})", __func__, curToken().line, curToken().column);
                    return nullptr;
                }

                Token tok = curToken();
                std::string paramName = tok.value;
                next();

                std::string type = "None";
                if (match(TokenType::Delimeter, dm[Delimeters::Colon])) {
                    next();
                    if (!match(TokenType::Identifier)) {
                        std::println(std::cerr, "[Neoluma/Parser][{}] Expected type after ':' in decorator param (L{}:{})", __func__, curToken().line, curToken().column);
                        return nullptr;
                    }
                    type = curToken().value;
                    next();
                }
                params.push_back(ASTBuilder::createParameter(paramName, type, nullptr));
                if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
                else break;
            }

            if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Unterminated decorator params (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next();
        }

        auto block = parseBlock();
        if (!block) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Expected block after decorator declaration (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }

        return ASTBuilder::createDecorator(name, std::move(params), std::move(block), std::move(decorators), std::move(modifiers));
    }

    if (!match(TokenType::Decorator)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Expected decorator name (L{}:{})", __func__, nameToken.line, nameToken.column);
        return nullptr;
    }
    std::string name = nameToken.value;
    next();

    std::vector<MemoryPtr<ASTNode>> args;
    if (match(TokenType::Delimeter, dm[Delimeters::LeftParen])) {
        next();
        while (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            auto arg = parseExpression();
            if (arg) args.push_back(std::move(arg));

            if (match(TokenType::Delimeter, dm[Delimeters::Comma])) next();
            else break;
        }
        if (!match(TokenType::Delimeter, dm[Delimeters::RightParen])) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Unterminated decorator params (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        next();
    }
    return ASTBuilder::createCallExpression(ASTBuilder::createVariable(name), std::move(args));
}

std::vector<MemoryPtr<CallExpressionNode>> Parser::parseDecoratorCalls() {
    std::vector<MemoryPtr<CallExpressionNode>> calls;
    while (match(TokenType::Decorator)) {
        auto node = parseDecorator({}, {}, true);
        if (!node) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Failed to parse decorator call (L{}:{})", __func__, curToken().line, curToken().column);
            break;
        }
        auto call = as<CallExpressionNode>(std::move(node));
        if (!call) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Decorator parsed to a non-call node (L{}:{})", __func__, curToken().line, curToken().column);
            break;
        }
        calls.push_back(std::move(call));
        next();
    }

    return calls;
}

std::vector<MemoryPtr<ModifierNode>> Parser::parseModifiers() {
    std::vector<MemoryPtr<ModifierNode>> modifiers;
    auto kn = getKeywordNames();
    
    while (curToken().type == TokenType::Keyword) {
        if (match(TokenType::Keyword, kn[Keywords::Static])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Static));
        else if (match(TokenType::Keyword, kn[Keywords::Const])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Const));
        else if (match(TokenType::Keyword, kn[Keywords::Public])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Public));
        else if (match(TokenType::Keyword, kn[Keywords::Protected])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Protected));
        else if (match(TokenType::Keyword, kn[Keywords::Private])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Private));
        else if (match(TokenType::Keyword, kn[Keywords::Override])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Override));
        else if (match(TokenType::Keyword, kn[Keywords::Async])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Async));
        else if (match(TokenType::Keyword, kn[Keywords::Debug])) 
            modifiers.push_back(ASTBuilder::createModifier(ASTModifierType::Debug));
        else break;
        next();
    }

    return modifiers;
}

MemoryPtr<ASTNode> Parser::parsePreprocessor() {
    auto pn = getPreprocessorNames();

    if (match(TokenType::Preprocessor, pn[Preprocessors::Import])) {
        next();
        if (!match(TokenType::String)) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Couldn't find what to import after #import (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        auto moduleStr = curToken().value;
        ASTImportType importType = ASTImportType::Native;
        if ((moduleStr.contains("/") || moduleStr.contains(".")) && moduleStr.contains(":")) importType = ASTImportType::ForeignRelative;
        else if (moduleStr.contains("/") || moduleStr.contains(".")) importType = ASTImportType::Relative;
        else if (moduleStr.contains(":")) importType = ASTImportType::Foreign;

        std::string alias;
        next();
        if (match(TokenType::Preprocessor, pn[Preprocessors::As])) {
            next();
            alias = curToken().value;
        }
        return makeMemoryPtr<ImportNode>(moduleName, alias, importType);
    }
    if (match(TokenType::Preprocessor, pn[Preprocessors::Macro])) {
        next();
        if (!match(TokenType::Identifier)) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Couldn't find identifier after #macro{} (L{}:{})", __func__, "", curToken().line, curToken().column);
            return nullptr;
        }
        next();
        std::string value;
        while (!isNextLine()) {
            value += curToken().value;
            next();
        }
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Macro, value);
    }
    if (match(TokenType::Preprocessor, pn[Preprocessors::Baremetal])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Baremetal);
    }
    if (match(TokenType::Preprocessor, pn[Preprocessors::Unsafe])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Unsafe);
    }
    if (match(TokenType::Preprocessor, pn[Preprocessors::Float])) {
        next();
        return ASTBuilder::createPreprocessor(ASTPreprocessorDirectiveType::Float);
    }

    auto type = ASTPreprocessorDirectiveType::None; // Fallback to none, if this somehow got out of hand.
    return makeMemoryPtr<PreprocessorDirectiveNode>(type);
}

MemoryPtr<EnumNode> Parser::parseEnum(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();

    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Enum does not have a name (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Missing '{{' after enum name (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    while (isNextLine()) next();

    std::vector<MemoryPtr<EnumMemberNode>> elements;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type != TokenType::Identifier) {
            std::println(std::cerr, "[Neoluma/Parser][{}] A non-identifier found in enum (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        } 
        auto name = curToken().value;
        MemoryPtr<LiteralNode> value = nullptr;
    
        next();
        if (match(TokenType::Operator, on[Operators::Assign])) {
            auto tmp = parsePrimary();
            if (typeid(tmp) != typeid(MemoryPtr<LiteralNode>)) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Non-literal value detected in enum after identifier '{}' (L{}:{})", __func__, name, curToken().line, curToken().column);
                return nullptr;
            }
            value = as<LiteralNode>(std::move(tmp));
        }
        elements.push_back(ASTBuilder::createEnumMember(name, std::move(value)));
        next();

        if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
        else if (isNextLine()) next();
        else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
            std::println(std::cerr, "[Neoluma/Parser][{}] Delimeter at enum found that is not either comma or right braces. (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
        else break;
    }
    if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Missing '}}' at end of enum (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    return ASTBuilder::createEnum(std::move(elements), std::move(decorators), std::move(modifiers));
}

MemoryPtr<InterfaceNode> Parser::parseInterface(std::vector<MemoryPtr<CallExpressionNode>> decorators, std::vector<MemoryPtr<ModifierNode>> modifiers) {
    next();
    auto dn = getDelimeterNames();
    auto on = getOperatorNames();
    auto kn = getKeywordNames();

    if (!match(TokenType::Identifier)) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Interface does not have a name (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();
    if (!match(TokenType::Delimeter, dn[Delimeters::LeftBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Missing '{{' after interface name (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    } 
    next();
    while (isNextLine()) next();

    std::vector<MemoryPtr<InterfaceFieldNode>> elements;
    std::vector<MemoryPtr<ParameterNode>> params;
    
    while (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        if (curToken().type == TokenType::Identifier) {
            auto name = curToken().value;
            bool isNullable = false;
            next();
            if (match(TokenType::Operator, on[Operators::Nullable])) isNullable = true;

            if (!match(TokenType::Delimeter, dn[Delimeters::Colon])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] No colon after identifier in an interface (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next();

            if (curToken().type != TokenType::Identifier) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected type after ':' in interface field declaration (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            std::string rawType = curToken().value;
            next();

            elements.push_back(ASTBuilder::createInterfaceField(name, rawType, isNullable));
            next();
            if (isNextLine()) next();
            else if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Delimeter at enum found that is not either '\\n', ';', or right braces. (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            else break;
        } else if (match(TokenType::Keyword, kn[Keywords::Function])) {
            next();
            if (curToken().type != TokenType::Identifier) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected function name in interface method declaration (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            std::string methodName = curToken().value;
            next();
            if (!match(TokenType::Delimeter, dn[Delimeters::LeftParen])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected '(' after method name in interface (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next();
            while (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                Token token = curToken();
                if (!match(TokenType::Identifier)) {
                    std::println(std::cerr, "[Neoluma/Parser][{}] Expected parameter name in interface method declaration (L{}:{})", __func__, token.line, token.column);
                    return nullptr;
                }
                std::string paramName = token.value;
                next();
                std::string type = "None";
                if (match(TokenType::Delimeter, dn[Delimeters::Colon])) {
                    next();
                    if (curToken().type != TokenType::Identifier) {
                        std::println(std::cerr, "[Neoluma/Parser][{}] Expected parameter type after ':' (L{}:{})", __func__, curToken().line, curToken().column);
                        return nullptr;
                    }
                    type = curToken().value;
                    next();
                }
                params.push_back(ASTBuilder::createParameter(paramName, type, nullptr));
                if (match(TokenType::Delimeter, dn[Delimeters::Comma])) next();
                else break;
            }
            if (!match(TokenType::Delimeter, dn[Delimeters::RightParen])) {
                std::println(std::cerr, "[Neoluma/Parser][{}] Expected ')' after method parameters in interface (L{}:{})", __func__, curToken().line, curToken().column);
                return nullptr;
            }
            next();
            MemoryPtr<VariableNode> returnType = nullptr;
            if (match(TokenType::Operator, on[Operators::TypeArrow])) {
                next();
                if (curToken().type != TokenType::Identifier) {
                    std::println(std::cerr, "[Neoluma/Parser][{}] Expected return type after '->' (L{}:{})", __func__, curToken().line, curToken().column);
                    return nullptr;
                }
                returnType = ASTBuilder::createVariable(curToken().value);
                next();
            }
            elements.push_back(ASTBuilder::createInterfaceField(methodName, "None", false, true, std::move(params), std::move(returnType)));
            params.clear();
            if (isNextLine()) next();
        } else {
            std::println(std::cerr, "[Neoluma/Parser][{}] A non-identifier found in interface (L{}:{})", __func__, curToken().line, curToken().column);
            return nullptr;
        }
    }
    if (!match(TokenType::Delimeter, dn[Delimeters::RightBraces])) {
        std::println(std::cerr, "[Neoluma/Parser][{}] Missing '}}' at end of interface (L{}:{})", __func__, curToken().line, curToken().column);
        return nullptr;
    }
    next();

    return ASTBuilder::createInterface(std::move(elements), std::move(decorators), std::move(modifiers));
}

// ==== Helper functions ====

// Parses either a block or a single statement after an 'if' condition.
MemoryPtr<ASTNode> Parser::parseBlockorStatement() {
    auto dm = getDelimeterNames();

    if (match(TokenType::Delimeter, dm[Delimeters::LeftBraces])) {
        MemoryPtr<BlockNode> block = parseBlock();
        if(!block) {
            std::println("[Neoluma/Parser][{}] Expected block after 'if/elif/else/fn' condition", __func__);
            return nullptr;
        }
        return std::move(block);
    }

    MemoryPtr<ASTNode> block = parseStatement();
    if(!block) {
        std::println("[Neoluma/Parser][{}] Expected statement after 'if/elif/else/fn' condition", __func__);
        return nullptr;
    }
    return std::move(block);
}

// Detects nextline expression
bool Parser::isNextLine(){
    auto dn = getDelimeterNames();
    if (match(TokenType::Delimeter, dn[Delimeters::Semicolon])) return true;
    return false;
}

// Detects whether upcoming tokens are identifier or member access followed by an assignment operator
// This code is pure unreadable garbage so lemme explain
bool Parser::isAssignableAhead(size_t offset) {
    // Take up new offset
    size_t p = pos + offset;
    auto dn = getDelimeterNames();

    // First token must be identifier for sure
    if (p >= tokens.size()) return false;
    if (tokens[p].type != TokenType::Identifier) return false;
    p++;

    // Then we create a chain of member accesses and identifiers/function calls inside them
    while (p < tokens.size()) {

        // If function call
        // can't use match() here btw ;c
        if (tokens[p].type == TokenType::Delimeter && tokens[p].value == dn[Delimeters::LeftParen]) {
            int depth = 1;
            p++; // consume left parenthesis

            // to be fair, i could not care less what's inside the () while we look ahead, so just skip until we find the matching right parenthesis
            while (p < tokens.size() && depth > 0) {
                if (tokens[p].type == TokenType::Delimeter && tokens[p].value == dn[Delimeters::LeftParen]) depth++;
                else if (tokens[p].type == TokenType::Delimeter && tokens[p].value == dn[Delimeters::RightParen]) depth--;
                p++;
            }

            continue;
        }

        // If member access however, we parse it too.
        if (tokens[p].type == TokenType::Delimeter && tokens[p].value == dn[Delimeters::Dot]) {

            p++; // consume dot
            if (p >= tokens.size()) return false;
            if (tokens[p].type != TokenType::Identifier) return false;
            p++;
            continue;
        }
        break;
    }

    // After all of that we check if there's assignment operator.
    if (p < tokens.size() && tokens[p].type == TokenType::Operator && isAssignmentOperator(tokens[p].value)) return true;
    // whoops, not an assignment
    return false;
}

// to make math order
int getOperatorPrecedence(const std::string& op){
    auto on = getOperatorNames();
    if (op == on[Operators::Power]) return 7;
    if (op == on[Operators::Multiply] || op == on[Operators::Divide] || op == on[Operators::Modulo]) return 6;
    if (op == on[Operators::Add] || op == on[Operators::Subtract]) return 5;
    if (op == on[Operators::BitwiseLeftShift] || op == on[Operators::BitwiseRightShift]) return 4;
    if (op == on[Operators::BitwiseAnd]) return 3;
    if (op == on[Operators::BitwiseXOr]) return 2;
    if (op == on[Operators::BitwiseOr]) return 1;
    if (op == on[Operators::Equal] || op == on[Operators::NotEqual] || op == on[Operators::LessThan] || op == on[Operators::GreaterThan] || op == on[Operators::LessThanOrEqual] || op == on[Operators::GreaterThanOrEqual]) return 0;
    if (op == on[Operators::LogicalAnd]) return -1;
    if (op == on[Operators::LogicalOr]) return -2;
    return -3;
}

bool isAssignmentOperator(const std::string& op) {
    auto on = getOperatorNames();
    return (op == on[Operators::Assign] || op == on[Operators::AddAssign] || op == on[Operators::SubAssign] ||
            op == on[Operators::MulAssign] || op == on[Operators::DivAssign] || op == on[Operators::ModAssign] || op == on[Operators::PowerAssign]);
}