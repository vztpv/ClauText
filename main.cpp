

#define _CRT_SECURE_NO_WARNINGS

//#define ARRAYS_DEBUG
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <algorithm>
#include <string>
#include <ctime>
#include <cstdlib>
using namespace std;

#include <conio.h>

#include <wiz/load_data.h>
#include <wiz/stacks.h>
#include <wiz/cpp_string.h>


class EventInfo
{
public:
	wiz::load_data::UserType* eventUT;
	wiz::ArrayStack< wiz::load_data::UserType* > nowUT; //
	int item_idx;
	wiz::ArrayStack<int> userType_idx;
	vector<pair<string, string>> parameters;
	map<string, string> locals;
	string id; //
	wiz::ArrayStack<string> conditionStack;
	wiz::ArrayStack<int> state;
	string return_value;
	string option;
public:
	EventInfo() : eventUT(NULL), return_value("")
	{

	}
};

class SortInfo // need to rename
{
public:
	string name;
	int iElement; // 2 : userType, // 1 : item
	size_t idx;
public:
	SortInfo() { }
	SortInfo(const string& name, int iElement, size_t idx)
		: name(name), iElement(iElement), idx(idx)
	{

	}
	// for sorting..
	bool operator<(const SortInfo& info) const
	{
		return this->name < info.name;
	}
};
// only one exist or do not exist
inline string GetItem(const wiz::load_data::UserType* ut, const string& name) {
	return ut->GetItem(name)[0].Get(0);
}

inline wiz::load_data::UserType* GetUserType(const wiz::load_data::UserType* ut, const string& name)
{
	return ut->GetUserTypeItem(name)[0].Get(0);
}
string Find(wiz::load_data::UserType* ut, const string& str)
{
	int count = 0;
	int idx = -1;
	for (int i = str.size() - 1; i >= 0; --i) {
		if ('/' == str[i]) {
			if (count == 0) {
				idx = i;
			}
			count++;
		}
	}

	if (count == 1)
	{
		return "";
	}
	else {
		auto x = wiz::load_data::UserType::Find(ut,
			wiz::String::substring(str, 0, idx));
		if (x.first == false) { return ""; }
		else if (x.second[0]->GetItem(wiz::String::substring(str, idx + 1)).empty())
		{
			if ((x = wiz::load_data::UserType::Find(ut, str)).first)
			{
				return str;
			}
			else
			{
				return "";
			}
		}
		else { return x.second[0]->GetItem(wiz::String::substring(str, idx + 1))[0].Get(0); }
	}
}
inline bool Exist(wiz::load_data::UserType* ut, const string& dir)
{
	auto x = wiz::load_data::UserType::Find(ut, dir);
	return x.first;
}
// to do - rename!
pair<string, string> Find2(wiz::load_data::UserType* ut, const string& str)
{
	int idx = -1;
	for (int i = str.size() - 1; i >= 0; --i) {
		if ('/' == str[i]) {
			idx = i;
			break;
		}
	}
	return{ wiz::String::substring(str, 0, idx), wiz::String::substring(str,idx + 1) };
}
string reverse(string str) { /// to std::reverse 
	std::reverse(str.begin(), str.end());
	return str;
}

string GetType(const string& str) {
	if (wiz::load_data::Utility::IsInteger(str)) { return "INTEGER"; }
	else if (wiz::load_data::Utility::IsDouble(str)) { return "DOUBLE"; }
	else if (wiz::load_data::Utility::IsDate(str)) { return "DATE"; }
	else return "STRING";
}
string Compare(const string& str1, const string& str2, const int type = 0)
{
	string type1 = GetType(str1);
	string type2 = GetType(str2);

	if (type1 != type2) {
		return "ERROR";
	}

	if ("STRING" == type1 || type == 1)
	{
		if (str1 < str2) {
			return "< 0";
		}
		else if (str1 == str2) {
			return "== 0";
		}
		return "> 0";
	}
	else if ("INTEGER" == type1)
	{
		if (wiz::load_data::Utility::IsMinus(str1) && !wiz::load_data::Utility::IsMinus(str2)) { return "< 0"; }
		else if (!wiz::load_data::Utility::IsMinus(str1) && wiz::load_data::Utility::IsMinus(str2)) { return "> 0"; }

		const bool minusComp = wiz::load_data::Utility::IsMinus(str1) && wiz::load_data::Utility::IsMinus(str2);

		if (false == minusComp) {
			string x = reverse(str1);
			string y = reverse(str2);

			if (x[0] == '+') { x = string(x.c_str() + 1); }
			if (y[0] == '+') { y = string(y.c_str() + 1); }

			if (x.size() < y.size()) {
				for (int i = 0; i < y.size() - x.size(); ++i) {
					x.push_back('0');
				}
			}
			else {
				for (int i = 0; i < x.size() - y.size(); ++i) {
					y.push_back('0');
				}
			}
			return Compare(reverse(x), reverse(y), 1);
		}
		else {
			return Compare(string(str2.c_str() + 1), string(str1.c_str() + 1));
		}
	}
	else if ("DOUBLE" == type1)
	{
		wiz::StringTokenizer tokenizer1(str1, ".");
		wiz::StringTokenizer tokenizer2(str2, ".");

		string x = tokenizer1.nextToken();
		string y = tokenizer2.nextToken();

		string z = Compare(x, y);
		if ("= 0" == z)
		{
			x = tokenizer1.nextToken();
			y = tokenizer2.nextToken();

			if (x.size() < y.size()) {
				for (int i = 0; i < y.size() - x.size(); ++i) {
					x.push_back('0');
				}
			}
			else {
				for (int i = 0; i < x.size() - y.size(); ++i) {
					y.push_back('0');
				}
			}
			return Compare(x, y, 1);
		}
		else
		{
			return z;
		}
	}
	else if ("DATE" == type1)
	{
		wiz::StringTokenizer tokenizer1(str1, ".");
		wiz::StringTokenizer tokenizer2(str2, ".");

		for (int i = 0; i < 3; ++i) {
			const string x = tokenizer1.nextToken();
			const string y = tokenizer2.nextToken();

			const string comp = Compare(x, y);

			if (comp == "< 0") { return comp; }
			else if (comp == "> 0") { return comp; }
		}
		return "== 0";
	}

	return "ERROR";
}

