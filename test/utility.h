/*
	utility.h - Utility functions.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2018/08/30, Maya Posch
*/


#ifndef UTILITY_H
#define UTILITY_H


#include <string>
#include <vector>


// --- SPLIT_STRING ---
// Split string using the provided separator and return a vector with the parts.
void split_string(const std::string& str, char chr, std::vector<std::string>& vec);

#endif
