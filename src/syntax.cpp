#include <cdecl/c/syntax.hpp>
#include <cdecl/c/type.hpp>

namespace Cdecl {
	bool Type::IsPrimitiveIntegral(Primitive p) {
		switch (p) {
		case Primitive::Int8_t:
		case Primitive::Int16_t:
		case Primitive::Int32_t:
		case Primitive::Int64_t:
		case Primitive::Uint8_t:
		case Primitive::Uint16_t:
		case Primitive::Uint32_t:
		case Primitive::Uint64_t:
		case Primitive::Char:
		case Primitive::Int:
			return true;
		default:
			return false;
		}
	}
	bool Type::IsPrimitiveNumeric(Primitive p) {
		if (IsPrimitiveIntegral(p))
			return true;
		switch (p) {
		case Primitive::Float:
		case Primitive::Double:
			return true;
		default:
			return false;
		}
	}

	Type::Flags::ParseResult Type::Flags::Parse(TokenCursor cur) {
		// TODO: Separate flags parser for INT flags, POINTER flags, and CALL CONV flags
		// TODO: Separate "pointer" type structure

		uint32_t flags = 0;
		std::optional<CallConvention> call_conv;
		int call_conv_counter = 0;

		while (const Token* tk_flag = cur.MatchAny(
			TokenId::Const, TokenId::Volatile, TokenId::Signed, TokenId::Unsigned, TokenId::Short, TokenId::Long,
			TokenId::Cdecl, TokenId::Stdcall, TokenId::Fastcall, TokenId::Thiscall, TokenId::Vectorcall
		))
		{
			size_t begin = cur.Pos();

			switch (tk_flag->id) {
			case TokenId::Const: flags |= Flags::Const; break;
			case TokenId::Volatile: flags |= Flags::Volatile; break;
			case TokenId::Signed: flags |= Flags::Signed; break;
			case TokenId::Unsigned: flags |= Flags::Unsigned; break;
			case TokenId::Short: flags |= Flags::Short; break;
			case TokenId::Long:
				if (flags & Flags::Long)
					flags = (flags & ~Flags::Long) | Flags::LongLong;
				else if (flags & Flags::LongLong)
					return ParseResult::Err{ cur.FormatWithLoc(begin, "Invalid combination of 'long' specifiers").str() };
				else
					flags |= Flags::Long;
				break;

			case TokenId::Cdecl:
				call_conv = CallConvention::Cdecl, ++call_conv_counter; break;
			case TokenId::Stdcall:
				call_conv = CallConvention::Stdcall, ++call_conv_counter; break;
			case TokenId::Fastcall:
				call_conv = CallConvention::Fastcall, ++call_conv_counter; break;
			case TokenId::Thiscall:
				call_conv = CallConvention::Thiscall, ++call_conv_counter; break;
			case TokenId::Vectorcall:
				call_conv = CallConvention::Vectorcall, ++call_conv_counter; break;

			default:
				return ParseResult::Err{ cur.FormatWithLoc(begin, "Unhandled token id ", tk_flag->id).str() };
			}

			if (call_conv_counter > 1)
				return ParseResult::Err{ cur.FormatWithLoc(begin, "Cannot specify multiple calling conventions").str() };
		}

		return ParseResult::Ok{ std::pair(Flags{flags, call_conv}, cur) };
	}
	Type::Flags::CombineResult Type::Flags::Combine(const Flags& other) const {
		std::optional<CallConvention> new_call_conv = call_conv;
		if (other.call_conv.has_value()) {
			if (new_call_conv.has_value())
				return CombineResult::Err{ "Cannot specify multiple calling conventions" };
			else
				new_call_conv = other.call_conv;
		}

		uint32_t new_flags = bits | other.bits;
		if (bits & Flags::Long && other.bits & Flags::Long) {
			new_flags &= ~Flags::Long;
			new_flags |= Flags::LongLong;
		}

		uint32_t overlap = bits & other.bits;
		bool overlapping_long = bits & Flags::LongLong && overlap & (Flags::Long | Flags::LongLong);
		bool long_and_short = false;
		if (bits & Flags::Short)
			long_and_short = bits & (Flags::Long | Flags::LongLong);

		if (overlapping_long)
			return CombineResult::Err{ "Invalid combination of 'long' specifiers" };
		else if (long_and_short)
			return CombineResult::Err{ "Cannot specify 'long' and 'short' together" };

		return CombineResult::Ok{ Flags{new_flags, new_call_conv} };
	}

