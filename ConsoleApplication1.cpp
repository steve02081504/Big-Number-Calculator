#include <elc/stream>
#include <elc/string>
#include <elc/bignum>

using namespace elc;

string& cutspace(string& str)noexcept{
	const auto beg=str.find_first_not_of(U" \t");
	if (beg==string::npos)
		return str.clear(), str;
	const auto end=str.find_last_not_of(U" \t");
	str=str.substr(beg, end-beg+1);
	return str;
}

struct bad_expr{};

struct base_expr{
	virtual ~base_expr()=default;
	virtual bigfloat eval()const=0;
};

struct num_expr:base_expr{
	bigfloat num;
	num_expr(bigfloat num):num(num){}
	bigfloat eval()const override{
		return num;
	}
};

struct op_expr:base_expr{
	enum class op_type{
		add, sub, mul, div
	};
	op_type op;
	base_expr* lhs;
	base_expr* rhs;
	op_expr(op_type op, base_expr* lhs, base_expr* rhs):op(op), lhs(lhs), rhs(rhs){}
	~op_expr(){
		delete lhs;
		delete rhs;
	}
	bigfloat eval()const override{
		switch (op){
		case op_type::add:
			return lhs->eval()+rhs->eval();
		case op_type::sub:
			return lhs->eval()-rhs->eval();
		case op_type::mul:
			return lhs->eval()*rhs->eval();
		case op_type::div:
			return lhs->eval()/rhs->eval();
		default:
			throw bad_expr();
		}
	}
};

base_expr* parse_expr(string str){
	cutspace(str);
	if (str.empty())
		throw bad_expr();
	string l_str;
	if (str.front()==U'('){
		size_t bracket_num=1;
		for (size_t i=1; i<str.size(); ++i){
			if (str[i]==U'(')
				++bracket_num;
			else if (str[i]==U')')
				--bracket_num;
			if (bracket_num==0){
				l_str=str.substr(1, i-1);
				str=str.substr(i+1);
				break;
			}
		}
		if (bracket_num!=0)
			throw bad_expr();
	}
	else{
		const auto pos=str.find_first_of(U"+-*/(");
		if (pos==string::npos){
			decltype(from_string_get<bigfloat>)::state_t state;
			auto num=from_string_get<bigfloat>(str, state);
			if(state.success())
				return new num_expr(num);
			throw bad_expr();
		}
		l_str=str.substr(0,pos);
		str=str.substr(pos);
	}
	cutspace(str);
	cutspace(l_str);
	if (str.empty())
		return parse_expr(l_str);
	op_expr::op_type op=op_expr::op_type::mul;//default
	if(str.front()==U'+'){
		op=op_expr::op_type::add;
		str.remove_front();
	}
	else if(str.front()==U'-'){
		op=op_expr::op_type::sub;
		str.remove_front();
	}
	else if(str.front()==U'*'){
		op=op_expr::op_type::mul;
		str.remove_front();
	}
	else if(str.front()==U'/'){
		op=op_expr::op_type::div;
		str.remove_front();
	}
	cutspace(str);
	if(str.empty())
		return parse_expr(l_str);
	return new op_expr(op, parse_expr(l_str), parse_expr(str));
}

int main(){
	string str;
	while (true){
		out<<U">>> ";
		getline(in, str);
		if (str==U"exit")
			break;
		try{
			auto expr=parse_expr(str);
			out<<expr->eval()<<endline;
			delete expr;
		}
		catch (bad_expr){
			out<<U"bad expr"<<endline;
		}
	}
	return 0;
}
