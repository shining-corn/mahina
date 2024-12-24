#pragma once

#include <stdint.h>
#include <string>

bool toBoolean(const std::string& str, bool& b);
bool toInt32(const std::string& str, int32_t& n);
bool toInt64(const std::string& str, int64_t& n);
bool toU32(const std::string& str, uint32_t& n);
bool toU64(const std::string& str, uint64_t& n);
bool toFloat(const std::string& str, float& f);
bool toDouble(const std::string& str, double& d);
