#pragma once

#include "ClassFile.h"

namespace Parse {
    class ConstantUtf8Info : public CpInfoBase {
    public:
        ConstantUtf8Info() noexcept : CpInfoBase(CPoolTags::Utf8) {}

        explicit ConstantUtf8Info(IParser& parser)
            : CpInfoBase(CPoolTags::Utf8), Length(parser.ReadU2()), Bytes(parser.ReadBytes(Length)) { }

        void Resolve() override {}
        U2 Length{};
        std::vector<U1> Bytes; // NOTE: Modified, not standard utf8s
    };

    class ConstantIntegerInfo : public CpInfoBase {
    public:
        ConstantIntegerInfo() noexcept : CpInfoBase(CPoolTags::Integer) {}
        explicit ConstantIntegerInfo(IParser& parser): CpInfoBase(CPoolTags::Integer), Bytes(parser.ReadU4()) {}
        void Resolve() override {}
        U4 Bytes{};
    };

    class ConstantFloatInfo : public CpInfoBase {
    public:
        ConstantFloatInfo() noexcept : CpInfoBase(CPoolTags::Float) {}
        explicit ConstantFloatInfo(IParser& parser): CpInfoBase(CPoolTags::Float), Bytes(parser.ReadU4()) {}
        void Resolve() override {}
        U4 Bytes{}; // IEEE 754?
    };

    class ConstantLongInfo : public CpInfoBase {
    public:
        ConstantLongInfo() noexcept : CpInfoBase(CPoolTags::Long) {}

        explicit ConstantLongInfo(IParser& parser): CpInfoBase(CPoolTags::Long) {
            HighBytes = parser.ReadU4();
            LowBytes = parser.ReadU4();
        }

        void Resolve() override {}
        U4 HighBytes{};
        U4 LowBytes{}; // Consider Field Merging?
    };

    class ConstantDoubleInfo : public CpInfoBase {
    public:
        ConstantDoubleInfo() noexcept : CpInfoBase(CPoolTags::Double) {}

        explicit ConstantDoubleInfo(IParser& parser): CpInfoBase(CPoolTags::Double) {
            HighBytes = parser.ReadU4();
            LowBytes = parser.ReadU4();
        }

        void Resolve() override {}
        U4 HighBytes{};
        U4 LowBytes{}; // Consider Field Merging and IEEE754 double rep?	
    };

    class ConstantClassInfo : public CpInfoBase {
    public:
        ConstantClassInfo() noexcept : CpInfoBase(CPoolTags::Class), NameIndex(0) {}
        explicit ConstantClassInfo(IParser& parser): CpInfoBase(CPoolTags::Class), NameIndex(parser.ReadU2()) {}
        void Resolve() override {}
        U2 NameIndex; // TODO: Resolve
    };

    class ConstantStringInfo : public CpInfoBase {
    public:
        ConstantStringInfo() noexcept : CpInfoBase(CPoolTags::String) {}
        explicit ConstantStringInfo(IParser& parser): CpInfoBase(CPoolTags::String), StringIndex(parser.ReadU2()) {}
        void Resolve() override {}
        U2 StringIndex{}; // TODO: Resolve
    };

    class ConstantFieldRefInfo : public CpInfoBase {
    public:
        ConstantFieldRefInfo() noexcept : CpInfoBase(CPoolTags::FieldRef) {}

        explicit ConstantFieldRefInfo(IParser& parser): CpInfoBase(CPoolTags::FieldRef) {
            ClassIndex = parser.ReadU2();
            NameAndTypeIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 ClassIndex{}; // TODO: Resolve
        U2 NameAndTypeIndex{}; // TODO: Resolve
    };

    class ConstantMethodRefInfo : public CpInfoBase {
    public:
        ConstantMethodRefInfo() noexcept : CpInfoBase(CPoolTags::MethodRef) {}

        explicit ConstantMethodRefInfo(IParser& parser): CpInfoBase(CPoolTags::MethodRef) {
            ClassIndex = parser.ReadU2();
            NameAndTypeIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 ClassIndex{}; // TODO: Resolve
        U2 NameAndTypeIndex{}; // TODO: Resolve
    };

