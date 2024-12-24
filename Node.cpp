#include "Node.h"
#include <sstream>
#include "util.h"
#include "Tokenizer.h"

extern std::vector<std::string> debugLogs;

namespace {
	void debugLog(size_t line) {
		std::stringstream ss;
		ss << __FILE__ << ":" << line;
		debugLogs.push_back(ss.str());
	}

	bool castConstantToValueType_BasicType(Generator& g, const ValueType& srcType, Generator::Value srcValue, const ValueType& destType, Generator::Value& result);
	bool castConstantToValueType_ArrayType(Generator& g, Context& ctx, const std::shared_ptr<ExpressionNode>& src, const ValueType& destType, Generator::Value& result);
	bool castConstantToValueType(Generator& g, Context& ctx, const std::shared_ptr<ExpressionNode>& src, const ValueType& destType, Generator::Value& result) {
		auto srcType = src->getValueType();
		auto srcValue = src->getGeneratedValue();

		if (!Token::isConstant(srcType.basicType)) {
			result = srcValue;
			return false;
		}

		if (srcType.pointerCount != 0) {
			result = srcValue;
		}
		else if (srcType.arraySizes.empty()) {
			return castConstantToValueType_BasicType(g, srcType, srcValue, destType, result);
		}
		else {
			return castConstantToValueType_ArrayType(g, ctx, src, destType, result);
		}
		return false;
	}

	bool castConstantToValueType_BasicType(Generator& g, const ValueType& srcType, Generator::Value srcValue, const ValueType& destType, Generator::Value& result) {
		if ((srcType.basicType == Token::Type::CONSTANT_INTEGER) && (Token::isIntegerType(destType.basicType))) {
			if (g.createCast(srcType.basicType, srcValue, destType.basicType, result)) {
				debugLog(__LINE__);
				return true;
			}
		}
		else if ((srcType.basicType == Token::Type::CONSTANT_FLOAT) && (destType.basicType == Token::Type::TYPE_F32)) {
			if (g.createCast(srcType.basicType, srcValue, destType.basicType, result)) {
				debugLog(__LINE__);
				return true;
			}
		}
		else {
			result = srcValue;
		}
		return false;
	}

	bool castConstantToValueType_ArrayType(Generator& g, Context& ctx, const std::shared_ptr<ExpressionNode>& src, const ValueType& destType, Generator::Value& result) {
		const ValueType& srcType = src->getValueType();
		if ((srcType.basicType == Token::Type::CONSTANT_BOOL) && (destType.basicType == Token::Type::TYPE_BOOL)) {
			result = src->getGeneratedValueBoolArray();
		}
		else if (srcType.basicType == Token::Type::CONSTANT_INTEGER) {
			switch (destType.basicType) {
			case Token::Type::TYPE_I8:
				result = src->getGeneratedValueI8Array();
				break;
			case Token::Type::TYPE_I16:
				result = src->getGeneratedValueI16Array();
				break;
			case Token::Type::TYPE_I32:
				result = src->getGeneratedValueI32Array();
				break;
			case Token::Type::TYPE_I64:
				result = src->getGeneratedValueI64Array();
				break;
			case Token::Type::TYPE_U8:
				result = src->getGeneratedValueU8Array();
				break;
			case Token::Type::TYPE_U16:
				result = src->getGeneratedValueU16Array();
				break;
			case Token::Type::TYPE_U32:
				result = src->getGeneratedValueU32Array();
				break;
			case Token::Type::TYPE_U64:
				result = src->getGeneratedValueU64Array();
				break;
			default:
				debugLog(__LINE__);
				return true;
			}
		}
		else if (srcType.basicType == Token::Type::CONSTANT_FLOAT) {
			switch (destType.basicType) {
			case Token::Type::TYPE_F32:
				result = src->getGeneratedValueF32Array();
				break;
			case Token::Type::TYPE_F64:
				result = src->getGeneratedValueF64Array();
				break;
			default:
				debugLog(__LINE__);
				return true;
			}
		}
		else if ((srcType.basicType == Token::Type::CONSTANT_STRING) && (destType.basicType == Token::Type::TYPE_I8) && (destType.pointerCount == 1)) {
			result = src->getGeneratedValueStringArray();
		}
		else {
			result = src->getGeneratedValue();
		}

		if (result == nullptr) {
			ctx.addCompileError(std::make_shared<TypeMismatchError>(src->getToken(), destType, src->getValueType()));
			return true;
		}

		return false;
	}
}

//
// Debug Print
//

void TypeNode::debugPrint(DebugPrinter& dp) {
	dp.o << Tokenizer::getKeywordString(type_.basicType);

	if (type_.isReference) {
		dp.o << "&";
	}

	for (size_t i = 0; i < type_.pointerCount; ++i) {
		dp.o << "*";
	}
}

void AggregateConstantNode::debugPrint(struct DebugPrinter& dp) {
	dp.o << "[";
	values_.debugPrint(dp);
	dp.o << "]";
}

void VariableDefinitionNode::debugPrint(DebugPrinter& dp) {
	dp.o << name_.getString() << " ";
	type_.debugPrint(dp);
}

void VariableValueNode::debugPrint(DebugPrinter& dp) {
	dp.o << name_.getString();
	if (arrayIndex_) {
		dp.o << "[";
		arrayIndex_->debugPrint(dp);
		dp.o << "]";
	}
	if (member_) {
		dp.o << ".";
		member_->debugPrint(dp);
	}
}

void ValueListNode::debugPrint(DebugPrinter& dp) {
	bool isFirst = true;
	for (auto value : values_) {
		if (!isFirst) {
			dp.o << ", ";
		}
		value->debugPrint(dp);
		isFirst = false;
	}
}

void UnaryOperationNode::debugPrint(DebugPrinter& dp) {
	dp.o << "-(";
	value_->debugPrint(dp);
	dp.o << ")";
}

void BinaryOperationNode::debugPrint(DebugPrinter& dp) {
	dp.o << "(";
	lhs_->debugPrint(dp);
	dp.o << ") " << operatorType_.getString() << " (";
	rhs_->debugPrint(dp);
	dp.o << ")";
}

void CallNode::debugPrint(DebugPrinter& dp) {
	f_.debugPrint(dp);
	dp.o << "(";
	values_.debugPrint(dp);
	dp.o << ")";
}

