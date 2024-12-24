#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <memory>
#include "Token.h"
#include "DebugPrinter.h"
#include "Generator.h"
#include "CompileError.h"

class TypeNode;
class VariableDefinitionNode;
class VariableValueNode;
class ValueListNode;

class ExpressionNode;
class UnaryOperationNode;
class BinaryOperationNode;
class CallNode;
class ConstantNode;
class CastNode;
class BlockNode;

class LetNode;
class IfNode;
class WhileNode;
class ReturnNode;
class BreakNode;
class AssignNode;

class CompileUnitNode;
class StructNode;
class FunctionNode;

class Context;

class Node {
public:
	Node() = default;
	Node(const Token& token) : token_(token) {}
	virtual ~Node() = default;

	virtual void setToken(const Token& token) {
		token_ = token;
	}

	virtual const Token& getToken() const {
		return token_;
	}

	virtual const std::vector<std::shared_ptr<ExpressionNode>>* getValues() const {
		return nullptr;
	}

	virtual const std::vector<ValueType>* getValueTypes() const {
		return nullptr;
	}

protected:
	Token token_;
};

class StatementNode : virtual public Node {
public:
	StatementNode() = default;
	virtual ~StatementNode() = default;
	virtual void debugPrint(DebugPrinter&) = 0;
	virtual bool generate(Generator&, Context&) = 0;
};

class ExpressionNode : virtual public Node {
public:
	ExpressionNode() :
		generatedValue_(nullptr), generatedValueBoolArray_(nullptr),
		generatedValueI8Array_(nullptr), generatedValueI16Array_(nullptr), generatedValueI32Array_(nullptr), generatedValueI64Array_(nullptr),
		generatedValueU8Array_(nullptr), generatedValueU16Array_(nullptr), generatedValueU32Array_(nullptr), generatedValueU64Array_(nullptr),
		generatedValueF32Array_(nullptr), generatedValueF64Array_(nullptr), generatedValueStringArray_(nullptr),
		constantBool_(false), constantInteger_(0), constantDouble_(0.0) {}
	virtual ~ExpressionNode() = default;
	virtual void debugPrint(DebugPrinter&) = 0;
	virtual bool generate(Generator&, Context&) = 0;

	virtual const Generator::Value& getGeneratedValue() const {
		return generatedValue_;
	}

	virtual const Generator::Value& getGeneratedValueBoolArray() const {
		return generatedValueBoolArray_;
	}

	virtual const Generator::Value& getGeneratedValueI8Array() const {
		return generatedValueI8Array_;
	}

	virtual const Generator::Value& getGeneratedValueI16Array() const {
		return generatedValueI16Array_;
	}

	virtual const Generator::Value& getGeneratedValueI32Array() const {
		return generatedValueI32Array_;
	}

	virtual const Generator::Value& getGeneratedValueI64Array() const {
		return generatedValueI64Array_;
	}

	virtual const Generator::Value& getGeneratedValueU8Array() const {
		return generatedValueU8Array_;
	}

	virtual const Generator::Value& getGeneratedValueU16Array() const {
		return generatedValueU16Array_;
	}

	virtual const Generator::Value& getGeneratedValueU32Array() const {
		return generatedValueU32Array_;
	}

	virtual const Generator::Value& getGeneratedValueU64Array() const {
		return generatedValueU64Array_;
	}

	virtual const Generator::Value& getGeneratedValueF32Array() const {
		return generatedValueF32Array_;
	}

	virtual const Generator::Value& getGeneratedValueF64Array() const {
		return generatedValueF64Array_;
	}

	virtual const Generator::Value& getGeneratedValueStringArray() const {
		return generatedValueStringArray_;
	}

	virtual const ValueType& getValueType() const {
		return valueType_;
	}

	virtual const bool getConstantBool() const {
		return constantBool_;
	}

	virtual const int64_t getConstantInteger() const {
		return constantInteger_;
	}

	virtual const double getConstantDouble() const {
		return constantDouble_;
	}

	virtual const std::string& getConstantString() const {
		return constantString_;
	}

protected:
	Generator::Value generatedValue_;
	Generator::Value generatedValueBoolArray_;
	Generator::Value generatedValueI8Array_;
	Generator::Value generatedValueI16Array_;
	Generator::Value generatedValueI32Array_;
	Generator::Value generatedValueI64Array_;
	Generator::Value generatedValueU8Array_;
	Generator::Value generatedValueU16Array_;
	Generator::Value generatedValueU32Array_;
	Generator::Value generatedValueU64Array_;
	Generator::Value generatedValueF32Array_;
	Generator::Value generatedValueF64Array_;
	Generator::Value generatedValueStringArray_;
	ValueType valueType_;
	bool constantBool_;
	int64_t constantInteger_;
	double constantDouble_;
	std::string constantString_;
};

