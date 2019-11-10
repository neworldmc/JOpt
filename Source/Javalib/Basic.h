/*
 * type basic_type =
 *   | Bool
 *   | Byte
 *   | Char
 *   | Short
 *   | Int
 *   | Long
 *   | Float
 *   | Double
 *
 * type object_type =
 *   | Class of string (* bit 1 unset *)
 *   | Array of jtype  (* bit 1 set *)
 *
 * and jtype =
 *   | Basic of basic_type   (* bit 0 set *)
 *   | Object of object_type (* bit 0 unset *)
 */

/* To be honest, this feels like a bit of a stunt. */

// BasicType or pointer to ObjectTypeB (valid field depends on bit 1 of ptr)
typedef uintptr_t JType;

constexpr uintptr_t BOOL = 1;
constexpr uintptr_t BYTE = 3;
constexpr uintptr_t CHAR = 5;
constexpr uintptr_t SHORT = 7;
constexpr uintptr_t INT = 9;
constexpr uintptr_t LONG = 11;
constexpr uintptr_t FLOAT = 13;
constexpr uintptr_t DOUBLE = 15;

constexpr uintptr_t ARRAY = 2;
constexpr uintptr_t OBJECT = 4;

union ObjectTypeB {
    const char *class_name;
    JType elem_type;
};

typedef ObjectTypeB *ObjectType;

struct InvalidDescriptor : public std::exception {
    const char *what() const noexcept {
        return "invalid descriptor";
    }
};

struct MethodType {
    JType ReturnType_opt; // 0 = void
    int NumArg;
    JType *ArgTypes;
};

JType ParseFieldDescriptor(const char *s, Region &r);
MethodType ParseMethodDescriptor(const char *s, Region &r);

JType ElemType(JType);
const char *ClassName(JType);

inline uintptr_t
JTypeKind(JType t)
{
    assert(t);
    if (t&1) return t;
    if (t&2) return ARRAY;
    return OBJECT;
}

inline JType
ElemType(JType t)
{
    return ((ObjectType)(t&-4))->elem_type;
}

inline const char *
ClassName(JType t)
{
    return ((ObjectType)t)->class_name;
}

void PP_JType(Buf *, JType);
void PP_JType_opt(Buf *b, JType);
