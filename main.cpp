#include <string>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <clang-c/Index.h>

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
    args.push_back(file);
    for (int i=1; i<argc; ++i) {
        if (argv[i][0] == '-') {
            args.push_back(argv[i]);
        } else {
            fprintf(f, "#include <%s>\n", argv[i]);
        }
    }
    fclose(f);
    ::close(fd);
    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit unit = nullptr;
    CXErrorCode error = clang_parseTranslationUnit2(index, file, &args[0], args.size(), nullptr, 0, clang_defaultEditingTranslationUnitOptions(), &unit);

    printf("%d\n", error);
    // for (const char *a : args) {
    //     printf("%s ", a);
    // }
    printf("\n");

    if (unit)
        clang_disposeTranslationUnit(unit);
    if (index)
        clang_disposeIndex(index);

    printf("%s %p %p\n", file, unit, index);
    // unlink(file);
    return 0;

    // for (int i=1; i<argc; ++i) {
    //     parseFile(argv[i]);
    // }
    // return foo.bar();
}
