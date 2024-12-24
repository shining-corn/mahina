#include "Parser.h"
#include <memory>
#include <iostream>
#include "Tokenizer.h"

bool Parser::fail() const {
	return src_.fail();
}

bool Parser::parse() {
	// initialize
	std::shared_ptr<CompileError> error;
	if (tokenizer_.initialize(error)) {
		errors_.push_back(error);
		return true;
	}
	if (next()) {
		return true;
	}

	// parse
	CompileUnitNode cu;
	while (currentToken_.getType() == Token::Type::STRUCT) {
		StructNode s;
		if (parseStruct(s)) {
			return true;
		}
		cu.addStruct(s);
	}

	if (currentToken_.getType() == Token::Type::EXTERN) {
		if (next()) {
			return true;
		}

		Token externType = currentToken_;
		if (expect(Token::Type::CONSTANT_STRING)) {
			return true;
		}
		if (externType.getString() != "C") {
			errors_.push_back(std::make_shared<InvalidExternTypeError>(externType));
			return true;
		}

		if (expect(Token::Type::CURLY_BRACKET_LEFT)) {
			return true;
		}

		while (currentToken_.getType() == Token::Type::FUNCTION) {
			FunctionNode declare;
			if (parseDeclare(declare)) {
				return true;
			}
			cu.addFunction(declare);
		}

		if (expect(Token::Type::CURLY_BRACKET_RIGHT)) {
			return true;
		}
	}

	while (currentToken_.getType() == Token::Type::FUNCTION) {
		FunctionNode f;
		if (parseFunction(f)) {
			return true;
		}
		cu.addFunction(f);
	}

	context_.addCompileUnit(cu);

	return false;
}

bool Parser::next() {
	std::shared_ptr<CompileError> error;
	if (tokenizer_.getToken(currentToken_, error)) {
		errors_.push_back(error);
		return true;
	}
	return false;
}

bool Parser::expect(Token::Type expected) {
	if (currentToken_.getType() != expected) {
		auto error = std::make_shared <UnexpectedTokenError>(currentToken_);
		error->setExpected(std::make_shared<UnexpectedTokenError::ExpectedToken>(expected));
		errors_.push_back(error);
		return true;
	}
	if (next()) {
		return true;
	}
	return false;
}

bool Parser::expectTypeOrSymbol() {
	if (!currentToken_.isType() && (currentToken_.getType() != Token::Type::SYMBOL)) {
		auto error = std::make_shared<UnexpectedTokenError>(currentToken_);
		error->setExpected(std::make_shared<UnexpectedTokenError::ExpectedTypeToken>());
		errors_.push_back(error);
		return true;
	}
	if (next()) {
		return true;
	}
	return false;
}

bool Parser::parseStruct(StructNode& result) {
	if (expect(Token::Type::STRUCT)) {
		return true;
	}

	result.setName(currentToken_);
	if (expect(Token::Type::SYMBOL)) {
		return true;
	}

	if (expect(Token::Type::CURLY_BRACKET_LEFT)) {
		return true;
	}

	while (currentToken_.getType() == Token::Type::SYMBOL) {
		Token memberName = currentToken_;
		if (next()) {
			return true;
		}

		TypeNode type;
		if (parseType(type)) {
			return true;
		}

		VariableDefinitionNode memberNode(memberName, type);
		result.addMember(memberNode);
	}

	if (expect(Token::Type::CURLY_BRACKET_RIGHT)) {
		return true;
	}

	return false;
}