void ConstantNode::debugPrint(DebugPrinter& dp) {
	if (constant_.getType() == Token::Type::CONSTANT_STRING) {
		dp.o << "\"";
		std::string str = constant_.getString();

		size_t pos = 0;
		while (pos != std::string::npos) {
			pos = str.find('\x0D', pos);
			if (pos != std::string::npos) {
				str = str.replace(pos, 1, "\\r");
			}
		}
		pos = 0;
		while (pos != std::string::npos) {
			pos = str.find('\x0A', pos);
			if (pos != std::string::npos) {
				str = str.replace(pos, 1, "\\n");
			}
		}
		pos = 0;
		while (pos != std::string::npos) {
			pos = str.find('\x09', pos);
			if (pos != std::string::npos) {
				str = str.replace(pos, 1, "\\t");
			}
		}

		dp.o << str << "\"";
	}
	else {
		dp.o << constant_.getString();
	}
}

void CastNode::debugPrint(DebugPrinter& dp) {
	destType_.debugPrint(dp);
	dp.o << "(";
	value_->debugPrint(dp);
	dp.o << ")";
}

void BlockNode::debugPrint(DebugPrinter& dp) {
	for (auto s : statements_) {
		dp.o << dp;
		s->debugPrint(dp);
		dp.o << ";\n";
	}
}

void LetNode::debugPrint(DebugPrinter& dp) {
	dp.o << "let " << name_.getString() << " ";
	if (type_) {
		type_->debugPrint(dp);
	}
	if (initialValue_) {
		dp.o << " = ";
		initialValue_->debugPrint(dp);
	}
}

void IfNode::debugPrint(DebugPrinter& dp) {
	dp.o << "if ";
	if (condition_) {
		condition_->debugPrint(dp);
	}
	dp.o << " {\n";
	dp.indentLevel++;
	thenBlock_.debugPrint(dp);
	dp.indentLevel--;
	dp.o << dp << "}";
	if (elseBlock_) {
		dp.o << "\n" << dp << "else {\n";
		dp.indentLevel++;
		elseBlock_->debugPrint(dp);
		dp.indentLevel--;
		dp.o << dp << "}";
	}
}

void WhileNode::debugPrint(DebugPrinter& dp) {
	dp.o << "while ";
	if (condition_) {
		condition_->debugPrint(dp);
	}
	dp.o << " {\n";
	dp.indentLevel++;
	block_.debugPrint(dp);
	dp.indentLevel--;
	dp.o << dp << "}";
}

void ReturnNode::debugPrint(DebugPrinter& dp) {
	dp.o << "return ";
	if (value_) {
		value_->debugPrint(dp);
	}
}

void BreakNode::debugPrint(DebugPrinter& dp) {
	dp.o << "break";
}

void AssignNode::debugPrint(DebugPrinter& dp) {
	dest_.debugPrint(dp);
	dp.o << " = ";
	if (value_) {
		value_->debugPrint(dp);
	}
}

void CompileUnitNode::debugPrint(DebugPrinter& dp) {
	for (auto& s : structs_) {
		s.debugPrint(dp);
	}
	dp.o << "\n";
	dp.o << "extern \"C\" {\n";
	dp.indentLevel++;
	for (auto& f : functions_) {
		if (f.getFunctionType() == FunctionNode::Type::C) {
			f.debugPrint(dp);
		}
	}
	dp.indentLevel--;
	dp.o << "}\n\n";

	for (auto& f : functions_) {
		if (f.getFunctionType() == FunctionNode::Type::MAHINA) {
			f.debugPrint(dp);
		}
	}
}

void StructNode::debugPrint(DebugPrinter& dp) {
	dp.o << "struct " << name_.getString() << " {\n";
	dp.indentLevel++;
	for (auto member : members_) {
		dp.o << dp;
		member.debugPrint(dp);
		dp.o << "\n";
	}
	dp.indentLevel--;
	dp.o << "}\n";
}

void FunctionNode::debugPrint(DebugPrinter& dp) {
	dp.o << dp << "fn " << name_.getString() << "(";
	bool isFirst = true;
	for (auto arg : args_) {
		if (!isFirst) {
			dp.o << ", ";
		}
		arg.debugPrint(dp);
		isFirst = false;
	}
	if (hasVariableArgument_) {
		dp.o << ", ...";
	}
	dp.o << ") ";
	returnType_.debugPrint(dp);

	if (block_ != nullptr) {
		dp.o << " {\n";
		dp.indentLevel++;
		block_->debugPrint(dp);
		dp.indentLevel--;
		dp.o << dp << "}\n\n";
	}
	else {
		dp.o << ";\n\n";
	}
}

void Context::debugPrint(DebugPrinter& dp) {
	for (auto& cu : compileUnits_) {
		cu.debugPrint(dp);
	}
}

//
// Generate LLVM IR
//

