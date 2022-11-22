#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include "util.hpp"
#include "stringcursor.hpp"

namespace Cdecl {
	using tokenid_t = uint32_t;

	struct Token {
		const tokenid_t id;
		const string_view view;

		Token(tokenid_t id_, string_view&& view_) : id(id_), view(view_) {}
	};

	struct TokenDef {
		enum class Kind {
			Static,
			Dynamic
		};

		struct Static {
			enum Flag : uint32_t {
				Keyword = 1 << 0,
				CaseInsensitive = 1 << 1,
			};

			const tokenid_t id;
			const std::basic_string<char_t> str;
			const uint32_t flags = 0;

			template <class ...T>
			Static(tokenid_t id_, std::basic_string<char_t>&& str_, Flag flag, T... flags_)
				: id(id_), str(str_), flags(CombineFlags(flag, flags_...)) {}
			Static(tokenid_t id_, std::basic_string<char_t>&& str_) : id(id_), str(str_), flags(0) {}
		};

		struct Dynamic {
			using callback_t = std::optional<Token>(StringCursor cursor);
			callback_t* const callback;
			Dynamic(callback_t* callback_) : callback(callback_) {}
		};

		const Kind kind;
		const std::variant<Static, Dynamic> variant;

		TokenDef(Static&& statik_) : kind(Kind::Static), variant(statik_) {}
		TokenDef(Dynamic&& dynamic_) : kind(Kind::Dynamic), variant(dynamic_) {}

		bool IsStatic() const { return std::holds_alternative<Static>(variant); }
		bool IsDynamic() const { return std::holds_alternative<Dynamic>(variant); }

		const Static& GetStatic() const { return std::get<Static>(variant); }
		const Dynamic& GetDynamic() const { return std::get<Dynamic>(variant); }
	};

	class Tokenizer {
		std::vector<TokenDef> m_defs;

	public:
		Tokenizer(std::vector<TokenDef>&& defs_) : m_defs(defs_) {}

		using ParseResult = Result<std::vector<Token>, std::string>;

		std::optional<Token> ParseAt(StringCursor cursor) const {
			const char_t* begin = cursor.Peek();
			if (!begin)
				return std::optional<Token>();

			for (const TokenDef& def : m_defs) {
				if (def.IsStatic()) {
					const TokenDef::Static& statik = def.GetStatic();
					if (cursor.MatchString(statik.str))
						return std::optional<Token>(Token(statik.id, string_view(begin, statik.str.length())));
				}
				else if (def.IsDynamic()) {
					const TokenDef::Dynamic& dynamic = def.GetDynamic();
					std::optional<Token> tk = dynamic.callback(cursor);
					if (tk.has_value())
						return tk;
				}
			}

			return {};
		}

		ParseResult ParseAll(const string_view& str) const {
			StringCursor cur = StringCursor(str);
			std::vector<Token> buffer;

			while (true) {
				cur.SkipWhitespace();
				if (cur.Pos() >= str.length())
					return ParseResult::Ok{ buffer };

				std::optional<Token> tk = ParseAt(cur);
				if (!tk.has_value())
					break;

				cur.Seek(cur.Pos() + tk.value().view.length());
				buffer.emplace_back(std::move(tk.value()));
			};

			return ParseResult::Err{
				Format("Unknown token at char ", cur.Pos(), " \"", str.substr(cur.Pos(), 15), '"').str()
			};
		}
	};
}