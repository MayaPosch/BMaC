/*
	utility.cpp - Utility functions.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2018/08/30, Maya Posch
*/


#include "utility.h"

#include <algorithm>


// --- SPLIT_STRING ---
// Split string using the provided separator and return a vector with the parts.
void split_string(const std::string& str, char chr, std::vector<std::string>& vec) {
    std::string::const_iterator first = str.cbegin();
    std::string::const_iterator second = std::find(first + 1, str.cend(), chr);

    while (second != str.cend()) {
        vec.emplace_back(first, second);
        first = second;
        second = std::find(second + 1, str.cend(), chr);
    }

    vec.emplace_back(first, str.cend());
}
