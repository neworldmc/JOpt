#pragma once

#include <cstdint>
#include <vector>

namespace Parse {
	using U1 = uint8_t;
	using U2 = uint16_t;
	using U4 = uint32_t;

	struct CpInfo {
		U1 Tag;
		std::vector<U1> Info;
	};

	struct FieldInfo {
		
	};

	struct MethodInfo {
		
	};

	struct AttributeInfo {
		
	};
	
	struct ClassFile {
		U4 Magic;
		U2 MinorVersion;
		U2 MajorVersion;
		U2 ConstantPoolCount;
		std::vector<CpInfo> ConstantPool;
		U2 AccessFlags;
		U2 ThisClass;
		U2 SuperClass;
		U2 InterfaceCount;
		std::vector<U2> Interfaces;
		U2 FieldCount;
		std::vector<FieldInfo> Fields;
		U2 MethodsCount;
		std::vector<MethodInfo> Methods;
		U2 AttributesCount;
		std::vector<AttributeInfo> Attributes;
	};
}