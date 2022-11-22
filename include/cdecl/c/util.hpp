#pragma once
#include <cdecl/util.hpp>
#include <cdecl/tokencursor.hpp>

namespace Cdecl {
	template <class T>
	class ParseResult : private Result<std::pair<T, TokenCursor>, string> {
		using base_type = Result<std::pair<T, TokenCursor>, string>;
		static constexpr size_t value_index = 0;
		static constexpr size_t cursor_index = 1;

		ParseResult(T&& value, TokenCursor cur) : base_type(Result<std::pair<T, TokenCursor>, string>::Ok{ std::pair(value, cur) }) {}
		ParseResult(string&& err) : base_type(base_type::Err{ std::pair(value, cur) }) {}

	public:
		bool IsOk() const { return IsOk(); }
		bool IsErr() const { return IsErr(); }
		operator bool() const { return (bool)*this; }

		const string& GetErr() const { return base_type::GetErr(); }
		const TokenCursor& GetCursor() const { return std::get<cursor_index>(base_type::GetOk()); }
		const T& GetValue() const { return std::get<value_index>(base_type::GetOk()); }

		string&& GetErr() { return base_type::GetErr(); }
		TokenCursor&& GetCursor() { return std::get<cursor_index>(base_type::GetOk()); }
		T&& GetValue() { return std::get<value_index>(base_type::GetOk()); }

		static ParseResult Ok(T&& value, TokenCursor cur) { return ParseResult(value, cur); }
		static ParseResult Err(string&& err) { return ParseResult(err); }
	};
}