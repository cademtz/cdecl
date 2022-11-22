#include <cdecl/c/tokendefs.hpp>
#include <cdecl/c/primitive.hpp>

namespace Cdecl {
	ParseResult<BaseType> BaseType::Parse(TokenCursor cur) {
		uint8_t flags = 0;

		while (const Token* tk = cur.Peek()) {
			switch (tk->id) {
			case TokenId::Const: flags |= Const; break;
			case TokenId::Volatile: flags |= Volatile; break;
			default:
				tk = nullptr;
			}

			if (tk)
				cur.Skip();
			else
				break;
		}
	}

	ParseResult<Primitive> Primitive::Parse(TokenCursor cur) {
		std::optional<BaseType> flags;
		if (auto result = BaseType::Parse(cur)) {
			flags = result.GetValue();
			cur = result.GetCursor();
		}
		else
			return ParseResult<Primitive>::Err(result.GetErr());

		std::optional<EPrimitive> prim;
		while (const Token* tk = cur.Peek()) {

			switch (tk->id) {
			case TokenId::Short:
			case TokenId::Int:
			case TokenId::Long:
			case TokenId::Int8_t:
			case TokenId::Int16_t:
			case TokenId::Int32_t:
			case TokenId::Int64_t:
			case TokenId::Uint8_t:
			case TokenId::Uint16_t:
			case TokenId::Uint32_t:
			case TokenId::Uint64_t:
			case TokenId::Char:
			case TokenId::Enum:
			case TokenId::Float:
			case TokenId::Double:
			case TokenId::Struct:
			case TokenId::Union:
			case TokenId::Void:
			default:
				tk = nullptr;
			}

			if (tk)
				cur.Skip();
			else
				break;
		}
	}
}