    class ConstantInterfaceMethodRefInfo : public CpInfoBase {
    public:
        ConstantInterfaceMethodRefInfo() noexcept : CpInfoBase(CPoolTags::InterfaceMethodRef) {}

        explicit ConstantInterfaceMethodRefInfo(IParser& parser): CpInfoBase(CPoolTags::InterfaceMethodRef) {
            ClassIndex = parser.ReadU2();
            NameAndTypeIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 ClassIndex{}; // TODO: Resolve
        U2 NameAndTypeIndex{}; // TODO: Resolve
    };

    class ConstantNameAndTypeInfo : public CpInfoBase {
    public:
        ConstantNameAndTypeInfo() noexcept : CpInfoBase(CPoolTags::NameAndType) {}

        explicit ConstantNameAndTypeInfo(IParser& parser): CpInfoBase(CPoolTags::NameAndType) {
            NameIndex = parser.ReadU2();
            DescriptorIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 NameIndex{}; // TODO: Resolve
        U2 DescriptorIndex{}; // TODO: Resolve
    };

    class ConstantMethodHandleInfo : public CpInfoBase {
    public:
        ConstantMethodHandleInfo() noexcept : CpInfoBase(CPoolTags::MethodHandle) {}

        explicit ConstantMethodHandleInfo(IParser& parser): CpInfoBase(CPoolTags::MethodHandle) {
            ReferenceKind = parser.ReadU1();
            ReferenceIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U1 ReferenceKind{}; // TODO: Resolve
        U2 ReferenceIndex{}; // TODO: Resolve
    };

    class ConstantMethodTypeInfo : public CpInfoBase {
    public:
        ConstantMethodTypeInfo() noexcept : CpInfoBase(CPoolTags::MethodType) {}

        explicit ConstantMethodTypeInfo(IParser& parser): CpInfoBase(CPoolTags::MethodType),
                                                          DescriptorIndex(parser.ReadU2()) {}

        void Resolve() override {}
        U2 DescriptorIndex{}; // TODO: Resolve
    };

    class ConstantDynamicInfo : public CpInfoBase {
    public:
        ConstantDynamicInfo() noexcept : CpInfoBase(CPoolTags::Dynamic) {}

        explicit ConstantDynamicInfo(IParser& parser): CpInfoBase(CPoolTags::Dynamic) {
            BootstrapMethodAttrIndex = parser.ReadU2();
            NameAndTypeIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 BootstrapMethodAttrIndex{}; // TODO: Resolve
        U2 NameAndTypeIndex{}; // TODO: Resolve
    };

    class ConstantInvokeDynamicInfo : public CpInfoBase {
    public:
        ConstantInvokeDynamicInfo() noexcept : CpInfoBase(CPoolTags::InvokeDynamic) {}

        explicit ConstantInvokeDynamicInfo(IParser& parser): CpInfoBase(CPoolTags::InvokeDynamic) {
            BootstrapMethodAttrIndex = parser.ReadU2();
            NameAndTypeIndex = parser.ReadU2();
        }

        void Resolve() override {}
        U2 BootstrapMethodAttrIndex{}; // TODO: Resolve
        U2 NameAndTypeIndex{}; // TODO: Resolve
    };

    class ConstantModuleInfo : public CpInfoBase {
    public:
        ConstantModuleInfo() noexcept : CpInfoBase(CPoolTags::Module) {}
        explicit ConstantModuleInfo(IParser& parser): CpInfoBase(CPoolTags::Module), NameIndex(parser.ReadU2()) {}
        void Resolve() override {}
        U2 NameIndex{}; // TODO: Resolve
    };

    class ConstantPackageInfo : public CpInfoBase {
    public:
        ConstantPackageInfo() noexcept : CpInfoBase(CPoolTags::Package) {}
        explicit ConstantPackageInfo(IParser& parser): CpInfoBase(CPoolTags::Package), NameIndex(parser.ReadU2()) {}
        void Resolve() override {}
        U2 NameIndex{}; // TODO: Resolve
    };
}
