#pragma once
#include "config.hpp"
#include <variant>
#include <string_view>
#include <sstream>

namespace Cdecl {
	using string_view = std::basic_string_view<char_t>;
	using stringstream = std::basic_stringstream<char_t>;
	using string_view = std::basic_string_view<char_t>;
	using string = std::basic_string<char_t>;

	template <class TOk, class TErr>
	class Result {
	public:
		struct Ok {
			TOk value;
		};
		struct Err {
			TErr value;
		};

	private:
		std::variant<Ok, Err> m_variant;

	public:
		Result(Ok&& ok) : m_variant(ok) {}
		Result(Err&& err) : m_variant(err) {}

		operator bool() const { return IsOk(); }

		bool IsOk() const { return std::holds_alternative<Ok>(m_variant); }
		bool IsErr() const { return !IsOk(); }

		const TOk& GetOk() const { return std::get<Ok>(m_variant).value; }
		const TErr& GetErr() const { return std::get<Err>(m_variant).value; }
	};

	inline uint32_t CombineFlags(uint32_t start_flag) { return start_flag; }

	template <class TInt, class TFlag, class ...T>
	inline TInt CombineFlags(TInt start_flag, TFlag next_flag, T... more) {
		return CombineFlags(start_flag | (TInt)next_flag, more...);
	}

	template <class TInt, class TFlag, class ...T>
	inline TInt CombineFlags(TFlag next_flag, T... more) {
		return CombineFlags(0, (TInt)next_flag, more...);
	}

	inline stringstream Format(stringstream&& ss) { return std::move(ss); }

	template <class T, class ...TMore>
	inline stringstream Format(stringstream&& ss, T item, TMore... more) {
		ss << item;
		return Format(std::move(ss), more...);
	}

	template <class T, class ...TMore>
	inline stringstream Format(T item, TMore... more) {
		return Format(stringstream(), item, more...);
	}
}