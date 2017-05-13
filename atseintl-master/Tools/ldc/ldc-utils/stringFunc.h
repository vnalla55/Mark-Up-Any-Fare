#ifndef stringFunc_H
#define stringFunc_H

#include <stdio.h>
#include <string>
#include <vector>

void
stringTok(const std::string& input, std::vector<std::string>& output, const std::string& delimiter);
void
stringUpper(std::string& input);
void
dumpHexData(const void* data, int size, bool zero = false, FILE* fp = stdout);

#endif
