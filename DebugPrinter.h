#pragma once

#include <ostream>

struct DebugPrinter {
	std::ostream& o;
	size_t indentLevel;
};

std::ostream& operator<<(std::ostream& o, const DebugPrinter& dp);