bool Parser::parseDeclare(FunctionNode& result) {
	if (expect(Token::Type::FUNCTION)) {
		return true;
	}

	Token name = currentToken_;
	if (expect(Token::Type::SYMBOL)) {
		return true;
	}
	result.setName(name);

	if (expect(Token::Type::PARENTHESIS_LEFT)) {
		return true;
	}

	while (currentToken_.getType() == Token::Type::SYMBOL) {
		Token name = currentToken_;
		if (next()) {
			return true;
		}

		TypeNode type;
		if (parseType(type)) {
			return true;
		}

		VariableDefinitionNode variable(name, type);
		result.addArgument(variable);

		if (currentToken_.getType() == Token::Type::COMMA) {
			if (next()) {
				return true;
			}

			if (currentToken_.getType() == Token::Type::TRIPLE_DOT) {
				result.setVariableArgument();
				if (next()) {
					return true;
				}

				break;
			}

			continue;
		}
		else {
			break;
		}
	}

	if (expect(Token::Type::PARENTHESIS_RIGHT)) {
		return true;
	}

	TypeNode returnType;
	if (currentToken_.getType() == Token::Type::SEMICOLON) {
		returnType.setType(Token::Type::TYPE_VOID);
	}
	else {
		if (parseType(returnType)) {
			return true;
		}
	}
	result.setReturnType(returnType);

	if (expect(Token::Type::SEMICOLON)) {
		return true;
	}

	result.setFunctionType(FunctionNode::Type::C);

	return false;
}

bool Parser::parseFunction(FunctionNode& result) {
	if (expect(Token::Type::FUNCTION)) {
		return true;
	}

	Token name = currentToken_;
	if (expect(Token::Type::SYMBOL)) {
		return true;
	}
	result.setName(name);

	if (expect(Token::Type::PARENTHESIS_LEFT)) {
		return true;
	}

	while (currentToken_.getType() == Token::Type::SYMBOL) {
		Token name = currentToken_;
		if (next()) {
			return true;
		}

		TypeNode type;
		if (parseType(type)) {
			return true;
		}

		auto& valueType = type.getValueType();
		if ((valueType.basicType == Token::Type::TYPE_VOID) && (valueType.pointerCount == 0)) {
			errors_.push_back(std::make_shared<ArgumentCanNotBeVoidTypeError>(type.getToken()));
			return true;
		}

		VariableDefinitionNode variable(name, type);
		result.addArgument(variable);

		if (currentToken_.getType() == Token::Type::COMMA) {
			if (next()) {
				return true;
			}
			continue;
		}
		else {
			break;
		}
	}

	if (expect(Token::Type::PARENTHESIS_RIGHT)) {
		return true;
	}

	TypeNode returnType;
	if (currentToken_.getType() == Token::Type::CURLY_BRACKET_LEFT) {
		returnType.setType(Token::Type::TYPE_VOID);
	}
	else {
		if (parseType(returnType)) {
			return true;
		}
	}
	result.setReturnType(returnType);

	BlockNode block;
	if (parseBlock(block)) {
		return  true;
	}
	result.setBlock(block);

	result.setFunctionType(FunctionNode::Type::MAHINA);

	return false;
}

bool Parser::parseBlock(BlockNode& result) {
	if (expect(Token::Type::CURLY_BRACKET_LEFT)) {
		return true;
	}

	for (;;) {
		switch (currentToken_.getType()) {
		case Token::Type::LET:
		{
			Token letToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseLet(statement)) {
				return true;
			}
			statement->setToken(letToken);
			result.addStatement(statement);
			if (expect(Token::Type::SEMICOLON)) {
				return true;
			}
		}
		break;
		case Token::Type::IF:
		{
			Token ifToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseIf(statement)) {
				return true;
			}
			statement->setToken(ifToken);
			result.addStatement(statement);
		}
		break;
		case Token::Type::WHILE:
		{
			Token whileToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseWhile(statement)) {
				return true;
			}
			statement->setToken(whileToken);
			result.addStatement(statement);
		}
		break;
		case Token::Type::SYMBOL:
		{
			Token symbolToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseAssignOrCall(statement)) {
				return true;
			}
			statement->setToken(symbolToken);
			result.addStatement(statement);
			if (expect(Token::Type::SEMICOLON)) {
				return true;
			}
		}
		break;
		case Token::Type::RETURN:
		{
			Token returnToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseReturn(statement)) {
				return true;
			}
			statement->setToken(returnToken);
			result.addStatement(statement);
			if (expect(Token::Type::SEMICOLON)) {
				return true;
			}
		}
		break;
		case Token::Type::BREAK:
		{
			Token breakToken = currentToken_;
			std::shared_ptr<StatementNode> statement;
			if (parseBreak(statement)) {
				return true;
			}
			statement->setToken(breakToken);
			result.addStatement(statement);
			if (expect(Token::Type::SEMICOLON)) {
				return true;
			}
		}
		break;
		case Token::Type::SEMICOLON:
			if (next()) {
				return true;
			}
			break;
		default:
			result.addRightCurlyBracketToken(currentToken_);
			if (expect(Token::Type::CURLY_BRACKET_RIGHT)) {
				return true;
			}
			else {
				return false;
			}
		}
	}
}

