#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "Parser.h"

using namespace Parse;

std::vector<std::byte> slurp(const char* path) {
    auto fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    auto len = ftell(fp);
    assert(len >= 0);
    fseek(fp, 0, SEEK_SET);

    std::vector<std::byte> result(len);

    auto nread = fread(result.data(), 1, len, fp);
    assert(nread == len);

    fclose(fp);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fputs("too few arguments\n", stderr);
        return 1;
    }
    char* path = argv[1];
    const auto data = slurp(path);
    Parser parser {};
    ClassFile class_file {};
    parser.ParseOnto(data, class_file);
    return 0;
}
