#pragma once
#include <cdecl/tokenizer.hpp>
#include <cdecl/util.hpp>
#include <vector>

namespace Cdecl {
	namespace TokenId {
		enum TokenId : tokenid_t {
			Cdecl,
			Stdcall,
			Fastcall,
			Thiscall,
			Vectorcall,

			Const,
			Volatile,
			Char,
			Enum,
			Extern,
			Static,
			Float,
			Double,
			Int,
			Long,
			Short,
			Unsigned,
			Signed,
			Struct,
			Union,
			Void,

			Int8_t,
			Int16_t,
			Int32_t,
			Int64_t,
			Uint8_t,
			Uint16_t,
			Uint32_t,
			Uint64_t,

			Curly_Open,
			Curly_Close,
			Square_Open,
			Square_Close,
			Round_Open,
			Round_Close,
			Comma,
			Asterisk,
			Period,

			Identifier,
		};
	}

	inline std::optional<Token> rule_identifier(StringCursor cur) {
		const char_t* begin = cur.Peek();
		if (!begin)
			return {};

		const char_t* next = begin;
		while (*next == '_' || std::isalpha(*next)) {
			++next;
			if (!cur.Skip())
				break;
		}

		if (!begin || next == begin)
			return {};
		return Token(TokenId::Identifier, string_view(begin, next - begin));
	}

	const Tokenizer tokenizer = Tokenizer({
		TokenDef::Static(TokenId::Cdecl, "__cdecl", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Stdcall, "__stdcall", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Fastcall, "__fastcall", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Thiscall, "__thiscall", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Vectorcall, "__vectorcall", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Const, "const", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Volatile, "volatile", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Char, "char", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Enum, "enum", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Extern, "extern", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Static, "static", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Float, "float", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Double, "double", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Int, "int", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Long, "long", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Short, "short", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Unsigned, "unsigned", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Signed, "signed", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Struct, "struct", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Union, "union", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Void, "void", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Int8_t, "int8_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Int16_t, "int16_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Int32_t, "int32_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Int64_t, "int64_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Uint8_t, "uint8_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Uint16_t, "uint16_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Uint32_t, "uint32_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Uint64_t, "uint64_t", TokenDef::Static::Keyword),
		TokenDef::Static(TokenId::Curly_Open, "{"),
		TokenDef::Static(TokenId::Curly_Close, "}"),
		TokenDef::Static(TokenId::Square_Open, "["),
		TokenDef::Static(TokenId::Square_Close, "]"),
		TokenDef::Static(TokenId::Round_Open, "("),
		TokenDef::Static(TokenId::Round_Close, ")"),
		TokenDef::Static(TokenId::Comma, ","),
		TokenDef::Static(TokenId::Asterisk, "*"),
		TokenDef::Static(TokenId::Period, "."),
		TokenDef::Dynamic(rule_identifier),
	});
}