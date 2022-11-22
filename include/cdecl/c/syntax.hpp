#include "tokendefs.hpp"
#include "type.hpp"
#include <cdecl/tokencursor.hpp>
#include <variant>
#include <sstream>

namespace Cdecl {
	class Variable {
		const std::shared_ptr<const Type> m_type;
		const string m_name;

	public:
		Variable(const std::shared_ptr<const Type>& type, string&& name) : m_type(type), m_name(name) {}

		const std::shared_ptr<const Type>& GetType() const { return m_type; }
		const string& GetName() const { return m_name; }

		using ParseResult = Result<std::pair<Variable, TokenCursor>, string>;
		static ParseResult Parse(TokenCursor cur, TypeParseMask mask);
	};

	class Argument {
		using type_type = std::shared_ptr<const Type>;
		using base_type = std::optional<std::variant<type_type, Variable>>;

		const base_type m_base;

	public:
		Argument() : m_base() {}
		Argument(const std::shared_ptr<const Type>& type) : m_base(type) {}
		Argument(Variable&& var) : m_base(var) {}

		bool IsType() const { return !IsVariadic() && std::holds_alternative<type_type>(m_base.value()); }
		bool IsVariable() const { return !IsVariadic() && std::holds_alternative<Variable>(m_base.value()); }
		bool IsVariadic() const { return !m_base.has_value(); }

		const std::shared_ptr<const Type>& GetType() const { return std::get<type_type>(m_base.value()); }
		const Variable& GetVar() const { return std::get<Variable>(m_base.value()); }

		using ParseResult = Result<std::pair<Argument, TokenCursor>, string>;
		static ParseResult Parse(TokenCursor cur);
	};

	class FunctionProto {
		const std::shared_ptr<const Type> m_ret_type;
		const std::vector<Argument> m_args;

	public:
		FunctionProto(string name, std::shared_ptr<const Type>& ret_type, std::vector<Argument>& args, CallConvention conv = CallConvention::Cdecl)
			: m_ret_type(ret_type) {}

		bool HasDecl() const { return m_ret_type->HasDecl(); }

		const std::shared_ptr<const Type> GetReturnType() const { return m_ret_type; }
		const std::vector<Argument>& GetArgs() const { return m_args; }
		const string& GetDecl() const { return m_ret_type->GetDecl(); }

		CallConvention GetConventionOrDefault(CallConvention default_) const {
			return m_ret_type->HasCallConvention() ? m_ret_type->GetConvention() : default_;
		}

		using ParseResult = Result<std::pair<FunctionProto, StringCursor>, string>;
		static ParseResult Parse(TokenCursor cur);
	};
}