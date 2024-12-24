#include <string>
#include <fstream>
#include <iostream>
#include "Token.h"
#include "Tokenizer.h"
#include "CompileError.h"
#include "Parser.h"

std::vector<std::string> debugLogs;

namespace {
	struct Flag {
		std::string sourceFilepath;
		std::string sourceFilename;

		bool parse(int argc, char** argv) {
			if (argc != 2) {
				return true;
			}

			sourceFilepath = argv[1];

			return false;
		}
	};

	bool splitPath(const std::string& filepath, std::string* dir, std::string* filename);
	bool skipUtf8Bom(std::istream& src);
}

int main(int argc, char** argv) {
	Flag flag;
	if (flag.parse(argc, argv)) {
		return 1;
	}

	if (splitPath(flag.sourceFilepath, nullptr, &flag.sourceFilename)) {
		return 1;
	}

	std::ifstream ifs(flag.sourceFilepath);
	if (!ifs) {
		return 1;
	}

	Generator exampleGenerator(flag.sourceFilename);
	exampleGenerator.init();
	exampleGenerator.llvmExample();

	Parser parser(flag.sourceFilepath);
	if (parser.fail()) {
		return 1;
	}
	if (parser.parse()) {
		for (auto& error : parser.getErrors()) {
			error->printErrorMessage(std::cerr);
			std::cerr << "\n";
		}

		return 1;
	}

	std::ofstream ofs("a.txt");
	DebugPrinter dp = { ofs, 0 };
	parser.getRootNode().debugPrint(dp);

	Generator generator(flag.sourceFilename);
	if (generator.init() ||
		parser.getRootNode().generate(generator)) {
		auto& errors = parser.getRootNode().getCompileErrors();
		//if (!errors.empty()) {
			for (auto& error : errors) {
				error->printErrorMessage(std::cerr);
				std::cerr << "\n";
			}
		//}
		//else {
			for (auto& log : debugLogs) {
				std::cerr << log << "\n";
			}
		//}
		return 1;
	}

	if (generator.writeString("a.ll")) {
		return 1;
	}

	//if (generator.writeObjectFile("a.obj")) {
	//	return 1;
	//}

	return 0;
}

namespace {
	bool splitPath(const std::string& filepath, std::string* dir, std::string* filename) {
		size_t pos = filepath.find_last_of("\\/");
		if (pos == std::string::npos) {
			if (dir) {
				dir->clear();
			}
			if (filename) {
				*filename = filepath;
			}
		}
		else {
			if (dir) {
				*dir = filepath.substr(0, pos);
			}
			if (filename) {
				*filename = filepath.substr(pos + 1);
			}
		}

		return false;
	}

	bool skipUtf8Bom(std::istream& src) {
		int c = src.get();

		// read UTF-8 BOM
		if (c == 0xEF) {
			c = src.get();
			if (c != 0xBB) {
				return true;	// invalid BOM
			}

			c = src.get();
			if (c != 0xBF) {
				return true;	// invalid BOM
			}

			c = src.get();
		}

		return false;
	}
}
