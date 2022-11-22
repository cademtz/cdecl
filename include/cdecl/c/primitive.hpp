#pragma once
#include <cdecl/tokencursor.hpp>
#include "util.hpp"

namespace Cdecl {
	class BaseType {
		uint8_t m_flags;

	public:
		enum Flag : uint8_t {
			Const = 1 << 0,
			Volatile = 1 << 1,
		};

		BaseType(uint8_t bits) : m_flags(bits) {}

		bool IsConst() const { return m_flags & Const; }
		bool IsVolatile() const { return m_flags & Volatile; }

		static ParseResult<BaseType> Parse(TokenCursor cur);
	};

	class Primitive : public BaseType {


	public:
		enum EPrimitive {
			Short,
			Int,
			Long,
			LongLong,
			Int8_t,
			Int16_t,
			Int32_t,
			Int64_t,
			Uint8_t,
			Uint16_t,
			Uint32_t,
			Uint64_t,
			Char,
			Enum,
			Float,
			Double,
			Struct,
			Union,
			Void,
		};

		static ParseResult<Primitive> Parse(TokenCursor cur);
	};

	class Pointer : public BaseType {
	public:
		static ParseResult<Pointer> Parse(TokenCursor cur);
	};
}