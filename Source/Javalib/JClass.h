/* High-level representation of Java class file */

#include <cstdint>

struct Attribute {
    const char *Name;
    int Length; // of Info[]
    uint8_t *Info;
};

struct Field {
    uint16_t AccessFlags;
    const char *Name;
    const char *Desc; // descriptor; will be typed out often so shorten it
    int AttributeCount;
    Attribute *Attributes;
};

struct Method {
    uint16_t AccessFlags;
    const char *Name;
    const char *Desc;
    int AttributeCount;
    Attribute *Attributes;
};

struct JClass {
    uint16_t MinorVersion;
    uint16_t MajorVersion;
    uint16_t AccessFlags;
    const char *ThisClass;
    const char *SuperClass_opt; // might be NULL, thus _opt
    int InterfaceCount;
    const char **Interfaces;
    int FieldCount;
    Field *Fields;
    int MethodCount;
    Method *Methods;
    int AttributeCount;
    Attribute *Attributes;
};
