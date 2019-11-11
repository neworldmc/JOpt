/* Convert ClassFile to high-level class representation */

#include <string>
#include <stdexcept>
#include "Util/u.h"
#include "CpInfo.h"
#include "Javalib/Basic.h"
#include "Javalib/Class.h"

struct NotFound : public std::exception {
    std::string msg;
    NotFound(std::string&& msg): msg(std::move(msg)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

// thrown when cp index is <= 0 or >= size of cp
// OR the referenced entry is not of expected type
struct InvalidCPIndex : public std::exception {
    std::string msg;
    InvalidCPIndex(std::string&& msg): msg(std::move(msg)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

struct StringTableEntry {
    int key;
    char* val;

    StringTableEntry(int key, char* val):
        key(key), val(val) {}
};

static const char*
lookup_string(const std::vector<StringTableEntry>& table, int key) {
    int lo = 0;
    int hi = table.size();
    if (hi == 0 || key < table[lo].key || key > table[hi - 1].key) { throw NotFound(std::to_string(key)); }
    for (;;) {
        if (lo == hi) { throw NotFound(std::to_string(key)); }
        int mid = lo + hi >> 1; // max size of constant pool is 0x10000
        int mid_key = table[mid].key;
        if (key == mid_key) return table[mid].val;
        if (key < mid_key) { hi = mid; }
        else { lo = mid + 1; }
    }
}

Class*
ConvertClassFile(const Parse::ClassFile* cf, Region& r) {
    using namespace Parse;

    auto jc = new(r) Class;

    std::vector<StringTableEntry> strtab;

    // copy utf8 strings in constant pool to destination region
    int n_cpinfo = cf->ConstantPoolCount;
    for (int i = 1; i < n_cpinfo; i++) {
        const auto& cpinfo = cf->ConstantPool[i];
        if (cpinfo->Tag == CPoolTags::Utf8) {
            const auto& utf8info = ConstantUtf8Info::Reference(cpinfo);
            const auto& bytes = utf8info.Bytes;
            int len = bytes.size();
            char* s = new_string(len+1, r);
            memcpy(s, bytes.data(), len);
            s[len] = 0;
            strtab.emplace_back(i, s);
        }
    }

    auto get_class = [cf,&strtab](uint16_t index) {
        if (index <= 0 || index >= cf->ConstantPoolCount) { throw InvalidCPIndex(std::to_string(index)); }
        const auto& cpinfo = cf->ConstantPool[index];
        if (cpinfo->Tag != CPoolTags::Class) { throw InvalidCPIndex("#" + std::to_string(index) + " is not Class"); }
        auto& classinfo = ConstantClassInfo::Reference(cpinfo);
        return lookup_string(strtab, classinfo.NameIndex);
    };

    auto convert_attribute_info = [&strtab,&r](const AttributeInfo& ai, Attribute& a) {
        a.Name = lookup_string(strtab, ai.AttributeNameIndex);
        a.Length = ai.AttributeLength;
        a.Info = new(r) uint8_t[a.Length];
        memcpy(a.Info, ai.Info.data(), a.Length);
    };

    auto convert_field_info = [&strtab,&r,&convert_attribute_info](const FieldInfo& fi, Field& f) {
        f.AccessFlags = fi.AccessFlags;
        f.Name = lookup_string(strtab, fi.NameIndex);
        f.Desc = lookup_string(strtab, fi.DescriptorIndex);
        f.Type = ParseFieldDescriptor(f.Desc, r);
        f.AttributeCount = fi.AttributesCount;
        int n = f.AttributeCount;
        f.Attributes = new(r) Attribute[n];
        for (int i = 0; i < n; i++) { convert_attribute_info(fi.Attributes[i], f.Attributes[i]); }
    };

    auto convert_method_info = [&strtab,&r,&convert_attribute_info](const MethodInfo& mi, Method& m) {
        m.AccessFlags = mi.AccessFlags;
        m.Name = lookup_string(strtab, mi.NameIndex);
        m.Desc = lookup_string(strtab, mi.DescriptorIndex);
        m.Type = ParseMethodDescriptor(m.Desc, r);
        m.AttributeCount = mi.AttributesCount;
        int n = m.AttributeCount;
        m.Attributes = new(r) Attribute[n];
        for (int i = 0; i < n; i++) { convert_attribute_info(mi.Attributes[i], m.Attributes[i]); }
    };

    jc->MinorVersion = cf->MinorVersion;
    jc->MajorVersion = cf->MajorVersion;
    jc->AccessFlags = cf->AccessFlags;
    jc->ThisClass = get_class(cf->ThisClass);
    jc->SuperClass_opt = cf->SuperClass ? get_class(cf->SuperClass) : nullptr;
    int n;
    n = jc->InterfaceCount = cf->InterfaceCount;
    jc->Interfaces = new(r) const char*[n];
    for (int i = 0; i < n; i++) { jc->Interfaces[i] = get_class(cf->Interfaces[i]); }
    n = jc->FieldCount = cf->FieldCount;
    jc->Fields = new(r) Field[n];
    for (int i = 0; i < n; i++) { convert_field_info(cf->Fields[i], jc->Fields[i]); }
    n = jc->MethodCount = cf->MethodsCount;
    jc->Methods = new(r) Method[n];
    for (int i = 0; i < n; i++) { convert_method_info(cf->Methods[i], jc->Methods[i]); }
    n = jc->AttributeCount = cf->AttributesCount;
    jc->Attributes = new(r) Attribute[n];
    for (int i = 0; i < n; i++) { convert_attribute_info(cf->Attributes[i], jc->Attributes[i]); }

    return jc;
}
