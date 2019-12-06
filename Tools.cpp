#include "Tools.h"

static CXChildVisitResult findFirstChildVisitor(CXCursor cursor, CXCursor, CXClientData data)
{
    *reinterpret_cast<CXCursor*>(data) = cursor;
    return CXChildVisit_Break;
}

CXCursor findFirstChild(CXCursor parent)
{
    CXCursor ret = clang_getNullCursor();
    if (!clang_isInvalid(clang_getCursorKind(parent)))
        clang_visitChildren(parent, findFirstChildVisitor, &ret);
    return ret;
}

bool endsWith(const std::string &haystack, const std::string &needle)
{
    return needle.size() >= haystack.size() && !strncmp(haystack.c_str() + (haystack.size() - needle.size()), needle.c_str(), needle.size());
}

inline const char *builtinTypeName(CXTypeKind kind)
{
    const char *ret = nullptr;
    switch (kind) {
    case CXType_Void: ret = "void"; break;
    case CXType_Bool: ret = "bool"; break;
    case CXType_Char_U: ret = "unsigned char"; break;
    case CXType_UChar: ret = "unsigned char"; break;
    case CXType_Char16: ret = "char16"; break;
    case CXType_Char32: ret = "char32"; break;
    case CXType_UShort: ret = "unsigned short"; break;
    case CXType_UInt: ret = "unsigned int"; break;
    case CXType_ULong: ret = "unsigned long"; break;
    case CXType_ULongLong: ret = "unsigned long long"; break;
    case CXType_UInt128: ret = "uint128"; break;
    case CXType_Char_S: ret = "char"; break;
    case CXType_SChar: ret = "schar"; break;
    case CXType_WChar: ret = "wchar"; break;
    case CXType_Short: ret = "short"; break;
    case CXType_Int: ret = "int"; break;
    case CXType_Long: ret = "long"; break;
    case CXType_LongLong: ret = "long long"; break;
    case CXType_Int128: ret = "int128"; break;
    case CXType_Float: ret = "float"; break;
    case CXType_Double: ret = "double"; break;
    case CXType_LongDouble: ret = "long double"; break;
    default: break;
    }
    return ret;
}

std::string typeName(const CXCursor &cursor)
{
    std::string ret;
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_FunctionTemplate:
        // ### If the return value is a template type we get an empty string here
    case CXCursor_FunctionDecl:
    case CXCursor_CXXMethod:
        ret = typeString(clang_getResultType(clang_getCursorType(cursor)));
        break;
    case CXCursor_ClassTemplate:
    case CXCursor_ClassDecl:
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
    case CXCursor_TypedefDecl:
    case CXCursor_EnumDecl:
        ret = eatString(clang_getCursorSpelling(cursor));
        break;
    case CXCursor_VarDecl: {
        const CXCursor initType = findFirstChild(cursor);
        if (clang_getCursorKind(initType) == CXCursor_InitListExpr) {
            ret = typeString(clang_getCursorType(initType));
        } else {
            ret = typeString(clang_getCursorType(cursor));
        }
        break; }
    case CXCursor_FieldDecl: // ### If the return value is a template type we get an empty string here
    case CXCursor_ParmDecl:
        ret = typeString(clang_getCursorType(cursor));
        break;
    default:
        return std::string();
    }
    return ret;
}

std::string typeString(const CXType &type)
{
    std::string ret;
    if (clang_isConstQualifiedType(type))
        ret = "const ";

    const char *builtIn = builtinTypeName(type.kind);
    if (builtIn) {
        ret += builtIn;
        return ret;
    }

    if (char pointer = (type.kind == CXType_Pointer ? '*' : (type.kind == CXType_LValueReference ? '&' : 0))) {
        const CXType pointee = clang_getPointeeType(type);
        ret += typeString(pointee);
        if (endsWith(ret, '*') || endsWith(ret, '&')) {
            ret += pointer;
        } else {
            ret += ' ';
            ret += pointer;
        }
        return ret;
    }

    if (type.kind == CXType_ConstantArray) {
        ret += typeString(clang_getArrayElementType(type));
        const int64_t count = clang_getNumElements(type);
        ret += '[';
        if (count >= 0)
            ret += std::to_string(count);
        ret += ']';
        return ret;
    }
    ret += typeName(clang_getTypeDeclaration(type));
    if (endsWith(ret, ' '))
        ret.resize(ret.size() - 1);
    return ret;
}

std::string eatString(CXString str)
{
    std::string ret = clang_getCString(str);
    clang_disposeString(str);
    return ret;
}

bool isMatchingInclude(CXSourceLocation loc)
{
    CXFile file = nullptr;
    unsigned line = 0, column = 0, offset = 0;
    clang_getSpellingLocation(loc,
                              &file,
                              &line,
                              &column,
                              &offset);

    std::string path = eatString(clang_getFileName(file));
    // printf("SHEKKING %s %d\n", path.c_str(), !strcmp(path.c_str(), "/usr/include/openssl/"));
    return !strncmp(path.c_str(), "/usr/include/openssl/", 21);
}

std::string toString(CXSourceLocation loc)
{
    CXFile file = nullptr;
    unsigned line = 0, column = 0, offset = 0;
    clang_getSpellingLocation(loc,
                              &file,
                              &line,
                              &column,
                              &offset);

    std::string path = eatString(clang_getFileName(file));
    if (path.empty())
        return std::string();

    size_t slash = path.rfind('/');
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s:%d:%d:", path.c_str() + (slash + 1), line, column);
    return buf;
}

const char *opensslincludes()
{
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

