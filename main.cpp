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
#include "Tools.h"

struct Data {
    std::set<std::string> seenStructs;
    std::string structsOut;
    std::string funcsOut;
};

static CXChildVisitResult visitor(CXCursor cursor,
                                  CXCursor /* parent */,
                                  CXClientData client_data)
{
    const CXSourceLocation loc = clang_getCursorLocation(cursor);
    if (!clang_Location_isFromMainFile(loc) && !isMatchingInclude(loc)) {
        // CXFile file = nullptr;
        // clang_getSpellingLocation(loc, &file, nullptr, nullptr, nullptr);
        // fprintf(stderr, "DITCHING FILE: %s\n", eatString(clang_getFileName(file)).c_str());
        return CXChildVisit_Continue;
    // } else {
    //     CXFile file = nullptr;
    //     clang_getSpellingLocation(loc, &file, nullptr, nullptr, nullptr);
    //     fprintf(stderr, "NOT DITCHING FILE: %s\n", eatString(clang_getFileName(file)).c_str());
    }

    char buf[1024];
    Data &data = *reinterpret_cast<Data *>(client_data);
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_InclusionDirective:
        return CXChildVisit_Continue;
    case CXCursor_FunctionDecl: {
        // printf("GOT FUNC %s\n", eatString(clang_getCursorSpelling(cursor)).c_str());
        // if (eatString(clang_getCursorSpelling(cursor)) != "RAND_DRBG_set_callbacks")
        //     break;
        if (data.funcsOut.empty()) {
            data.funcsOut = "[";
        } else {
            data.funcsOut += ",";
        }
        std::string args;
        for (int i=0, max = clang_Cursor_getNumArguments(cursor); i<max; ++i) {
            CXCursor arg = clang_Cursor_getArgument(cursor, i);
            if (!args.empty())
                args += ", ";
            CXType type = clang_getCursorType(arg);

            snprintf(buf, sizeof(buf), "{\"name\": \"%s\", \"type\": \"%s\"}",
                     eatString(clang_getCursorSpelling(arg)).c_str(), typeString(type).c_str());

            args += buf;
        }
        snprintf(buf, sizeof(buf), "{ \"name\": \"%s\", \"location\": \"%s\", \"returnValue\": \"%s\", \"arguments\": [%s] }",
                 eatString(clang_getCursorSpelling(cursor)).c_str(), toString(loc).c_str(),
                 typeString(clang_getCursorResultType(cursor)).c_str(),
                 args.c_str());
        data.funcsOut += buf;
        break; }
    case CXCursor_StructDecl: {
        std::string name = eatString(clang_getCursorSpelling(cursor));
        if (name.empty() || !data.seenStructs.insert(name).second)
            break;
        // printf("GOT STRUCT %s\n", eatString(clang_getCursorSpelling(cursor)).c_str());
        if (data.funcsOut.empty()) {
            data.structsOut = "[";
        } else {
            data.structsOut += ",";
        }

        snprintf(buf, sizeof(buf), "{ \"name\": \"%s\", \"location\": \"%s\" }",
                 eatString(clang_getCursorSpelling(cursor)).c_str(), toString(loc).c_str());
        data.structsOut += buf;
        break; }
    default:
        break;
    }

    return CXChildVisit_Recurse;
}

int main(int argc, char **argv)
{
    char file[1024];
    strcpy(file, "/tmp/parselib_XXXXXX");
    int fd = mkstemp(file);
    FILE *f = fdopen(fd, "w");
    // printf("%s -> %d\n", file, fd);
    std::vector<const char *> args;
    // args.push_back("gcc");
    args.push_back("-x");
    args.push_back("c++");
    // args.push_back("-c");
    // args.push_back(file);
    bool includes = false;
    for (int i=1; i<argc; ++i) {
        if (argv[i][0] == '-') {
            args.push_back(argv[i]);
        } else {
            includes = true;
            fprintf(f, "#include <%s>\n", argv[i]);
        }
    }
    if (!includes) {
        fprintf(f, "%s", opensslincludes());
    }
    fclose(f);
    ::close(fd);
    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit unit = nullptr;
    CXErrorCode error = clang_parseTranslationUnit2(index, file, &args[0], args.size(), nullptr, 0, clang_defaultEditingTranslationUnitOptions(), &unit);
    (void)error;
    Data data;
    clang_visitChildren(clang_getTranslationUnitCursor(unit), visitor, &data);

    // printf("%d\n", error);
    // for (const char *a : args) {
    //     printf("%s ", a);
    // }
    // printf("\n");

    if (unit)
        clang_disposeTranslationUnit(unit);
    if (index)
        clang_disposeIndex(index);

    if (data.structsOut.empty()) {
        data.structsOut = "[]";
    } else {
        data.structsOut += ']';
    }

    if (data.funcsOut.empty()) {
        data.funcsOut = "[]";
    } else {
        data.funcsOut += ']';
    }

    // printf("%s %p %p\n", file, unit, index);
    printf("{\"structs\":%s,\"functions\":%s}\n", data.structsOut.c_str(), data.funcsOut.c_str());
    unlink(file);
    return 0;

    // for (int i=1; i<argc; ++i) {
    //     parseFile(argv[i]);
    // }
    // return foo.bar();
}

