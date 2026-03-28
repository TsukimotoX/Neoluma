#include "IRGenerator.hpp"
#include "Core/Compiler.hpp"

// LLVM Primitives
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"

void IRGenerator::generate(const ProgramUnit& programUnit, const std::vector<ModuleInfo>& infos) {
    program = makeMemoryPtr<llvm::Module>("program", context);
    builder = makeMemoryPtr<llvm::IRBuilder<>>(context);

    for (ModuleId id : programUnit.order) generateModule(infos[id].module);

    if (llvm::verifyModule(*program, &llvm::errs())) {
        // later make a handler
        return;
    };
    program->print(llvm::outs(), nullptr);
}

void IRGenerator::generateModule(ModuleNode* module) {
    // Declaration pass
    for (const auto& statement : module->body) {
        switch (statement->type) {
            case ASTNodeType::Function:
            case ASTNodeType::Decorator:
                generateFunctionSignature(static_cast<FunctionNode*>(statement.get())); break;
            case ASTNodeType::Class: generateClassType(static_cast<ClassNode*>(statement.get())); break;
            case ASTNodeType::Enum: generateEnum(static_cast<EnumNode*>(statement.get())); break;
            default: break;
        }
    }

    // Generation pass
    for (const auto& statement : module->body) {
        switch (statement->type) {
            case ASTNodeType::Function:
            case ASTNodeType::Decorator:
                generateFunctionBody(static_cast<FunctionNode*>(statement.get())); break;
            case ASTNodeType::Class: generateClassMethods(static_cast<ClassNode*>(statement.get())); break;
            default: break;
        }
    }
}

llvm::Value* IRGenerator::generateStatement(ASTNode* node) {
    switch(node->type) {
        case ASTNodeType::Declaration: return generateDeclaration(static_cast<DeclarationNode*>(node));
        case ASTNodeType::Assignment: return generateAssignment(static_cast<AssignmentNode*>(node));
        case ASTNodeType::Function:
            generateFunctionSignature(static_cast<FunctionNode*>(node));
            if (!static_cast<FunctionNode*>(node)->isIntrinsic) generateFunctionBody(static_cast<FunctionNode*>(node));
            return nullptr;
        case ASTNodeType::ReturnStatement: return generateReturn(static_cast<ReturnStatementNode*>(node));
        case ASTNodeType::IfStatement: return generateIf(static_cast<IfNode*>(node));
        case ASTNodeType::WhileLoop: return generateWhile(static_cast<WhileLoopNode*>(node));
        case ASTNodeType::ForLoop: return generateFor(static_cast<ForLoopNode*>(node));
        case ASTNodeType::CallExpression: return generateCall(static_cast<CallExpressionNode*>(node));
        case ASTNodeType::BreakStatement: return generateBreak();
        case ASTNodeType::ContinueStatement: return generateContinue();
        case ASTNodeType::ThrowStatement: return generateThrow(static_cast<ThrowStatementNode*>(node));
        case ASTNodeType::Switch: return generateSwitch(static_cast<SwitchNode*>(node));
        case ASTNodeType::TryCatch: return generateTryCatch(static_cast<TryCatchNode*>(node));
        case ASTNodeType::Lambda: return generateLambda(static_cast<LambdaNode*>(node));
        default: return generateExpression(node);
    }
}

llvm::Value* IRGenerator::generateIf(IfNode* node) {
    auto* thenBlock = llvm::BasicBlock::Create(context, "if.thenBlock", currentFunction);
    auto* elseBlock = llvm::BasicBlock::Create(context, "if.elseBlock", currentFunction);
    // A block following after If. LLVM has to fallback to something after meeting condition somehow
    auto* afterBlock = llvm::BasicBlock::Create(context, "if.afterBlock", currentFunction);

    // condition
    auto* condition = generateExpression(node->condition.get());
    builder->CreateCondBr(condition, thenBlock, elseBlock);

    // then
    builder->SetInsertPoint(thenBlock);
    generateStatement(node->thenBlock.get());
    builder->CreateBr(afterBlock);

    // else
    builder->SetInsertPoint(elseBlock);
    if (node->elseBlock) generateStatement(node->elseBlock.get());
    builder->CreateBr(afterBlock);

    // merge back from condition
    builder->SetInsertPoint(afterBlock);
    return nullptr;
}

