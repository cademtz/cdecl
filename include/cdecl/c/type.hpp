#pragma once
#include <cstdint>
#include <optional>
#include <cdecl/util.hpp>
#include <cdecl/tokencursor.hpp>

namespace Cdecl {
	enum class CallConvention {
		Cdecl,
		Stdcall,
		Fastcall,
		Thiscall,
		Vectorcall,
	};

	enum class TypeParseMask : uint32_t {
		Structs = 1 << 0,
		Enums = 1 << 1,
		NameDecl = 1 << 2,
		Arrays = 1 << 3,
		CallConvs = 1 << 4,
		Functions = 1 << 5,
	};

	static TypeParseMask ParseMaskWhitelist() { return (TypeParseMask)0; }
	template<class ...TMore>
	static TypeParseMask ParseMaskWhitelist(TypeParseMask bit, TMore... more) {
		uint32_t mask = (uint32_t)bit | (uint32_t)ParseMaskWhitelist(more...);
		return (TypeParseMask)mask;
	}

	static TypeParseMask ParseMaskBlacklist() { return (TypeParseMask)~(uint32_t)0; }
	template<class ...TMore>
	static TypeParseMask ParseMaskBlacklist(TMore... more) {
		return (TypeParseMask)~(uint32_t)ParseMaskWhitelist(more...);
	}

	// Forward declare everything that uses Type while also used by Type
	class FunctionProto;

	/*
	 * Type info that can parse and hold everything from calling conventions to structs
	 */
	class Type {
	public:
		enum class Primitive : uint32_t {
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
			Int,
			Struct,
			Union,
			Void,
		};

	private:
		struct Flags {
			enum EFlags : uint32_t {
				Pointer = 1 << 0,
				Volatile = 1 << 1,
				Const = 1 << 2,
				Signed = 1 << 3,
				Unsigned = 1 << 4,
				Long = 1 << 5,
				LongLong = 1 << 6,
				Short = 1 << 7,
			};

			uint32_t bits;
			std::optional<CallConvention> call_conv;

			bool IsPointer() const { return bits & Pointer; }
			bool IsConst() const { return bits & Const; }
			bool IsVolatile() const { return bits & Volatile; }
			bool IsLong() const { return bits & Long; }
			bool IsLongLong() const { return bits & LongLong; }

			using ParseResult = Result<std::pair<Flags, TokenCursor>, string>;
			static ParseResult Parse(TokenCursor cur);

			using CombineResult = Result<Flags, string>;
			CombineResult Combine(const Flags& other) const;
		};

		static const uint32_t FLAGS_INT = Flags::Short | Flags::Long | Flags::LongLong | Flags::Unsigned | Flags::Signed;
		static const uint32_t BADFLAGS_FLOAT = Flags::Short | Flags::LongLong | Flags::Unsigned | Flags::Signed;

		std::variant<
			Primitive,
			std::shared_ptr<const Type>,
			std::shared_ptr<const FunctionProto>
		> m_base;
		std::optional<CallConvention> m_conv;
		std::optional<string> m_decl;
		Flags m_flags;

		static bool IsPrimitiveIntegral(Primitive p);
		static bool IsPrimitiveNumeric(Primitive p);

		using ParsePrimitiveResult = Result<std::pair<Primitive, TokenCursor>, string>;
		static ParsePrimitiveResult ParsePrimitive(TokenCursor cur, TypeParseMask mask);

		using ParseBaseTypeResult = Result<std::pair<std::shared_ptr<const Type>, TokenCursor>, string>;
		static ParseBaseTypeResult ParseBaseType(TokenCursor cur, TypeParseMask mask);

		using ParseProtoResult = Result<std::pair<std::shared_ptr<const FunctionProto>, TokenCursor>, string>;
		static ParseProtoResult ParseProto(std::shared_ptr<const Type> ret_type, TokenCursor cur);

		template <class ...TFlags>
		Type(TFlags... flags) : m_flags(Flags{ CombineFlags(flags...) }) {}
		Type(Flags flags) : m_flags(flags) {}

	public:
		template <class ...TFlags>
		Type(Primitive prim, TFlags... flags) : Type(flags...) {
			m_base = prim;
		}
		template <class ...TFlags>
		Type(const std::shared_ptr<const Type>& type, TFlags... flags) : Type(flags...) {
			m_base = type;
		}
		template <class ...TFlags>
		Type(const std::shared_ptr<const FunctionProto>& proto, TFlags... flags) : Type(flags...) {
			m_base = proto;
		}

		bool IsPointer() const { return m_flags.IsPointer(); }
		bool IsPrimitive() const { return std::holds_alternative<Primitive>(m_base); }
		bool IsFunctionProto() const { return std::holds_alternative<std::shared_ptr<const FunctionProto>>(m_base); }

		bool IsConst() const { return m_flags.IsConst(); }
		bool IsVolatile() const { return m_flags.IsVolatile(); }
		bool IsLong() const { return m_flags.IsLong(); }
		bool IsLongLong() const { return m_flags.IsLongLong(); }
		bool HasCallConvention() const { return m_conv.has_value(); }
		bool HasDecl() const { return m_decl.has_value(); }

		Primitive GetPrimitiveType() const { return std::get<Primitive>(m_base); }
		const std::shared_ptr<const Type>& GetPointedType() const { return std::get<std::shared_ptr<const Type>>(m_base); }
		const std::shared_ptr<const FunctionProto>& GetFunctionProto() const { return std::get <std::shared_ptr<const FunctionProto>>(m_base); }

		// ! Access this through FunctionProto instead !
		CallConvention GetConvention() const { return m_conv.value(); }

		// ! Access this through Variable instead !
		const string& GetDecl() const { return m_decl.value(); }

		using ParseResult = Result<std::pair<std::shared_ptr<const Type>, TokenCursor>, string>;
		static ParseResult Parse(TokenCursor cur, TypeParseMask mask);
	};
}