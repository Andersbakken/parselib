#include <string>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <clang-c/Index.h>

const char *opensslincludes();
struct Data {


};


CXChildVisitResult visitor(CXCursor cursor,
                           CXCursor /* parent */,
                           CXClientData client_data)
{
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_FunctionDecl:
        printf("GOT FUNC\n");
        break;
    case CXCursor_StructDecl:
        printf("GOT STRUCT\n");
        break;
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
    Data data;
    clang_visitChildren(clang_getTranslationUnitCursor(unit), visitor, &data);

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

const char *opensslincludes() {
    return ("#include </usr/include/openssl/aes.h>\n"
            "#include </usr/include/openssl/ebcdic.h>\n"
            "#include </usr/include/openssl/rand_drbg.h>\n"
            "#include </usr/include/openssl/asn1.h>\n"
            "#include </usr/include/openssl/ec.h>\n"
            "#include </usr/include/openssl/randerr.h>\n"
            "#include </usr/include/openssl/ecdh.h>\n"
            "#include </usr/include/openssl/rc2.h>\n"
            "#include </usr/include/openssl/asn1err.h>\n"
            "#include </usr/include/openssl/ecdsa.h>\n"
            "#include </usr/include/openssl/rc4.h>\n"
            "#include </usr/include/openssl/asn1t.h>\n"
            "#include </usr/include/openssl/ecerr.h>\n"
            "#include </usr/include/openssl/rc5.h>\n"
            "#include </usr/include/openssl/async.h>\n"
            "#include </usr/include/openssl/engine.h>\n"
            "#include </usr/include/openssl/ripemd.h>\n"
            "#include </usr/include/openssl/asyncerr.h>\n"
            "#include </usr/include/openssl/engineerr.h>\n"
            "#include </usr/include/openssl/rsa.h>\n"
            "#include </usr/include/openssl/bio.h>\n"
            "#include </usr/include/openssl/err.h>\n"
            "#include </usr/include/openssl/rsaerr.h>\n"
            "#include </usr/include/openssl/bioerr.h>\n"
            "#include </usr/include/openssl/evp.h>\n"
            "#include </usr/include/openssl/safestack.h>\n"
            "#include </usr/include/openssl/blowfish.h>\n"
            "#include </usr/include/openssl/evperr.h>\n"
            "#include </usr/include/openssl/seed.h>\n"
            "#include </usr/include/openssl/bn.h>\n"
            "#include </usr/include/openssl/hmac.h>\n"
            "#include </usr/include/openssl/sha.h>\n"
            "#include </usr/include/openssl/bnerr.h>\n"
            "#include </usr/include/openssl/idea.h>\n"
            "#include </usr/include/openssl/srp.h>\n"
            "#include </usr/include/openssl/buffer.h>\n"
            "#include </usr/include/openssl/kdf.h>\n"
            "#include </usr/include/openssl/srtp.h>\n"
            "#include </usr/include/openssl/buffererr.h>\n"
            "#include </usr/include/openssl/kdferr.h>\n"
            "#include </usr/include/openssl/ssl.h>\n"
            "#include </usr/include/openssl/camellia.h>\n"
            "#include </usr/include/openssl/lhash.h>\n"
            "#include </usr/include/openssl/ssl2.h>\n"
            "#include </usr/include/openssl/cast.h>\n"
            "#include </usr/include/openssl/md2.h>\n"
            "#include </usr/include/openssl/ssl3.h>\n"
            "#include </usr/include/openssl/cmac.h>\n"
            "#include </usr/include/openssl/md4.h>\n"
            "#include </usr/include/openssl/sslerr.h>\n"
            "#include </usr/include/openssl/cms.h>\n"
            "#include </usr/include/openssl/md5.h>\n"
            "#include </usr/include/openssl/stack.h>\n"
            "#include </usr/include/openssl/cmserr.h>\n"
            "#include </usr/include/openssl/mdc2.h>\n"
            "#include </usr/include/openssl/store.h>\n"
            "#include </usr/include/openssl/comp.h>\n"
            "#include </usr/include/openssl/modes.h>\n"
            "#include </usr/include/openssl/storeerr.h>\n"
            "#include </usr/include/openssl/comperr.h>\n"
            "#include </usr/include/openssl/obj_mac.h>\n"
            "#include </usr/include/openssl/symhacks.h>\n"
            "#include </usr/include/openssl/conf.h>\n"
            "#include </usr/include/openssl/objects.h>\n"
            "#include </usr/include/openssl/tls1.h>\n"
            "#include </usr/include/openssl/conf_api.h>\n"
            "#include </usr/include/openssl/objectserr.h>\n"
            "#include </usr/include/openssl/ts.h>\n"
            "#include </usr/include/openssl/conferr.h>\n"
            "#include </usr/include/openssl/ocsp.h>\n"
            "#include </usr/include/openssl/tserr.h>\n"
            "#include </usr/include/openssl/crypto.h>\n"
            "#include </usr/include/openssl/ocsperr.h>\n"
            "#include </usr/include/openssl/txt_db.h>\n"
            "#include </usr/include/openssl/cryptoerr.h>\n"
            "#include </usr/include/openssl/opensslv.h>\n"
            "#include </usr/include/openssl/ui.h>\n"
            "#include </usr/include/openssl/ct.h>\n"
            "#include </usr/include/openssl/ossl_typ.h>\n"
            "#include </usr/include/openssl/uierr.h>\n"
            "#include </usr/include/openssl/cterr.h>\n"
            "#include </usr/include/openssl/pem.h>\n"
            "#include </usr/include/openssl/whrlpool.h>\n"
            "#include </usr/include/openssl/des.h>\n"
            "#include </usr/include/openssl/pem2.h>\n"
            "#include </usr/include/openssl/x509.h>\n"
            "#include </usr/include/openssl/dh.h>\n"
            "#include </usr/include/openssl/pemerr.h>\n"
            "#include </usr/include/openssl/x509_vfy.h>\n"
            "#include </usr/include/openssl/dherr.h>\n"
            "#include </usr/include/openssl/pkcs12.h>\n"
            "#include </usr/include/openssl/x509err.h>\n"
            "#include </usr/include/openssl/dsa.h>\n"
            "#include </usr/include/openssl/pkcs12err.h>\n"
            "#include </usr/include/openssl/x509v3.h>\n"
            "#include </usr/include/openssl/dsaerr.h>\n"
            "#include </usr/include/openssl/pkcs7.h>\n"
            "#include </usr/include/openssl/x509v3err.h>\n"
            "#include </usr/include/openssl/dtls1.h>\n"
            "#include </usr/include/openssl/pkcs7err.h>\n"
            "#include </usr/include/openssl/e_os2.h>\n"
            "#include </usr/include/openssl/rand.h>\n");
}