llvm::Value* IRGenerator::generateFor(ForLoopNode* node) {
    auto* iterable = generateExpression(node->iterable.get());

    auto* conditionBlock = llvm::BasicBlock::Create(context, "for.condition", currentFunction);
    auto* body = llvm::BasicBlock::Create(context, "for.body", currentFunction);
    auto* afterBlock = llvm::BasicBlock::Create(context, "for.afterBlock", currentFunction);

    // save old breaks and continues for nested loops
    auto* prevBreak = currentBreakBB;
    auto* prevContinue = currentContinueBB;
    currentBreakBB = afterBlock;
    currentContinueBB = conditionBlock;

    // initialization of iterable variable
    pushScope();
    auto* iterableAlloc = builder->CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, node->variable->varName);
    declareVariable(node->variable->varName, iterableAlloc);
    builder->CreateBr(conditionBlock);

    // fixme: conditionBlock is temporarily mute, we don't have range() in std for now
    builder->SetInsertPoint(conditionBlock);
    builder->CreateCondBr(true, body, afterBlock);

    // body
    builder->SetInsertPoint(body);
    generateBlock(node->body.get());
    builder->CreateBr(conditionBlock);

    builder->SetInsertPoint(afterBlock);

    popScope();
    currentBreakBB = prevBreak;
    currentContinueBB = prevContinue;
    return nullptr;
}

llvm::Value* IRGenerator::generateWhile(WhileLoopNode* node) {
    auto* conditionBlock = llvm::BasicBlock::Create(context, "while.condition", currentFunction);
    auto* body = llvm::BasicBlock::Create(context, "while.body", currentFunction);
    auto* afterBlock = llvm::BasicBlock::Create(context, "while.afterBlock", currentFunction);

    // for nested loops we save the old breaks and continues
    auto* prevBreak = currentBreakBB;
    auto* prevContinue = currentContinueBB;

    currentBreakBB = afterBlock;
    currentContinueBB = conditionBlock;

    // condition
    builder->CreateBr(conditionBlock);
    builder->SetInsertPoint(conditionBlock);
    auto* condition = generateExpression(node->condition.get());
    builder->CreateCondBr(condition, body, afterBlock);

    // body
    builder->SetInsertPoint(body);
    generateBlock(node->body.get());
    builder->CreateBr(conditionBlock);

    // after the loop
    builder->SetInsertPoint(afterBlock);

    // recover the nested breaks and continues
    currentBreakBB = prevBreak;
    currentContinueBB = prevContinue;

    return nullptr;
}

llvm::Value* IRGenerator::generateSwitch(SwitchNode* node) {
    auto* expression = generateExpression(node->expression.get());

    auto* defaultBlock = llvm::BasicBlock::Create(context, "switch.default", currentFunction);
    auto* afterBlock = llvm::BasicBlock::Create(context, "switch.afterBlock", currentFunction);

    // preservation
    auto* prevBreak = currentBreakBB;
    currentBreakBB = afterBlock;

    auto* switchInstance = builder->CreateSwitch(expression, defaultBlock, node->cases.size());

    // cases
    for (const auto* sCase : node->cases) {
        auto* caseBlock = llvm::BasicBlock::Create(context, "switch.case", currentFunction);
        auto* caseValue = generateExpression(sCase->condition.get()); // should be ConstantInt
        switchInstance->addCase(llvm::cast<llvm::ConstantInt>(caseValue), caseBlock);
        builder->SetInsertPoint(caseBlock);
        generateStatement(sCase->body.get());
        builder->CreateBr(afterBlock);
    }

    // default
    builder->SetInsertPoint(defaultBlock);
    if (node->defaultCase) generateStatement(node->defaultCase->body.get());
    builder->CreateBr(afterBlock);

    builder->SetInsertPoint(afterBlock);
    currentBreakBB = prevBreak;
    return nullptr;
}

llvm::Function* IRGenerator::generateFunctionSignature(FunctionNode* node) {
    if (node->isIntrinsic) intrinsicFunctions[node->name] = true;

    std::vector<llvm::Type*> parameterTypes;
    for (const auto& parameter : node->parameters)
        parameterTypes.push_back(resolveType(parameter->parameterRawType->varType->varName));

    llvm::Type* returnType = node->returnType ? resolveType(node->returnType->varType->varName)
                        : llvm::Type::getVoidTy(context);

    llvm::FunctionType* functionType = llvm::FunctionType::get(returnType, parameterTypes, false);
    llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, node->name, program.get());
    functions.emplace(node->name, function);
    return function;
}