bool Parser::parseLet(std::shared_ptr<StatementNode>& result) {
	Token letToken = currentToken_;
	auto letNode = std::make_shared<LetNode>();

	if (expect(Token::Type::LET)) {
		return true;
	}

	letNode->setName(currentToken_);
	if (expect(Token::Type::SYMBOL)) {
		return true;
	}

	TypeNode type;
	if (currentToken_.isType() || (currentToken_.getType() == Token::Type::SYMBOL) || (currentToken_.getType() == Token::Type::SQUARE_BRACKET_LEFT)) {
		if (parseType(type)) {
			return true;
		}
		letNode->setType(type);
	}

	if (letNode->hasType()) {
		if (currentToken_.getType() == Token::Type::SEMICOLON) {
			result = letNode;
			return false;
		}
	}
	else {
		if (currentToken_.getType() != Token::Type::ASSIGN_EQUAL) {
			errors_.push_back(std::make_shared<TypeOrInitializerMustBeSpecifiedError>(letToken));
			return true;
		}
	}

	if (expect(Token::Type::ASSIGN_EQUAL)) {
		return true;
	}

	if (currentToken_.getType() == Token::Type::NEW) {
		if (next()) {
			return true;
		}

		TypeNode newType;
		if (parseType(newType)) {
			return true;
		}
		newType.setIsReference(true);
		letNode->setIsHeap(true);

		if (letNode->hasType()) {
			if (type.getValueType() != newType.getValueType()) {
				ValueType expected = type.getValueType();
				errors_.push_back(std::make_shared<TypeMismatchError>(letToken, expected, newType.getValueType()));
				return true;
			}
			else {
				// DO NOTHING
			}
		}
		else {
			letNode->setType(newType);
		}

		if (currentToken_.getType() == Token::Type::SEMICOLON) {
			result = letNode;
			return false;
		}
	}

	std::shared_ptr<ExpressionNode> value;
	if (parseExpression(value)) {
		return true;
	}
	letNode->setInitialValue(value);

	result = letNode;
	return false;
}

bool Parser::parseIf(std::shared_ptr<StatementNode>& result) {
	auto temp = std::make_shared<IfNode>();

	if (expect(Token::Type::IF)) {
		return true;
	}

	std::shared_ptr<ExpressionNode> condition;
	if (parseExpression(condition)) {
		return true;
	}
	temp->setCondition(condition);

	BlockNode block;
	if (parseBlock(block)) {
		return true;
	}
	temp->setThenBlock(block);

	if (currentToken_.getType() == Token::Type::ELSE) {
		if (next()) {
			return true;
		}

		if (currentToken_.getType() == Token::Type::IF) {
			std::shared_ptr<StatementNode> elseIf;
			if (parseIf(elseIf)) {
				return true;
			}
			BlockNode elseBlock;
			elseBlock.addStatement(elseIf);
			temp->setElseBlock(elseBlock);
		}
		else {
			BlockNode elseBlock;
			if (parseBlock(elseBlock)) {
				return true;
			}
			temp->setElseBlock(elseBlock);
		}
	}

	result = temp;
	return false;
}

bool Parser::parseWhile(std::shared_ptr<StatementNode>& result) {
	auto temp = std::make_shared<WhileNode>();

	if (expect(Token::Type::WHILE)) {
		return true;
	}

	std::shared_ptr<ExpressionNode> condition;
	if (parseExpression(condition)) {
		return true;
	}
	temp->setCondition(condition);

	BlockNode block;
	if (parseBlock(block)) {
		return true;
	}
	temp->setBlock(block);

	result = temp;
	return false;
}

bool Parser::parseReturn(std::shared_ptr<StatementNode>& result) {
	auto temp = std::make_shared<ReturnNode>(currentToken_);

	if (expect(Token::Type::RETURN)) {
		return true;
	}

	if (currentToken_.getType() != Token::Type::SEMICOLON) {
		std::shared_ptr<ExpressionNode> value;
		if (parseExpression(value)) {
			return true;
		}
		temp->setValue(value);
	}

	result = temp;
	return false;
}

