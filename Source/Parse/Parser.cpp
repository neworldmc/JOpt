#include "u.h"
#include "Parser.h"
#include "JClass.h"
#include "Convert.h"

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
    try {
        parser.ParseOnto(data, class_file);
    } catch (const InvalidClassFile &e) {
        fputs(e.what(), stderr);
        return 1;
    }

    Region r;
    rinit(&r);

    JClass *jclass = ConvertClassFile(&class_file, r);
    printf("class name: %s\n", jclass->ThisClass);
    const char *sc = jclass->SuperClass_opt;
    if (!sc) sc = "(none)";
    printf("super class: %s\n", sc);
    puts("interfaces:");
    for (int i=0; i<jclass->InterfaceCount; i++) {
        printf("  %s\n", jclass->Interfaces[i]);
    }
    puts("fields:");
    for (int i=0; i<jclass->FieldCount; i++) {
        const Field &f = jclass->Fields[i];
        printf("  %s %s\n", f.Name, f.Desc);
    }
    puts("methods:");
    for (int i=0; i<jclass->MethodCount; i++) {
        const Method &m = jclass->Methods[i];
        printf("  %s %s\n", m.Name, m.Desc);
    }
    puts("attributes:");
    for (int i=0; i<jclass->AttributeCount; i++) {
        const Attribute &a = jclass->Attributes[i];
        printf("  %s\n", a.Name);
    }
    return 0;
}