class TypeNode : public Node {
public:
	TypeNode() : type_(), generatedType_(nullptr) {}
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

	void setType(Token::Type type) {
		type_.basicType = type;
	}

	void setPointerCount(size_t pointerCount) {
		type_.pointerCount = pointerCount;
	}

	const ValueType& getValueType() const {
		return type_;
	}

	void setIsReference(bool isReference) {
		type_.isReference = isReference;
	}

	void setIsArgument(bool isArgument) {
		type_.isArgument = isArgument;
	}

	const Generator::Type& getGeneratedType() const {
		return generatedType_;
	}

	void addArraySize(const std::shared_ptr<ExpressionNode> size) {
		arraySizes_.push_back(size);
	}

	void setValueType(const ValueType& type) {
		type_ = type;
	}

private:
	ValueType type_;
	Generator::Type generatedType_;
	std::vector<std::shared_ptr<ExpressionNode>> arraySizes_;
};

class VariableDefinitionNode : public Node {
public:
	VariableDefinitionNode(const Token& name, const TypeNode& type) : Node(name), name_(name), type_(type) {}
	void debugPrint(DebugPrinter& dp);
	bool generateType(Generator& g, Context& ctx);

	const Token& getName() const {
		return name_;
	}

	const Generator::Type& getGeneratedType() const {
		return type_.getGeneratedType();
	}

	const ValueType& getValueType() const {
		return type_.getValueType();
	}

	void setIsArgument(bool isArgument) {
		type_.setIsArgument(true);
	}

private:
	Token name_;
	TypeNode type_;
};

class UnaryOperationNode :public ExpressionNode {
public:
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

	void setOperator(const Token& operatorType) {
		operatorType_ = operatorType;
	}

	void setValue(const std::shared_ptr<ExpressionNode> value) {
		value_ = value;
	}

private:
	Token operatorType_;
	std::shared_ptr<ExpressionNode> value_;
};

class BinaryOperationNode : public ExpressionNode {
public:
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

	void setOperator(const Token& operatorType) {
		operatorType_ = operatorType;
	}

	void setLhs(const std::shared_ptr<ExpressionNode> lhs) {
		lhs_ = lhs;
	}

	void setRhs(const std::shared_ptr<ExpressionNode> rhs) {
		rhs_ = rhs;
	}

private:
	Token operatorType_;
	std::shared_ptr<ExpressionNode> lhs_;
	std::shared_ptr<ExpressionNode> rhs_;

	bool castIfCompatible(Generator& g, Context& ctx, Generator::Value&, Generator::Value&, ValueType&);
	bool checkOperand(Context& ctx, const Token& operatorToken, const ValueType& operandType, const std::shared_ptr<ExpressionNode>& value);
};

class VariableValueNode : public ExpressionNode {
public:
	VariableValueNode() : generatedVariablePtr_(nullptr), isRhsValue_(false) {}
	VariableValueNode(const VariableValueNode& other)
		: Node(other.name_), name_(other.name_), arrayIndex_(other.arrayIndex_), member_(other.member_), generatedVariablePtr_(other.generatedVariablePtr_), isRhsValue_(other.isRhsValue_) {}
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

	void setName(const Token& name) {
		name_ = name;
	}

	const Token& getName() const {
		return name_;
	}

	void setArrayIndex(const std::shared_ptr<ExpressionNode>& arrayIndex) {
		arrayIndex_ = arrayIndex;
	}

	void setMember(const VariableValueNode& member) {
		member_ = std::make_shared<VariableValueNode>(member);
	}


	const Generator::Value& getGeneratedVariablePtr() const {
		return generatedVariablePtr_;
	}

	void setIsRhsValue(bool isRhsValue) {
		isRhsValue_ = isRhsValue;
	}

private:
	Token name_;
	std::shared_ptr<ExpressionNode> arrayIndex_;
	std::shared_ptr<VariableValueNode> member_;
	Generator::Value generatedVariablePtr_;
	bool isRhsValue_;
};

class ValueListNode : public Node {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);
	bool generateForFunctionArgumants(Generator& g, Context& ctx, const CallNode& call, const FunctionNode& function);

	void addValue(const std::shared_ptr<ExpressionNode>& value) {
		values_.push_back(value);
	}

	const std::vector<std::shared_ptr<ExpressionNode>>* getValues() const {
		return &values_;
	}

	const std::vector<Generator::Value>& getGeneratedValues() const {
		return generatedValues_;
	}

	const std::vector<ValueType>* getValueType() const {
		return &valueTypes_;
	}

private:
	std::vector<std::shared_ptr<ExpressionNode>> values_;
	std::vector<Generator::Value> generatedValues_;
	std::vector<ValueType> valueTypes_;
};

