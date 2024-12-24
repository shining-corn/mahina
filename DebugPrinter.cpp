#include "DebugPrinter.h"

std::ostream& operator<<(std::ostream& o, const DebugPrinter& dp) {
	for (size_t i = 0; i < dp.indentLevel; ++i) {
		o << "    ";
	}
	return o;
}