bool TypeNode::generate(Generator& g, Context& ctx) {
	if (type_.isReference) {
		if (type_.basicType == Token::Type::TYPE_VOID) {
			ctx.addCompileError(std::make_shared<InvalidReferenceTypeError>(token_));
			return true;
		}
	}

	for (auto arraySize : arraySizes_) {
		if (arraySize->generate(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		if (arraySize->getValueType().basicType != Token::Type::CONSTANT_INTEGER) {
			ctx.addCompileError(std::make_shared<ArraySizeMustBeConstantIntegerError>(arraySize->getToken()));
			return true;
		}
		type_.arraySizes.push_back(arraySize->getConstantInteger());
	}

	if (g.createType(type_, generatedType_)) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool VariableDefinitionNode::generateType(Generator& g, Context& ctx) {
	return type_.generate(g, ctx);
}

bool VariableValueNode::generate(Generator& g, Context& ctx) {
	Generator::Value temp;
	if (ctx.getSymbol(name_.getString(), valueType_, temp)) {
		ctx.addCompileError(std::make_shared<UndefinedSymbolError>(name_));
		return true;
	}

	if (valueType_.isArgument) {
		generatedValue_ = temp;
	}
	else {
		generatedVariablePtr_ = temp;
		if (!isRhsValue_) {
			if (g.createLoad(temp, generatedValue_)) {
				debugLog(__LINE__);
				return true;
			}
		}
	}

	return false;
}

bool ValueListNode::generate(Generator& g, Context& ctx) {
	generatedValues_.clear();

	for (auto& v : values_) {
		if (v->generate(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		generatedValues_.push_back(v->getGeneratedValue());
		valueTypes_.push_back(v->getValueType());
	}

	return false;
}

bool ValueListNode::generateForFunctionArgumants(Generator& g, Context& ctx, const CallNode& call, const FunctionNode& function) {
	generatedValues_.clear();

	auto arg = function.getArguments().begin();
	auto argEnd = function.getArguments().end();
	for (auto& v : values_) {
		if (v->generate(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		Generator::Value temp;
		if (arg != argEnd) {
			auto& valueType = v->getValueType();
			auto& argType = arg->getValueType();


			if (!valueType.isCompatible(argType)) {
				ctx.addCompileError(std::make_shared<TypeMismatchError>(v->getToken(), argType, valueType));
				return true;
			}
			if (castConstantToValueType(g, ctx, v, argType, temp)) {
				debugLog(__LINE__);
				return true;
			}

			valueTypes_.push_back(argType);

			++arg;
		}
		else {
			if (!function.hasVariableArgument()) {
				ctx.addCompileError(std::make_shared<InvalidCallArgumentLength>(call.getToken(), function.getName()));
				return true;
			}

			temp = v->getGeneratedValue();
		}

		generatedValues_.push_back(temp);
	}

	if (arg != argEnd) {
		ctx.addCompileError(std::make_shared<InvalidCallArgumentLength>(call.getToken(), function.getName()));
		return true;
	}

	return false;
}

bool UnaryOperationNode::generate(Generator& g, Context& ctx) {
	if (value_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}
	valueType_ = value_->getValueType();

	if (!valueType_.isArithmetic()) {
		ctx.addCompileError(std::make_shared<NotArithmeticTypeError>(operatorType_, value_->getValueType()));
		return true;
	}

	switch (operatorType_.getType()) {
	case Token::Type::MINUS:
		if (g.createNegate(valueType_.basicType, value_->getGeneratedValue(), generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t num = value_->getConstantInteger();
			if (num == INT64_MIN) {
				ctx.addCompileError(std::make_shared<ConstantTooLarge>(operatorType_));
				return true;
			}
			constantInteger_ = -num;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			double num = value_->getConstantDouble();
			constantDouble_ = -num;
		}

		break;
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool BinaryOperationNode::generate(Generator& g, Context& ctx) {
	if (lhs_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}
	if (rhs_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	ValueType lhsType = lhs_->getValueType();
	ValueType rhsType = rhs_->getValueType();
	if (!lhsType.isCompatible(rhsType)) {
		ctx.addCompileError(std::make_shared<TypeMismatchError>(operatorType_, rhsType, lhsType));
		return true;
	}

	Generator::Value newLhs;
	Generator::Value newRhs;
	if (Token::isConstant(lhsType.basicType)) {
		if (castConstantToValueType(g, ctx, lhs_, rhsType, newLhs)) {
			debugLog(__LINE__);
			return true;
		}
		newRhs = rhs_->getGeneratedValue();
		valueType_ = rhsType;
	}
	else {
		if (castConstantToValueType(g, ctx, rhs_, lhsType, newRhs)) {
			debugLog(__LINE__);
			return true;
		}
		newLhs = lhs_->getGeneratedValue();
		valueType_ = lhsType;
	}

	if (checkOperand(ctx, operatorType_, lhsType, lhs_)) {
		debugLog(__LINE__);
		return true;
	}

	switch (operatorType_.getType()) {
	case Token::Type::PLUS:
		if (g.createAdd(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t lhs = lhs_->getConstantInteger();
			int64_t rhs = rhs_->getConstantInteger();
			// TODO: check overflow
			constantInteger_ = lhs + rhs;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			int64_t lhs = lhs_->getConstantDouble();
			int64_t rhs = rhs_->getConstantDouble();
			// TODO: check overflow
			constantDouble_ = lhs + rhs;
		}

		break;
	case Token::Type::MINUS:
		if (g.createSub(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t lhs = lhs_->getConstantInteger();
			int64_t rhs = rhs_->getConstantInteger();
			// TODO: check overflow
			constantInteger_ = lhs - rhs;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			int64_t lhs = lhs_->getConstantDouble();
			int64_t rhs = rhs_->getConstantDouble();
			// TODO: check overflow
			constantDouble_ = lhs - rhs;
		}

		break;
	case Token::Type::ASTERISK:
		if (g.createMul(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t lhs = lhs_->getConstantInteger();
			int64_t rhs = rhs_->getConstantInteger();
			// TODO: check overflow
			constantInteger_ = lhs * rhs;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			int64_t lhs = lhs_->getConstantDouble();
			int64_t rhs = rhs_->getConstantDouble();
			// TODO: check overflow
			constantDouble_ = lhs * rhs;
		}

		break;
	case Token::Type::SLASH:
		if (g.createDiv(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t lhs = lhs_->getConstantInteger();
			int64_t rhs = rhs_->getConstantInteger();
			// TODO: check overflow
			constantInteger_ = lhs / rhs;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			int64_t lhs = lhs_->getConstantDouble();
			int64_t rhs = rhs_->getConstantDouble();
			// TODO: check overflow
			constantDouble_ = lhs / rhs;
		}

		break;
	case Token::Type::PERCENT:
		if (g.createRem(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			int64_t lhs = lhs_->getConstantInteger();
			int64_t rhs = rhs_->getConstantInteger();
			// TODO: check overflow
			constantInteger_ = lhs % rhs;
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			int64_t lhs = lhs_->getConstantDouble();
			int64_t rhs = rhs_->getConstantDouble();
			// TODO: check overflow
			constantDouble_ = lhs % rhs;
		}

		break;
	case Token::Type::COMPARE_LESSER_THAN:
		if (g.createCommpareLesserThan(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() < rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() < rhs_->getConstantInteger();
		}

		break;
	case Token::Type::COMPARE_LESSER_EQUAL:
		if (g.createCommpareLesserEqual(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() <= rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() <= rhs_->getConstantInteger();
		}

		break;
	case Token::Type::COMPARE_GREATER_THAN:
		if (g.createCommpareGreaterThan(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() > rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() > rhs_->getConstantInteger();
		}

		break;
	case Token::Type::COMPARE_GREATER_EQUAL:
		if (g.createCommpareGreaterEqual(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() >= rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() >= rhs_->getConstantInteger();
		}

		break;
	case Token::Type::COMPARE_EQUAL:
		if (g.createCommpareEqual(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() == rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() == rhs_->getConstantInteger();
		}

		break;
	case Token::Type::COMPARE_NOT_EQUAL:
		if (g.createCommpareNotEqual(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_INTEGER) {
			constantBool_ = lhs_->getConstantInteger() != rhs_->getConstantInteger();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantInteger() != rhs_->getConstantInteger();
		}

		break;
	case Token::Type::LOGICAL_OR:
		if (g.createLogicalOr(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_BOOL) {
			constantBool_ = lhs_->getConstantBool() | rhs_->getConstantBool();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantBool() | rhs_->getConstantBool();
		}

		break;
	case Token::Type::LOGICAL_AND:
		if (g.createLogicalAnd(valueType_.basicType, newLhs, newRhs, generatedValue_)) {
			debugLog(__LINE__);
			return true;
		}

		if (valueType_.basicType == Token::Type::CONSTANT_BOOL) {
			constantBool_ = lhs_->getConstantBool() & rhs_->getConstantBool();
		}
		else if (valueType_.basicType == Token::Type::CONSTANT_FLOAT) {
			constantBool_ = lhs_->getConstantBool() & rhs_->getConstantBool();
		}

		break;
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool BinaryOperationNode::checkOperand(Context& ctx, const Token& operatorToken, const ValueType& operandType, const std::shared_ptr<ExpressionNode>& value) {
	if (operandType.pointerCount != 0) {
		debugLog(__LINE__);
		return true;
	}

	switch (operatorToken.getType()) {
	case Token::Type::PLUS:
	case Token::Type::MINUS:
	case Token::Type::ASTERISK:
	case Token::Type::SLASH:
	case Token::Type::PERCENT:
		if (!operandType.isArithmetic()) {
			ctx.addCompileError(std::make_shared<NotArithmeticTypeError>(value->getToken(), value->getValueType()));
			return true;
		}
		break;
	case Token::Type::COMPARE_LESSER_THAN:
	case Token::Type::COMPARE_LESSER_EQUAL:
	case Token::Type::COMPARE_GREATER_THAN:
	case Token::Type::COMPARE_GREATER_EQUAL:
		if (!operandType.isComparable()) {
			ctx.addCompileError(std::make_shared<NotComparableTypeError>(value->getToken(), value->getValueType()));
			return true;
		}
		break;
	case Token::Type::COMPARE_EQUAL:
	case Token::Type::COMPARE_NOT_EQUAL:
		if (!operandType.isAbleToEqual()) {
			ctx.addCompileError(std::make_shared<NotBeAbleToEqualTypeError>(value->getToken(), value->getValueType()));
			return true;
		}
		break;
	case Token::Type::LOGICAL_OR:
	case Token::Type::LOGICAL_AND:
		if (!operandType.isBool()) {
			ctx.addCompileError(std::make_shared<TypeMismatchError>(value->getToken(), Token::Type::TYPE_BOOL, value->getValueType()));
			return true;
		}
		break;
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool CallNode::generate(Generator& g, Context& ctx) {
	auto f = ctx.getFunctionNode(f_.getName().getString());
	if (f == nullptr) {
		ctx.addCompileError(std::make_shared<UndefinedSymbolError>(f_.getName()));
		return true;
	}

	if (values_.generateForFunctionArgumants(g, ctx, *this, *f)) {
		debugLog(__LINE__);
		return true;
	}

	if (g.createCall(f->getGeneratedFunction(), values_.getGeneratedValues(), generatedValue_)) {
		debugLog(__LINE__);
		return true;
	}
	valueType_ = f->getReturnType().getValueType();

	return false;
}

bool ConstantNode::generate(Generator& g, Context& ctx) {
	switch (constant_.getType()) {
	case Token::Type::CONSTANT_BOOL:
	{
		if (toBoolean(constant_.getString(), constantBool_)) {
			debugLog(__LINE__);
			return true;
		}
		Generator::Constant temp;
		if (g.createBooleanConstant(constantBool_, temp)) {
			debugLog(__LINE__);
			return true;
		}
		generatedValue_ = temp;
		valueType_ = ValueType(constant_.getType(), 0, false);

		break;
	}
	case Token::Type::CONSTANT_INTEGER:
	{
		if (toInt64(constant_.getString(), constantInteger_)) {
			debugLog(__LINE__);
			return true;
		}

		Generator::Constant temp;
		if (g.createI64Constant(constantInteger_, temp)) {
			debugLog(__LINE__);
			return true;
		}
		generatedValue_ = temp;
		valueType_ = ValueType(constant_.getType(), 0, false);

		break;
	}
	case Token::Type::CONSTANT_FLOAT:
	{
		if (toDouble(constant_.getString(), constantDouble_)) {
			debugLog(__LINE__);
			return true;
		}
		Generator::Constant temp;
		if (g.createDoubleConstant(constantDouble_, temp)) {
			debugLog(__LINE__);
			return true;
		}
		generatedValue_ = temp;
		valueType_ = ValueType(constant_.getType(), 0, false);

		break;
	}
	case Token::Type::CONSTANT_STRING:
	{
		constantString_ = constant_.getString();
		Generator::Constant temp;
		if (g.createStringConstant(constantString_, temp)) {
			debugLog(__LINE__);
			return true;
		}
		generatedValue_ = temp;
		valueType_ = ValueType(constant_.getType(), 0, false);

		break;
	}
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool AggregateConstantNode::generate(Generator& g, Context& ctx) {
	if (values_.generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	switch (token_.getType()) {
	case Token::Type::SQUARE_BRACKET_LEFT:
		valueType_ = values_.getValues()->front()->getValueType();
		valueType_.isArgument = false;
		valueType_.arraySizes.push_back(values_.getValues()->size());

		switch (valueType_.basicType) {
		case Token::Type::CONSTANT_BOOL:
			if (generateArrayConstant(g, ctx, generatedValueBoolArray_)) {
				debugLog(__LINE__);
				return true;
			}
			break;
		case Token::Type::CONSTANT_INTEGER:
		{
			valueType_.basicType = Token::Type::TYPE_I8;
			if (generateArrayConstant(g, ctx, generatedValueI8Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_I16;
			if (generateArrayConstant(g, ctx, generatedValueI16Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_I32;
			if (generateArrayConstant(g, ctx, generatedValueI32Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_I64;
			if (generateArrayConstant(g, ctx, generatedValueI64Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_U8;
			if (generateArrayConstant(g, ctx, generatedValueU8Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_U16;
			if (generateArrayConstant(g, ctx, generatedValueU16Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_U32;
			if (generateArrayConstant(g, ctx, generatedValueU32Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_U64;
			if (generateArrayConstant(g, ctx, generatedValueU64Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::CONSTANT_INTEGER;

			break;
		}
		case Token::Type::CONSTANT_FLOAT:
		{
			valueType_.basicType = Token::Type::TYPE_F32;
			if (generateArrayConstant(g, ctx, generatedValueF32Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::TYPE_F64;
			if (generateArrayConstant(g, ctx, generatedValueF64Array_)) {
				debugLog(__LINE__);
				return true;
			}
			valueType_.basicType = Token::Type::CONSTANT_FLOAT;

			break;
		}
		case Token::Type::CONSTANT_STRING:
			if (generateArrayConstant(g, ctx, generatedValueStringArray_)) {
				debugLog(__LINE__);
				return true;
			}
			break;
		default:
			debugLog(__LINE__);
			return true;
		}
		break;
	case Token::Type::CURLY_BRACKET_LEFT:
		// TODO
		debugLog(__LINE__);
		return true;
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool AggregateConstantNode::generateArrayConstant(Generator& g, Context& ctx, Generator::Value& result) {
	auto type = values_.getValueType()->begin();
	auto end = values_.getValueType()->end();
	if (type != end) {
		auto i = values_.getValueType()->begin();
		++i;
		for (; i != end; ++i) {
			if (!Token::isConstant(i->basicType)) {
				debugLog(__LINE__);
				return true;
			}

			if (*i != *type) {
				ctx.addCompileError(std::make_shared<EachElementMustHaveIdenticallyTypeError>(token_));
				return true;
			}
		}
	}

	Generator::Type arrayType;
	if (g.createType(valueType_, arrayType)) {
		debugLog(__LINE__);
		return true;
	}

	std::vector<Generator::Constant> values;
	for (auto value : *values_.getValues()) {
		Generator::Constant temp;
		if (value->getToken().getType() == Token::Type::SQUARE_BRACKET_LEFT) {
			switch (valueType_.basicType) {
			case Token::Type::TYPE_BOOL:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueBoolArray());
				break;
			case Token::Type::TYPE_I8:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueI8Array());
				break;
			case Token::Type::TYPE_I16:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueI16Array());
				break;
			case Token::Type::TYPE_I32:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueI32Array());
				break;
			case Token::Type::TYPE_I64:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueI64Array());
				break;
			case Token::Type::TYPE_U8:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueU8Array());
				break;
			case Token::Type::TYPE_U16:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueU16Array());
				break;
			case Token::Type::TYPE_U32:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueU32Array());
				break;
			case Token::Type::TYPE_U64:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueU64Array());
				break;
			case Token::Type::TYPE_F32:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueF32Array());
				break;
			case Token::Type::TYPE_F64:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueF64Array());
				break;
			case Token::Type::CONSTANT_STRING:
				temp = static_cast<Generator::Constant>(value->getGeneratedValueStringArray());
				break;
			default:
				debugLog(__LINE__);
				return true;
			}

			if (temp == nullptr) {
				result = nullptr;
				return false;
			}
		}
		else {
			switch (value->getValueType().basicType) {
			case Token::Type::CONSTANT_BOOL:
			{
				bool b = value->getConstantBool();
				if (g.createBooleanConstant(b, temp)) {
					debugLog(__LINE__);
					return true;
				}

				break;
			}
			case Token::Type::CONSTANT_INTEGER:
				switch (valueType_.basicType) {
				case Token::Type::TYPE_I8:
				{
					int64_t n = value->getConstantInteger();
					if ((INT8_MIN <= n) && (n <= INT8_MAX)) {
						if (g.createI8Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_I16:
				{
					int64_t n = value->getConstantInteger();
					if ((INT16_MIN <= n) && (n <= INT16_MAX)) {
						if (g.createI16Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_I32:
				{
					int64_t n = value->getConstantInteger();
					if ((INT32_MIN <= n) && (n <= INT32_MAX)) {
						if (g.createI32Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_I64:
					if (g.createI64Constant(value->getConstantInteger(), temp)) {
						debugLog(__LINE__);
						return true;
					}
					break;
				case Token::Type::TYPE_U8:
				{
					int64_t n = value->getConstantInteger();
					if ((0 <= n) && (n <= UINT8_MAX)) {
						if (g.createU8Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_U16:
				{
					int64_t n = value->getConstantInteger();
					if ((0 <= n) && (n <= UINT16_MAX)) {
						if (g.createU16Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_U32:
				{
					int64_t n = value->getConstantInteger();
					if ((0 <= n) && (n <= UINT32_MAX)) {
						if (g.createU32Constant(n, temp)) {
							debugLog(__LINE__);
							return true;
						}
					}
					else {
						result = nullptr;
						return false;
					}

					break;
				}
				case Token::Type::TYPE_U64:
					if (g.createU64Constant(value->getConstantInteger(), temp)) {
						debugLog(__LINE__);
						return true;
					}
					break;
				default:
					debugLog(__LINE__);
					return true;
				}
				break;
			case Token::Type::CONSTANT_FLOAT:
				switch (valueType_.basicType) {
				case Token::Type::TYPE_F32:
					if (g.createF32Constant(value->getConstantDouble(), temp)) {
						debugLog(__LINE__);
						return true;
					}
					break;
				case Token::Type::TYPE_F64:
					if (g.createF64Constant(value->getConstantDouble(), temp)) {
						debugLog(__LINE__);
						return true;
					}
					break;
				default:
					debugLog(__LINE__);
					return true;
				}
				break;
			case Token::Type::CONSTANT_STRING:
				if (g.createStringConstant(value->getConstantString(), temp)) {
					debugLog(__LINE__);
					return true;
				}
				break;
			default:
				debugLog(__LINE__);
				return true;
			}
		}
		values.push_back(temp);
	}

	Generator::Constant temp;
	if (g.createArrayConstant(arrayType, values, temp)) {
		debugLog(__LINE__);
		return true;
	}
	result = temp;

	return false;
}

bool CastNode::generate(Generator& g, Context& ctx) {
	if (value_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}
	if (destType_.generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	auto& srcType = value_->getValueType();
	auto& destType = destType_.getValueType();

	if (srcType.isReference) {
		// TODO
		debugLog(__LINE__);
		return true;
	}
	if (srcType.pointerCount != 0) {
		// TODO: support? compile error?
		debugLog(__LINE__);
		return true;
	}

	if (destType.isReference) {
		// TODO
		debugLog(__LINE__);
		return true;
	}
	if (destType.pointerCount != 0) {
		// TODO: support? compile error?
		debugLog(__LINE__);
		return true;
	}

	// TODO: compatible check
	// TODO: overflow check

	if (g.createCast(srcType.basicType, value_->getGeneratedValue(), destType_.getValueType().basicType, generatedValue_)) {
		debugLog(__LINE__);
		return true;
	}
	valueType_ = destType_.getValueType();

	return false;
}


bool BlockNode::generateBlock(Generator& g, const Generator::Function& function, const Generator::BasicBlock& insertBefore) {
	return g.createBasicBlock(function, insertBefore, generatedBlock_);
}

bool BlockNode::generateStatements(Generator& g, Context& ctx, const Generator::BasicBlock& successorBlock) {
	Generator::BasicBlock previousBlock = g.getCurrentBlock();
	g.setInsertPoint(generatedBlock_);

	ctx.addSymbolTable();
	for (auto& s : statements_) {
		if (ctx.isBreaked() || ctx.isReturned()) {
			ctx.addCompileError(std::make_shared < CanNotGiveInstructionAfterBreakOrReturn>(s->getToken()));
			return true;
		}
		if (s->generate(g, ctx)) {
			return true;
		}
	}
	ctx.removeSymbolTable();
	if ((successorBlock != nullptr) && !ctx.isBreaked() && !ctx.isReturned()) {
		if (g.createGoto(successorBlock)) {
			debugLog(__LINE__);
			return true;
		}
	}
	ctx.setBreaked(false);
	g.setInsertPoint(previousBlock);

	return false;
}

bool LetNode::generate(Generator& g, Context& ctx) {
	if (initialValue_ != nullptr) {
		if (initialValue_->generate(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
		if (type_ == nullptr) {
			ValueType type = initialValue_->getValueType();
			if (type.arraySizes.empty()) {
				switch (type.basicType) {
				case Token::Type::CONSTANT_BOOL:
					type.basicType = Token::Type::TYPE_BOOL;
					break;
				case Token::Type::CONSTANT_INTEGER:
				{
					auto temp = initialValue_->getConstantInteger();
					if ((temp < INT32_MIN) || (INT32_MAX < temp)) {
						ctx.addCompileError(std::make_shared<ConstantTooLarge>(name_));
						return true;
					}
					type.basicType = Token::Type::TYPE_I32;

					break;
				}
				case Token::Type::CONSTANT_FLOAT:
					type.basicType = Token::Type::TYPE_F64;
					break;
				case Token::Type::CONSTANT_STRING:
					type.basicType = Token::Type::TYPE_I8;
					type.pointerCount = 1;
					break;
				default:
					// DO NOTHING
					break;
				}

				type_ = std::make_shared<TypeNode>();
				type.isArgument = false;
				type_->setValueType(type);
			}
			else {
				Generator::Value temp;
				switch (type.basicType) {
				case Token::Type::CONSTANT_BOOL:
					temp = initialValue_->getGeneratedValueBoolArray();
					type.basicType = Token::Type::TYPE_BOOL;
					break;
				case Token::Type::CONSTANT_INTEGER:
					temp = initialValue_->getGeneratedValueI32Array();
					if (temp != nullptr) {
						type.basicType = Token::Type::TYPE_I32;
					}
					else {
						ctx.addCompileError(std::make_shared<ConstantTooLarge>(name_));
						return true;
					}
					break;
				case Token::Type::CONSTANT_FLOAT:
					temp = initialValue_->getGeneratedValueF64Array();
					if (temp != nullptr) {
						type.basicType = Token::Type::TYPE_F64;
					}
					else {
						debugLog(__LINE__);
						return true;
					}
					break;
				case Token::Type::CONSTANT_STRING:
					temp = initialValue_->getGeneratedValueStringArray();
					type.basicType = Token::Type::TYPE_I8;
					type.pointerCount = 1;
					break;
				default:
					// DO NOTHING
					break;
				}

				type_ = std::make_shared<TypeNode>();
				type.isArgument = false;
				type_->setValueType(type);
			}
		}
	}

	if (type_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}
	if (g.createAlloc(type_->getGeneratedType(), generatedPtr_)) {
		debugLog(__LINE__);
		return true;
	}

	if (isHeap_) {
		Generator::Value ptr1;
		if (g.createCallMalloc(type_->getGeneratedType(), ptr1)) {
			debugLog(__LINE__);
			return true;
		}

		Generator::Value ptr2;
		if (g.createBitCast(ptr1, type_->getGeneratedType(), ptr2)) {
			debugLog(__LINE__);
			return true;
		}

		if (g.createStore(ptr2, generatedPtr_)) {
			debugLog(__LINE__);
			return true;
		}

		Generator::Value value = nullptr;
		if (initialValue_ != nullptr) {
			if (!type_->getValueType().isCompatible(initialValue_->getValueType())) {
				ctx.addCompileError(std::make_shared<TypeMismatchError>(token_, type_->getValueType(), initialValue_->getValueType()));
				return true;
			}
			if (castConstantToValueType(g, ctx, initialValue_, type_->getValueType(), value)) {
				debugLog(__LINE__);
				return true;
			}
		}

		// TODO: set type id

		if (g.createInitializeObject(ptr2, value)) {
			debugLog(__LINE__);
			return true;
		}
	}
	else {
		Generator::Value value = nullptr;
		if (initialValue_ != nullptr) {
			if (!type_->getValueType().isCompatible(initialValue_->getValueType())) {
				ctx.addCompileError(std::make_shared<TypeMismatchError>(token_, type_->getValueType(), initialValue_->getValueType()));
				return true;
			}
			if (castConstantToValueType(g, ctx, initialValue_, type_->getValueType(), value)) {
				debugLog(__LINE__);
				return true;
			}
		}

		if (g.createStore(value, generatedPtr_)) {
			debugLog(__LINE__);
			return true;
		}
	}

	if (ctx.addSymbol(name_.getString(), type_->getValueType(), generatedPtr_)) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool IfNode::generate(Generator& g, Context& ctx) {
	if (condition_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	Generator::BasicBlock successorBlock;
	if (g.createBasicBlock(nullptr, ctx.getLastBlock(), successorBlock)) {
		debugLog(__LINE__);
		return true;
	}
	ctx.setLastBlock(successorBlock);

	if (thenBlock_.generateBlock(g, nullptr, successorBlock)) {
		debugLog(__LINE__);
		return true;
	}
	if (thenBlock_.generateStatements(g, ctx, successorBlock)) {
		debugLog(__LINE__);
		return true;
	}
	ctx.setLastBlock(successorBlock);

	if (elseBlock_ != nullptr) {
		if (elseBlock_->generateBlock(g, nullptr, successorBlock)) {
			debugLog(__LINE__);
			return true;
		}
		if (elseBlock_->generateStatements(g, ctx, successorBlock)) {
			debugLog(__LINE__);
			return true;
		}
		ctx.setLastBlock(successorBlock);

		if (g.createIf(condition_->getGeneratedValue(), thenBlock_.getGeneratedBlock(), elseBlock_->getGeneratedBlock())) {
			debugLog(__LINE__);
			return true;
		}
	}
	else {
		if (g.createIf(condition_->getGeneratedValue(), thenBlock_.getGeneratedBlock(), successorBlock)) {
			debugLog(__LINE__);
			return true;
		}
	}

	g.setInsertPoint(successorBlock);
	ctx.setReturned(false);	// TODO: thenとelseの両方でreturnしてたらtrueのままにする

	return false;
}

bool WhileNode::generate(Generator& g, Context& ctx) {
	Generator::BasicBlock conditionBlock;
	if (g.createBasicBlock(nullptr, ctx.getLastBlock(), conditionBlock)) {
		debugLog(__LINE__);
		return true;
	}
	if (g.createGoto(conditionBlock)) {
		debugLog(__LINE__);
		return true;
	}
	g.setInsertPoint(conditionBlock);

	if (condition_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	Generator::BasicBlock successorBlock;
	if (g.createBasicBlock(nullptr, ctx.getLastBlock(), successorBlock)) {
		debugLog(__LINE__);
		return true;
	}

	ctx.setLastBlock(successorBlock);
	ctx.addSuccessorBlockForBreak(successorBlock);

	if (block_.generateBlock(g, nullptr, successorBlock)) {
		debugLog(__LINE__);
		return true;
	}

	if (g.createIf(condition_->getGeneratedValue(), block_.getGeneratedBlock(), successorBlock)) {
		debugLog(__LINE__);
		return true;
	}
	g.setInsertPoint(block_.getGeneratedBlock());

	if (block_.generateStatements(g, ctx, conditionBlock)) {
		debugLog(__LINE__);
		return true;
	}
	ctx.setLastBlock(successorBlock);

	ctx.removeSuccessorBlockForBreak();

	g.setInsertPoint(successorBlock);
	ctx.setReturned(false);	// TODO: 無限ループかつループ中にreturnしかなかったらtrueのままにする

	return false;
}

bool ReturnNode::generate(Generator& g, Context& ctx) {
	if (value_) {
		if (value_->generate(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		ValueType returnType = g.getCurrentReturnType();
		ValueType valueType = value_->getValueType();
		Generator::Value returnValue = value_->getGeneratedValue();

		if ((valueType.pointerCount == 0) && valueType.arraySizes.empty() && !valueType.isReference) {
			if ((returnType.pointerCount != 0) || (!returnType.arraySizes.empty())) {
				ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
				return true;
			}

			switch (valueType.basicType) {
			case Token::Type::CONSTANT_BOOL:
				if (returnType.basicType != Token::Type::TYPE_BOOL) {
					ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
					return true;
				}
				if (g.createReturn(value_->getGeneratedValue())) {
					debugLog(__LINE__);
					return true;
				}
				break;
			case Token::Type::CONSTANT_INTEGER:
			{
				if (!Token::isIntegerType(returnType.basicType)) {
					ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
					return true;
				}

				if (returnType.basicType != Token::Type::TYPE_I64) {
					if (g.createCast(value_->getValueType().basicType, returnValue, returnType.basicType, returnValue)) {
						debugLog(__LINE__);
						return true;
					}
				}

				if (g.createReturn(returnValue)) {
					debugLog(__LINE__);
					return true;
				}

				break;
			}
			case Token::Type::CONSTANT_FLOAT:
			{
				if (!Token::isFloatingPointType(returnType.basicType)) {
					ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
					return true;
				}

				if (returnType.basicType != Token::Type::TYPE_F64) {
					if (g.createCast(value_->getValueType().basicType, returnValue, returnType.basicType, returnValue)) {
						debugLog(__LINE__);
						return true;
					}
				}

				if (g.createReturn(returnValue)) {
					debugLog(__LINE__);
					return true;
				}

				break;
			}
			case Token::Type::CONSTANT_STRING:
				// FALL THROUGH
			default:
				if (value_->getValueType() != returnType) {
					ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
					return true;
				}
				if (g.createReturn(value_->getGeneratedValue())) {
					debugLog(__LINE__);
					return true;
				}

				break;
			}
		}
		else {
			if (!returnType.isCompatible(value_->getValueType())) {
				ctx.addCompileError(std::make_shared<TypeMismatchError>(returnToken_, returnType, valueType));
				return true;
			}
			Generator::Value returnValue;
			if (castConstantToValueType(g, ctx, value_, returnType, returnValue)) {
				debugLog(__LINE__);
				return true;
			}
			if (g.createReturn(returnValue)) {
				debugLog(__LINE__);
				return true;
			}
		}
	}
	else {
		if (g.createReturnVoid()) {
			debugLog(__LINE__);
			return true;
		}
	}

	ctx.setBreaked(true);
	ctx.setReturned(true);

	return false;
}

bool BreakNode::generate(Generator& g, Context& ctx) {
	Generator::BasicBlock successor = ctx.getSuccessorBlockForBreak();
	if (successor == nullptr) {
		ctx.addCompileError(std::make_shared<InvalidBreakError>(token_));
		return true;
	}

	if (g.createGoto(successor)) {
		debugLog(__LINE__);
		return true;
	}

	ctx.setBreaked(true);

	return false;
}

bool AssignNode::generate(Generator& g, Context& ctx) {
	if (dest_.generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	if (dest_.getValueType().isArgument) {
		ctx.addCompileError(std::make_shared<CanNotOverwriteArgumentError>(this->token_));
		return true;
	}

	if (value_->generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	Generator::Value temp;
	if (!value_->getValueType().isCompatible(dest_.getValueType())) {
		ctx.addCompileError(std::make_shared<TypeMismatchError>(token_, dest_.getValueType(), value_->getValueType()));
		return true;
	}
	if (castConstantToValueType(g, ctx, value_, dest_.getValueType(), temp)) {
		debugLog(__LINE__);
		return true;
	}

	if (g.createStore(temp, dest_.getGeneratedVariablePtr())) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool CompileUnitNode::generate(Generator& g, Context& ctx) {
	for (auto& s : structs_) {
		if (s.generateType(g)) {
			debugLog(__LINE__);
			return true;
		}
	}
	for (auto& s : structs_) {
		if (s.generateMember(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
	}

	for (auto& f : functions_) {
		if (f.generateDeclare(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
	}
	for (auto& f : functions_) {
		if (f.generateDefine(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
	}

	return false;
}

const FunctionNode* CompileUnitNode::getFunctionNode(const std::string& name) const {
	for (auto& f : functions_) {
		if (f.getName().getString() == name) {
			return &f;
		}
	}
	return nullptr;
}

bool StructNode::generateType(Generator& g) {
	return g.createStructType(name_.getString(), generatedType_);
}

bool StructNode::generateMember(Generator& g, Context& ctx) {
	std::vector<Generator::Type> types;
	types.push_back(g.getSizeType());
	for (auto member : members_) {
		if (member.generateType(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
		types.push_back(member.getGeneratedType());
	}

	generatedType_->setBody(types, false);

	return false;
}

bool FunctionNode::generateDeclare(Generator& g, Context& ctx) {

	// TODO: duplicate check

	if (returnType_.generate(g, ctx)) {
		debugLog(__LINE__);
		return true;
	}

	std::vector<Generator::Type> argumentTypes;
	for (auto& arg : args_) {
		if (arg.generateType(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}
		arg.setIsArgument(true);
		argumentTypes.push_back(arg.getGeneratedType());
	}

	Generator::FunctionType ft;
	if (g.createFunctionType(returnType_.getGeneratedType(), argumentTypes, hasVariableArgument_, ft)) {
		debugLog(__LINE__);
		return true;
	}
	if (g.createFunctionDeclare(ft, name_.getString(), generatedFunction_)) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool FunctionNode::generateDefine(Generator& g, Context& ctx) {
	if (block_ != nullptr) {
		if (block_->generateBlock(g, generatedFunction_, nullptr)) {
			debugLog(__LINE__);
			return true;
		}
		g.setInsertPoint(block_->getGeneratedBlock());

		ctx.addSymbolTable();
		if (addArgumentToSymbolTable(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		ValueType returnType = returnType_.getValueType();
		g.setCurrentReturnType(returnType);
		if (block_->generateStatements(g, ctx)) {
			debugLog(__LINE__);
			return true;
		}

		if (ctx.removeSymbolTable()) {
			debugLog(__LINE__);
			return true;
		}

		if (!ctx.isReturned()) {
			auto& returnType = returnType_.getValueType();
			if ((returnType.basicType == Token::Type::TYPE_VOID) && (returnType.pointerCount == 0) && (returnType.isReference == false)) {
				Generator::BasicBlock temp = ctx.getLastBlock();
				if (temp == nullptr) {
					g.setInsertPoint(block_->getGeneratedBlock());
				}
				else {
					g.setInsertPoint(ctx.getLastBlock());
				}
				g.createReturnVoid();
			}
			else {
				ctx.addCompileError(std::make_shared<MissingReturnError>(block_->getRightCurlyBracketToken()));
				return true;
			}
		}
		ctx.setReturned(false);

		ctx.setLastBlock(nullptr);
	}
	return false;
}

bool FunctionNode::addArgumentToSymbolTable(Generator& g, Context& ctx) {
	size_t index = 0;
	for (auto& arg : args_) {
		auto argValue = g.getArgument(index);
		if (argValue == nullptr) {
			debugLog(__LINE__);
			return true;
		}

		if (ctx.addSymbol(arg.getName().getString(), arg.getValueType(), argValue)) {
			debugLog(__LINE__);
			return true;
		}

		index++;
	}
	return false;
}

void Context::addCompileUnit(const CompileUnitNode& cu) {
	compileUnits_.push_back(cu);
}

bool Context::generate(Generator& g) {
	for (auto& unit : compileUnits_) {
		if (unit.generate(g, *this)) {
			debugLog(__LINE__);
			return true;
		}
	}
	return false;
}

void Context::addSymbolTable() {
	symbolTables_.emplace_back();
}

bool Context::removeSymbolTable() {
	if (symbolTables_.empty()) {
		debugLog(__LINE__);
		return true;
	}
	symbolTables_.pop_back();
	return false;
}

bool Context::addSymbol(const std::string& name, const ValueType& type, Generator::Value value) {
	auto table = symbolTables_.rbegin();
	if (table == symbolTables_.rend()) {
		debugLog(__LINE__);
		return true;
	}
	table->push_back(Symbol({ name, type, value }));
	return false;
}

bool Context::getSymbol(const std::string& name, ValueType& resultType, Generator::Value& resultValue) const {
	auto tableEnd = symbolTables_.rend();
	for (auto table = symbolTables_.rbegin(); table != tableEnd; ++table) {
		auto symbolEnd = table->rend();
		for (auto symbol = table->rbegin(); symbol != symbolEnd; ++symbol) {
			if (name == symbol->name) {
				resultType = symbol->type;
				resultValue = symbol->value;
				return false;
			}
		}
	}

	return true;
}