inline string FindParameters(const vector<pair<string, string>>& parameters, const string& operand)
{
	for (int i = 0; i < parameters.size(); ++i) {
		if (wiz::String::startsWith(operand, "$parameter.")
			&& wiz::String::substring(operand, 11) == parameters[i].first) {
			return parameters[i].second;
		}
	}
	return "";
}
inline string FindLocals(map<string, string>& locals, const string& operand)
{
	if (wiz::String::startsWith(operand, "$local.") && locals.end() != locals.find(wiz::String::substring(operand, 7)))
	{
		return locals[wiz::String::substring(operand, 7)];
	}
	return "";
}
//need to renewal. add $AND $OR $NOT
/// remove /, parameter chk!!
void operation(wiz::load_data::UserType& global, const vector<pair<string, string>>& parameters, const string& str, wiz::ArrayStack<string>& operandStack, EventInfo  info)
{
	if (!operandStack.empty() && operandStack.top() == "ERROR") { return; }

	if ("$EQ" == str) {
		string x, y;
		int idx = -1;
		x = operandStack.pop();
		y = operandStack.pop();

		if (x == y) {
			operandStack.push("TRUE");
		}
		else {
			operandStack.push("FALSE");
		}
	}
	else if ("$NOTEQ" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (x != y) {
			operandStack.push("TRUE");
		}
		else {
			operandStack.push("FALSE");
		}
	}
	else if ("$AND" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (x == "TRUE" && y == "TRUE") {
			operandStack.push("TRUE");
		}
		else {
			operandStack.push("FALSE");
		}
	}
	else if ("$OR" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (x == "TRUE" || y == "TRUE") {
			operandStack.push("TRUE");
		}
		else {
			operandStack.push("FALSE");
		}
	}
	else if ("$NOT" == str)
	{
		string x;
		x = operandStack.pop();

		if (x == "TRUE") {
			operandStack.push("FALSE");
		}
		else {
			operandStack.push("TRUE");
		}
	}
	else if ("$COMP<" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (Compare(x, y) == "< 0") {
			operandStack.push("TRUE");
		}
		else
		{
			operandStack.push("FALSE");
		}
	}
	else if ("$COMP>" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (Compare(x, y) == "> 0") {
			operandStack.push("TRUE");
		}
		else
		{
			operandStack.push("FALSE");
		}
	}
	else if ("$COMP<EQ" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (Compare(x, y) == "< 0" || Compare(x, y) == "== 0") {
			operandStack.push("TRUE");
		}
		else
		{
			operandStack.push("FALSE");
		}
	}
	else if ("$COMP>EQ" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (Compare(x, y) == "> 0" || Compare(x, y) == "== 0") {
			operandStack.push("TRUE");
		}
		else
		{
			operandStack.push("FALSE");
		}
	}
	else if ("$add" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (GetType(x) == GetType(y) && (GetType(y) == "INTEGER")) { /// only integer -> BigInteger
			operandStack.push(wiz::toStr(atoll(x.c_str()) + atoll(y.c_str())));
		}
		else
		{
			operandStack.push("ERROR");
		}
	}
	else if ("$multiple" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();



		if (GetType(x) == GetType(y) && (GetType(y) == "INTEGER")) { /// only integer -> BigInteger
			operandStack.push(wiz::toStr(atoll(x.c_str()) * atoll(y.c_str())));
		}
		else
		{
			operandStack.push("ERROR");
		}
	}
	else if ("$divide" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (GetType(x) == GetType(y) && (GetType(y) == "INTEGER")) { /// only integer -> BigInteger
			operandStack.push(wiz::toStr(atoll(x.c_str()) / atoll(y.c_str())));
		}
		else
		{
			operandStack.push("ERROR");
		}
	}
	else if ("$modular" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (GetType(x) == GetType(y) && (GetType(y) == "INTEGER")) { /// only integer -> BigInteger
			operandStack.push(wiz::toStr(atoll(x.c_str()) % atoll(y.c_str())));
		}
		else
		{
			operandStack.push("ERROR");
		}
	}
	else if ("$rand" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		if (GetType(x) == GetType(y) && (GetType(y) == "INTEGER")) { /// only integer -> BigInteger
			long long _x = atoll(x.c_str());
			long long _y = atoll(y.c_str());
			long long _z = rand() % (_y - _x + 1) + _x;
			operandStack.push(wiz::toStr(_z));
		}
		else
		{
			operandStack.push("ERROR");
		}
	}
	else if ("$concat" == str)
	{
		string x, y;
		x = operandStack.pop();
		y = operandStack.pop();

		operandStack.push(x + y);
	}
	else if ("$return_value" == str)
	{
		operandStack.push(info.return_value);
	}
	else if ("$back" == str) // ex) for x  = { 0 1 2 3 .. }, for usertaypelist? and mixed?
	{
		string x = operandStack.pop();

		string value = wiz::load_data::LoadData::GetItemListData(global, x, "TRUE");
		wiz::load_data::UserType ut;
		wiz::load_data::LoadData::LoadDataFromString(value, ut);

		if (ut.IsItemList(ut.GetIList().size()-1))
		{
			auto x = (wiz::load_data::TypeArray<string>*)ut.GetList(ut.GetIList().size() - 1);
			operandStack.push(x->Get(0));
		}
		else {
			auto x = (wiz::load_data::TypeArray<wiz::load_data::UserType*>*)ut.GetList(ut.GetIList().size() - 1);
			operandStack.push("\"" + x->Get(0)->ToString() + "\"");
		}
	}
	else if ("$pop_back" == str) // and for usertypelist? and mixed?, usertype-> "~"
	{
		string x = operandStack.pop();
		string name;
		for (int i = x.size() - 1; i >= 0; --i)
		{
			if (x[i] == '/' && i != 0) {
				name = wiz::String::substring(x, i + 1);
				x = wiz::String::substring(x, 0, i - 1);
				break;
			}
		}

		if (x.empty()) { x = "."; }

		wiz::load_data::UserType* ut = NULL;
		auto finded = wiz::load_data::UserType::Find(&global, x);
		ut = finded.second[0];

		if (ut->IsItemList(ut->GetIList().size() - 1))
		{
			auto x = (wiz::load_data::TypeArray<string>*)ut->GetList(ut->GetIList().size() - 1);
			operandStack.push(x->Get(0));
			ut->RemoveItemList(ut->GetItemListSize() - 1);
		}
		else {
			auto x = (wiz::load_data::TypeArray<wiz::load_data::UserType*>*)ut->GetList(ut->GetIList().size() - 1);
			operandStack.push("\"" + x->Get(0)->ToString() + "\"");
			ut->RemoveUserTypeList(ut->GetUserTypeListSize() - 1);
		}
	}
	// todo - $front, pop-front.
	else if ("$front" == str)
	{
		string x = operandStack.pop();

		string value = wiz::load_data::LoadData::GetItemListData(global, x, "TRUE");
		wiz::load_data::UserType ut;
		wiz::load_data::LoadData::LoadDataFromString(value, ut);

		if (ut.IsItemList(0))
		{
			auto x = (wiz::load_data::TypeArray<string>*)ut.GetList(0);
			operandStack.push(x->Get(0));
		}
		else {
			auto x = (wiz::load_data::TypeArray<wiz::load_data::UserType*>*)ut.GetList(0);
			operandStack.push("\"" + x->Get(0)->ToString() + "\"");
		}
	}
	else if ("$pop_front" == str)
	{
		string x = operandStack.pop();
		string name;
		for (int i = x.size() - 1; i >= 0; --i)
		{
			if (x[i] == '/' && i != 0) {
				name = wiz::String::substring(x, i + 1);
				x = wiz::String::substring(x, 0, i - 1);
				break;
			}
		}

		if (x.empty()) { x = "."; }

		wiz::load_data::UserType* ut = NULL;
		auto finded = wiz::load_data::UserType::Find(&global, x);
		ut = finded.second[0];

		if (ut->IsItemList(0))
		{
			auto x = (wiz::load_data::TypeArray<string>*)ut->GetList(0);
			operandStack.push(x->Get(0));
			ut->RemoveItemList(0);
		}
		else {
			auto x = (wiz::load_data::TypeArray<wiz::load_data::UserType*>*)ut->GetList(0);
			operandStack.push("\"" + x->Get(0)->ToString() + "\"");
			ut->RemoveUserTypeList(0);
		}
	}
	else if ("$link" == str) // removal
	{
		// to do...
	}
	else if ("$findIndex" == str) // removal
	{
		// todo
	}
	else if ("$get" == str)
	{
		string x = operandStack.pop();

		if ('/' == x[0])
		{
			string temp = Find(&global, x); if (!temp.empty()) { x = temp; }
		}
		{
			string temp = FindParameters(parameters, x);
			if (!temp.empty()) { x = temp; }
		}
		{
			string temp = FindLocals(info.locals, x);
			if (!temp.empty()) { x = temp; }
		}
		operandStack.push(x);
	}
	else if ("$val" == str)
	{
		string x = operandStack.pop();

		// todo..

		operandStack.push(x);
	}
	else if ("$size" == str)
	{
		string x = operandStack.pop();

		if ('/' == x[0])
		{
			wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, x).second[0];
			x = wiz::toStr(ut->GetItemListSize());
		}
		else
		{
			x = "FALSE";
		}

		operandStack.push(x);
	}
	else if ("$element" == str) // for list
	{
		string x = operandStack.pop(); // list_name
		string y = operandStack.pop(); // idx
		long long idx = atoll(y.c_str());

		if ('/' == x[0])
		{
			wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, x).second[0];
			x = ut->GetItemList(idx).Get(0);
		}
		else
		{
			x = "NONE";
		}

		operandStack.push(x);
	}
}

