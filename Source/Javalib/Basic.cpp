#include "Util/u.h"
#include <stdexcept>
#include <vector>
#include "Javalib/Basic.h"

JType
make_array_type(JType elem_type, Region& r) {
    auto p = new(r) ObjectTypeB;
    p->elem_type = elem_type;
    return (uintptr_t)p | 2;
}

JType
make_class_type(const char* class_name, Region& r) {
    auto p = new(r) ObjectTypeB;
    p->class_name = class_name;
    return (uintptr_t)p;
}

JType
parse_type(const char** ps, Region& r) {
    JType result;
    const char* s = *ps;
    switch (*s++) {
    case 'Z':
        result = BOOL;
        break;
    case 'B':
        result = BYTE;
        break;
    case 'C':
        result = CHAR;
        break;
    case 'S':
        result = SHORT;
        break;
    case 'I':
        result = INT;
        break;
    case 'J':
        result = LONG;
        break;
    case 'F':
        result = FLOAT;
        break;
    case 'D':
        result = DOUBLE;
        break;
    case '[':
        result = make_array_type(parse_type(&s, r), r);
        break;
    case 'L': {
        /* s -> end of 'L' */
        const char* psemi = strchr(s, ';');
        if (!psemi) throw InvalidDescriptor();
        int len = psemi - s;
        char* class_name = new(r) char[len + 1];
        memcpy(class_name, s, len);
        s = psemi + 1;
        class_name[len] = 0;
        result = make_class_type(class_name, r);
    }
    break;
    default:
        throw InvalidDescriptor();
    }
    *ps = s;
    return result;
}

JType
ParseFieldDescriptor(const char* s, Region& r) {
    JType result = parse_type(&s, r);
    if (*s) throw InvalidDescriptor(); // trailing characters
    return result;
}

MethodType
ParseMethodDescriptor(const char* s, Region& r) {
    MethodType result;
    if (*s++ != '(') throw InvalidDescriptor();
    std::vector<JType> v;
    while (*s != ')') { v.push_back(parse_type(&s, r)); }
    // *s == ')', skip it
    s++;
    if (*s == 'V') {
        result.ReturnType_opt = 0;
        s++;
    }
    else { result.ReturnType_opt = parse_type(&s, r); }
    if (*s) throw InvalidDescriptor(); // trailing characters
    auto size = v.size();
    result.NumArg = size;
    if (size) {
        JType* a = new(r) JType[size];
        memcpy(a, v.data(), size * sizeof *a);
        result.ArgTypes = a;
    }
    else { result.ArgTypes = nullptr; }
    return result;
}

void
PP_JType(Buf* b, JType t) {
    static const char* basic_type_name_table[8] = {
        "boolean", "byte", "char", "short", "int", "long", "float", "double"
    };
    auto k = JTypeKind(t);
    if (k & 1) {
        int i = t >> 1;
        assert(i>=0&&i<8);
        bputs(b, basic_type_name_table[i]);
        return;
    }
    switch (k) {
    case ARRAY:
        bprintf(b, "%a[]", PP_JType, ElemType(t));
        break;
    case OBJECT:
        bputs(b, ClassName(t));
        break;
    default:
        unreachable();
    }
}

void
PP_JType_opt(Buf* b, JType t) {
    if (t) { PP_JType(b, t); }
    else { bputs(b, "void"); }
}