class CallNode : public StatementNode, public ExpressionNode {
public:
	CallNode(const VariableValueNode& f, const ValueListNode& values) : Node(f.getToken()), f_(f), values_(values) {}
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

private:
	VariableValueNode f_;
	ValueListNode values_;
};

class ConstantNode : public ExpressionNode {
public:
	ConstantNode(const Token& constant) : Node(constant), constant_(constant){}
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

private:
	Token constant_;
};

class AggregateConstantNode : public ExpressionNode {
public:
	AggregateConstantNode(const Token& bracketLeft) : Node(bracketLeft) {}
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

	void setValues(const ValueListNode& values) {
		values_ = values;
	}

	const std::vector<Generator::Value>& getGeneratedValues() const {
		return values_.getGeneratedValues();
	}

	const std::vector<ValueType>* getValueTypes() const {
		return values_.getValueType();
	}

private:
	ValueListNode values_;

	bool generateArrayConstant(Generator&, Context&, Generator::Value&);
};

class CastNode : public ExpressionNode {
public:
	void debugPrint(DebugPrinter&);
	bool generate(Generator&, Context&);

	void setValue(const std::shared_ptr<ExpressionNode>& value) {
		value_ = value;
	}

	void setDestType(const TypeNode& destType) {
		destType_ = destType;
	}

private:
	std::shared_ptr<ExpressionNode> value_;
	TypeNode destType_;
};

class BlockNode : public Node {
public:
	void debugPrint(DebugPrinter& dp);
	bool generateBlock(Generator& g, const Generator::Function& function, const Generator::BasicBlock& insertBefore);
	bool generateStatements(Generator& g, Context& ctx, const Generator::BasicBlock& successorBlock = nullptr);

	void addStatement(const std::shared_ptr<StatementNode>& statement) {
		statements_.push_back(statement);
	}

	const Generator::BasicBlock& getGeneratedBlock() const {
		return generatedBlock_;
	}

	void addRightCurlyBracketToken(const Token& token) {
		rightCurlyBracketToken_ = token;
	}

	const Token& getRightCurlyBracketToken() const {
		return rightCurlyBracketToken_;
	}

private:
	std::vector<std::shared_ptr<StatementNode>> statements_;
	Generator::BasicBlock generatedBlock_;
	Token rightCurlyBracketToken_;
};

class LetNode : public StatementNode {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

	void setName(const Token& name) {
		name_ = name;
		token_ = name;
	}

	void setType(const TypeNode& type) {
		type_ = std::make_shared<TypeNode>(type);
	}

	bool hasType() const {
		return type_.operator bool();
	}

	void setIsHeap(bool isHeap) {
		isHeap_ = isHeap;
	}

	void setInitialValue(const std::shared_ptr<ExpressionNode>& initialValue) {
		initialValue_ = initialValue;
	}

private:
	Token name_;
	std::shared_ptr<TypeNode> type_;
	bool isHeap_;
	std::shared_ptr<ExpressionNode> initialValue_;
	Generator::Value generatedPtr_;
};

class IfNode : public StatementNode {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

	void setCondition(const std::shared_ptr<ExpressionNode>& condition) {
		condition_ = condition;
	}

	void setThenBlock(const BlockNode& block) {
		thenBlock_ = block;
	}

	void setElseBlock(const BlockNode& block) {
		elseBlock_ = std::make_shared<BlockNode>(block);
	}

private:
	std::shared_ptr<ExpressionNode> condition_;
	BlockNode thenBlock_;
	std::shared_ptr<BlockNode> elseBlock_;
};

class WhileNode : public StatementNode {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

	void setCondition(const std::shared_ptr<ExpressionNode>& condition) {
		condition_ = condition;
	}

	void setBlock(const BlockNode& block) {
		block_ = block;
	}

private:
	std::shared_ptr<ExpressionNode> condition_;
	BlockNode block_;
};

class ReturnNode : public StatementNode {
public:
	ReturnNode(const Token& returnToken) : Node(returnToken), returnToken_(returnToken) {}
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);


	void setValue(const std::shared_ptr<ExpressionNode>& value) {
		value_ = value;
	}

private:
	Token returnToken_;
	std::shared_ptr<ExpressionNode> value_;
};

class BreakNode : public StatementNode {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);
};

class AssignNode : public StatementNode {
public:
	AssignNode(const VariableValueNode& dest, const std::shared_ptr<ExpressionNode>& value) : dest_(dest), value_(value) {}
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

private:
	VariableValueNode dest_;
	std::shared_ptr<ExpressionNode> value_;
};