string ToBool3(wiz::load_data::UserType& global, const vector<pair<string, string>>& parameters, const string& temp, EventInfo info) /// has bug!
{
	wiz::StringTokenizer tokenizer(temp, vector<string>{ "/" });
	vector<string> tokenVec;
	string result;

	while (tokenizer.hasMoreTokens()) {
		tokenVec.push_back(tokenizer.nextToken());
	}

	for (int i = 0; i < tokenVec.size(); ++i)
	{
		result = result + "/";
		if (wiz::String::startsWith(tokenVec[i], "$parameter.")) {
			int last = -1;
			for (int j = 0; j < tokenVec[i].size(); ++j)
			{
				if (tokenVec[i][j] == ' ' || tokenVec[i][j] == '{' || tokenVec[i][j] == '}' || tokenVec[i][j] == '=') {
					last = j - 1;
					break;
				}
			}
			if (last != -1)
			{
				string temp = FindParameters(parameters, wiz::String::substring(tokenVec[i], 0, last));

				if (!temp.empty()) {
					tokenVec[i] = wiz::String::replace(wiz::String::substring(tokenVec[i], 0, last), wiz::String::substring(tokenVec[i], 0, last), temp)
						+ wiz::String::substring(tokenVec[i], last + 1);
				}
			}
			else
			{
				string temp = FindParameters(parameters, tokenVec[i]);
				if (!temp.empty()) {
					tokenVec[i] = temp;
				}
			}
		}
		else if (wiz::String::startsWith(tokenVec[i], "$local.")) {
			int last = -1;
			for (int j = 0; j < tokenVec[i].size(); ++j)
			{
				if (tokenVec[i][j] == ' ' || tokenVec[i][j] == '{' || tokenVec[i][j] == '}' || tokenVec[i][j] == '=') {
					last = j - 1;
					break;
				}
			}
			if (last != -1)
			{
				string temp = FindLocals(info.locals, wiz::String::substring(tokenVec[i], 0, last));

				if (!temp.empty()) {
					tokenVec[i] = wiz::String::replace(wiz::String::substring(tokenVec[i], 0, last), wiz::String::substring(tokenVec[i], 0, last), temp)
						+ wiz::String::substring(tokenVec[i], last + 1);
				}
			}
			else
			{
				string temp = FindLocals(info.locals, tokenVec[i]);
				if (!temp.empty()) {
					tokenVec[i] = temp;
				}
			}
		}

		result = result + tokenVec[i];
	}
	return result;
}
string ToBool4(wiz::load_data::UserType& global, const vector<pair<string, string>>& parameters, const string& temp, EventInfo  info)
{
	string result = temp;
	wiz::ArrayStack<string> resultStack;
	wiz::load_data::UserType ut;

	bool flag_A = false;
	if (result.size() > 1 && result[0] == '/')
	{
		flag_A = true;
	}
	result = ToBool3(global, parameters, result, info);
	if (result.empty()) { return ""; }
	if (!flag_A) {
		result = string(result.c_str() + 1);
	}
	wiz::load_data::LoadData::LoadDataFromString(result, ut);
	result = ut.ToString();
	if (result.empty()) { return result; }
	result.pop_back();// 여백제거.
					  // chk
	if (ut.empty()) {
		return "";
	}
	if (ut.GetUserTypeListSize() == 0 && ut.GetItemListSize() == 1) /// chk
	{
		if ('/' == result[0] && result.size() > 1)
		{
			string temp = Find(&global, result);
			if (!temp.empty()) {
				result = temp;
			}
		}
		else if (wiz::String::startsWith(result, "$local.")) {
			string _temp = FindLocals(info.locals, result);
			if (!_temp.empty()) {
				result = _temp;
			}
		}
		else if (wiz::String::startsWith(result, "$parameter.")) {
			string _temp = FindParameters(parameters, result);
			if (!_temp.empty()) {
				result = _temp;
			}
		}
		return result;
	}

	{
		result = wiz::load_data::Utility::ChangeStr(result, "^", "^0");
		result = wiz::load_data::Utility::ChangeStr(result, " ", "^1");
		result = wiz::load_data::Utility::ChangeStr(result, "\t", "^2");
		result = wiz::load_data::Utility::ChangeStr(result, "\r", "^3");
		result = wiz::load_data::Utility::ChangeStr(result, "\n", "^4");
	}

	{ // chk..
		wiz::StringTokenizer tokenizer(result, { " ", "\n", "\t", "\r", "{", "=", "}" });
		wiz::StringTokenizer tokenizer2(result, { " ", "\n", "\t", "\r" });
		vector<string> tokenVec;
		vector<string> tokenVec2;

		while (tokenizer.hasMoreTokens()) {
			tokenVec.push_back(tokenizer.nextToken());
		}

		for (int i = tokenVec.size() - 1; i >= 0; --i)
		{
			string before = tokenVec[i];
			if ('/' == tokenVec[i][0])
			{
				string _temp = Find(&global, tokenVec[i]);
				if ("" != _temp) {
					tokenVec[i] = _temp;
				}


			}
			else if (wiz::String::startsWith(tokenVec[i], "$local.")) {
				string _temp = FindLocals(info.locals, tokenVec[i]);
				if (!_temp.empty()) {
					tokenVec[i] = _temp;
				}
			}
			else if (wiz::String::startsWith(tokenVec[i], "$parameter.")) {
				string _temp = FindParameters(parameters, tokenVec[i]);
				if (!_temp.empty()) {
					tokenVec[i] = _temp;
				}
			}
		}

		while (tokenizer2.hasMoreTokens()) {
			tokenVec2.push_back(tokenizer2.nextToken());
		}

		result = "";

		int j = tokenVec.size();
		for (int i = tokenVec2.size() - 1; i >= 0; --i) {
			if (tokenVec2[i] == "{" || tokenVec2[i] == "}" || tokenVec2[i] == "=") {
				continue;
			}
			else {
				--j;
				tokenVec2[i] = tokenVec[j];
			}
		}

		for (int i = 0; i < tokenVec2.size(); ++i) {
			if (i != 0) { result = result + " "; }
			result = result + tokenVec2[i];
		}
	}

	//
	wiz::ArrayStack<string> operandStack; // 피연산자
	wiz::ArrayStack<string> operatorStack; // 연산자

	wiz::StringTokenizer tokenizer(result, { " ", "\n", "\t", "\r" });
	vector<string> tokenVec;

	while (tokenizer.hasMoreTokens()) {
		tokenVec.push_back(tokenizer.nextToken());
	}

	for (int i = tokenVec.size() - 1; i >= 0; --i)
	{
		if ('/' == tokenVec[i][0] && tokenVec[i].size() > 1)
		{
			string temp = Find(&global, tokenVec[i]);
			if (!temp.empty()) {
				tokenVec[i] = temp;
			}
		}

		if ('$' != tokenVec[i][0]) {
			operandStack.push(tokenVec[i]);
		}
		else
		{
			operandStack.pop();
			operandStack.pop();
			operatorStack.push(tokenVec[i]);
			operation(global, parameters, tokenVec[i], operandStack, info);
			operandStack[operandStack.size() - 2] = operandStack[operandStack.size() - 1];
			operandStack.pop();
		}
	}

	// ex) A = { B = 1 C = { 3 } } D = { E }
	// =>  A = { B = 1 C = 3  }  D = E

	//result = "";
	vector<string> strVec;
	stack<int> chkBrace;

	chkBrace.push(0);

	for (int i = operandStack.size() - 1; i >= 0; --i)
	{
		if (operandStack[i] == "}") {
			chkBrace.top()++;
			if (chkBrace.top() == 2)
			{
				string temp = strVec.back();
				strVec.pop_back();
				strVec.pop_back();
				strVec.push_back(temp);
				
				chkBrace.pop();
				continue;
			}
			chkBrace.pop();
		}
		else if (operandStack[i] == "{") {
			chkBrace.top()++;
			chkBrace.push(0);
		}
		else {
			chkBrace.top()++;
		}
		strVec.push_back(operandStack[i]);
	}

	result = "";
	for (int i = 0; i < strVec.size(); ++i) {
		if (i != 0) { result = result + " "; }
		result = result + strVec[i];
	}

	{
		wiz::load_data::Utility::ChangeCharInString(result, "^4", "\n");
		wiz::load_data::Utility::ChangeCharInString(result, "^3", "\r");
		wiz::load_data::Utility::ChangeCharInString(result, "^2", "\t");
		wiz::load_data::Utility::ChangeCharInString(result, "^1", " ");
		wiz::load_data::Utility::ChangeCharInString(result, "^0", "^");
	}
	return result;
}