bool Parser::parseBreak(std::shared_ptr<StatementNode>& result) {
	auto temp = std::make_shared<BreakNode>();

	if (expect(Token::Type::BREAK)) {
		return true;
	}

	result = temp;
	return false;
}

bool Parser::parseType(TypeNode& result) {
	size_t arrayDepth = 0;
	while (currentToken_.getType() == Token::Type::SQUARE_BRACKET_LEFT) {
		if (next()) {
			return true;
		}
		arrayDepth++;
	}

	if (parsePrimitiveType(result)) {
		return true;
	}

	for (size_t i = 0; i < arrayDepth; ++i) {
		std::shared_ptr<ExpressionNode> size;
		if (parseValue(size)) {
			return true;
		}
		result.addArraySize(size);

		if (expect(Token::Type::SQUARE_BRACKET_RIGHT)) {
			return true;
		}
	}

	return false;
}

bool Parser::parsePrimitiveType(TypeNode& result) {
	Token type = currentToken_;
	if (expectTypeOrSymbol()) {
		return true;
	}

	if (currentToken_.getType() == Token::Type::AMPERSAND) {
		if (next()) {
			return true;
		}
		result.setToken(type);
		result.setType(type.getType());
	}
	else {
		size_t pointerCount = 0;
		while (currentToken_.getType() == Token::Type::ASTERISK) {
			if (next()) {
				return true;
			}
			pointerCount++;
		}

		result.setToken(type);
		result.setType(type.getType());
		result.setPointerCount(pointerCount);
	}

	return false;
}

bool Parser::parseAssignOrCall(std::shared_ptr<StatementNode>& result) {
	VariableValueNode variable;
	if (parseVariableValue(variable)) {
		return true;
	}

	switch (currentToken_.getType()) {
	case Token::Type::PARENTHESIS_LEFT:
	{
		if (next()) {
			return true;
		}

		ValueListNode values;
		if (parseValueList(values)) {
			return true;
		}
		result = std::make_shared<CallNode>(variable, values);

		if (expect(Token::Type::PARENTHESIS_RIGHT)) {
			return true;
		}
	}
	break;
	case Token::Type::ASSIGN_EQUAL:
	{
		if (next()) {
			return  true;
		}

		std::shared_ptr<ExpressionNode> value;
		if (parseExpression(value)) {
			return true;
		}

		variable.setIsRhsValue(true);
		result = std::make_shared<AssignNode>(variable, value);
	}
	break;
	default:
		errors_.push_back(std::make_shared<UnexpectedTokenError>(currentToken_));
		return true;
	}

	return false;
}

bool Parser::parseVariableValue(VariableValueNode& result) {
	result.setName(currentToken_);
	if (expect(Token::Type::SYMBOL)) {
		return true;
	}

	if (currentToken_.getType() == Token::Type::SQUARE_BRACKET_LEFT) {
		if (next()) {
			return true;
		}

		std::shared_ptr<ExpressionNode> arrayIndex;
		if (parseExpression(arrayIndex)) {
			return true;
		}
		result.setArrayIndex(arrayIndex);

		if (expect(Token::Type::SQUARE_BRACKET_RIGHT)) {
			return true;
		}
	}

	if (currentToken_.getType() == Token::Type::DOT) {
		if (next()) {
			return true;
		}

		VariableValueNode member;
		if (parseVariableValue(member)) {
			return true;
		}
		result.setMember(member);
	}

	return false;
}

bool Parser::parseValueList(ValueListNode& result) {
	if ((currentToken_.getType() != Token::Type::PARENTHESIS_RIGHT)  && (currentToken_.getType() != Token::Type::SQUARE_BRACKET_RIGHT)) {
		for (;;) {
			std::shared_ptr<ExpressionNode> value;
			if (parseExpression(value)) {
				return true;
			}
			result.addValue(value);

			if (currentToken_.getType() == Token::Type::COMMA) {
				if (next()) {
					return true;
				}
			}
			else {
				break;
			}
		}
	}

	return false;
}

