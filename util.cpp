#include "util.h"

bool toBoolean(const std::string& str, bool& b) {
	if (str == "true") {
		b = true;
		return false;
	}
	if (str == "false") {
		b = false;
		return false;
	}

	return true;
}

bool toInt32(const std::string& str, int32_t& n) {
	size_t index = 0;
	int32_t num = std::stoi(str, &index, 0);
	if (index - str.size() != 0) {
		return true;
	}

	n = num;
	return false;
}

bool toInt64(const std::string& str, int64_t& n) {
	size_t index = 0;
	int64_t num = std::stoll(str, &index, 0);
	if (index - str.size() != 0) {
		return true;
	}

	n = num;
	return false;
}

bool toU32(const std::string& str, uint32_t& n) {
	size_t index = 0;
	uint32_t num = std::stoul(str, &index, 0);
	if (index - str.size() != 0) {
		return true;
	}

	n = num;
	return false;
}

bool toU64(const std::string& str, uint64_t& n) {
	size_t index = 0;
	uint64_t num = std::stoull(str, &index, 0);
	if (index - str.size() != 0) {
		return true;
	}

	n = num;
	return false;
}

bool toFloat(const std::string& str, float& f) {
	size_t index = 0;
	float num = std::stof(str, &index);
	if (index - str.size() != 0) {
		return true;
	}

	f = num;
	return false;
}

bool toDouble(const std::string& str, double& d) {
	size_t index = 0;
	double num = std::stod(str, &index);
	if (index - str.size() != 0) {
		return true;
	}

	d = num;
	return false;
}