void IRGenerator::generateFunctionBody(FunctionNode* node) {
    llvm::Function* function = functions.at(node->name);
    currentFunction = function;
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", function);
    builder->SetInsertPoint(entryBlock);

    pushScope();

    auto* llvmArg = function->arg_begin();
    for (const auto& parameter : node->parameters) {
        llvmArg->setName(parameter->parameterName);

        // fixme: for now, array sizes are syntactically supported, but IR doesn't address it. i will fix that after the working alpha
        auto* paramAlloc = builder->CreateAlloca(llvmArg->getType(), nullptr, parameter->parameterName);
        builder->CreateStore(&*llvmArg, paramAlloc);
        llvmArg++;
    }

    generateBlock(node->body.get());
    popScope();

    auto* closingBlock = builder->GetInsertBlock();
    if (!closingBlock->getTerminator())
        builder->CreateRetVoid();
}

llvm::Value* IRGenerator::generateBlock(BlockNode* node) {
    // In LLVM, the block returns only the last statement, other statements
    // before that are already in IR via builder.
    llvm::Value* lastStmt = nullptr;
    for (const auto& stmt : node->statements)
        lastStmt = generateStatement(stmt.get());
    return lastStmt;
}

llvm::Value* IRGenerator::generateCall(CallExpressionNode* node) {
    auto varName = static_cast<VariableNode*>(node->callee.get())->varName;
    auto* function = program->getFunction(varName);

    // generate arguments
    std::vector<llvm::Value*> args;
    for (const auto& arg : node->arguments)
        args.push_back(generateExpression(arg.get()));

    // if intrinsic - handle differently
    if (intrinsicFunctions.count(varName))
        return generateIntrinsicCall(varName, args);

    // usual call
    return builder->CreateCall(function, args, "calltmp");
}

// all std magic happens here
llvm::Value* IRGenerator::generateIntrinsicCall(const std::string& name, std::vector<llvm::Value*>& args) {
    /*
    Intrinsic system calls list:
    Neoluma | glibc
    __read -> read
    __write -> write
    __malloc -> malloc
    __free -> free
    __memcpy -> memcpy
    __memset -> memset
    __exit -> exit
    __panic -> not in libc, made via __panic -> fprintf(stderr) + exit(1)
     */
    auto* ptr = llvm::PointerType::getUnqual(context);
    auto* i8  = llvm::Type::getInt8Ty(context);
    auto* i32 = llvm::Type::getInt32Ty(context);
    auto* i64 = llvm::Type::getInt64Ty(context);
    auto* voidTy = llvm::Type::getVoidTy(context);

    if (name == "__write") {
        auto* fnType = llvm::FunctionType::get(i64, {i32, ptr, i64}, false);
        auto fn = program->getOrInsertFunction("write", fnType);
        return builder->CreateCall(fn, args);
    }
    // todo for later finish ts
}

llvm::Value* IRGenerator::generateBreak() {
    builder->CreateBr(currentBreakBB);
    return nullptr;
}

llvm::Value* IRGenerator::generateContinue() {
    builder->CreateBr(currentContinueBB);
    return nullptr;
}

llvm::Value* IRGenerator::generateReturn(ReturnStatementNode* node) {
    if (node->expression) {
        auto val = generateExpression(node->expression.get());
        builder->CreateRet(val);
    }
    else builder->CreateRetVoid();
    return nullptr;
}

llvm::Value* IRGenerator::generateThrow(ThrowStatementNode* node) {
    generateExpression(node->expression.get());
    builder->CreateUnreachable(); // this is pain in the ass to make im skipping ts
    return nullptr;
}

llvm::Type* IRGenerator::resolveType(const std::string& typeName) {
    auto tm = getTypeNames();

    if (typeName == tm[ResolvedType::Int8]) return llvm::Type::getInt8Ty(context);
    if (typeName == tm[ResolvedType::Int16]) return llvm::Type::getInt16Ty(context);
    if (typeName == tm[ResolvedType::Int]) return llvm::Type::getInt32Ty(context);
    if (typeName == tm[ResolvedType::Int64]) return llvm::Type::getInt64Ty(context);
    if (typeName == tm[ResolvedType::Int128]) return llvm::Type::getInt128Ty(context);
    if (typeName == tm[ResolvedType::Bool]) return llvm::Type::getInt1Ty(context);
    if (typeName == tm[ResolvedType::Float]) return llvm::Type::getFloatTy(context);
    if (typeName == tm[ResolvedType::Float64]) return llvm::Type::getDoubleTy(context);
    if (typeName == tm[ResolvedType::Void]) return llvm::Type::getVoidTy(context);
    if (typeName == tm[ResolvedType::Str]) return llvm::Type::getInt8Ty(context)->getPointerTo(0);
    if (typeName == tm[ResolvedType::Bool]) return llvm::Type::getInt1Ty(context);

    // fixme: address the pointer of str and class
    for (const auto& classType : classTypes)
        if (typeName == classType.first) return llvm::TypedPointerType::get(classType.second, 0);

    return llvm::Type::getInt8Ty(context); // default
}