bool Parser::parseExpression(std::shared_ptr<ExpressionNode>& result) {
	std::stack<std::pair<Token, std::shared_ptr<ExpressionNode>>> stack;

	std::shared_ptr<ExpressionNode> value;
	if (parseValue(value)) {
		return true;
	}
	stack.push(std::make_pair(Token(Token::Type::START_OPERATOR), value));

	for (;;) {
		auto priority1 = stack.top().first.getPriority();
		auto priority2 = currentToken_.getPriority();
		if (priority1 < priority2) {
			auto operatorToken = currentToken_;
			if (next()) {
				return true;
			}
			if (parseValue(value)) {
				return true;
			}
			stack.push(std::make_pair(operatorToken, value));
		}
		else if (priority1 == 0) {
			result = stack.top().second;
			return false;
		}
		else {
			auto rhs = stack.top();
			stack.pop();
			auto lhs = stack.top();
			stack.pop();

			auto node = std::make_shared<BinaryOperationNode>();
			node->setLhs(lhs.second);
			node->setOperator(rhs.first);
			node->setRhs(rhs.second);

			stack.push(std::make_pair(lhs.first, node));
		}
	}
}

bool Parser::parseValue(std::shared_ptr<ExpressionNode>& result) {
	switch (currentToken_.getType()) {
	case Token::Type::PARENTHESIS_LEFT:
		if (next()) {
			return true;
		}
		if (parseExpression(result)) {
			return true;
		}
		if (expect(Token::Type::PARENTHESIS_RIGHT)) {
			return true;
		}
		break;
	case Token::Type::SYMBOL:
	{
		VariableValueNode value;
		value.setToken(currentToken_);
		if (parseVariableValue(value)) {
			return true;
		}
		if (currentToken_.getType() == Token::Type::PARENTHESIS_LEFT) {
			if (next()) {
				return true;
			}

			ValueListNode values;
			if (parseValueList(values)) {
				return true;
			}
			auto call = std::make_shared<CallNode>(value, values);
			result = call;

			if (expect(Token::Type::PARENTHESIS_RIGHT)) {
				return true;
			}
		}
		else {
			result = std::make_shared<VariableValueNode>(value);
		}
	}
	break;
	case Token::Type::MINUS:
	{
		auto unaryMinus = std::make_shared<UnaryOperationNode>();
		unaryMinus->setOperator(currentToken_);
		if (next()) {
			return true;
		}

		std::shared_ptr<ExpressionNode> value;
		if (parseValue(value)) {
			return true;
		}
		unaryMinus->setValue(value);

		result = unaryMinus;
	}
	break;
	case Token::Type::SQUARE_BRACKET_LEFT:
	{
		auto arrayConstant = std::make_shared<AggregateConstantNode>(currentToken_);
		if (next()) {
			return true;
		}

		ValueListNode values;
		if (parseValueList(values)) {
			return true;
		}
		arrayConstant->setValues(values);

		result = arrayConstant;

		if (expect(Token::Type::SQUARE_BRACKET_RIGHT)) {
			return true;
		}
	}
	break;
	default:
		if (currentToken_.isConstant()) {
			result = std::make_shared<ConstantNode>(currentToken_);
			if (next()) {
				return true;
			}
		}
		else if ((currentToken_.isType()) || (currentToken_.getType() == Token::Type::SYMBOL)) {
			std::shared_ptr<ExpressionNode> cast;
			if (parseCast(cast)) {
				return true;
			}
			result = cast;
		}
		else {
			errors_.push_back(std::make_shared<UnexpectedTokenError>(currentToken_));
			return true;
		}
	}
	return false;
}

bool Parser::parseCast(std::shared_ptr<ExpressionNode>& result) {
	auto cast = std::make_shared<CastNode>();

	TypeNode typeNode;
	if (parseType(typeNode)) {
		return true;
	}
	cast->setDestType(typeNode);

	if (expect(Token::Type::PARENTHESIS_LEFT)) {
		return true;
	}

	std::shared_ptr<ExpressionNode> value;
	if (parseExpression(value)) {
		return true;
	}
	cast->setValue(value);

	if (expect(Token::Type::PARENTHESIS_RIGHT)) {
		return true;
	}

	result = cast;
	return false;
}
