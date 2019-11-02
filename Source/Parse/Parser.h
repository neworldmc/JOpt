#pragma once

#include "ClassFile.h"
#include <stdexcept>

namespace Parse {
    using PByte = const std::byte*;

    class Parser {
    public:
        U4 ReadU4() { return PeekU4(Vpa(4)); }

        U2 ReadU2() { return PeekU2(Vpa(2)); }

        void ParseOnto(const std::vector<std::byte>& bytes, ClassFile& f) {
            Cur = bytes.data();
            Bound = Cur + bytes.size();
            f.Magic = ReadU4();
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
        static uint16_t PeekU1(PByte ptr) noexcept {
            return static_cast<uint8_t>(ptr[0]);
        }

        static uint16_t PeekU2(PByte ptr) noexcept {
            return static_cast<uint16_t>(ptr[0]) << 8 | static_cast<uint16_t>(ptr[1]);
        }

        static uint32_t PeekU4(PByte ptr) noexcept {
            return static_cast<uint32_t>(PeekU2(ptr)) << 16 | static_cast<uint32_t>(PeekU2(ptr + 2));
        }

        std::vector<CpInfo> LoadConstantPool(U2 count) {
            // TODO
            return std::vector<CpInfo>();
        }

        std::vector<U2> LoadInterfaces(U2 count) {
            std::vector<U2> result(count);
            for (U2 i=0; i<count; i++) {
                result[i] = ReadU2();
            }
            return result;
        }

        std::vector<FieldInfo> LoadFields(U2 count) {
            std::vector<FieldInfo> result(count);
            for (U2 i=0; i<count; i++) {
                result[i] = ReadFieldInfo();
            }
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

        std::vector<MethodInfo> LoadMethods(U2 count) {
            std::vector<MethodInfo> result(count);
            for (U2 i=0; i<count; i++) {
                result[i] = ReadMethodInfo();
            }
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

        std::vector<AttributeInfo> LoadAttributes(U2 count) {
            std::vector<AttributeInfo> result(count);
            for (U2 i=0; i<count; i++) {
                result[i] = ReadAttributeInfo();
            }
            return result;
        }

        AttributeInfo ReadAttributeInfo() {
            AttributeInfo result;
            result.AttributeNameIndex = ReadU2();
            result.AttributeLength = ReadU4();
            result.Info = ReadBytes(result.AttributeLength);
            return result;
        }

        std::vector<std::byte> ReadBytes(int n) {
            if (n < 0) {
                throw std::invalid_argument("ReadBytes: negative count");
            }
            if (Cur + n > Bound) {
                throw std::range_error("ReadBytes: out of range");
            }
            PByte start = Cur;
            Cur += n;
            return std::vector<std::byte>(start, Cur);
        }

        PByte Vpa(const int count) {
            if (Cur + count <= Bound) {
                const auto old = Cur;
                Cur += count;
                return old;
            }
            throw std::range_error("Read Out of File Bound");
        }

        PByte Cur, Bound;
    };
}
