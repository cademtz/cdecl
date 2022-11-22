#pragma once
#include <string>
#include "util.hpp"

namespace Cdecl {
	class StringCursor {
		const string_view m_view;
		size_t m_pos = 0;

	public:
		StringCursor(const string_view& str) : m_view(str) {}

		size_t Pos() const { return m_pos; }

		const char_t* Peek() const {
			if (m_pos >= m_view.length())
				return nullptr;
			return &m_view[m_pos];
		}

		const char_t* Skip() {
			const char_t* ch = Peek();
			if (ch)
				++m_pos;
			return ch;
		}

		bool Seek(size_t pos) {
			if (pos <= m_view.length()) {
				m_pos = pos;
				return true;
			}
			return false;
		}

		bool MatchChar(char_t expected, bool case_sensitive = true) {
			const char_t* ch = Peek();
			if (ch) {
				bool match;
				if (case_sensitive)
					match = *ch == expected;
				else match = std::tolower(*ch) == std::tolower(expected);

				++m_pos;
				return true;
			}
			return false;
		}

		bool MatchString(const string_view& view) {
			if (view.length() > m_view.length() - m_pos)
				return false;

			for (size_t i = 0; i < view.length(); ++i) {
				if (view[i] != m_view[m_pos + i])
					return false;
			}

			m_pos += view.length();
			return true;
		}

		void SkipWhitespace() {
			while (const char_t* ch = Peek()) {
				if (*ch >= -1 && *ch <= UINT8_MAX && std::isspace((int)*ch))
					++m_pos;
				else
					break;
			}
		}
	};
}