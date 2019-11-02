#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Parser.h"

using namespace Parse;

std::vector<std::byte>
slurp(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    assert(len >= 0);
    fseek(fp, 0, SEEK_SET);

    std::vector<std::byte> result(len);

    size_t nread = fread(result.data(), 1, len, fp);
    assert(nread == len);

    fclose(fp);
    return result;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fputs("too few arguments\n", stderr);
        return 1;
    }
    char *path = argv[1];
    std::vector<std::byte> data = slurp(path);
    Parser parser;
    ClassFile class_file;
    parser.ParseOnto(data, class_file);
    return 0;
}