	Type::ParsePrimitiveResult Type::ParsePrimitive(TokenCursor cur, TypeParseMask mask) {
		size_t begin = cur.Pos();

		const Token* tk_prim = cur.MatchAny(
			TokenId::Int8_t, TokenId::Int16_t, TokenId::Int32_t, TokenId::Int64_t,
			TokenId::Uint8_t, TokenId::Uint16_t, TokenId::Uint32_t, TokenId::Uint64_t,
			TokenId::Char, TokenId::Int, TokenId::Float, TokenId::Double, TokenId::Void
		);

		if (!tk_prim)
			return ParsePrimitiveResult::Err{ cur.FormatWithLoc(begin, "Expected a primitive numerical type or void").str() };

		Primitive prim;

		switch (tk_prim->id) {
		case TokenId::Int8_t:	prim = Primitive::Int8_t; break;
		case TokenId::Int16_t:	prim = Primitive::Int16_t; break;
		case TokenId::Int32_t:	prim = Primitive::Int32_t; break;
		case TokenId::Int64_t:	prim = Primitive::Int64_t; break;
		case TokenId::Uint8_t:	prim = Primitive::Uint8_t; break;
		case TokenId::Uint16_t:	prim = Primitive::Uint16_t; break;
		case TokenId::Uint32_t:	prim = Primitive::Uint32_t; break;
		case TokenId::Uint64_t:	prim = Primitive::Uint64_t; break;
		case TokenId::Char:		prim = Primitive::Char; break;
		case TokenId::Int:		prim = Primitive::Int; break;
		case TokenId::Double:	prim = Primitive::Double; break;
		case TokenId::Void:		prim = Primitive::Void; break;
		default: {
			return ParsePrimitiveResult::Err{ cur.FormatWithLoc(begin, "Unhandled token id ", tk_prim->id).str() };
		}
		}

		return ParsePrimitiveResult::Ok{ std::pair(prim, cur) };
	}
	Type::ParseBaseTypeResult Type::ParseBaseType(TokenCursor cur, TypeParseMask mask) {
		size_t begin = cur.Pos();

		Flags flags_prefix;
		if (auto result = Flags::Parse(cur)) {
			flags_prefix = std::get<Flags>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseBaseTypeResult::Err{ result.GetErr() };

		Primitive prim;
		if (auto result = ParsePrimitive(cur, mask)) {
			prim = std::get<Primitive>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else {
			if (flags_prefix.bits & FLAGS_INT)
				prim = Primitive::Int; // Default to int if int-related flags are given
			else
				return ParseBaseTypeResult::Err{ result.GetErr() };
		}

		Flags flags_postfix;
		if (auto result = Flags::Parse(cur)) {
			flags_postfix = std::get<Flags>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseBaseTypeResult::Err{ result.GetErr() };

		// TODO: Write a helper function to validate type specifiers (and create a cleaned-up type)

		Flags flags;
		if (auto result = flags_prefix.Combine(flags_postfix))
			flags = result.GetOk();
		else
			return ParseBaseTypeResult::Err{ cur.FormatWithLoc(begin, result.GetErr()).str() };

		if (prim == Primitive::Float || prim == Primitive::Double) {
			if (flags.bits & BADFLAGS_FLOAT)
				return ParseBaseTypeResult::Err{ cur.FormatWithLoc(begin, "Invalid combination of type specifiers").str() };

			// MSVC allows `long float` to mean `double`
			if (prim == Primitive::Float && flags.bits & Flags::Long) {
				flags.bits &= ~Flags::Long;
				prim = Primitive::Double;
			}
		}
		else if (!IsPrimitiveIntegral(prim) && flags.bits & FLAGS_INT)
			return ParseBaseTypeResult::Err{ cur.FormatWithLoc(begin, "Cannot use integer-only type specifiers on a non-integer").str() };

		return ParseBaseTypeResult::Ok{ std::pair(std::make_shared<Type>(prim, flags), cur) };
	}
	Type::ParseResult Type::Parse(TokenCursor cur, TypeParseMask mask) {
		std::shared_ptr<const Type> base_type;
		if (auto result = ParseBaseType(cur, mask)) {
			base_type = std::get<std::shared_ptr<const Type>>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseResult::Err{ result.GetErr() };

		while (cur.Match(TokenId::Asterisk)) {
			size_t begin = cur.Pos();

			Flags flags;
			if (auto result = Flags::Parse(cur)) {
				flags = std::get<Flags>(result.GetOk());
				cur = std::get<TokenCursor>(result.GetOk());
			}
			else
				return ParseResult::Err{ cur.FormatWithLoc(begin, result.GetErr()).str() };

			if (flags.bits & FLAGS_INT)
				return ParseResult::Err{ cur.FormatWithLoc(begin, "Cannot use integer-only type specifiers on a pointer").str() };

			flags.bits |= Flags::Pointer;
			base_type = std::make_shared<Type>(std::move(base_type), flags);
		}

		return ParseResult::Ok{ std::pair(base_type, cur) };
	}

	Variable::ParseResult Variable::Parse(TokenCursor cur, TypeParseMask mask) {
		std::shared_ptr<const Type> type;
		if (auto result = Type::Parse(cur, mask)) {
			type = std::get<std::shared_ptr<const Type>>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseResult::Err{ result.GetErr() };
	}

	Argument::ParseResult Argument::Parse(TokenCursor cur) {
		static const TypeParseMask mask = ParseMaskBlacklist(TypeParseMask::Structs);
		size_t begin = cur.Pos();

		if (cur.MatchSequence(TokenId::Period, TokenId::Period, TokenId::Period))
			return ParseResult::Ok{ std::pair(Argument(), cur) };

		std::shared_ptr<const Type> type;
		if (auto result = Type::Parse(cur, mask)) {
			type = std::get<std::shared_ptr<const Type>>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseResult::Err{ result.GetErr() };

		if (type->HasDecl()) {
			Variable var = Variable(type, std::string(type->GetDecl()));
			return ParseResult::Ok{ std::pair(Argument(std::move(var)), cur) };
		}
		else
			return ParseResult::Ok{ std::pair(Argument(type), cur) };
	}

	FunctionProto::ParseResult FunctionProto::Parse(TokenCursor cur) {
		/*
		TODO: Include calling conventions as a type specifier.
		Variable::Parse() should scream if any calling convention is set
		Argument::Parse() will use Variable::Parse(), propogating that check

		This also begs the questions:
		Should Type contain both 'name' and 'conv' attributes on its own?
		Even if not, conv needs to be parsed by it. How would they be shared privately?
		*/

		std::shared_ptr<const Type> ret_type;
		if (auto result = Type::Parse(cur, ParseMaskBlacklist())) {
			ret_type = std::get<std::shared_ptr<const Type>>(result.GetOk());
			cur = std::get<TokenCursor>(result.GetOk());
		}
		else
			return ParseResult::Err{ result.GetErr() };

		string name;
		if (const Token* tk_name = cur.Match(TokenId::Identifier))
			name = tk_name->view;
		else
			return ParseResult::Err{ cur.FormatWithLoc(cur.Pos(), "Expected an identifier").str() };

		if (!cur.Match(TokenId::Round_Open))
			return ParseResult::Err{ cur.FormatWithLoc(cur.Pos(), "Expected function arguments in parentheses").str() };

		std::vector<Argument> args;

		size_t begin = cur.Pos();
		while (auto result = Argument::Parse(cur)) {

			begin = cur.Pos();
		}
	}
}