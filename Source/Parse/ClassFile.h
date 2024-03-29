#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace Parse {
    using U1 = uint8_t;
    using U2 = uint16_t;
    using U4 = uint32_t;

    struct Object {
        Object() noexcept = default;
        Object(Object&&) = default;
        Object& operator = (Object&&) = default;
        Object(const Object&) = delete;
        Object& operator = (const Object&) = delete;
        virtual ~Object() noexcept = default;
    };

    struct IParser: virtual Object {
        virtual U4 ReadU4() = 0;
        virtual U2 ReadU2() = 0;
        virtual U1 ReadU1() = 0;
        virtual std::vector<U1> ReadBytes(int n) = 0;
    };

    struct IResolvable: virtual Object {
        virtual void Resolve() = 0;
    };

    enum class CPoolTags : U1 {
        Utf8 = 1,
        Integer = 3,
        Float = 4,
        Long = 5,
        Double = 6,
        Class = 7,
        String = 8,
        FieldRef = 9,
        MethodRef = 10,
        InterfaceMethodRef = 11,
        NameAndType = 12,
        MethodHandle = 15,
        MethodType = 16,
        Dynamic = 17,
        InvokeDynamic = 18,
        Module = 19,
        Package = 20
    };

    struct CpInfoBase : virtual Object, IResolvable {
        explicit CpInfoBase(const CPoolTags tag) noexcept: Tag(tag) {}
        const CPoolTags Tag;
    };

    using CpInfo = std::unique_ptr<CpInfoBase>;

    struct AttributeInfo {
        U2 AttributeNameIndex{};
        U4 AttributeLength{}; // of Info
        std::vector<U1> Info {};
    };

    struct FieldInfo {
        U2 AccessFlags{};
        U2 NameIndex{};
        U2 DescriptorIndex{};
        U2 AttributesCount{};
        std::vector<AttributeInfo> Attributes{};
    };

    struct MethodInfo {
        U2 AccessFlags{};
        U2 NameIndex{};
        U2 DescriptorIndex{};
        U2 AttributesCount{};
        std::vector<AttributeInfo> Attributes{};
    };

    struct ClassFile {
        U4 Magic{};
        U2 MinorVersion{};
        U2 MajorVersion{};
        U2 ConstantPoolCount{};
        std::vector<CpInfo> ConstantPool;
        U2 AccessFlags{};
        U2 ThisClass{};
        U2 SuperClass{};
        U2 InterfaceCount{}; // the spec actually says "interfaces_count"
        std::vector<U2> Interfaces;
        U2 FieldCount{}; // "fields_count"
        std::vector<FieldInfo> Fields;
        U2 MethodsCount{};
        std::vector<MethodInfo> Methods;
        U2 AttributesCount{};
        std::vector<AttributeInfo> Attributes;
    };
}
