#pragma once

#include "ClassFile.h"
#include <stdexcept>
#include "CpInfo.h"

namespace Parse {
    using PByte = const std::byte*;

    struct InvalidClassFile : public std::exception {
        std::string msg;
        InvalidClassFile(const char *wrapped_msg):
            msg(std::string("invalid class_file: ") + wrapped_msg) {}
        const char *what() const noexcept {
            return msg.c_str();
        }
    };

    class Parser : public IParser {
    public:
        U4 ReadU4() override { return PeekU4(Vpa(4)); }

        U2 ReadU2() override { return PeekU2(Vpa(2)); }

        U1 ReadU1() override { return PeekU1(Vpa(1)); }

        std::vector<U1> ReadBytes(const int n) override {
            if (n < 0) { throw std::invalid_argument("ReadBytes: negative count"); }
            if (Cur + n > Bound) { throw std::range_error("ReadBytes: out of range"); }
            const auto start = Cur;
            Cur += n;
            return std::vector<U1>(reinterpret_cast<const U1*>(start), reinterpret_cast<const U1*>(Cur));
        }

        void ParseOnto(const std::vector<std::byte>& bytes, ClassFile& f) {
            Cur = bytes.data();
            Bound = Cur + bytes.size();
            f.Magic = ReadU4();
            if (f.Magic != 0xcafebabe) {
                throw InvalidClassFile("invalid magic");
            }
            f.MinorVersion = ReadU2();
            f.MajorVersion = ReadU2();
            f.ConstantPoolCount = ReadU2();
            f.ConstantPool = LoadConstantPool(f.ConstantPoolCount);
            f.AccessFlags = ReadU2();
            f.ThisClass = ReadU2();
            f.SuperClass = ReadU2();
            f.InterfaceCount = ReadU2();
            f.Interfaces = LoadInterfaces(f.InterfaceCount);
            f.FieldCount = ReadU2();
            f.Fields = LoadFields(f.FieldCount);
            f.MethodsCount = ReadU2();
            f.Methods = LoadMethods(f.MethodsCount);
            f.AttributesCount = ReadU2();
            f.Attributes = LoadAttributes(f.AttributesCount);
        }

    private:
        static uint16_t PeekU1(const PByte ptr) noexcept { return static_cast<uint8_t>(ptr[0]); }

        static uint16_t PeekU2(const PByte ptr) noexcept {
            return static_cast<uint16_t>(ptr[0]) << 8 | static_cast<uint16_t>(ptr[1]);
        }

        static uint32_t PeekU4(const PByte ptr) noexcept {
            return static_cast<uint32_t>(PeekU2(ptr)) << 16 | static_cast<uint32_t>(PeekU2(ptr + 2));
        }

        CpInfo LoadConstant(const CPoolTags type) {
            switch (type) {
            case CPoolTags::Utf8: return std::make_unique<ConstantUtf8Info>(*this);
            case CPoolTags::Integer: return std::make_unique<ConstantIntegerInfo>(*this);
            case CPoolTags::Float: return std::make_unique<ConstantFloatInfo>(*this);
            case CPoolTags::Long: return std::make_unique<ConstantLongInfo>(*this);
            case CPoolTags::Double: return std::make_unique<ConstantDoubleInfo>(*this);
            case CPoolTags::Class: return std::make_unique<ConstantClassInfo>(*this);
            case CPoolTags::String: return std::make_unique<ConstantStringInfo>(*this);
            case CPoolTags::FieldRef: return std::make_unique<ConstantFieldRefInfo>(*this);
            case CPoolTags::MethodRef: return std::make_unique<ConstantMethodRefInfo>(*this);
            case CPoolTags::InterfaceMethodRef: return std::make_unique<ConstantInterfaceMethodRefInfo>(*this);
            case CPoolTags::NameAndType: return std::make_unique<ConstantNameAndTypeInfo>(*this);
            case CPoolTags::MethodHandle: return std::make_unique<ConstantMethodHandleInfo>(*this);
            case CPoolTags::MethodType: return std::make_unique<ConstantMethodTypeInfo>(*this);
            case CPoolTags::Dynamic: return std::make_unique<ConstantDynamicInfo>(*this);
            case CPoolTags::InvokeDynamic: return std::make_unique<ConstantInvokeDynamicInfo>(*this);
            case CPoolTags::Module: return std::make_unique<ConstantModuleInfo>(*this);
            case CPoolTags::Package: return std::make_unique<ConstantPackageInfo>(*this);
            default: ;
            }
            throw std::runtime_error("unexpected constant type");
        }

        std::vector<CpInfo> LoadConstantPool(const U2 count) {
            std::vector<CpInfo> result(count);
            for (U2 i = 1; i < count; i++) { result[i] = LoadConstant(static_cast<CPoolTags>(ReadU1())); }
            return result;
        }

        std::vector<U2> LoadInterfaces(const U2 count) {
            std::vector<U2> result(count);
            for (U2 i = 0; i < count; i++) { result[i] = ReadU2(); }
            return result;
        }

        std::vector<FieldInfo> LoadFields(const U2 count) {
            std::vector<FieldInfo> result(count);
            for (U2 i = 0; i < count; i++) { result[i] = ReadFieldInfo(); }
            return result;
        }

        FieldInfo ReadFieldInfo() {
            FieldInfo result;
            result.AccessFlags = ReadU2();
            result.NameIndex = ReadU2();
            result.DescriptorIndex = ReadU2();
            result.AttributesCount = ReadU2();
            result.Attributes = LoadAttributes(result.AttributesCount);
            return result;
        }

        std::vector<MethodInfo> LoadMethods(const U2 count) {
            std::vector<MethodInfo> result(count);
            for (U2 i = 0; i < count; i++) { result[i] = ReadMethodInfo(); }
            return result;
        }

        MethodInfo ReadMethodInfo() {
            MethodInfo result;
            result.AccessFlags = ReadU2();
            result.NameIndex = ReadU2();
            result.DescriptorIndex = ReadU2();
            result.AttributesCount = ReadU2();
            result.Attributes = LoadAttributes(result.AttributesCount);
            return result;
        }

        std::vector<AttributeInfo> LoadAttributes(const U2 count) {
            std::vector<AttributeInfo> result(count);
            for (U2 i = 0; i < count; i++) { result[i] = ReadAttributeInfo(); }
            return result;
        }

        AttributeInfo ReadAttributeInfo() {
            AttributeInfo result;
            result.AttributeNameIndex = ReadU2();
            result.AttributeLength = ReadU4();
            result.Info = ReadBytes(result.AttributeLength);
            return result;
        }

        PByte Vpa(const int count) {
            if (Cur + count <= Bound) {
                const auto old = Cur;
                Cur += count;
                return old;
            }
            throw std::range_error("Read Out of File Bound");
        }

        PByte Cur = nullptr, Bound = nullptr;
    };
}