bool chkFunc(wiz::ArrayStack<EventInfo>& eventStack, wiz::load_data::UserType** val)
{
	if (!eventStack.top().state.empty() && (eventStack.top().state.top() == 1 || eventStack.top().state.top() == 2))
	{

		if (eventStack.top().userType_idx.top() < eventStack.top().nowUT.top()->GetUserTypeListSize())
		{
			(*val) = eventStack.top().nowUT.top()->GetUserTypeList(eventStack.top().userType_idx.top()).Get(0);
			return false;
		}
		else
		{
			eventStack.top().nowUT.pop();
			eventStack.top().userType_idx.pop();
			if (!eventStack.top().conditionStack.empty())
			{
				eventStack.top().conditionStack.pop();
			}
			eventStack.top().state.pop(); // remove state
		}
	}
	else {
		if (!eventStack.empty() && !eventStack.top().userType_idx.empty() && !eventStack.top().nowUT.empty()
			&& eventStack.top().userType_idx.top() >= eventStack.top().nowUT.top()->GetUserTypeListSize())
		{
			eventStack.top().nowUT.pop();
			eventStack.top().userType_idx.pop();
		}
	}
	return true;
}


string excute_module(wiz::load_data::UserType& global)
{
	string module_value = "";
	// data, event load..
	wiz::ArrayStack<EventInfo> eventStack;
	map<string, int> convert;
	auto events = global.GetUserTypeItem("Event");
	if (global.GetUserTypeItem("Main").empty())
	{
		cout << "do not exist Main" << endl;
		return "ERROR -1";
	}
	auto Main = GetUserType(&global, "Main"); // Main이 없으면 에러..!
	

	// event table setting
	for (int i = 0; i < events.size(); ++i)
	{
		auto x = events[i].Get(0)->GetItem("id");
		if (!x.empty()) {
			//cout <<	x[0].Get(0) << endl;
			convert.insert(pair<string, int>(x[0].Get(0), i));
		}
		else {
			// error
		}
	}

	// start from Main.
	{
		EventInfo info;
		info.eventUT = Main;
		info.item_idx = 0;
		info.userType_idx.push(0);
		info.parameters.push_back(
			make_pair("id", info.eventUT->GetUserTypeItem("$call")[0].Get(0)->GetItem("id")[0].Get(0))
		);
		info.id = info.parameters[0].second;

		const int no = convert.at(info.id);
		for (int i = 0; i < events[no].Get(0)->GetUserTypeListSize(); ++i) {
			if (events[no].Get(0)->GetUserTypeList(i).Get(0)->GetName() == "$local") {
				for (int j = 0; j < events[no].Get(0)->GetUserTypeList(i).Get(0)->GetItemListSize(); ++j) {
					string name = events[no].Get(0)->GetUserTypeList(i).Get(0)->GetItemList(j).Get(0);
					string value = "";
					info.locals.insert(make_pair(name, value));
				}
				break;
			}
		}

		eventStack.push(info);
	}

	// main loop
	while (!eventStack.empty())
	{
		EventInfo info = eventStack.top();
		string str;

		for (int i = 0; i < info.parameters.size(); ++i) {
			if (info.parameters[i].first == "id") {
				str = info.parameters[i].second;
				break;
			}
		}

		int no = convert.at(str);

		bool pass = false;
		int state = 0;

		if (info.userType_idx.size() == 1 && info.userType_idx[0] >= events[no].Get(0)->GetUserTypeListSize())
		{
			if (eventStack.size() == 1)
			{
				module_value = eventStack.top().return_value;
			}

			eventStack.pop();
			continue;
		}

		{
			wiz::load_data::UserType* val = NULL;
			if (eventStack.top().userType_idx.size() == 1) {
				val = events[no].Get(0)->GetUserTypeList(eventStack.top().userType_idx.top()).Get(0);
			}
			else
			{
				val = eventStack.top().nowUT.top();
			}

			while (true) {
				if ("$module" == val->GetName())
				{
					string moduleFileName = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					string input;

					if (val->GetUserTypeListSize() >= 2) {
						input = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(1).Get(0)->ToString(), eventStack.top());
					}

					moduleFileName = wiz::String::substring(moduleFileName, 1, moduleFileName.size() - 2);

					wiz::load_data::UserType moduleUT;
					wiz::load_data::LoadData::LoadDataFromFile(moduleFileName, moduleUT);
					wiz::load_data::LoadData::AddData(moduleUT, "", input, "TRUE");

					eventStack.top().return_value = excute_module(moduleUT);

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$option" == val->GetName()) // first
				{
					eventStack.top().option = ToBool4(global, eventStack.top().parameters, val->ToString(), eventStack.top());

					eventStack.top().userType_idx.top()++;
					break;
				}
				// todo - ($push_back-insert!) $pop_back, $push_front, $pop_front ($front?, $back?)
				else if ("$pop_back" == val->GetName()) {
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					wiz::load_data::UserType* ut = NULL;
					auto finded = wiz::load_data::UserType::Find(&global, dir);
					ut = finded.second[0];

					wiz::load_data::LoadData::Remove(global, dir, ut->GetIList().size() - 1, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$push_front" == val->GetName()) {
					string value = val->GetUserTypeList(1).Get(0)->ToString();
					string dir;
					if (val->GetUserTypeList(0).Get(0)->GetItemListSize() > 0) {
						dir = val->GetUserTypeList(0).Get(0)->GetItemList(0).Get(0);
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}
					else ///val->Ge
					{
						dir = string(val->GetUserTypeList(0).Get(0)->ToString());
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}

					value = ToBool4(global, eventStack.top().parameters, value, eventStack.top());
					
					wiz::load_data::LoadData::AddDataAtFront(global, dir, value, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$pop_front" == val->GetName()) {
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					
					wiz::load_data::LoadData::Remove(global, dir, 0, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$call" == val->GetName()) {
					if (!val->GetItem("id").empty()) {
						info.id = val->GetItem("id")[0].Get(0);
					}
					else {
						info.id = ToBool4(global, eventStack.top().parameters, val->GetUserTypeItem("id")[0].Get(0)->ToString(), eventStack.top());
					}
					// cf) id =  { $local.i }
					// 추가 todo

					info.eventUT = events[no].Get(0);
					info.userType_idx.clear();
					info.userType_idx.push(0);
					info.return_value.clear();

					EventInfo info2;
					info2 = info;

					if (info.id != eventStack.top().id) {
						info.parameters.clear();
					}
					info.conditionStack.clear();

					//
					if (info.id != eventStack.top().id) {
						for (int j = 0; j < val->GetItemListSize(); ++j) {
							if (val->GetItemListSize() > 0) {
								string temp = ToBool4(global, info2.parameters, val->GetItemList(j).Get(0), info2);
								info.parameters.push_back(make_pair(val->GetItemList(j).GetName(), temp));
							}
						}
						for (int j = 0; j < val->GetUserTypeListSize(); ++j) {
							if (val->GetUserTypeListSize() > 0) {
								string temp = ToBool4(global, info2.parameters, val->GetUserTypeList(j).Get(0)->ToString(), info2);
								info.parameters.push_back(make_pair(val->GetUserTypeList(j).GetName(), temp));
							}
						}
						eventStack.top().userType_idx.top()++;
					}
					else {
						if (val->GetItemListSize() > 0) {
							for (int j = 0; j < val->GetItemListSize(); ++j) {
								string temp = ToBool4(global, info2.parameters, val->GetItemList(j).Get(0), info);
								for (int k = 0; k < info.parameters.size(); ++k)
								{
									if (info.parameters[k].first == val->GetItemList(j).GetName())
									{
										info.parameters[k].second = temp;
									}
								}
								// debug 
								//cout << temp << endl;
							}
						}
						if (val->GetUserTypeListSize() > 0) {
							for (int j = 0; j < val->GetUserTypeListSize(); ++j) {
								string temp = ToBool4(global, info2.parameters, val->GetUserTypeList(j).Get(0)->ToString(), info);
								for (int k = 0; k < info.parameters.size(); ++k)
								{
									if (info.parameters[k].first == val->GetUserTypeList(j).GetName())
									{
										info.parameters[k].second = temp;
									}
								}
								// debug 
								//cout << temp << endl;
							}
						}

						eventStack.top().userType_idx.top()++;

						// remove remove_now_event_stack_a?
						if (eventStack.top().option == "REMOVE_NOW_EVENT_STACK_A" || "REMOVE_IF_CALL_ONESELF_EVENT" == eventStack.top().option) //
						{
							eventStack.pop();
						}
					}

					if (eventStack.top().option == "REMOVE_IF_CALL_ANY_EVENT")
					{
						eventStack.pop();
					}


					info.locals.clear();
					const int no = convert.at(info.id);
					for (int i = 0; i < events[no].Get(0)->GetUserTypeListSize(); ++i) {
						if (events[no].Get(0)->GetUserTypeList(i).Get(0)->GetName() == "$local") {
							for (int j = 0; j < events[no].Get(0)->GetUserTypeList(i).Get(0)->GetItemListSize(); ++j) {
								string name = events[no].Get(0)->GetUserTypeList(i).Get(0)->GetItemList(j).Get(0);
								string value = "";
								info.locals.insert(make_pair(name, value));
							}
							break;
						}
					}

					eventStack.push(info);

					pass = true;
					break;
				}
				else if ("$assign" == val->GetName())
				{
					pair<string, string> dir = Find2(&global, val->GetItemList(0).Get(0));
					string data = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					if (dir.first == "" && wiz::String::startsWith(dir.second, "$local."))
					{
						eventStack.top().locals[wiz::String::substring(dir.second, 7)] = data;
					}
					else {
						wiz::load_data::LoadData::SetData(global, dir.first, dir.second, data, "TRUE");
					}
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$assign2" == val->GetName())
				{
					pair<string, string> dir = Find2(&global, ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top()));
					string data = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(1).Get(0)->ToString(), eventStack.top());
					
					{
						wiz::load_data::LoadData::SetData(global, dir.first, dir.second, data, "TRUE");
					}
					
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("push_back" == val->GetName() || "$insert" == val->GetName() || "$insert2" == val->GetName())
				{
					string value = val->GetUserTypeList(1).Get(0)->ToString();
					string dir;
					if (val->GetUserTypeList(0).Get(0)->GetItemListSize() > 0) {
						dir = val->GetUserTypeList(0).Get(0)->GetItemList(0).Get(0);
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}
					else ///val->Ge
					{
						dir = string(val->GetUserTypeList(0).Get(0)->ToString());
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}

					value = ToBool4(global, eventStack.top().parameters, value, eventStack.top());

					wiz::load_data::LoadData::AddData(global, dir, value, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$make" == val->GetName()) // To Do
				{
					string dir;

					if (val->GetItemListSize() > 0) {
						dir = val->GetItemList(0).Get(0);
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}
					else
					{
						dir = string(val->ToString());
						dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					}

					string name;
					for (int i = dir.size() - 1; i >= 0; --i)
					{
						if (dir[i] == '/') {
							name = wiz::String::substring(dir, i + 1);
							dir = wiz::String::substring(dir, 0, i - 1);
							break;
						}
					}
					if (dir.empty()) { dir = "."; }

					if (dir == ".")
					{
						wiz::load_data::LoadData::AddUserType(global, dir, name, "", "TRUE");
					}
					else
					{
						wiz::load_data::LoadData::AddUserType(global, dir, name, "", "TRUE");
					}

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$findIndex" == val->GetName()) // For list : { 1 2  3 4 5 }
				{
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					string value = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(1).Get(0)->ToString(), eventStack.top());

					wiz::load_data::UserType ut;
					wiz::load_data::LoadData::LoadDataFromString(wiz::load_data::UserType::Find(&global, dir).second[0]->ToString(), ut);

					for (int i = 0; i < ut.GetItemListSize(); ++i) {
						if (ut.GetItemList(i).Get(0) == value) {
							eventStack.top().return_value = wiz::toStr(i);
							break;
						}
					}

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$remove" == val->GetName()) // remove by dir.
				{
					string dir = string(val->GetItemList(0).Get(0).c_str()); // item -> userType

					dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());

					wiz::load_data::LoadData::Remove(global, dir, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$remove2" == val->GetName()) // remove /dir/name
				{
					string dir = string(val->GetItemList(0).Get(0).c_str()); // item -> userType
					dir = ToBool4(global, eventStack.top().parameters, dir, eventStack.top());
					string name;
					for (int i = dir.size() - 1; i >= 0; --i)
					{
						if (dir[i] == '/') {
							name = wiz::String::substring(dir, i + 1);
							dir = wiz::String::substring(dir, 0, i - 1);
							break;
						}
					}

					wiz::load_data::LoadData::Remove(global, dir, name, "TRUE");

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$remove3" == val->GetName()) /// remove itemlist by idx.
				{
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					string value = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(1).Get(0)->ToString(), eventStack.top());

					long long idx = atoll(value.c_str());

					wiz::load_data::UserType::Find(&global, dir).second[0]->RemoveItemList(idx);

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$setElement" == val->GetName())
				{
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					string idx = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(1).Get(0)->ToString(), eventStack.top());
					string value = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(2).Get(0)->ToString(), eventStack.top());

					long long _idx = stoll(idx);
					wiz::load_data::UserType::Find(&global, dir).second[0]->SetItem(_idx, value);

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$swap" == val->GetName()) // $swap2
				{
					string dir = string(val->GetItemList(0).Get(0).c_str()); // + 0
					string value1 = val->GetUserTypeList(0).Get(0)->ToString();
					string value2 = val->GetUserTypeList(1).Get(0)->ToString();

					value1 = ToBool4(global, eventStack.top().parameters, value1, eventStack.top());
					value2 = ToBool4(global, eventStack.top().parameters, value2, eventStack.top());
					if (value1 != value2) {
						long long x = atoll(value1.c_str());
						long long y = atoll(value2.c_str());

						string temp = wiz::load_data::UserType::Find(&global, dir).second[0]->GetItemList(x).Get(0);
						string temp2 = wiz::load_data::UserType::Find(&global, dir).second[0]->GetItemList(y).Get(0);

						wiz::load_data::LoadData::SetData(global, dir, x, temp2, "TRUE");
						wiz::load_data::LoadData::SetData(global, dir, y, temp, "TRUE");
					}

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$print" == val->GetName()) /// using tobool4, has many bugs..!!
				{
					if (val->GetUserTypeListSize() == 1
						&& val->GetUserTypeList(0).Get(0)->GetItemListSize() == 1)
					{
						string listName = val->GetUserTypeList(0).Get(0)->GetItemList(0).Get(0);

						if (listName.size() >= 2 && listName[0] == '\"' && listName.back() == '\"')
						{
							listName = wiz::String::substring(listName, 1, listName.size() - 2);
							cout << listName;
						}
						else if (listName.size() == 2 && listName[0] == '\\' && listName[1] == 'n')
						{
							cout << endl;
						}
						else if (wiz::String::startsWith(listName, "$local.")
							|| wiz::String::startsWith(listName, "$parameter.")
							)
						{
							string temp = ToBool4(global, eventStack.top().parameters, listName, eventStack.top());
							if (temp.empty()) {
								cout << "EMPTY";
							}
							else {
								cout << temp;
							}
						}
						else if (wiz::String::startsWith(listName, "/") && listName.size() > 1)
						{
							string temp = ToBool4(global, eventStack.top().parameters, listName, eventStack.top());
							if (temp != listName) // chk 
							{
								cout << temp;
							}
							else {
								wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, listName).second[0];
								if (ut->GetItemList(0).GetName().empty()) {
									cout << ut->GetItemList(0).Get(0);
								}
							}
						}
						else
						{
							wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, listName).second[0];
							if (ut->GetItemList(0).GetName().empty()) {
								cout << ut->GetItemList(0).Get(0);
							}
						}
					}
					else if (val->GetUserTypeListSize() == 1
						&& val->GetUserTypeList(0).Get(0)->GetItemListSize() == 0
						&& val->GetUserTypeList(0).Get(0)->GetUserTypeListSize() == 1)
					{
						string name = ToBool4(global, eventStack.top().parameters,
							val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
						cout << name;
					}
					else
					{
						string start = val->GetUserTypeList(1).Get(0)->ToString();
						string last = val->GetUserTypeList(2).Get(0)->ToString();

						start = ToBool4(global, eventStack.top().parameters, start, eventStack.top());
						last = ToBool4(global, eventStack.top().parameters, last, eventStack.top());

						string listName = val->GetUserTypeList(0).Get(0)->GetItemList(0).Get(0);
						long long _start = atoll(start.c_str());
						long long _last = atoll(last.c_str());
						wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, listName).second[0];
						for (int i = _start; i <= _last; ++i)
						{
							if (i != _start) { cout << " "; }
							cout << ut->GetItemList(i).Get(0);
						}
					}

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$print2" == val->GetName())
				{
					string dir = ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top());
					str = wiz::load_data::UserType::Find(&global, dir).second[0]->ToString();
					
					cout << str;

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$load" == val->GetName())
				{
					// to do, load data and events from other file!
					for (int i = 0; i < val->GetItemListSize(); ++i) {
						string fileName = ToBool4(global, eventStack.top().parameters, val->GetItemList(i).Get(0), eventStack.top());
						fileName = wiz::String::substring(fileName, 1, fileName.size() - 2);
						wiz::load_data::UserType ut;
						if (wiz::load_data::LoadData::LoadDataFromFile(fileName, ut)) {
							wiz::load_data::LoadData::AddData(global, "", ut.ToString(), "TRUE");
							events = global.GetUserTypeItem("Event");

							auto _Main = ut.GetUserTypeItem("Main");
							if (NULL != Main && !_Main.empty())
							{
								// error!
								cout << "err" << endl;
								return "ERROR -2";
							}
						}
						else {
							// error!
						}
					}

					{
						convert.clear();

						// event table setting
						for (int i = 0; i < events.size(); ++i)
						{
							auto x = events[i].Get(0)->GetItem("id");
							if (!x.empty()) {
								//cout <<	x[0].Get(0) << endl;
								convert.insert(pair<string, int>(x[0].Get(0), i));
							}
							else {
								// error
							}
						}

						// update no
						no = convert[str];
					}

					eventStack.top().userType_idx.top()++;
					break;

				}
				else if ("$clear_screen" == val->GetName())
				{
					system("cls");
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$_getch" == val->GetName())
				{
					_getch();
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$input" == val->GetName())
				{
					string str;
					cin >> str;
					eventStack.top().return_value = str;
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$return" == val->GetName())
				{
					eventStack.top().userType_idx.top()++;
					if (eventStack.size() > 1)
					{
						string temp = ToBool4(global, eventStack.top().parameters, val->ToString(), eventStack.top());
						/// if temp just one
						eventStack[eventStack.size() - 2].return_value = temp;
					}

					if (eventStack.size() == 1)
					{
						module_value = eventStack.top().return_value;
					}
					eventStack.pop();
					break;
				}
				else if ("$parameter" == val->GetName())
				{
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$local" == val->GetName())
				{
					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$sort" == val->GetName()) {
					// todo
					vector<SortInfo> siVec;
					wiz::load_data::UserType* utTemp = 
						wiz::load_data::UserType::Find(&global, 
							ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top())).second[0];
	
					vector<wiz::load_data::Type*> temp;


					int item_count = 0, ut_count = 0;
					for (int i = 0; i < utTemp->GetIList().size(); ++i) {
						if (utTemp->GetIList()[i] == 1) {
							temp.push_back(&(utTemp->GetItemList(item_count)));
							siVec.emplace_back(utTemp->GetItemList(item_count).GetName(), 1, i);
							item_count++;
						}
						else {
							temp.push_back(utTemp->GetUserTypeList(ut_count).Get(0));
							siVec.emplace_back(utTemp->GetUserTypeList(ut_count).GetName(), 2, i);
							ut_count++;
						}
					}

					std::sort(siVec.begin(), siVec.end());
					

					wiz::load_data::UserType ut;
					for (int i = 0; i < temp.size(); ++i)
					{
						if (siVec[i].iElement == 1) {
							ut.AddItem(siVec[i].name, static_cast<wiz::load_data::TypeArray<string>*>(temp[siVec[i].idx])->Get(0));
						}
						else {
							ut.AddUserTypeItem(*(static_cast<wiz::load_data::UserType*>(temp[siVec[i].idx])));
						}
					}

					utTemp->Remove();

					wiz::load_data::LoadData::AddData(*(utTemp), "", ut.ToString(), "TRUE");
					

					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$stable_sort" == val->GetName()) {
					// todo
					// todo
					vector<SortInfo> siVec;
					wiz::load_data::UserType* utTemp =
						wiz::load_data::UserType::Find(&global,
							ToBool4(global, eventStack.top().parameters, val->GetUserTypeList(0).Get(0)->ToString(), eventStack.top())).second[0];

					vector<wiz::load_data::Type*> temp;


					int item_count = 0, ut_count = 0;
					for (int i = 0; i < utTemp->GetIList().size(); ++i) {
						if (utTemp->GetIList()[i] == 1) {
							temp.push_back(&(utTemp->GetItemList(item_count)));
							siVec.emplace_back(utTemp->GetItemList(item_count).GetName(), 1, i);
							item_count++;
						}
						else {
							temp.push_back(utTemp->GetUserTypeList(ut_count).Get(0));
							siVec.emplace_back(utTemp->GetUserTypeList(ut_count).GetName(), 2, i);
							ut_count++;
						}
					}

					std::stable_sort(siVec.begin(), siVec.end());


					wiz::load_data::UserType ut;
					for (int i = 0; i < temp.size(); ++i)
					{
						if (siVec[i].iElement == 1) {
							ut.AddItem(siVec[i].name, static_cast<wiz::load_data::TypeArray<string>*>(temp[siVec[i].idx])->Get(0));
						}
						else {
							ut.AddUserTypeItem(*(static_cast<wiz::load_data::UserType*>(temp[siVec[i].idx])));
						}
					}

					utTemp->Remove();

					wiz::load_data::LoadData::AddData(*(utTemp), "", ut.ToString(), "TRUE");


					eventStack.top().userType_idx.top()++;
					break;
				}
				else if ("$if" == val->GetName()) // ToDo!!
				{
					string temp = val->GetUserTypeList(0).Get(0)->ToString();
					temp = ToBool4(global, eventStack.top().parameters, temp, eventStack.top());

					if (!eventStack.top().conditionStack.empty())
					{
						if ("TRUE" == temp && eventStack.top().conditionStack.top() == "FALSE")
						{
							temp = "FALSE";
						}
						else if ("FALSE" == temp && eventStack.top().conditionStack.top() == "FALSE")
						{
							temp = "FALSE";
						}
						else if (!eventStack.top().nowUT.empty() && eventStack.top().userType_idx.top() + 1 < eventStack.top().nowUT.top()->GetUserTypeListSize()
							&& (eventStack.top().nowUT.top()->GetUserTypeList(eventStack.top().userType_idx.top() + 1).GetName() == "$else"))
						{
							eventStack.top().conditionStack.push(temp);
						}
						else if ("TRUE" == temp)
						{
							eventStack.top().conditionStack.push(temp);
						}
					}
					else
					{
						if (!eventStack.top().nowUT.empty() && eventStack.top().userType_idx.top() + 1 < eventStack.top().nowUT.top()->GetUserTypeListSize()
							&& (eventStack.top().nowUT.top()->GetUserTypeList(eventStack.top().userType_idx.top() + 1).GetName() == "$else"))
						{
							eventStack.top().conditionStack.push(temp);
						}
						else if (eventStack.top().userType_idx.top() + 1 < events[no].Get(0)->GetUserTypeListSize() &&
							events[no].Get(0)->GetUserTypeList(eventStack.top().userType_idx.top() + 1).GetName() == "$else")
						{
							eventStack.top().conditionStack.push(temp);
						}
						else if ("TRUE" == temp)
						{
							eventStack.top().conditionStack.push(temp);
						}
					}

					if ("TRUE" == temp)
					{
						eventStack.top().nowUT.push(val->GetUserTypeList(1).Get(0));
						val = eventStack.top().nowUT.top()->GetUserTypeList(0).Get(0); // empty chk
						eventStack.top().userType_idx.push(0);
						eventStack.top().state.push(1);
						state = 1;
						//	eventStack.top().if_depth++;
					}
					else if ("FALSE" == temp)
					{
						//if (chkFunc(eventStack, &val)) {

						eventStack.top().userType_idx.top()++;
						break;
						//	}
					}
					else
					{
						// debug..
						cout << "Error Debug : " << temp << endl;
						return "ERROR -3";
					}
				}
				else if ("$else" == val->GetName())
				{
					// if가 바로 앞에 나와야한다. - later
					//
					if (!eventStack.top().conditionStack.empty() && "FALSE" == eventStack.top().conditionStack.top())
					{
						eventStack.top().conditionStack.top() = "TRUE";
						eventStack.top().nowUT.push(val->GetUserTypeList(0).Get(0));
						val = eventStack.top().nowUT.top()->GetUserTypeList(0).Get(0); // empty chk
						eventStack.top().userType_idx.push(0);
						eventStack.top().state.push(2);
						state = 2;
					}
					else
					{
						eventStack.top().userType_idx.top()++;
						break;
					}
				}

				{
					if (chkFunc(eventStack, &val)) {
						eventStack.top().userType_idx.top()++;
						break;
					}
				}
			}
		}
	}

	return module_value;
}


int main(int argc, char* argv[])
{
	srand(time(NULL)); // 

	string fileName;

	if (argc == 1) {
		cout << "FileName: ";
		getline(cin, fileName);
	}
	else
	{
		fileName = string(argv[1]);
	}

	wiz::load_data::UserType global;			// 6 -> 5
	wiz::load_data::LoadData::LoadDataFromFile(fileName, global); // "Onecard_Test/main.txt", global);
	
	cout << "fileName is " << fileName << endl;
	cout << "excute result is " << excute_module(global) << endl;

	return 0;
}

