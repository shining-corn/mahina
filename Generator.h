#pragma once

#include <string>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LegacyPassManager.h"
#include "Token.h"
#include "ValueType.h"

class Generator
{
public:
	typedef llvm::Type* Type;
	typedef llvm::StructType* StructType;
	typedef llvm::ArrayType* ArrayType;
	typedef llvm::FunctionType* FunctionType;
	typedef llvm::Function* Function;
	typedef llvm::BasicBlock* BasicBlock;
	typedef llvm::Value* Value;
	typedef llvm::Constant* Constant;

	Generator(const std::string& filename) : builder_(context_), module_(filename, context_), targetMachine_(nullptr), fMalloc_(nullptr), builtInObjectTypes_() {}
	~Generator() = default;

	bool init();
	bool writeString(const std::string& outputPath) const;
	bool writeObjectFile(const std::string& outputPath);

	void llvmExample();

	void setCurrentPackageName(const std::string& name) {
		currentPackageName_ = name;
	}

	void setCurrentReturnType(const ValueType& type) {
		currentReturnType_ = type;
	}

	ValueType& getCurrentReturnType() {
		return currentReturnType_;
	}

	Type getSizeType();
	Type getTypeIdType();
	Value getArgument(size_t index);
	BasicBlock getCurrentBlock();
	void setInsertPoint(BasicBlock destBlock);

	bool createType(const ValueType& type, Type& result);
	bool createStructType(const std::string& name, StructType& result);
	bool createStructMember(const std::vector<Type>& typeList, StructType dest);
	bool createFunctionType(Type returnType, const std::vector<Type>& argumentTypes, bool hasVariableArguments, FunctionType& result);
	bool createFunctionDeclare(FunctionType functionType, const std::string& name, Function& result);
	bool createBasicBlock(const Function& function, const BasicBlock& insertBefore, BasicBlock& result);
	bool createIf(const Value& condition, const BasicBlock& blockTrue, const BasicBlock& blockFalse);
	bool createGoto(const BasicBlock& dest);
	bool createReturnVoid();
	bool createReturn(Value value);
	bool createNegate(Token::Type valueType, Value value, Value& result);
	bool createAdd(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createSub(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createMul(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createDiv(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createRem(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareLesserThan(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareLesserEqual(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareGreaterThan(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareGreaterEqual(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareEqual(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createCommpareNotEqual(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createLogicalOr(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createLogicalAnd(Token::Type valueType, Value lhs, Value rhs, Value& result);
	bool createBooleanConstant(bool b, Constant& result);
	bool createI8Constant(uint32_t value, Constant& result);
	bool createI16Constant(int64_t value, Constant& result);
	bool createI32Constant(uint32_t value, Constant& result);
	bool createI64Constant(int64_t value, Constant& result);
	bool createU8Constant(uint32_t value, Constant& result);
	bool createU16Constant(uint64_t value, Constant& result);
	bool createU32Constant(uint32_t value, Constant& result);
	bool createU64Constant(uint64_t value, Constant& result);
	bool createF32Constant(float value, Constant& result);
	bool createF64Constant(double value, Constant& result);
	bool createDoubleConstant(double value, Constant& result);
	bool createStringConstant(const std::string& str, Constant& resultValue);
	bool createArrayConstant(const Type& arrayType, const std::vector<Constant>& values, Constant& result);
	bool createCast(Token::Type srcType, Value srcValue, Token::Type destType, Value& result);
	bool createBitCast(Value src, Type destType, Value& result);
	bool createCall(Function f, const std::vector<Value>& values, Value& result);
	bool createCallMalloc(Type type, Value& result);
	bool createAlloc(Type type, Value& result);
	bool createStore(Value value, Value destPtr);
	bool createLoad(Value srcPtr, Value& resultValue);
	bool createInitializeObject(Value object, Value initializer);
	bool createGetArrayElement(const Type& type, const Value& array, uint64_t index, Value& result);
	bool createPtrType(const Type& type, Type& result);
	bool createGlobalVariable(const Type& type, const Constant& value, Value& result);

private:
	class BuiltInObjectTypes {
	public:
		bool init(Generator& g);
		StructType getType(Token::Type type) const;

	private:
		StructType types_[static_cast<size_t>(Token::Type::TYPE_F64) - static_cast<size_t>(Token::Type::TYPE_BOOL) + 1];
	};

	llvm::LLVMContext context_;
	llvm::IRBuilder<> builder_;
	llvm::Module module_;
	llvm::TargetMachine* targetMachine_;
	std::string currentPackageName_;
	ValueType currentReturnType_;
	Function fMalloc_;
	BuiltInObjectTypes builtInObjectTypes_;

	// see also createStructMember()
	const unsigned int kReferenceCountMemberIndex = 0;
	const unsigned int kTypeIdMemberIndex = 1;
	const unsigned int kStructEntityMemberIndex = 2;

	bool getType(Token::Type, Generator::Type&);
	bool createReferenceType(Token::Type, Type&);
	bool createBuiltInCode();
	bool createMallocDeclare();
	bool createTruncOrExt(Value&, Type, Value&);
	bool createSizeOf(Type, Value&);
};
