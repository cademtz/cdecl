#pragma once
#include "tokenizer.hpp"

namespace Cdecl {
	class TokenCursor {
		const Token* m_begin, * m_end;
		size_t m_pos = 0;

		const Token* MatchAny() { return nullptr; }

	public:
		TokenCursor(const Token* begin, const Token* end) : m_begin(begin), m_end(end) {}
		TokenCursor(const std::vector<Token>& tokens) : TokenCursor(&tokens.front(), &tokens.back() + 1) {}

		size_t Pos() const { return m_pos; }

		template <class ...TArgs>
		stringstream FormatWithLoc(size_t start_pos, TArgs... args) const {
			stringstream ss;

			ss << '"';
			size_t count = 15;
			for (size_t i = start_pos; count > 0 && m_begin + i < m_end; ++i) {
				string_view view = m_begin[i].view;
				if (view.length() < count) {
					ss << view << ' ';
					count -= view.length();
				}
				else {
					ss << view.substr(0, count) << "...";
					count = 0;
				}
			}
			ss << '"';

			size_t char_index = 0;
			if (m_begin && m_begin != m_end) {
				const char_t* begin = &m_begin->view.front();
				if (m_begin + start_pos < m_end)
					char_index = &m_begin[start_pos].view.front() - begin;
				else
					char_index = &(m_end - 1)->view.back() - begin;
			}
			ss << "(at char " << char_index << "): ";
			return Format(std::move(ss), args...);
		}

		const Token* Peek() const {
			if (m_begin + m_pos >= m_end)
				return nullptr;
			return &m_begin[m_pos];
		}

		const Token* Skip() {
			const Token* tk = Peek();
			if (tk)
				++m_pos;
			return tk;
		}

		bool Seek(size_t pos) {
			if (m_begin + pos <= m_end) {
				m_pos = pos;
				return true;
			}
			return false;
		}

		const Token* Match(tokenid_t id) {
			const Token* tk = Peek();
			if (tk && tk->id == (tokenid_t)id) {
				++m_pos;
				return tk;
			}
			return nullptr;
		}

		template <class TId, class ...TMore>
		const Token* MatchAny(TId id, TMore... more) {
			const Token* tk = Match((tokenid_t)id);
			return tk ? tk : MatchAny(more...);
		}

		bool MatchSequence() {
			return true;
		}

		template <class TId, class ...TMore>
		bool MatchSequence(TId id, TMore... more) {
			size_t begin = Pos();
			bool result = Match((tokenid_t)id) && MatchSequence(more...);
			if (!result)
				Seek(begin);
			return result;
		}
	};
}