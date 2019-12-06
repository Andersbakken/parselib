#ifndef Tools_h
#define Tools_h

#include <string>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <set>
#include <assert.h>
#include <unistd.h>
#include <clang-c/Index.h>

const char *builtinTypeName(CXTypeKind kind);
CXCursor findFirstChild(CXCursor parent);
bool endsWith(const std::string &haystack, const std::string &needle);
inline bool endsWith(const std::string &haystack, char ch) { return endsWith(haystack, std::string(&ch, 1)); }
std::string typeName(const CXCursor &cursor);
std::string typeString(const CXType &type);
std::string eatString(CXString str);
bool isMatchingInclude(CXSourceLocation loc);
std::string toString(CXSourceLocation loc);

const char *opensslincludes();

#endif