class CompileUnitNode {
public:
	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g, Context& ctx);

	void addStruct(const StructNode& structNode) {
		structs_.push_back(structNode);
	}

	void addFunction(const FunctionNode& functionNode) {
		functions_.push_back(functionNode);
	}

	const FunctionNode* getFunctionNode(const std::string& name) const;

	std::vector<FunctionNode>& getFunctions() {
		return functions_;
	}

private:
	std::vector<StructNode> structs_;
	std::vector<FunctionNode> functions_;
};

class StructNode : public Node {
public:
	StructNode() : generatedType_(nullptr) {}
	void debugPrint(DebugPrinter& dp);
	bool generateType(Generator& g);
	bool generateMember(Generator& g, Context& ctx);

	void setName(const Token& name) {
		name_ = name;
		token_ = name;
	}

	void addMember(const VariableDefinitionNode& member) {
		members_.push_back(member);
	}

private:
	Token name_;
	std::vector<VariableDefinitionNode> members_;
	Generator::StructType generatedType_;
};

class FunctionNode : public Node {
public:
	enum class Type {
		MAHINA,
		C,
	};

	void debugPrint(DebugPrinter& dp);
	bool generateDeclare(Generator& g, Context& ctx);
	bool generateDefine(Generator& g, Context& ctx);

	void setName(const Token& name) {
		name_ = name;
		token_ = name;
	}

	const Token& getName() const {
		return name_;
	}

	void addArgument(const VariableDefinitionNode& argument) {
		args_.push_back(argument);
	}

	const std::vector<VariableDefinitionNode>& getArguments() const {
		return args_;
	}

	void setVariableArgument() {
		hasVariableArgument_ = true;
	}

	bool hasVariableArgument() const {
		return hasVariableArgument_;
	}

	void setReturnType(const TypeNode& type) {
		returnType_ = type;
	}

	const TypeNode& getReturnType() const {
		return returnType_;
	}

	void setBlock(const BlockNode& block) {
		block_ = std::make_shared<BlockNode>(block);
	}

	const Generator::Function& getGeneratedFunction() const {
		return generatedFunction_;
	}

	void setFunctionType(Type type) {
		type_ = type;
	}

	Type getFunctionType() const {
		return type_;
	}

private:
	Token name_;
	std::vector<VariableDefinitionNode> args_;
	bool hasVariableArgument_;
	TypeNode returnType_;
	std::shared_ptr<BlockNode> block_;
	Generator::Function generatedFunction_;
	Type type_;

	bool addArgumentToSymbolTable(Generator& g, Context& ctx);
};

class Context {
public:
	Context() : objectType_(nullptr), lastBlock_(nullptr), breaked_(false), returned_(false) {}
	void addSymbolTable();
	bool removeSymbolTable();
	bool addSymbol(const std::string& name, const ValueType& type, Generator::Value value);
	bool getSymbol(const std::string& name, ValueType& resultType, Generator::Value& resultValue) const;

	void addCompileUnit(const CompileUnitNode& cu);

	void debugPrint(DebugPrinter& dp);
	bool generate(Generator& g);

	void addCompileError(const std::shared_ptr<CompileError>& error) {
		errors_.push_back(error);
	}

	const std::vector<std::shared_ptr<CompileError>>& getCompileErrors() const {
		return errors_;
	}

	const FunctionNode* getFunctionNode(const std::string& name) const {
		for (auto& cu : compileUnits_) {
			auto* fp = cu.getFunctionNode(name);
			if (fp != nullptr) {
				return fp;
			}
		}
		return nullptr;
	}

	void addSuccessorBlockForBreak(const Generator::BasicBlock& successorBlock) {
		successorBlocks_.push(successorBlock);
	}

	Generator::BasicBlock getSuccessorBlockForBreak() const {
		if (successorBlocks_.empty()) {
			return nullptr;
		}
		return successorBlocks_.top();
	}

	void removeSuccessorBlockForBreak() {
		successorBlocks_.pop();
	}

	void setLastBlock(const Generator::BasicBlock& block) {
		lastBlock_ = block;
	}

	const Generator::BasicBlock& getLastBlock() const {
		return lastBlock_;
	}

	void setBreaked(bool flag) {
		breaked_ = flag;
	}

	bool isBreaked() const {
		return breaked_;
	}

	void setReturned(bool flag) {
		returned_ = flag;
	}

	bool isReturned() const {
		return returned_;
	}

private:
	std::vector<CompileUnitNode> compileUnits_;
	std::vector<std::shared_ptr<CompileError>> errors_;
	Generator::Type objectType_;
	std::stack<Generator::BasicBlock> successorBlocks_;
	Generator::BasicBlock lastBlock_;
	bool breaked_;
	bool returned_;

	struct Symbol {
		std::string name;
		ValueType type;
		Generator::Value value;
	};
	std::vector<std::vector<Symbol>> symbolTables_;
};
