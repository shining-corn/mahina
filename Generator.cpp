#include "Generator.h"
#include <sstream>
#include <iostream>

extern std::vector<std::string> debugLogs;

namespace {
	void debugLog(size_t line) {
		std::stringstream ss;
		ss << __FILE__ << ":" << line;
		debugLogs.push_back(ss.str());
	}
}

void Generator::llvmExample() {
	auto ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(context_), false);
	auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "example", module_);
	auto bb = llvm::BasicBlock::Create(context_, "", f);
	builder_.SetInsertPoint(bb);

	llvm::Value* v1 = builder_.CreateAlloca(llvm::Type::getInt32Ty(context_));
	llvm::Constant* con = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 123);
	builder_.CreateStore(con, v1);
	llvm::Value* v2 = builder_.CreateLoad(v1);
	llvm::Value* value = con;

	llvm::APInt ap = con->getUniqueInteger();
	uint64_t n1 = ap.getZExtValue();
	int64_t n2 = ap.getSExtValue();
}

bool Generator::init() {
	std::string targetTriple = llvm::sys::getDefaultTargetTriple();

	llvm::InitializeNativeTarget();
	std::string errorMessage;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, errorMessage);
	if (!target) {
		debugLog(__LINE__);
		return true;
	}

	llvm::TargetOptions targetOptions;
	targetMachine_ = target->createTargetMachine(targetTriple, "generic", "", targetOptions, llvm::Optional<llvm::Reloc::Model>());
	if (!targetMachine_) {
		debugLog(__LINE__);
		return true;
	}

	module_.setTargetTriple(targetTriple);
	module_.setDataLayout(targetMachine_->createDataLayout());

	if (createBuiltInCode()) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool Generator::createBuiltInCode() {
	if (createMallocDeclare() ||
		builtInObjectTypes_.init(*this))
	{
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool Generator::createMallocDeclare() {
	std::vector<Type> args = { getSizeType() };
	FunctionType ft = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(context_), args, false);
	if (createFunctionDeclare(ft, "malloc", fMalloc_)) {
		debugLog(__LINE__);
		return true;
	}
	return false;
}

bool Generator::writeString(const std::string& outputPath) const {
	std::error_code errorCode;
	llvm::raw_fd_ostream stream(outputPath, errorCode);
	if (errorCode) {
		debugLog(__LINE__);
		return true;
	}
	module_.print(stream, nullptr);

	return false;
}

bool Generator::writeObjectFile(const std::string& outputPath) {
	std::error_code errorCode;
	llvm::raw_fd_ostream stream(outputPath, errorCode);
	if (errorCode) {
		debugLog(__LINE__);
		return true;
	}

	llvm::InitializeNativeTargetAsmPrinter();
	llvm::legacy::PassManager passManager;
	bool error = targetMachine_->addPassesToEmitFile(passManager, stream, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile);
	if (error) {
		debugLog(__LINE__);
		return true;
	}

	passManager.run(module_);
	stream.flush();

	return false;
}

Generator::Type Generator::getSizeType() {
	if (sizeof(size_t) == 4) {
		return llvm::Type::getInt32Ty(context_);
	}
	return llvm::Type::getInt64Ty(context_);
}

Generator::Type Generator::getTypeIdType() {
	return llvm::Type::getInt32Ty(context_);
}

Generator::Value Generator::getArgument(size_t index) {
	auto block = builder_.GetInsertBlock();
	if (block == nullptr) {
		return nullptr;
	}
	auto f = block->getParent();
	if (f == nullptr) {
		return nullptr;
	}
	return f->getArg(index);
}

Generator::BasicBlock Generator::getCurrentBlock() {
	return builder_.GetInsertBlock();
}

void Generator::setInsertPoint(BasicBlock destBlock) {
	builder_.SetInsertPoint(destBlock);
}

bool Generator::createType(const ValueType& type, Type& result) {
	if (type.isReference) {
		if (createReferenceType(type.basicType, result)) {
			debugLog(__LINE__);
			return true;
		}
	}
	else {
		if ((type.basicType == Token::Type::TYPE_VOID) && (type.pointerCount != 0)) {
			result = llvm::Type::getInt8Ty(context_);
		}
		else {
			if (getType(type.basicType, result)) {
				debugLog(__LINE__);
				return true;
			}
		}
	}

	for (size_t i = 0; i < type.pointerCount; ++i) {
		result = llvm::PointerType::get(result, 0);
	}

	for (auto& arraySize : type.arraySizes) {
		if (arraySize != 0) {
			result = llvm::ArrayType::get(result, arraySize);
		}
		else {
			debugLog(__LINE__);
			return true;
		}
	}

	return false;
}

bool Generator::getType(Token::Type type, Generator::Type& result) {
	switch (type) {
	case Token::Type::TYPE_VOID:
		result = llvm::Type::getVoidTy(context_);
		break;
	case Token::Type::TYPE_BOOL:
	case Token::Type::CONSTANT_BOOL:
		result = llvm::Type::getInt1Ty(context_);
		break;
	case Token::Type::TYPE_I8:
	case Token::Type::TYPE_U8:
		result = llvm::Type::getInt8Ty(context_);
		break;
	case Token::Type::TYPE_I16:
	case Token::Type::TYPE_U16:
		result = llvm::Type::getInt16Ty(context_);
		break;
	case Token::Type::TYPE_I32:
	case Token::Type::TYPE_U32:
		result = llvm::Type::getInt32Ty(context_);
		break;
	case Token::Type::TYPE_I64:
	case Token::Type::TYPE_U64:
	case Token::Type::CONSTANT_INTEGER:
		result = llvm::Type::getInt64Ty(context_);
		break;
	case Token::Type::TYPE_F32:
		result = llvm::Type::getFloatTy(context_);
		break;
	case Token::Type::TYPE_F64:
	case Token::Type::CONSTANT_FLOAT:
		result = llvm::Type::getDoubleTy(context_);
		break;
	case Token::Type::CONSTANT_STRING:
		result = llvm::Type::getInt8PtrTy(context_);
		break;
	default:
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool Generator::createReferenceType(Token::Type type, Type& result) {
	auto llvmType = builtInObjectTypes_.getType(type);
	result = llvm::PointerType::get(llvmType, 0);
	return result == nullptr;
}

bool Generator::createStructType(const std::string& name, StructType& result) {
	result = llvm::StructType::create(context_, name);
	return result == nullptr;
}

bool Generator::createStructMember(const std::vector<Type>& typeList, StructType dest) {
	std::vector<llvm::Type*> members;

	members.push_back(getSizeType());
	members.push_back(getTypeIdType());

	for (auto type : typeList) {
		members.push_back(type);
	}

	dest->setBody(members, false);

	return false;
}

bool Generator::createFunctionType(Type returnType, const std::vector<Type>& argumentTypes, bool hasVariableArguments, FunctionType& result) {
	result = llvm::FunctionType::get(returnType, argumentTypes, hasVariableArguments);
	return result == nullptr;
}

bool Generator::createFunctionDeclare(Generator::FunctionType functionType, const std::string& name, Generator::Function& result) {
	result = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module_);
	return result == nullptr;
}

bool Generator::createBasicBlock(const Function& function, const BasicBlock& insertBefore, BasicBlock& result) {
	if (function == nullptr) {
		result = llvm::BasicBlock::Create(context_, "", builder_.GetInsertBlock()->getParent(), insertBefore);
	}
	else {
		result = llvm::BasicBlock::Create(context_, "", function, insertBefore);
	}
	return result == nullptr;
}

bool Generator::createIf(const Value& condition, const BasicBlock& blockTrue, const BasicBlock& blockFalse) {
	return builder_.CreateCondBr(condition, blockTrue, blockFalse) == nullptr;
}

bool Generator::createGoto(const BasicBlock& dest) {
	return builder_.CreateBr(dest) == nullptr;
}

bool Generator::createReturnVoid() {
	return (builder_.CreateRetVoid()) == nullptr;
}

bool Generator::createReturn(Value value) {
	return builder_.CreateRet(value) == nullptr;
}

bool Generator::createNegate(Token::Type valueType, Value value, Value& result) {
	if ((Token::isFloatingPointType(valueType)) || (valueType == Token::Type::CONSTANT_FLOAT)) {
		result = builder_.CreateFNeg(value);
	}
	else if ((Token::isIntegerType(valueType)) || (valueType == Token::Type::CONSTANT_INTEGER)) {
		result = builder_.CreateNeg(value);
	}
	else { // unsigned integer
		debugLog(__LINE__);
		return true;
	}

	return result == nullptr;
}

bool Generator::createAdd(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFAdd(lhs, rhs);
	}
	else {
		result = builder_.CreateAdd(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createSub(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFSub(lhs, rhs);
	}
	else {
		result = builder_.CreateSub(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createMul(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFMul(lhs, rhs);
	}
	else {
		result = builder_.CreateMul(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createDiv(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFDiv(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateSDiv(lhs, rhs);
	}
	else { // unsigned integer
		result = builder_.CreateUDiv(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createRem(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFRem(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateSRem(lhs, rhs);
	}
	else { // unsigned integer
		result = builder_.CreateURem(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareLesserThan(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpOLT(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateICmpSLT(lhs, rhs);
	}
	else { // unsigend integer
		result = builder_.CreateICmpULT(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareLesserEqual(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpOLE(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateICmpSLE(lhs, rhs);
	}
	else { // unsigend integer
		result = builder_.CreateICmpULE(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareGreaterThan(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpOGT(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateICmpSGT(lhs, rhs);
	}
	else { // unsigend integer
		result = builder_.CreateICmpUGT(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareGreaterEqual(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpOGE(lhs, rhs);
	}
	else if (Token::isSignedIntegerType(valueType)) {
		result = builder_.CreateICmpSGE(lhs, rhs);
	}
	else { // unsigend integer
		result = builder_.CreateICmpUGE(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareEqual(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpOEQ(lhs, rhs);
	}
	else {
		result = builder_.CreateICmpEQ(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createCommpareNotEqual(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	if (Token::isFloatingPointType(valueType)) {
		result = builder_.CreateFCmpONE(lhs, rhs);
	}
	else {
		result = builder_.CreateICmpNE(lhs, rhs);
	}
	return result == nullptr;
}

bool Generator::createLogicalOr(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	// TODO
	debugLog(__LINE__);
	return true;;
}

bool Generator::createLogicalAnd(Token::Type valueType, Value lhs, Value rhs, Value& result) {
	// TODO
	debugLog(__LINE__);
	return true;;
}

bool Generator::createBooleanConstant(bool b, Constant& result) {
	if (b) {
		result = llvm::ConstantInt::getTrue(context_);
	}
	else {
		result = llvm::ConstantInt::getFalse(context_);
	}
	return result == nullptr;
}

bool Generator::createI8Constant(uint32_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt8Ty(context_), value, true);
	return result == nullptr;
}

bool Generator::createI16Constant(int64_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt16Ty(context_), value, true);
	return result == nullptr;
}

bool Generator::createI32Constant(uint32_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), value, true);
	return result == nullptr;
}

bool Generator::createI64Constant(int64_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), value, true);
	return result == nullptr;
}

bool Generator::createU8Constant(uint32_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt8Ty(context_), value);
	return result == nullptr;
}

bool Generator::createU16Constant(uint64_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt16Ty(context_), value);
	return result == nullptr;
}

bool Generator::createU32Constant(uint32_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), value);
	return result == nullptr;
}

bool Generator::createU64Constant(uint64_t value, Constant& result) {
	result = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), value);
	return result == nullptr;
}

bool Generator::createF32Constant(float value, Constant& result) {
	result = llvm::ConstantFP::get(context_, llvm::APFloat(value));
	return result == nullptr;
}

bool Generator::createF64Constant(double value, Constant& result) {
	result = llvm::ConstantFP::get(context_, llvm::APFloat(value));
	return result == nullptr;
}

bool Generator::createDoubleConstant(double value, Constant& result) {
	result = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context_), value);
	return result == nullptr;
}

bool Generator::createStringConstant(const std::string& str, Constant& result) {
	result = builder_.CreateGlobalStringPtr(str);
	return result == nullptr;
}

bool Generator::createArrayConstant(const Type& arrayType, const std::vector<Constant>& values, Constant& result) {
	if (arrayType->isArrayTy()) {
		result = llvm::ConstantArray::get(static_cast<ArrayType>(arrayType), values);
		return result == nullptr;
	}
	return true;
}

bool Generator::createCast(Token::Type srcType, Value srcValue, Token::Type destType, Value& result) {
	Type llvmDestType;
	if (getType(destType, llvmDestType)) {
		debugLog(__LINE__);
		return true;
	}

	if (srcType == destType) {
		result = srcValue;
		return false;
	}

	if (srcType == Token::Type::TYPE_F32) {
		if (destType == Token::Type::TYPE_F64) {
			result = builder_.CreateFPExt(srcValue, llvmDestType);
		}
		else if (Token::isSignedIntegerType(destType)) {
			result = builder_.CreateFPToSI(srcValue, llvmDestType);
		}
		else { // unsigned integer
			result = builder_.CreateFPToUI(srcValue, llvmDestType);
		}
	}
	else if ((srcType == Token::Type::TYPE_F64) || (srcType == Token::Type::CONSTANT_FLOAT)) {
		if (destType == Token::Type::TYPE_F64) {
			result = srcValue;
		}
		else if (destType == Token::Type::TYPE_F32) {
			result = builder_.CreateFPTrunc(srcValue, llvmDestType);
		}
		else if (Token::isSignedIntegerType(destType)) {
			result = builder_.CreateFPToSI(srcValue, llvmDestType);
		}
		else { // unsigned integer
			result = builder_.CreateFPToUI(srcValue, llvmDestType);
		}
	}
	else if (Token::isSignedIntegerType(srcType)) {
		if (Token::isFloatingPointType(destType)) {
			result = builder_.CreateSIToFP(srcValue, llvmDestType);
		}
		else {
			if (createTruncOrExt(srcValue, llvmDestType, result)) {
				debugLog(__LINE__);
				return true;
			}
		}
	}
	else { // unsigned integer
		if (Token::isFloatingPointType(destType)) {
			result = builder_.CreateUIToFP(srcValue, llvmDestType);
		}
		else {
			if (createTruncOrExt(srcValue, llvmDestType, result)) {
				debugLog(__LINE__);
				return true;
			}
		}
	}

	return result == nullptr;
}

bool Generator::createBitCast(Value src, Type destType, Value& result) {
	result = builder_.CreateBitCast(src, destType);
	return result == nullptr;
}

bool Generator::createTruncOrExt(Value& srcValue, Type destType, Value& result) {
	size_t srcSize = srcValue->getType()->getIntegerBitWidth();
	size_t destSize = destType->getIntegerBitWidth();
	if (destSize < srcSize) {
		result = builder_.CreateTrunc(srcValue, destType);
	}
	else if (srcSize < destSize) {
		result = builder_.CreateZExt(srcValue, destType);
	}
	else {
		result = srcValue;
	}

	return result == nullptr;
}

bool Generator::createCall(Function f, const std::vector<Value>& values, Value& result) {
	result = builder_.CreateCall(f, values);
	return result == nullptr;
}

bool Generator::createCallMalloc(Type type, Value& result) {
	Value size;
	if (createSizeOf(type, size)) {
		debugLog(__LINE__);
		return true;
	}

	std::vector<Value> values = { size };
	if (createCall(fMalloc_, values, result)) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool Generator::createAlloc(Type type, Value& result) {
	auto f = builder_.GetInsertBlock()->getParent();
	auto& entryBlock = f->getEntryBlock();
	llvm::IRBuilder<> tempBuilder(&entryBlock, entryBlock.begin());
	result = tempBuilder.CreateAlloca(type);
	return result == nullptr;
}

bool Generator::createStore(Value value, Value destPtr) {
	if (value == nullptr) {
		value = llvm::Constant::getNullValue(destPtr->getType()->getPointerElementType());
	}
	return builder_.CreateStore(value, destPtr) == nullptr;
}

bool Generator::createLoad(Value srcPtr, Value& resultValue) {
	resultValue = builder_.CreateLoad(srcPtr);
	return resultValue == nullptr;
}

bool Generator::createInitializeObject(Value object, Value initializer) {
	static const std::vector<Value> referenceCountIndex = {
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), kReferenceCountMemberIndex),
	};

	auto referenceCount = builder_.CreateGEP(object, referenceCountIndex);
	if (referenceCount == nullptr) {
		debugLog(__LINE__);
		return true;
	}

	if (builder_.CreateStore(llvm::ConstantInt::get(getSizeType(), 1), referenceCount) == nullptr) {
		debugLog(__LINE__);
		return true;
	}

	static const std::vector<Value> entityIndex = {
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), kStructEntityMemberIndex),
	};

	auto entity = builder_.CreateGEP(object, entityIndex);
	if (entity == nullptr) {
		debugLog(__LINE__);
		return true;
	}


	if (initializer == nullptr) {
		initializer = llvm::Constant::getNullValue(entity->getType()->getPointerElementType());
	}
	if (builder_.CreateStore(initializer, entity) == nullptr) {
		debugLog(__LINE__);
		return true;
	}

	return false;
}

bool Generator::createSizeOf(Type type, Value& result) {
	auto ptr = builder_.CreateGEP(
		type,
		llvm::ConstantPointerNull::get(llvm::PointerType::get(type, 0)),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 1)
	);
	if (ptr == nullptr) {
		debugLog(__LINE__);
		return true;
	}

	result = builder_.CreatePtrToInt(ptr, getSizeType());
	return result == nullptr;
}

static const Token::Type builtInTypes[] = { Token::Type::TYPE_BOOL, Token::Type::TYPE_I8, Token::Type::TYPE_I16, Token::Type::TYPE_I32, Token::Type::TYPE_I64, Token::Type::TYPE_U8, Token::Type::TYPE_U16, Token::Type::TYPE_U32, Token::Type::TYPE_U64, Token::Type::TYPE_F32, Token::Type::TYPE_F64 };
static const std::string builtInTypeNames[sizeof(builtInTypes) / sizeof(Token::Type)] = { ".bool", ".i8", ".i16", ".i32", ".i64",".u8", ".u16", ".u32", ".u64",".f32", ".f64" };

bool Generator::BuiltInObjectTypes::init(Generator& g) {
	for (size_t i = 0; i < sizeof(builtInTypes) / sizeof(Token::Type); i++) {
		if (g.createStructType(builtInTypeNames[i], types_[i])) {
			debugLog(__LINE__);
			return true;
		}

		std::vector<llvm::Type*> members;
		members.push_back(g.getSizeType());
		members.push_back(g.getTypeIdType());
		Type llvmType;
		if (g.getType(builtInTypes[i], llvmType)) {
			debugLog(__LINE__);
			return true;
		}
		members.push_back(llvmType);

		types_[i]->setBody(members, false);
	}

	return false;
}

Generator::StructType Generator::BuiltInObjectTypes::getType(Token::Type type) const {
	for (size_t i = 0; i < sizeof(builtInTypes) / sizeof(Token::Type); i++) {
		if (type == builtInTypes[i]) {
			return types_[i];
		}
	}

	return nullptr;
}

bool Generator::createGetArrayElement(const Type& type, const Value& array, uint64_t index, Value& result) {
	const std::vector<Value> indexes = {
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), index),
	};

	result = builder_.CreateGEP(type, array, indexes);
	return result == nullptr;
}

bool Generator::createPtrType(const Type& type, Type& result) {
	result = llvm::PointerType::get(type, 0);
	return result == nullptr;
}

bool Generator::createGlobalVariable(const Type& arrayType, const Constant& value, Value& result) {
	result = new llvm::GlobalVariable(
		module_,
		arrayType,
		false,
		llvm::GlobalValue::CommonLinkage,
		value
	);
	return result == nullptr;
}
