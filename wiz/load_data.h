﻿
#ifndef LOAD_DATA_H_INCLUDED
#define LOAD_DATA_H_INCLUDED


#include <vector>
#include <stack>
#include <fstream>
#include <string>
#include <utility>
#include <algorithm>
#include <set>
#include <map>
#include <thread>
using namespace std;

#include <wiz/global.h>
//#include <wiz/Dictionary.h> /// change to map

#include <wiz/cpp_string.h>
#include <wiz/queues.h>
#include <wiz/stacks.h>
//#include <wiz/deck.h>

namespace wiz {
	class Token
	{
	public:
		string str;
		bool isComment;
	public:
		Token& operator=(const Token& token) {
			str = token.str;
			isComment = token.isComment;
			return *this;
		}
		void operator=(Token&& token) {
			str = move(token.str);
			isComment = token.isComment;
		}
		Token(Token&& token) : str(move(token.str)), isComment(token.isComment) { }
		Token(const Token& token) : str(token.str), isComment(token.isComment) { }
		explicit Token() : isComment(false) { }
		explicit Token(string&& str, bool isComment = false) : str(move(str)), isComment(isComment) { }
		explicit Token(const string& str, bool isComment = false) : str(str), isComment(isComment) { }
	};
}



#include <wiz/load_data_types.h>
#include <wiz/load_data_utility.h>
#include <wiz/load_data_reservers.h>
#include <wiz/load_data_condition.h>


const string LEFT = "{";
const string RIGHT = "}";
const string EQ_STR = "="; // EQ 충돌 -> EQ_STR로 변경

class EventInfo
{
public:
	wiz::load_data::UserType* eventUT;
	wiz::ArrayStack< wiz::load_data::UserType* > nowUT; //
	wiz::ArrayStack<int> userType_idx;
	map<string, string> parameters;
	map<string, string> locals;
	string id; //
	wiz::ArrayStack<string> conditionStack;
	wiz::ArrayStack<int> state;
	string return_value;
	string option;
public:
	EventInfo() : eventUT(nullptr), return_value("")
	{

	}
};

class ExcuteData
{
public:
	wiz::load_data::UserType* pEvents;
	EventInfo info; // chk!
	bool chkInfo;
	map<string, wiz::load_data::UserType>* pObjectMap;
	map<string, wiz::load_data::UserType>* pModule;

	long long depth;

	bool noUseInput;
	bool noUseOutput;
public:
	explicit ExcuteData()
		: pEvents(nullptr), pObjectMap(nullptr), pModule(nullptr), chkInfo(false), depth(0),
		noUseInput(false), noUseOutput(false)
	{
		//
	}
};

class SortInfo // need to rename
{
public:
	string data;
	int iElement; // 2 : userType, // 1 : item
	size_t idx; // for stable? - chk!!
	wiz::StringBuilder* builder;
public:
	SortInfo(wiz::StringBuilder* builder=nullptr) : idx(-1), builder(builder) { }
	SortInfo(const string& data, int iElement, size_t idx, wiz::StringBuilder* builder)
		: data(data), iElement(iElement), idx(idx), builder(builder)
	{

	}
	// for sorting..
	bool operator<(const SortInfo& info) const
	{
		string temp = wiz::load_data::Utility::Compare(this->data, info.data, builder);

		if (this->data == "") {
			return false;
		}
		if (info.data == "") {
			return true;
		}

		if (temp == "< 0") { return true; }
		else if (temp == "> 0") { return false; }
		else if (temp == "== 0") {
			return idx < info.idx;
		}
		else {
			throw "temp is not valid in sortinfo";
		}
	}
};

class SortInfo2 // need to rename, sortinfo by dsc..
{
public:
	string data;
	int iElement; // 2 : userType, // 1 : item
	size_t idx; // for stable? - chk!!
	wiz::StringBuilder* builder;
public:
	SortInfo2(wiz::StringBuilder* builder=nullptr) : idx(-1), builder(builder) { }
	SortInfo2(const string& data, int iElement, size_t idx, wiz::StringBuilder* builder)
		: data(data), iElement(iElement), idx(idx)
	{

	}
	// for sorting..
	bool operator<(const SortInfo2& info) const
	{
		if (this->data == "" && info.data == "") {
			return false;
		}
		if (this->data == "") {
			return true;
		}

		if (info.data == "") {
			return false;
		}

		string temp = wiz::load_data::Utility::Compare(this->data, info.data, builder);
		if (temp == "< 0") { return false; }
		else if (temp == "> 0") { return true; }
		else if (temp == "== 0") {
			return idx < info.idx;
		}
		else {
			cout << "sortInfo2" << endl;
			cout << data << " " << info.data << endl;
			throw "temp is not valid in sortinfo2";
		}
	}
};

string excute_module(const string& mainStr, wiz::load_data::UserType* _global, const ExcuteData& excuteData, int chk);

namespace wiz {
		
	namespace load_data {
		string ToBool4(wiz::load_data::UserType* now, wiz::load_data::UserType& global, const string& str, const ExcuteData& excuteData, wiz::StringBuilder* builder);
	
		class LoadData
		{
		private:
			static bool isState0(const long long state_reserve)
			{
				return 1 == state_reserve;
			}
			/// core
		public:
			template <class Reserver>
			static bool _LoadData(ArrayQueue<Token>& strVec, Reserver& reserver, UserType& global) // first, strVec.empty() must be true!!
			{
				int state = 0;
				int braceNum = 0;
				long long state_reserve=0;
				vector< UserType* > nestedUT(1);
				string var1, var2, val;

				bool varOn = false;

				nestedUT[0] = &global;
				{ 
					reserver(strVec);

					while (strVec.empty())
					{
						reserver(strVec);
						if (
							strVec.empty() &&
							reserver.end()
							) {
							return false; // throw "Err nextToken does not exist"; // cf) or empty file or empty string!
						}
					}
				}


				while (false == strVec.empty()) {
					switch (state)
					{
					case 0:
						if (LEFT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							state = 2;
						}
						else {
 							pair<bool, Token> bsPair = Utility::LookUp(strVec, nestedUT[braceNum], reserver);
							if (bsPair.first) {
								if (EQ_STR == bsPair.second.str) {
									Utility::Pop(strVec, &var2, nestedUT[braceNum], reserver);
									Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);
									state = 2;
								}
								else {
									if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver)) {
										nestedUT[braceNum]->AddItem("", move(var1));
										state = 0;
									}
								}
							}
							else {
								if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver)) {
									nestedUT[braceNum]->AddItem("", move(var1));
									state = 0;
								}
							}
						}
						break;
					case 1:
						if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);
							state = 0;
						}
						else {
							// syntax error.
							throw "syntax error 1 ";
						}
						break;
					case 2:
						if (LEFT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

							///
							nestedUT[braceNum]->AddUserTypeItem(UserType(var2));
							UserType* pTemp;
							nestedUT[braceNum]->GetLastUserTypeItemRef(var2, pTemp);

							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) /// changed 2014.01.23..
								nestedUT.push_back(nullptr);

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
							///
							state = 3;
						}
						else {
							if (Utility::Pop(strVec, &val, nestedUT[braceNum], reserver)) {

								nestedUT[braceNum]->AddItem(move(var2), move(val));
								var2 = "";
								val = "";

								state = 0;
							}
						}
						break;
					case 3:
						if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

							nestedUT[braceNum] = nullptr;
							braceNum--;

							state = 0;
						}
						else {
							{
								/// uisng struct
								state_reserve++;
								state = 4;
							}
							//else
							{
								//	throw  "syntax error 2 ";
							}
						}
						break;
					case 4:
						if (LEFT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

							UserType temp("");

							nestedUT[braceNum]->AddUserTypeItem(temp);
							UserType* pTemp;
							nestedUT[braceNum]->GetLastUserTypeItemRef("", pTemp);

							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) /// changed 2014.01.23..
								nestedUT.push_back(nullptr);

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
							///
							//}

							state = 5;
						}
						else if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);
							state = isState0(state_reserve) ? 0 : 4;
							state_reserve--;

							{
								nestedUT[braceNum] = nullptr;
								braceNum--;
							}
						}
						else {
							pair<bool, Token> bsPair = Utility::LookUp(strVec, nestedUT[braceNum], reserver);
							if (bsPair.first) {
								if (EQ_STR == bsPair.second.str) {
									// var2
									Utility::Pop(strVec, &var2, nestedUT[braceNum], reserver);
									Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver); // pass EQ_STR
									state = 6;
								}
								else {
									// var1
									if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver)) {
										nestedUT[braceNum]->AddItem("", move(var1));
										var1 = "";

										state = 4;
									}
								}
							}
							else
							{
								// var1
								if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver))
								{
									nestedUT[braceNum]->AddItem("", move(var1));
									var1 = "";

									state = 4;
								}
							}
						}
						break;
					case 5:
						if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

							//if (flag1 == 0) {
							nestedUT[braceNum] = nullptr;
							braceNum--;
							// }
							//
							state = 4;
						}

						else {
							int idx = -1;
							int num = -1;

							
							{
								/// uisng struct
								state_reserve++;
								state = 4;
							}
							//else
							{
								//	throw "syntax error 4  ";
							}
						}
						break;
					case 6:
						if (LEFT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

							///
							{
								nestedUT[braceNum]->AddUserTypeItem(UserType(var2));
								UserType* pTemp;
								nestedUT[braceNum]->GetLastUserTypeItemRef(var2, pTemp);
								var2 = "";
								braceNum++;

								/// new nestedUT
								if (nestedUT.size() == braceNum) /// changed 2014.01.23..
									nestedUT.push_back(nullptr);

								/// initial new nestedUT.
								nestedUT[braceNum] = pTemp;
							}
							///
							state = 7;
						}
						else {
							if (Utility::Pop(strVec, &val, nestedUT[braceNum], reserver)) {

								nestedUT[braceNum]->AddItem(move(var2), move(val));
								var2 = ""; val = "";
								if (strVec.empty())
								{
									//
								}
								else if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
									Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);

									{
										state = isState0(state_reserve) ? 0 : 4;
										state_reserve--;

										{
											nestedUT[braceNum] = nullptr;
											braceNum--;
										}
									}
									{
										//state = 4;
									}
								}
								else {
									state = 4;
								}
							}
						}
						break;
					case 7:
						if (RIGHT == Utility::Top(strVec, nestedUT[braceNum], reserver)) {
							Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver);
							//

							nestedUT[braceNum] = nullptr;
							braceNum--;
							//
							state = 4;
						}
						else {
							int idx = -1;
							int num = -1;
							
							{
								/// uisng struct
								state_reserve++;

								state = 4;
							}
							//else
							{
								//throw "syntax error 5 ";
							}
						}
						break;
					default:
						// syntax err!!

						throw "syntax error 6 ";
						break;
					}

					if (strVec.size() < 10) {
						reserver(strVec);

						while (strVec.empty()) // (strVec.empty())
						{
							reserver(strVec);
							if (
								strVec.empty() &&
								reserver.end()
								) {
								// throw "Err nextToken does not exist2";
								break;
							}
						}
					}
				}
				if (state != 0) {
					throw string("error final state is not 0!  : ") + toStr(state);
				}
				if (braceNum != 0) {
					throw string("chk braceNum is ") + toStr(braceNum);
				}

				return true;
			}

		public:

			static bool LoadDataFromFile(const string& fileName, UserType& global) /// global should be empty
			{
				ifstream inFile;
				inFile.open(fileName);
				if (true == inFile.fail())
				{
					inFile.close(); return false;
				}
				UserType globalTemp = global;
				ArrayQueue<Token> strVec;

				try {
					InFileReserver ifReserver(inFile);

					ifReserver.Num = 100000;
					// cf) empty file..
					if (false == _LoadData(strVec, ifReserver, globalTemp))
					{
						inFile.close();
						return false; // return true?
					}

					inFile.close();
				}
				catch (Error e) { std::cout << e << endl; inFile.close(); return false; }
				catch (const char* err) { std::cout << err << endl; inFile.close(); return false; }
				catch (const string& e) { std::cout << e << endl; inFile.close(); return false; }
				catch (exception e) { std::cout << e.what() << endl; inFile.close(); return false; }
				catch (...) { std::cout << "not expected error" << endl; inFile.close(); return false; }

				global = move(globalTemp);
				return true;
			}

		private:
			class LoadData2 {
			public:
				void operator()(StringBuilder* strVec, NoneReserver* reserver, UserType* global)
				{
					ArrayQueue<Token> aq;

					wiz::load_data::Utility::DoThread test(strVec, &aq);

					test();

					_LoadData(aq, *reserver, *global);
				}
			};
		public:
			static bool LoadDataFromFile2(const string& fileName, UserType& global) /// global should be empty
			{
				ifstream inFile;
				inFile.open(fileName);
				if (true == inFile.fail())
				{
					inFile.close(); return false;
				}
				UserType globalTemp = global;
				vector<StringBuilder> strVec(1, StringBuilder(128));
				int count = 0;

				try {

					while (wiz::load_data::Utility::Reserve3(inFile, strVec[count], 100000))
					{
						count++;
						strVec.push_back(StringBuilder(128));
					}
					strVec.pop_back();
					inFile.close();

					vector<NoneReserver> noneReserver(strVec.size());
					vector<UserType> ut(strVec.size());
					vector<std::thread> test(strVec.size());

					for (int i = 0; i < strVec.size(); ++i) {
						test[i] = thread(LoadData2(), &strVec[i], &noneReserver[i], &ut[i]);
					}

					for (int i = 0; i < strVec.size(); ++i) {
						test[i].join();
					}

					getchar();

					for (int i = 0; i < ut.size(); ++i) {
						for (int j = 0; j < ut[i].GetCommentListSize(); ++j) {
							globalTemp.PushComment(std::move(ut[i].GetCommentList(j)));
						}
						int itemCount = 0;
						int userTypeCount = 0;

						for (int j = 0; j < ut[i].GetIListSize(); ++j) {
							if (ut[i].IsItemList(j)) {
								ItemType<string> temp = ut[i].GetItemList(itemCount);
								globalTemp.AddItem(temp.GetName(), temp.Get(0));
								itemCount++;
							}
							else {
								globalTemp.AddUserTypeItem(*ut[i].GetUserTypeList(userTypeCount));
								userTypeCount++;
							}
						}
					}
					getchar();
				}
				catch (Error e) { std::cout << e << endl; if(inFile.is_open()) inFile.close(); return false; }
				catch (const char* err) { std::cout << err << endl; if (inFile.is_open())inFile.close(); return false; }
				catch (const string& e) { std::cout << e << endl; if (inFile.is_open())inFile.close(); return false; }
				catch (exception e) { std::cout << e.what() << endl; if (inFile.is_open())inFile.close(); return false; }
				catch (...) { std::cout << "not expected error" << endl; if (inFile.is_open())inFile.close(); return false; }

				global = std::move(globalTemp);
				return true;
			}

			static bool LoadDataFromString(const string& str, UserType& ut)
			{
				UserType utTemp = ut; 
				ArrayQueue<Token> strVec;

				wiz::StringBuilder builder(str.size(), str.c_str(), str.size());
				wiz::load_data::Utility::DoThread doThread(&builder, &strVec);

				doThread();

				try {
					// empty string!
					NoneReserver nonReserver;
					if (false == _LoadData(strVec, nonReserver, utTemp))
					{
						return true;
					}
				}
				catch (Error& e) { std::cout << e << endl; return false; }
				catch (const char* err) { std::cout << err << endl; return false; }
				catch (exception& e) { std::cout << e.what() << endl; return false; }
				catch (string str) { std::cout << str << endl; return false; }
				catch (...) { std::cout << "not expected error" << endl; return  false; }

				ut = std::move(utTemp);
				return true;
			}

		private:
			UserType global; // ToDo - remove!
		public:
			/// To Do.. static function -> ~.function.!
			// InitQuery or LoadQuery
			explicit LoadData() { InitWizDB(); }

			/// need testing!
			LoadData(const LoadData& ld)
				: global(ld.global)
			{
				//global = ld.global;
			}
			virtual ~LoadData() { AllRemoveWizDB(); }

			/// need testing!
			LoadData& operator=(const LoadData& ld)
			{
				if (this == &ld) { return *this; }

				global = ld.global;
				return *this;
			}
			//
			bool InitWizDB() {
				return InitWizDB(global);
			}
			// allRemove Query 
			bool AllRemoveWizDB() {
				return AllRemoveWizDB(global);
			}
			// AddQuery AddData, AddUserTypeData
			bool AddData(const string& position, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				return AddData(global, position, data, condition, excuteData, builder);
			}
			// 
			bool AddNoNameUserType(const string& position, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return AddNoNameUserType(global, position, data, condition, excuteData, builder);
			}
			// SetQuery
			bool SetData(const string& position, const string& varName, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return SetData(global, position, varName, data, condition, excuteData, builder);
			}
			/// 
			string GetData(const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)  {
				return GetData(global, position, condition, excuteData, builder);
			}
			string GetItemListData(const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return GetItemListData(global, position, condition, excuteData, builder);
			}
			string GetItemListNamesData(const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return GetItemListNamesData(global, position, condition, excuteData, builder);
			}
			string GetUserTypeListNamesData(const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return GetUserTypeListNamesData(global, position, condition, excuteData, builder);
			}
			/// varName = val - do
			/// varName = { val val val } - GetData(position+"/varName", ""); 
			/// varName = { var = val } - GetData(position+"/varname", var);
			string GetData(const string& position, const string& varName, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				return GetData(global, position, varName, condition, excuteData, builder);
			}
			bool Remove(const string& position, const string& var, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				return Remove(global, position, var, condition, excuteData, builder);
			}

			bool LoadWizDB(const string& fileName) {
				return LoadWizDB(global, fileName);
			}
			// SaveQuery
			bool SaveWizDB(const string& fileName, const string& option = "0") { /// , int option
				return SaveWizDB(global, fileName, option);
			}

			/// To Do - ExistItem, ExistUserType, SetUserType GetUserType
			bool ExistData(const string& position, const string& varName, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				return ExistData(global, position, varName, condition, excuteData, builder);
			}

			/// ToDo - recursive function
			string SearchItem(const string& var, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return SearchItem(global, var, condition, excuteData, "root", builder);
			}
			string SearchUserType(const string& var, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				return SearchUserType(global, var, condition, excuteData, builder);
			}
		private:
			void SearchItem(vector<string>& positionVec, const string& var, const string& nowPosition,
				UserType* ut, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				SearchItem(global, positionVec, var, nowPosition, ut, condition, excuteData, builder);
			}
			void SearchUserType(vector<string>& positionVec, const string& var, const string& nowPosition,
				UserType* ut, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				SearchUserType(global, positionVec, var, nowPosition, ut, condition, excuteData, builder);
			}
		private:
			// chk - Search(item or usertype) : add ~~~ (option?)?? and ToBool4? // chk more thinking!!
			static void SearchItem(UserType& global, vector<string>& positionVec, const string& var, const string& nowPosition,
				UserType* ut, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				string _var = var;
				if (_var == " " || _var == "_") { _var = ""; }

				// $it?

				if (ut->GetItem(_var).size() > 0) {
					string _condition = condition;

					if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
					else
						_condition = wiz::String::replace(_condition, "~~", _var); //
					
					_condition = wiz::String::replace(_condition, "////", nowPosition);
					_condition = ToBool4(ut, global, _condition, excuteData, builder);
					
					Condition cond(_condition, ut, &global, builder);


					while (cond.Next());

					if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
					{
						positionVec.push_back(nowPosition);
					}
				}

				for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
					string temp = ut->GetUserTypeList(i)->GetName();
					if (temp == "") { temp = " "; }
					SearchItem(
						global,
						positionVec,
						_var,
						nowPosition + "/" + temp,
						ut->GetUserTypeList(i),
						condition,
						excuteData,
						builder
					);
				}
			}

			static void SearchUserType(UserType& global, vector<string>& positionVec, const string& var, const string& nowPosition,
				UserType* ut, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				string _var = var;
				if (_var == " " || _var == "_") {
					_var = "";
				}
				if (ut->GetUserTypeItem(_var).size() > 0) {
					string _condition = condition;

					if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
					else  _condition = wiz::String::replace(_condition, "~~", _var); //


					_condition = wiz::String::replace(_condition, "////", nowPosition);
					_condition = ToBool4(ut, global, _condition, excuteData, builder);

					Condition cond(_condition, ut, &global, builder);

					while (cond.Next());

					if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
					{
						positionVec.push_back(nowPosition);
					}
				}

				for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
					string temp = ut->GetUserTypeList(i)->GetName();

					if (temp == "") { temp = " "; }
					SearchUserType(
						global,
						positionVec,
						_var,
						nowPosition + "/" + temp,
						ut->GetUserTypeList(i),
						condition,
						excuteData,
						builder
					);
				}
			}

		public:
			static bool InitWizDB(UserType& global) {
				global = UserType("global");
				return true;
			}
			// allRemove Query 
			static bool AllRemoveWizDB(UserType& global) {
				global = UserType("");
				return true;
			}
			// AddQuery AddData, AddUserTypeData
			static bool AddDataAtFront(UserType& global, const string& position, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				UserType utTemp = UserType("global");
				bool isTrue = false;

				if (false == LoadDataFromString(data, utTemp))
				{
					return false;
				}
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						int item_n = utTemp.GetItemListSize();
						int user_n = utTemp.GetUserTypeListSize();

						/// chk temp test codes - > using flag 1->Exist 2->Comparision
						//if (finded.second[i]->GetItem("base_tax").GetCount() > 0) { continue; }
						///~end
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0]) // || cond.Now().size()  != 1
							{
								//std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						for (int k = utTemp.GetIListSize() - 1; k >= 0; --k) {
							if (utTemp.IsItemList(k)) {
								finded.second[i]->AddItemAtFront(utTemp.GetItemList(item_n-1).GetName(), utTemp.GetItemList(item_n-1).Get(0));
								item_n--;
							}
							else {
								finded.second[i]->AddUserTypeItemAtFront(*utTemp.GetUserTypeList(user_n-1));
								user_n--;
							}
						}
						isTrue = true;
					}
					return isTrue;
				}
				else {UserType::Find(&global, position, builder);
					return false;
				}
			}
			static bool AddData(UserType& global, const string& position, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				UserType utTemp = UserType("global");
				bool isTrue = false;

				if (false == LoadDataFromString(data, utTemp))
				{
					return false;
				}
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						int item_n = 0;
						int user_n = 0;

						if (false == condition.empty()) {
							string _condition = condition;
							
							_condition = ToBool4(finded.second[i], global,  _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0]) // || cond.Now().size()  != 1
							{
								//std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						for (int k = 0; k < utTemp.GetIListSize(); ++k) {
							if (utTemp.IsItemList(k)) {
								finded.second[i]->AddItem(utTemp.GetItemList(item_n).GetName(), utTemp.GetItemList(item_n).Get(0));
								item_n++;
							}
							else {
								finded.second[i]->AddUserTypeItem(*utTemp.GetUserTypeList(user_n));
								user_n++;
							}
						}
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			static bool Insert(UserType& global, const string& position, const int idx, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				UserType utTemp = UserType("global");
				bool isTrue = false;

				if (false == LoadDataFromString(data, utTemp))
				{
					return false;
				}
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						int item_n = utTemp.GetIListSize();
						int user_n = utTemp.GetIListSize();

						/// chk temp test codes - > using flag 1->Exist 2->Comparision
						//if (finded.second[i]->GetItem("base_tax").GetCount() > 0) { continue; }
						///~end
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);
							
							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0]) // || cond.Now().size()  != 1
							{
								//std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						for (int k = utTemp.GetIListSize() - 1; k >= 0; --k) {
							if (utTemp.IsItemList(k)) {
								finded.second[i]->InsertItemByIlist(idx, utTemp.GetItemList(item_n - 1).GetName(),
									utTemp.GetItemList(item_n - 1).Get(0));
								item_n--;
							}
							else {
								finded.second[i]->InsertUserTypeByIlist(idx, *utTemp.GetUserTypeList(user_n - 1));
								user_n--;
							}
						}
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			static bool AddNoNameUserType(UserType& global, const string& position, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				UserType utTemp = UserType("");
				bool isTrue = false;

				if (false == LoadDataFromString(data, utTemp))
				{
					return false;
				}
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						int item_n = 0;
						int user_n = 0;

						/// chk temp test codes - > using flag 1->Exist 2->Comparision
						//if (finded.second[i]->GetItem("base_tax").GetCount() > 0) { continue; }
						///~end
						if (false == condition.empty()) {
							string _condition = condition;
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						finded.second[i]->AddUserTypeItem(utTemp);

						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}

			// todo - find example code?  a/b/c/d/e/f/ ??
			static bool AddUserType(UserType& global, const string& position, const string& var, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				bool isTrue = false;
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					string temp = var;
					if (temp == "") { temp = " "; }
					StringTokenizer tokenizer(temp, "/", builder, 1);
					UserType utTemp = UserType("");
					if (false == LoadDataFromString(data, utTemp))
					{
						return false;
					}

					while (tokenizer.hasMoreTokens()) {
						string utName = tokenizer.nextToken();
						vector<string> strVec;
						if (utName == " " || utName == "_") { utName = ""; }

						if (utName.size() >= 3 && utName[0] == '[' && utName[utName.size() - 1] == ']')
						{
							StringTokenizer tokenizer2(utName, vector<string>{ "[", "~", "]" }, builder, 1);
							while (tokenizer2.hasMoreTokens())
							{
								strVec.push_back(tokenizer2.nextToken());
							}
						}

						long long int a = 0, b = 0, Min = 0, Max = 0;
						bool chkInt = false;

						if (strVec.size() == 2 && Utility::IsInteger(strVec[0]) && Utility::IsInteger(strVec[1])) {
							chkInt = true;
							a = atoll(strVec[0].c_str());
							b = atoll(strVec[1].c_str());
							Min = min(a, b);
							Max = max(a, b);
						}

						for (auto x = Min; x <= Max; ++x)
						{
							if (strVec.size() == 2 && chkInt)
							{
								utName = std::to_string(x);
							}
							else {}
							utTemp.SetName(utName);

							for (int i = 0; i < finded.second.size(); ++i) {
								int item_n = 0;
								int user_n = 0;


								if (false == condition.empty()) {
									string _condition = condition;

									if (utName == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }// do not use ^ in condition!
									else
										_condition = wiz::String::replace(_condition, "~~", utName); //

									_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);
																									 //	cout << "condition is " << _condition << endl;

									Condition cond(_condition, finded.second[i], &global, builder);

									while (cond.Next());

									if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
									{
										//std::cout << cond.Now()[0] << endl;
										continue;
									}
								}

								finded.second[i]->AddUserTypeItem(utTemp);

								isTrue = true; // chk!!
							}

							// prevent from infinity loop.
							if (x == Max) { break; }
						}
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			/// SetData - Re Do!
			static bool SetData(UserType& global, const string& position, const string& varName, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					string temp = varName;
					if (temp == "") { temp = " "; }
					StringTokenizer tokenizer(temp, "/", builder, 1);
					UserType utTemp("");
					if (false == LoadDataFromString(data, utTemp)){
						return false;
					}
					while (tokenizer.hasMoreTokens()) {
						string _varName = tokenizer.nextToken();
						/// todo - if varName is "" then data : val val val ... 
						if (_varName == "" || _varName == " " || _varName == "_" ) { // re?
							const int n = utTemp.GetItem("").size();
							for (int i = 0; i < finded.second.size(); ++i) {
								if (false == condition.empty()) {
									string _condition = condition;
									if (_varName == "" || _varName == " " || _varName == "_" ) { _condition = wiz::String::replace(_condition, "~~", "^"); }
									else
										_condition = wiz::String::replace(_condition, "~~", _varName); //
									
									_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

									Condition cond(_condition, finded.second[i], &global, builder);

									while (cond.Next());

									if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
									{
										//	std::cout << cond.Now()[0] << endl;
										continue;
									}
								}
								finded.second[i]->RemoveItemList("");

								for (int j = 0; j < n; ++j) {
									finded.second[i]->AddItem("", utTemp.GetItem("")[j].Get(0));
								}
								isTrue = true;
							}
						}
						else {
							vector<string> strVec;

							if (_varName.size() >= 3 && _varName[0] == '[' && _varName[_varName.size() - 1] == ']')
							{
								StringTokenizer tokenizer2(_varName, vector<string>{ "[", "~", "]" }, builder, 1);
								while (tokenizer2.hasMoreTokens())
								{
									strVec.push_back(tokenizer2.nextToken());
								}
							}

							long long int a = 0, b = 0, Min = 0, Max = 0;
							bool chkInt = false;

							if (strVec.size() == 2 && Utility::IsInteger(strVec[0]) && Utility::IsInteger(strVec[1])) {
								chkInt = true;
								a = atoll(strVec[0].c_str());
								b = atoll(strVec[1].c_str());
								Min = min(a, b);
								Max = max(a, b);
							}
							for (long long x = Min; x <= Max; ++x) {
								if (strVec.size() == 2 && chkInt)
								{
									_varName = std::to_string(x);
								}
								else {}

								for (int i = 0; i < finded.second.size(); ++i) {
									if (false == condition.empty()) {
										string _condition = condition;
										if (_varName == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
										else
											_condition = wiz::String::replace(_condition, "~~", _varName); //

										_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

										Condition cond(_condition, finded.second[i], &global, builder);

										while (cond.Next());

										if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
										{
											//	std::cout << cond.Now()[0] << endl;
											continue;
										}
									}
									if (wiz::String::startsWith(_varName, "$it")) {
										int index = atoi(wiz::String::substring(_varName, 3).c_str());

										finded.second[i]->SetItem(index, data);
										isTrue = true;
									}
									else {
										finded.second[i]->SetItem(_varName, data); /// chk
										isTrue = true;
									}
								}

								// prevent from infinity loop.
								if (x == Max) { break; }
							}
						}
					}
					return isTrue;
				}
				else {
					return false;
				}
			}

			static bool SetData(UserType& global, const string& position, const int var_idx, const string& data, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					UserType utTemp("");
					if (false == LoadDataFromString(data, utTemp)) {
						return false;
					}
					long long int a = 0, b = 0, Min = 0, Max = 0;

					for (long long x = Min; x <= Max; ++x) {
						for (int i = 0; i < finded.second.size(); ++i) {
							if (false == condition.empty()) {
								string _condition = condition;


								_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

								Condition cond(_condition, finded.second[i], &global, builder);

								while (cond.Next());

								if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
								{
									//	std::cout << cond.Now()[0] << endl;
									continue;
								}
							}
							finded.second[i]->SetItem(var_idx, data); /// chk
							isTrue = true;
						}

						// prevent from infinity loop.
						if (x == Max) { break; }
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			/// 
			static string GetData(UserType& global, const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				string str;
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						str = str + finded.second[i]->ToString() + "\n";
					}
					return str;
				}
				else {
					return "";
				}
			}
			static string GetItemListData(UserType& global, const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				string str;
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;
							
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						str = str + finded.second[i]->ItemListToString() + "\n";
					}
					return str;
				}
				else {
					return "";
				}
			}
			static string GetItemListNamesData(UserType& global, const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				string str;
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						str = str + finded.second[i]->ItemListNamesToString() + "\n";
					}
					return str;
				}
				else {
					return "";
				}
			}
			static string GetUserTypeListNamesData(UserType& global, const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				string str;
				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						str = str + finded.second[i]->UserTypeListNamesToString() + "\n";
					}
					return str;
				}
				else {
					return "";
				}
			}
			/// varName = val - do
			/// varName = { val val val } - GetData(position+"/varName", ""); 
			/// varName = { var = val } - GetData(position+"/varname", var);
			static string GetData(UserType& global, const string& position, const string& varName, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				string str;
				string _var = varName;
				if (_var == " " || _var == "_") { _var = ""; }

				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							// ~~ and ^ -> do not used other mean?
							if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
							else
								_condition = wiz::String::replace(_condition, "~~", _var); /// varName -> _var.
							
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						if (wiz::String::startsWith(_var, "$it")) {
							int itemIdx = stoi(wiz::String::substring(_var, 3));
							
							str = str + finded.second[i]->GetItemList(itemIdx).Get(0) + "\n";
						}
						else {
							const int num = finded.second[i]->GetItem(_var).size();
							for (int k = 0; k < num; ++k) {
								str = str + finded.second[i]->GetItem(_var)[k].Get(0) + "\n";
							}
						}
					}
				}
				return str;
			}

			static bool Remove(UserType& global, const string& position, const string& var, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					string temp = var;
					if (temp == "") { temp = " "; }
					StringTokenizer tokenizer(temp, "/", builder, 1);
					while (tokenizer.hasMoreTokens()) {
						string _var = tokenizer.nextToken();
						if (_var == " " || _var == "_") { _var = ""; }
						vector<string> strVec;

						if (_var.size() >= 3 && _var[0] == '[' && _var[_var.size() - 1] == ']')
						{
							StringTokenizer tokenizer2(_var, vector<string>{ "[", "~", "]" }, builder, 1);
							while (tokenizer2.hasMoreTokens())
							{
								strVec.push_back(tokenizer2.nextToken());
							}
						}

						long long int a = 0, b = 0, Min = 0, Max = 0;
						bool chkInt = false;

						if (strVec.size() == 2 && Utility::IsInteger(strVec[0]) && Utility::IsInteger(strVec[1])) {
							chkInt = true;
							a = atoll(strVec[0].c_str());
							b = atoll(strVec[1].c_str());
							Min = min(a, b);
							Max = max(a, b);
						}
						for (long long x = Min; x <= Max; ++x) {
							if (strVec.size() == 2 && chkInt)
							{
								_var = std::to_string(x);
							}
							else {}

							for (int i = 0; i < finded.second.size(); ++i) {
								UserType* temp = finded.second[i];

								if (false == condition.empty()) {
									string _condition = condition;
									if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
									else
										_condition = wiz::String::replace(_condition, "~~", _var); //
									
									_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

									Condition cond(_condition, finded.second[i], &global, builder);
									while (cond.Next());

									if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
									{
										// std::cout << cond.Now()[0] << endl;
										continue;
									}
								}

								temp->RemoveItemList(_var);
								temp->RemoveUserTypeList(_var);
								isTrue = true;
							}

							// prevent from infinity loop.
							if (x == Max) { break; }
						}
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			static bool Remove(UserType& global, const string& position, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						UserType* temp = finded.second[i];

						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);
							
							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								// std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						temp->Remove();
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			static bool RemoveUserType(UserType& global, const string& position, const string& name, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						UserType* temp = finded.second[i];

						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								// std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						temp->RemoveUserTypeList(name);
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}


			static bool RemoveItemType(UserType& global, const string& position, const string& name, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						UserType* temp = finded.second[i];

						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								// std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						temp->RemoveItemList(name);
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}

			// todo - static bool Remove(UserType& global, const string& positiion, oonst int idx, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder)
			static bool Remove(UserType& global, const string& position, const int idx, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) {
				auto finded = UserType::Find(&global, position, builder);
				bool isTrue = false;

				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						UserType* temp = finded.second[i];

						if (false == condition.empty()) {
							string _condition = condition;
							
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								// std::cout << cond.Now()[0] << endl;
								continue;
							}
						}

						temp->RemoveList(idx);
						isTrue = true;
					}
					return isTrue;
				}
				else {
					return false;
				}
			}
			
			static bool LoadWizDB(UserType& global, const string& fileName) {
				UserType globalTemp = UserType("global");

				// Scan + Parse 
				if (false == LoadDataFromFile(fileName, globalTemp)) { return false; }
				std::cout << "LoadData End" << endl;

				global = move(globalTemp);
				return true;
			}
			// SaveQuery
			static bool SaveWizDB(const UserType& global, const string& fileName, const string& option = "0", const string& option2 = "") { /// , int option
				ofstream outFile;
				if (fileName.empty()) { return false; }
				if (option2 == "") {
					outFile.open(fileName);
					if (outFile.fail()) { return false; }
				}
				else {
					outFile.open(fileName, ios::app);
					if (outFile.fail()) { return false; }

					outFile << "\n";
				}

				/// saveFile
				if (option == "1") // for eu4.
					global.Save1(outFile); // cf) friend
				else if (option == "2")
					global.Save2(outFile);

				outFile.close();

				return true;
			}

			/// To Do - ExistItem, ExistUserType, SetUserType GetUserType
			static bool ExistData(UserType& global, const string& position, const string& varName, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				int count = 0;
				string _var = varName;
				if (_var == " " || _var == "_") { _var = ""; }

				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						if (wiz::String::startsWith(_var, "$it")) {
							int itemIdx = stoi(wiz::String::substring(_var, 3));
							count = count + (finded.second[i]->GetItemList(itemIdx).size());
						}
						else {
							count = count + (finded.second[i]->GetItem(_var).size());
						}
					}
				}
				return 0 != count;
			}
			static bool ExistUserType(UserType& global, const string& position, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				int count = 0;

				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;
							
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						count = count + (finded.second[i]->GetUserTypeListSize());
					}
				}
				return 0 != count;
			}
			static bool ExistOneUserType(UserType& global, const string& position, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				int count = 0;

				auto finded = UserType::Find(&global, position, builder);
				if (finded.second[0] == &global) {
					return true;
				}
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;
							
							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);
							
							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						count = count + (finded.second[i]->GetUserTypeListSize());
					}
				}
				return 1 == count;
			}
			static bool ExistItem(UserType& global, const string& position, const string& varName, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder) // 
			{
				int count = 0;
				string _var = varName;
				if (_var == " " || _var == "_") { _var = ""; }

				auto finded = UserType::Find(&global, position, builder);
				if (finded.first) {
					for (int i = 0; i < finded.second.size(); ++i) {
						if (false == condition.empty()) {
							string _condition = condition;

							_condition = ToBool4(finded.second[i], global, _condition, excuteData, builder);

							Condition cond(_condition, finded.second[i], &global, builder);

							while (cond.Next());

							if (cond.Now().size() != 1 || "TRUE" != cond.Now()[0])
							{
								//	std::cout << cond.Now()[0] << endl;
								continue;
							}
						}
						if (wiz::String::startsWith(_var, "$it")) {
							int itemIdx = stoi(wiz::String::substring(_var, 3));
							count = count + (finded.second[i]->GetItemList(itemIdx).size());
						}
						else {
							count = count + (finded.second[i]->GetItem(_var).size());
						}
					}
				}
				return 0 != count;
			}

			/// ToDo - global, position, var, condition + var is " "!
			// "root" -> position.
			static string SearchItem(UserType& global, const string& var, const string& condition, 
				const ExcuteData& excuteData, 
				const string& start_dir = "root",
				StringBuilder* builder = nullptr)
			{
				vector<string> positionVec;
				string temp;

				SearchItem(global, positionVec, var, start_dir, &global, condition, excuteData, builder);

				for (int i = 0; i < positionVec.size(); ++i)
				{
					temp = temp + positionVec[i] + "\n";
				}

				return temp;
			}
			static string SearchUserType(UserType& global, const string& var, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder)
			{
				vector<string> positionVec;
				string temp;

				SearchUserType(global, positionVec, var, "root", &global, condition, excuteData, builder);

				for (int i = 0; i < positionVec.size(); ++i)
				{
					temp = temp + positionVec[i] + "\n";
				}

				return temp;
			}
			static void ReplaceItem(UserType& global, const string& var, const string& val, const string& condition, const string& start_dir,
				const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						ReplaceItem(global, var, start_dir, ut, val, condition, excuteData, builder, recursive);
					}
				}
			}
			static void RemoveUserTypeTotal(UserType& global, const string& ut_name, const string& condition, const string& start_dir,
				const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						RemoveUserTypeTotal(global, ut_name, start_dir, ut, condition, excuteData, builder, recursive);
					}
				}
			}
			static void ReplaceDateType(UserType& global, const string& val, const string& condition, const string& start_dir,
				const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder
				) {
				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						ReplaceDateType(global, start_dir, ut, val, condition, excuteData, builder, recursive);
					}
				}
			}
			static void ReplaceDateType2(UserType& global, const string& val, const string& condition, const string& start_dir,
				const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						ReplaceDateType2(global, start_dir, ut, val, condition, excuteData, builder, recursive);
					}
				}
			}
			static void ReplaceDataType1(UserType& global, const string& rex, const vector<string>& val, const vector<string>& condition,
				const string& start_dir, const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				std::regex rgx(rex);
				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						ReplaceDataType1(global, start_dir, ut, rgx, val, condition, excuteData, builder, recursive);
					}
				}
			}
			static void ReplaceDataType1_2(UserType& global, const string& rex, const vector<string>& val, const vector<string>& condition,
				const string& start_dir, const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				std::regex rgx(rex);

				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];

						ReplaceDataType1_2(global, start_dir, ut, rgx, val, condition, excuteData, builder, recursive);
					}
				}
			}
			static void ReplaceDataType2(UserType& global, const string& rex, const vector<string>& val, const vector<string>& condition,
				const string& start_dir, const ExcuteData& excuteData, bool recursive, wiz::StringBuilder* builder)
			{
				std::regex rgx(rex);

				auto temp = wiz::load_data::UserType::Find(&global, start_dir, builder);
				if (temp.first) {
					for (int i = 0; i < temp.second.size(); ++i) // chk!!
					{
						UserType* ut = temp.second[i];
						ReplaceDataType2(global, start_dir, ut, rgx, val, condition, excuteData, builder, recursive);
					}
				}
			}
		private:
			static void ReplaceItem(UserType& global, const string& var, const string& nowPosition,
				UserType* ut, const string& val, const string& condition, const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				string _var = var;
				if (_var == " " || _var == "_") { _var = ""; }

				for (int i = 0; i < ut->GetItemListSize(); ++i) {
					if (ut->GetItemList(i).GetName() == _var) {
						string _condition = condition;

						if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
						else {
							_condition = wiz::String::replace(_condition, "~~~", val); //
							_condition = wiz::String::replace(_condition, "~~", _var); //
						}
						_condition = wiz::String::replace(_condition, "////", nowPosition);
						_condition = wiz::String::replace(_condition, "///", wiz::_toString(i));
						_condition = ToBool4(ut, global, _condition, excuteData, builder);

						Condition cond(_condition, ut, &global, builder);

						while (cond.Next());

						if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
						{
							string _val = val;
							_val = wiz::String::replace(_val, "~~~", val); //
							_val = wiz::String::replace(_val, "~~", _var); //
	
							_val = wiz::String::replace(_val, "////", nowPosition);
							_val = wiz::String::replace(_val, "///", wiz::_toString(i));
							_val = ToBool4(ut, global, _val, excuteData, builder);

							ut->GetItemList(i).Set(0, _val);
						}
					}
				}

				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }
						ReplaceItem(
							global,
							_var,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void RemoveUserTypeTotal(UserType& global, const string& ut_name, const string& nowPosition,
				UserType* ut, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				string _var = ut_name;
				
				if (_var == " " || _var == "_") { _var = ""; }

				for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
					if (ut->GetUserTypeList(i)->GetName() == _var) {
						string _condition = condition;

						if (_var == "") { _condition = wiz::String::replace(_condition, "~~", "^"); }
						else {
							_condition = wiz::String::replace(_condition, "~~", _var); //
						}

						_condition = wiz::String::replace(_condition, "////", nowPosition);

						_condition = ToBool4(ut, global, _condition, excuteData, builder);

						Condition cond(_condition, ut, &global, builder);

						while (cond.Next());

						if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
						{
							ut->RemoveUserTypeList(i);
							--i;
						}
					}
				}
				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }
						RemoveUserTypeTotal(
							global,
							_var,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void ReplaceDateType(UserType& global, const string& nowPosition,
				UserType* ut, const string& val, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				for (int i = 0; i < ut->GetItemListSize(); ++i) {
					if (Utility::IsDate(ut->GetItemList(i).GetName()) || Utility::IsDate(ut->GetItemList(i).Get(0))) {
						string _condition = condition;
						string _val = val;
						string _var = ut->GetItemList(i).GetName();

						_condition = wiz::String::replace(_condition, "~~~", ut->GetItemList(i).Get(0));
						_condition = wiz::String::replace(_condition, "~~", _var); //
						_condition = wiz::String::replace(_condition, "////", nowPosition);
						_condition = wiz::String::replace(_condition, "///", wiz::_toString(i));
						_condition = ToBool4(ut, global, _condition, excuteData, builder);
						
						Condition cond(_condition, ut, &global, builder);

						while (cond.Next());

						if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
						{
							_val = wiz::String::replace(_val, "~~~", ut->GetItemList(i).Get(0));
							_val = wiz::String::replace(_val, "~~", _var); //

							_val = wiz::String::replace(_val, "////", nowPosition);
							_val = wiz::String::replace(_val, "///", wiz::_toString(i));
							_val = ToBool4(ut, global, _val, excuteData, builder);

							if (Utility::IsDate(ut->GetItemList(i).GetName())) {
								ut->GetItemList(i).SetName(_val);
							}
							if (Utility::IsDate(ut->GetItemList(i).Get(0))) {
								ut->GetItemList(i).Set(0, _val);
							}
						}
					}
				}
				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }

						ReplaceDateType(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void ReplaceDateType2(UserType& global, const string& nowPosition,
				UserType* ut, const string& val, const string& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
					string temp = ut->GetUserTypeList(i)->GetName();
					if (temp == "") { temp = " "; }

					if (Utility::IsDate(temp)) {
						string _condition = condition;
						string _val = val;

						string _var = ut->GetUserTypeList(i)->GetName();

						_condition = wiz::String::replace(_condition, "~~", _var); //

						_condition = wiz::String::replace(_condition, "////", nowPosition);
						_condition = ToBool4(ut, global, _condition, excuteData, builder);


						Condition cond(_condition, ut, &global, builder);

						while (cond.Next());

						if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
						{
							_val = wiz::String::replace(_val, "~~", _var); //

							_val = wiz::String::replace(_val, "////", nowPosition);
							_val = ToBool4(ut, global, _val, excuteData, builder);
						
							ut->GetUserTypeList(i)->SetName(_val);
						}
					}
					if (recursive) {
						ReplaceDateType2(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void ReplaceDataType1(UserType& global, const string& nowPosition,
				UserType* ut, const std::regex& rgx, const vector<string>& val, const vector<string>& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				for (int i = 0; i < ut->GetItemListSize(); ++i) {
					if (std::regex_match(ut->GetItemList(i).GetName(), rgx)) { // || Utility::IsDate(ut->GetItemList(i).Get(0))) {
						for (int k = 0; k < val.size(); ++k) {
							string _condition = condition[k];
							string _val = val[k];
							string _var = ut->GetItemList(i).GetName();

							
							_condition = wiz::String::replace(_condition, "~~~", ut->GetItemList(i).Get(0));
							_condition = wiz::String::replace(_condition, "~~", _var); //
							_condition = wiz::String::replace(_condition, "////", nowPosition);
							_condition = wiz::String::replace(_condition, "///", wiz::_toString(i));
							_condition = ToBool4(ut, global, _condition, excuteData, builder);


							Condition cond(_condition, ut, &global, builder);

							while (cond.Next());

							if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
							{
								_val = wiz::String::replace(_val, "~~~", ut->GetItemList(i).Get(0));
								_val = wiz::String::replace(_val, "~~", ut->GetItemList(i).GetName());

								_val = wiz::String::replace(_val, "////", nowPosition);
								_val = wiz::String::replace(_val, "///", wiz::_toString(i));
								_val = ToBool4(ut, global, _val, excuteData, builder);
							
								//if (_val[0] == '@') { _val.erase(_val.begin()); }
								//_val = wiz::String::replace(_val, "~~", _var);
								//_val = ToBool4(ut, global, map<string, string>(), _val, EventInfo(), objectMapPtr,  excuteData.pEvents);

								ut->GetItemList(i).SetName(_val);
								break;
							}
						}
					}
				}
				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }

						ReplaceDataType1(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							rgx,
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void ReplaceDataType1_2(UserType& global, const string& nowPosition,
				UserType* ut, const std::regex& rgx, const vector<string>& val, const vector<string>& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				for (int i = 0; i < ut->GetItemListSize(); ++i) {
					if (std::regex_match(ut->GetItemList(i).GetName(), rgx)) { // || Utility::IsDate(ut->GetItemList(i).Get(0))) {
						for (int k = 0; k < val.size(); ++k) {
							string _condition = condition[k];
							string _val = val[k];
							string _var = ut->GetItemList(i).GetName();
							// ~~~ -> _val?

							_condition = wiz::String::replace(_condition, "~~~", ut->GetItemList(i).Get(0));
							_condition = wiz::String::replace(_condition, "~~", _var); //
							_condition = wiz::String::replace(_condition, "////", nowPosition);
							_condition = wiz::String::replace(_condition, "///", wiz::_toString(i));
							_condition = ToBool4(ut, global, _condition, excuteData, builder);

							Condition cond(_condition, ut, &global, builder);

							while (cond.Next());

							if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
							{
								_val = wiz::String::replace(_val, "~~~", ut->GetItemList(i).Get(0));
								_val = wiz::String::replace(_val, "~~", _var); //
								_val = wiz::String::replace(_val, "////", nowPosition);
								_val = wiz::String::replace(_val, "///", wiz::_toString(i));
								_val = ToBool4(ut, global, _val, excuteData, builder);
							
								// remove 3 lines?
							//	if (_val[0] == '@') { _val.erase(_val.begin()); } // chk, same do(chk @) to other functions?
							//	_val = wiz::String::replace(_val, "~~", _var);
						//		_val = ToBool4(ut, global, map<string, string>(), _val, EventInfo(), objectMapPtr,  excuteData.pEvents); // chk!!

								ut->GetItemList(i).Set(0, _val);
								break;
							}
						}
					}
				}
				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }

						ReplaceDataType1_2(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							rgx,
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
			static void ReplaceDataType1_3(UserType& global, const string& nowPosition,
				UserType* ut, const std::regex& rgx, const vector<string>& val, const vector<string>& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive)
			{
				for (int i = 0; i < ut->GetItemListSize(); ++i) {
					if (std::regex_match(ut->GetItemList(i).Get(0), rgx)) {
						for (int k = 0; k < val.size(); ++k) {
							string _condition = condition[k];
							string _val = val[k];

							string _var = ut->GetItemList(i).GetName();

							
							_condition = wiz::String::replace(_condition, "~~~", ut->GetItemList(i).Get(0));
							_condition = wiz::String::replace(_condition, "~~", _var); //
							_condition = wiz::String::replace(_condition, "////", nowPosition);
							_condition = wiz::String::replace(_condition, "///", wiz::_toString(i));
							_condition = ToBool4(ut, global, _condition, excuteData, builder);


							Condition cond(_condition, ut, &global, builder);

							while (cond.Next());

							if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
							{
								_val = wiz::String::replace(_val, "~~~", ut->GetItemList(i).Get(0));
								_val = wiz::String::replace(_val, "~~", _var); //
								_val = wiz::String::replace(_val, "////", nowPosition);
								_val = wiz::String::replace(_val, "///", wiz::_toString(i));
								_val = ToBool4(ut, global, _val, excuteData, builder);
							
								//if (_val[0] == '@') { _val.erase(_val.begin()); }
								//_val = wiz::String::replace(_val, "~~", _var);
								//_val = ToBool4(ut, global, map<string, string>(), _val, EventInfo(), objectMapPtr,  excuteData.pEvents);
								ut->GetItemList(i).Set(0, _val); 
								break;
							}
						}
					}
				}
				if (recursive) {
					for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
						string temp = ut->GetUserTypeList(i)->GetName();
						if (temp == "") { temp = " "; }

						ReplaceDataType1_3(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							rgx,
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}

			// cf) replacedatatype1_4  match value, change var_name?

			static void ReplaceDataType2(UserType& global, const string& nowPosition,
				UserType* ut, const std::regex& rgx, const vector<string>& val, const vector<string>& condition, 
				const ExcuteData& excuteData, wiz::StringBuilder* builder, bool recursive) // first val test
			{
				for (int i = 0; i < ut->GetUserTypeListSize(); ++i) {
					string temp = ut->GetUserTypeList(i)->GetName();
					if (temp == "") { temp = " "; }

					if (std::regex_match(temp, rgx)) {
						for (int k = 0; k < val.size(); ++k) {
							string _condition = condition[k];
							string _val = val[k];
							string _var = ut->GetUserTypeList(i)->GetName(); 
							
							
							_condition = wiz::String::replace(_condition, "~~", _var); //
							_condition = wiz::String::replace(_condition, "////", nowPosition);
							_condition = ToBool4(ut, global, _condition, excuteData, builder);



							Condition cond(_condition, ut, &global, builder);

							while (cond.Next());

							if (cond.Now().size() == 1 && "TRUE" == cond.Now()[0])
							{
								_val = wiz::String::replace(_val, "~~", _var); //
								_val = wiz::String::replace(_val, "////", nowPosition);
								_val = ToBool4(ut, global, _val, excuteData, builder);
							
								//if (_val[0] == '@') { _val.erase(_val.begin()); } // removal?

								//_val = wiz::String::replace(_val, "~~", _var);
								//_val = ToBool4(ut, global, map<string, string>(), _val, EventInfo(), objectMapPtr,  excuteData.pEvents);
								ut->GetUserTypeList(i)->SetName(_val);
								break;
							}
						}
					}
					if (recursive) {
						ReplaceDataType2(
							global,
							nowPosition + "/" + temp,
							ut->GetUserTypeList(i),
							rgx,
							val,
							condition,
							excuteData,
							builder,
							recursive
						);
					}
				}
			}
		};

		// only one exist or do not exist
		inline wiz::load_data::ItemType<string> GetItem(const wiz::load_data::UserType* ut, const string& name) {
			return ut->GetItem(name)[0];
		}

		inline vector<wiz::load_data::UserType*> GetUserType(const wiz::load_data::UserType* ut, const string& name)
		{
			return ut->GetUserTypeItem(name);
		}
		string Find(wiz::load_data::UserType* ut, const string& str, StringBuilder* builder)
		{ // string 대신 vector<string> ??
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
			
			string result;
			if (count == 1)
			{
				return "";
			}
			else {
				auto x = wiz::load_data::UserType::Find(ut,
					wiz::String::substring(str, 0, idx), builder);
				if (x.first == false) { return ""; }
				for (int i = 0; i < x.second.size(); ++i) {
					string itemName = wiz::String::substring(str, idx + 1);
					if (wiz::String::startsWith(itemName, "$it") && itemName.size() >= 4 ){
						int itemIdx = stoi(wiz::String::substring(itemName, 3));
						
						result += x.second[i]->GetItemList(itemIdx).Get(0);
						if (i < x.second.size() - 1) {
							result += "\n";
						}
					}
					else {
						if (itemName == "_") {
							itemName = "";
						}
						auto temp = x.second[i]->GetItem(itemName);
						if (!temp.empty()) {
							for (int j = 0; j < temp.size(); ++j) {
								result += temp[j].Get(0);
								if (j < temp.size() - 1) {
									result += " ";
								}
							}
							if (i < x.second.size() - 1) {
								result += "\n";
							}
						}
					}
				}
			}
			return result;
		}
		inline bool Exist(wiz::load_data::UserType* ut, const string& dir, StringBuilder* builder)
		{
			auto x = wiz::load_data::UserType::Find(ut, dir, builder);
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

		inline string FindParameters(const map<string, string>& parameters, const string& operand)
		{
			map<string, string>::const_iterator x;
			for (int i = 0; i < parameters.size(); ++i) {
				if (wiz::String::startsWith(operand, "$parameter.")
					&& (x = parameters.find( wiz::String::substring(operand, 11) ) ) != parameters.end())  {
					return x->second;
				}
			}
			return "";
		}
		inline string FindLocals(const map<string, string>& locals, const string& operand)
		{
			if (wiz::String::startsWith(operand, "$local.") && locals.end() != locals.find(wiz::String::substring(operand, 7)))
			{
				return locals.at(wiz::String::substring(operand, 7));
			}
			return "";
		}
		//need to renewal. add $AND $OR $NOT
	
		/// remove /, parameter chk!!
		string ToBool4(wiz::load_data::UserType* now, wiz::load_data::UserType& global, const string& temp, const ExcuteData& excuteData, wiz::StringBuilder* builder);
		bool operation(wiz::load_data::UserType* now, wiz::load_data::UserType& global, const string& str,
			wiz::ArrayStack<string>& operandStack, const ExcuteData& excuteData, wiz::StringBuilder* builder)
		{
			if (!operandStack.empty() && operandStack.top() == "ERROR") {
				return false;
			}

			int operandNum = 0;
			int count = 0; // for brace!
			{
				for (int i = operandStack.size() - 1; i >= 0; --i) {
					if ("{" == operandStack[i]) {
						count++;
					}
					if ("}" == operandStack[i]) {
						count--;
						if (count < 0) {
							break;
						}
						else {
							operandNum++;
						}
					}
					else {
						operandNum++;
					}
				}
			}

			for (int i = 0; i < operandNum; ++i) {
				if ("^" == operandStack[operandStack.size() - 1 - i]) {
					operandStack[operandStack.size() - 1 - i] = "";
				}
			}

			if ("$EQ" == str) {
				string x, y;
				int idx = -1;
				x = operandStack.pop();
				y = operandStack.pop();

				if (wiz::load_data::Utility::Compare(x, y, builder) == "== 0") {
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

				if (wiz::load_data::Utility::Compare(x, y, builder) != "== 0") {
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
			else if ("$AND_ALL" == str) {
				vector<string> store;
				for (int i = 0; i < operandNum; ++i) {
					store.push_back(operandStack.pop());
				}
				for (int i = 0; i < store.size(); ++i) {
					if ("TRUE" != store[i]) {
						operandStack.push("FALSE");
						return true;
					}
				}
				operandStack.push("TRUE");
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
			else if ("$OR_ALL" == str)
			{
				vector<string> store;

				for (int i = 0; i < operandNum; ++i) {
					store.push_back(operandStack.pop());
				}
				for (int i = 0; i < store.size(); ++i) {
					if ("TRUE" == store[i]) {
						operandStack.push("TRUE");
						return true;
					}
				}
				operandStack.push("FALSE");
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

				if (wiz::load_data::Utility::Compare(x, y, builder) == "< 0") {
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

				if (wiz::load_data::Utility::Compare(x, y, builder) == "> 0") {
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

				if (wiz::load_data::Utility::Compare(x, y, builder) == "< 0" || wiz::load_data::Utility::Compare(x, y, builder) == "== 0") {
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

				if (wiz::load_data::Utility::Compare(x, y, builder) == "> 0" || wiz::load_data::Utility::Compare(x, y, builder) == "== 0") {
					operandStack.push("TRUE");
				}
				else
				{
					operandStack.push("FALSE");
				}
			}
			else if ("$add" == str) // todo! = int operator double => double operator double!
			{
				string x, y;
				x = operandStack.pop();
				y = operandStack.pop();

				string typex, typey; // other *, /, mod, ..
				typex = wiz::load_data::Utility::GetType(x);
				typey = wiz::load_data::Utility::GetType(y);
				if (typex == typey && typey == "INTEGER") { /// only integer -> BigInteger
					operandStack.push(wiz::toStr(atoll(x.c_str()) + atoll(y.c_str())));
				}
				else if (typex == typey && typey == "DOUBLE") {
					operandStack.push(wiz::_toString(std::stold(x) + std::stold(y)));
				}
				else
				{
					operandStack.push("ERROR");
				}
			}
			else if ("$multiple" == str) // todo! = int operator double => double operator double!
			{
				string x, y;
				x = operandStack.pop();
				y = operandStack.pop();



				if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && (wiz::load_data::Utility::GetType(y) == "INTEGER")) { /// only integer -> BigInteger
					operandStack.push(wiz::toStr(atoll(x.c_str()) * atoll(y.c_str())));
				}
				else if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && wiz::load_data::Utility::GetType(y) == "DOUBLE") {
					operandStack.push(wiz::_toString(std::stold(x) * std::stold(y)));  /// chk?
				}
				else
				{
					operandStack.push("ERROR");
				}
			}
			else if ("$divide" == str) // todo! = int operator double => double operator double!
			{
				string x, y;
				x = operandStack.pop();
				y = operandStack.pop();

				if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && (wiz::load_data::Utility::GetType(y) == "INTEGER")) { /// only integer -> BigInteger
					operandStack.push(wiz::toStr(atoll(x.c_str()) / atoll(y.c_str())));
				}
				else if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && wiz::load_data::Utility::GetType(y) == "DOUBLE") {
					operandStack.push(wiz::_toString(std::stold(x) / std::stold(y)));
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

				if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && (wiz::load_data::Utility::GetType(y) == "INTEGER")) { /// only integer -> BigInteger
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

				if (wiz::load_data::Utility::GetType(x) == wiz::load_data::Utility::GetType(y) && (wiz::load_data::Utility::GetType(y) == "INTEGER")) { /// only integer -> BigInteger
					int _x = atoi(x.c_str());
					int _y = atoi(y.c_str());
					if (_y - _x + 1 <= 0) {
						operandStack.push("ERROR x / 0");

					}
					else {
						int _z = wiz::Rand2<int>() % (_y - _x + 1) + _x; // _x <= _z <= _y
						operandStack.push(wiz::toStr(_z));
					}
				}
				else
				{
					operandStack.push("ERROR");
				}
			}
			else if ("$concat" == str)
			{
				if (builder) {
					builder->Clear();

					int total_size = 0;
					for (int i = 0; i < 2; ++i)
					{
						total_size += operandStack.top().size();
						string temp = operandStack.pop();
						builder->Append(temp.c_str(), temp.size()); // chk
					}

					operandStack.push(string(builder->Str(), total_size));
				}
				else {
					string x, y;
					x = operandStack.pop();
					y = operandStack.pop();

					operandStack.push(x + y);
				}
			}
			else if ("$concat2" == str) /// with space
			{
				string x, y;
				x = operandStack.pop();
				y = operandStack.pop();

				operandStack.push(x + " " + y);
			}
			else if ("$concat_all" == str)
			{
				//cout << "----" << endl;
				if (builder) {
					builder->Clear();

					int total_size = 0;
					for (int i = 0; i < operandNum; ++i)
					{
						total_size += operandStack.top().size();
						string temp = operandStack.pop();
						builder->Append(temp.c_str(), temp.size()); // chk
					}

					operandStack.push(string(builder->Str(), total_size));
				}
				else {
					string result;

					for (int i = 0; i < operandNum; ++i) {
						result = result + operandStack.pop();
						//	cout << "test" <<  result << endl;
					}
					//cout << "----" << endl;
					operandStack.push(result);
				}
			}
			else if ("$concat_all2" == str)
			{
				string result;

				for (int i = 0; i < operandNum; ++i) {
					result += operandStack.pop();
					if (i < operandNum - 1) {
						result += " ";
					}
				}
				operandStack.push(result);
			}
			else if ("$concat3" == str) { // for special case? "abc" "def" "ghi" -> "abcdefghi"
				string result;

				for (int i = 0; i < operandNum; ++i) {
					string temp = operandStack.pop();
					if (temp.size() >= 3 && temp.back() == temp.front() && temp.back() == '\"') {}
					else {
						operandStack.push("ERROR in $concat3, 1. must be \" \" ");
						return false;
					}
					if (i < operandNum - 1) {
						temp.erase(temp.begin() + temp.size() - 1);

						if (0 != i) {
							temp.erase(temp.begin());
						}

						result += temp;
						result += " ";
					}
					else {
						temp.erase(temp.begin());

						result += temp;
					}
				}

				operandStack.push(result);
			}
			else if ("$return_value" == str)
			{
				operandStack.push(excuteData.info.return_value);
			}
			// cf) empty test!!
			///ToDo - GetList -> // GetItemListIdxByIListIdx, GetUserTypeLisIdxtByIListIdx ?
			else if ("$back" == str) // ex) for x  = { 0 1 2 3 .. }, for usertaypelist? and mixed? and need more test!
			{
				string x = operandStack.pop();

				string value = wiz::load_data::LoadData::GetItemListData(global, x, "TRUE", excuteData, builder);
				wiz::load_data::UserType ut;
				wiz::load_data::LoadData::LoadDataFromString(value, ut);

				if (ut.IsItemList(ut.GetIListSize() - 1))

				{
					auto x = ut.GetItemList(ut.GetItemListSize() - 1);
					operandStack.push(x.Get(0));
				}
				else {
					auto x = ut.GetUserTypeList(ut.GetUserTypeListSize() - 1);
					operandStack.push("\"" + x->ToString() + "\"");
				}
			}
			// pop_back or front - remove this function?
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

				wiz::load_data::UserType* ut = nullptr;
				auto finded = wiz::load_data::UserType::Find(&global, x, builder);
				ut = finded.second[0];

				if (ut->IsItemList(ut->GetIListSize() - 1))
				{
					auto x = ut->GetItemList(ut->GetItemListSize() - 1);
					operandStack.push(x.Get(0));
					ut->RemoveItemList(ut->GetItemListSize() - 1);
				}
				else {
					auto x = ut->GetUserTypeList(ut->GetUserTypeListSize() - 1);
					operandStack.push("\"" + x->ToString() + "\"");
					ut->RemoveUserTypeList(ut->GetUserTypeListSize() - 1);
				}
			}
			// todo - $front, pop-front.
			else if ("$front" == str)
			{
				string x = operandStack.pop();

				string value = wiz::load_data::LoadData::GetItemListData(global, x, "TRUE", excuteData, builder);
				wiz::load_data::UserType ut;
				wiz::load_data::LoadData::LoadDataFromString(value, ut);

				if (ut.IsItemList(0))
				{
					auto x = ut.GetItemList(0);
					operandStack.push(x.Get(0));
				}
				else {
					auto x = ut.GetUserTypeList(0);
					operandStack.push("\"" + x->ToString() + "\"");
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

				wiz::load_data::UserType* ut = nullptr;
				auto finded = wiz::load_data::UserType::Find(&global, x, builder);
				ut = finded.second[0];

				if (ut->IsItemList(0))
				{
					auto x = ut->GetItemList(0);
					operandStack.push(x.Get(0));
					ut->RemoveItemList(0);
				}
				else {
					auto x = ut->GetUserTypeList(0);
					operandStack.push("\"" + x->ToString() + "\"");
					ut->RemoveUserTypeList(0);
				}
			}
			else if ("$get" == str)
			{
				string x = operandStack.pop();

				if ('@' == x[0]) { // chk..
					x.erase(x.begin());
				}

				if (wiz::String::startsWith(x, "now.") && nullptr != now)
				{
					x = wiz::String::substring(x, 4);
					x = "/./" + x;
					if ('/' == x[0])
					{
						string temp = Find(now, x, builder); if (!temp.empty()) { x = temp; }
					}
				}
				else {
					if ('/' == x[0])
					{
						string temp = Find(&global, x, builder); if (!temp.empty()) { x = temp; }
					}
				}

				{
					string temp = FindParameters(excuteData.info.parameters, x);
					if (!temp.empty()) { x = temp; }
				}
				{
					string temp = FindLocals(excuteData.info.locals, x);
					if (!temp.empty()) { x = temp; }
				}

				operandStack.push(x);
			}
			else if ("$size" == str)
			{
				string x = operandStack.pop();

				if ('/' == x[0])
				{
					wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, x, builder).second[0];
					x = wiz::toStr(ut->GetItemListSize());
				}
				else
				{
					x = "FALSE";
				}

				operandStack.push(x);
			}
			else if ("$size2" == str)
			{
				string x = operandStack.pop();

				if ('/' == x[0])
				{
					wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, x, builder).second[0];
					x = wiz::toStr(ut->GetUserTypeListSize());
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
				int idx = atoi(y.c_str());

				if ('/' == x[0])
				{
					wiz::load_data::UserType* ut = wiz::load_data::UserType::Find(&global, x, builder).second[0];
					x = ut->GetItemList(idx).Get(0);
				}
				else
				{
					x = "NONE";
				}

				operandStack.push(x);
			}
			else if ("$regex" == str) {
				string str = operandStack.pop();
				string rgx_str = operandStack.pop();

				// " ~ " , "제거?
				if (rgx_str.size() > 2 && rgx_str[0] == rgx_str.back() && rgx_str[0] == '\"') {
					std::regex rgx(rgx_str.substr(1, rgx_str.size() - 2));

					if (std::regex_match(str, rgx))
					{
						operandStack.push("TRUE");
					}
					else {
						operandStack.push("FALSE");
					}
				}
				else {
					operandStack.push("ERROR in $regex");
				}
			}
			else if ("$eval" == str) {
				string str = operandStack.pop();

				bool chk = wiz::load_data::Utility::ChkExist(str);
				if (chk) {
				}
				else {
					operandStack.push("ERROR in $eval, must be \" \" ");
					return false;
				}
				str = str.substr(1, str.size() - 2);
				{
					string result = ToBool4(now, global, str, excuteData, builder);

					operandStack.push(move(result));
				}
			}
			// big
			else if ("$is_quoted_str" == str)
			{
				string str = operandStack.pop();
				if (str.size() >= 2 && str[0] == str.back() && '\"' == str[0])
				{
					operandStack.push("TRUE");
				}
				else {
					operandStack.push("FALSE");
				}
			}
			// small
			else if ("$is_quoted_str2" == str) 
			{
				string str = operandStack.pop();
				if (str.size() >= 2 && str[0] == str.back() && '\'' == str[0])
				{
					operandStack.push("TRUE");
				}
				else {
					operandStack.push("FALSE");
				}
			}
			else if ("$toQuotedStr" == str) {
				string str = operandStack.pop();
				str.push_back('\"');
				str.insert(str.begin(), '\"');
				operandStack.push(str);
			}
			else if ("$toQuotedStr2" == str) {
				string str = operandStack.pop();
				str.push_back('\'');
				str.insert(str.begin(), '\'');
				operandStack.push(str);
			}
			else if ("$addSmallQuoted" == str) {
				string str = operandStack.pop();
				str.push_back('\'');
				str.insert(str.begin(), '\'');
				operandStack.push(str);
			}
			else if ("$removeQuoted" == str) { // chk "" string problem?
				string str = operandStack.pop();

				if (str.size() > 0 && str.front() == str.back()
					&& '\"' == str.back()
					)
				{
					str = wiz::String::substring(str, 1, str.size() - 2);
				}

				operandStack.push(str);
			}
			else if ("$getObjectStr" == str) {
				string object_name = operandStack.pop();
				object_name = wiz::String::substring(object_name, 1, object_name.size() - 2);
				string event_id = operandStack.pop();

				wiz::load_data::UserType ut = (*excuteData.pObjectMap)[object_name];

				bool pass = false;
				for (int i = 0; i < ut.GetUserTypeListSize(); ++i) {
					for (int j = 0; j < ut.GetUserTypeList(i)->GetItemListSize(); ++j) {
						if ("id" == ut.GetUserTypeList(i)->GetItemList(j).GetName()
							&& event_id == ut.GetUserTypeList(i)->GetItemList(j).Get(0)
							) {
							operandStack.push("Event = { " + ut.GetUserTypeList(i)->ToString() + " } ");
							return true;
						}
					}
				}
				operandStack.push("ERROR in $getOjbectStr");
			}
			else if ("$add_paren" == str) { // removal?
				string temp = operandStack.pop();
				operandStack.push(" { " + temp + " } ");
			}
			else if ("$test" == str) { // for lambda test.
				string temp = operandStack.pop();

				temp = wiz::String::replace(temp, "\"", "");
				
				temp = "\"" + temp + "\"";

				operandStack.push(temp);
			}
			else if ("$test2" == str) { // for lambda test?
				string temp = operandStack.pop();

				temp = wiz::String::substring(temp, 1, temp.size() - 2);

				operandStack.push(temp);
			}
			else if ("$test3" == str) { // for lambda test?
				string temp = operandStack.pop();

				operandStack.push(temp);
			}
			else if ("$test4" == str) {
				string temp = operandStack.pop();

				if (temp.size() >= 3 && temp.back() == temp.front() && temp.back() == '\"') {
					temp = wiz::String::substring(temp, 1, temp.size() - 2);
				}

				operandStack.push(temp);
			}
			else if ("$test5" == str) { // chk!!
				string temp = operandStack.pop();
				int braceNum = 0;
				bool first = true;

				int start = String::find(temp, "$return", 0) + 7;
				for (int i = start; i < temp.size(); ++i) {
					if ('{' == temp[i]) {
						if (first) {
							temp.insert(temp.begin() + i + 1, '\"');
						}
						else {
							braceNum++;
						}
						first = false;
					}
					else if ('}' == temp[i]) {
						braceNum--;
						if (!first && 0 == braceNum) {
							temp.insert(temp.begin() + i + 1, '\"');
						}
					}
					//cout << temp << endl;
				}
				
				operandStack.push(temp);
			}
			else if ("$test6" == str) { // for lambda test.
				vector<string> vec;
				for (int i = 0; i < operandNum; ++i) {
					vec.push_back(operandStack.pop());
				}
				for (int i = vec.size()-1; i >= 0; --i) {
					vec[i] = wiz::String::replace(vec[i], "\"", "");
					operandStack.push(vec[i]);
				}
			}
			else if ("$get_item_name" == str) {
				wiz::load_data::UserType ut;
				string statement;
				
				for (int i = 0; i < operandNum; ++i) {
					statement = statement + operandStack.pop();
				}
				wiz::load_data::LoadData::LoadDataFromString(statement, ut);

				operandStack.push(ut.GetItemList(0).GetName());
			}
			else if ("$get_item_value" == str) { // why?
				wiz::load_data::UserType ut;
				string statement;
				int idx = 0;

				for (int i = 0; i < operandNum - 1; ++i) {
					statement = statement + operandStack.pop();
				}
				idx = stoi(operandStack.pop());

				wiz::load_data::LoadData::LoadDataFromString(statement, ut);

				operandStack.push(ut.GetItemList(idx).Get(0));
			}
			else if ("$get_item_size" == str) {
				wiz::load_data::UserType ut;
				string statement;

				for (int i = 0; i < operandNum; ++i) {
					statement = statement + operandStack.pop();
				}
				wiz::load_data::LoadData::LoadDataFromString(statement, ut);

				operandStack.push(wiz::_toString(ut.GetItem(ut.GetItemList(0).GetName()).size()));
			}
			else if ("$is_empty_string" == str) {
				operandStack.push(operandStack.pop().empty() ? "TRUE" : "FALSE");
			}
			else if ("$event_result" == str) {
				vector<string> eventVec;
				for (int i = 0; i < operandNum; ++i) {
					eventVec.push_back(operandStack.pop());
				}
				
				string statements2 = "Event = { id = NONE" + wiz::toStr(excuteData.depth + 1) + " $call = { ";
				for (int i = 0; i < eventVec.size(); ++i) {
					statements2 = statements2 + eventVec[i] + " ";
				}
				statements2 = statements2 + " } }";
				wiz::load_data::UserType* eventsTemp = excuteData.pEvents;
				wiz::load_data::LoadData::AddData(*eventsTemp, "/root", statements2, "TRUE", ExcuteData(), builder);
				//cout << " chk " << statements2 << endl;
				ExcuteData _excuteData;
				_excuteData.pModule = excuteData.pModule;
				_excuteData.pObjectMap = excuteData.pObjectMap;
				_excuteData.pEvents = eventsTemp;
				_excuteData.depth = excuteData.depth + 1;
				
				operandStack.push(excute_module("Main = { $call = { id = NONE" + wiz::toStr(_excuteData.depth) + " } }", &global, _excuteData, 0));
				
				{
					for (int idx = 0; idx < eventsTemp->GetUserTypeListSize(); ++idx) {
						if (eventsTemp->GetUserTypeList(idx)->GetItem("id")[0].Get(0) == "NONE" + wiz::toStr(_excuteData.depth)) {
							eventsTemp->RemoveUserTypeList(idx);
							break;
						}
					}
				}
			}
			else if ("$get_item_value2" == str) {
				const int i = stoi(operandStack.pop());

				if (now) {
					operandStack.push(now->GetItemList(i).Get(0)); //chk
					return true;
				}
				else {
					operandStack.push("ERROR");
					return false;
				}
			}
			else if ("$space" == str) {
				operandStack.push(" ");
				return true;
			}
			else if ("$empty" == str) {
				operandStack.push("");
				return true;
			}
			else if ("$move_up" == str) {
				string dir;
				
				for (int i = 0; i < operandNum; ++i) {
					string temp = operandStack.pop();
					dir = dir + temp;
				//	cout << "temp is " << temp << endl;
				}

				//cout << " dir is  " << dir << endl;
				if (String::startsWith(dir, "/."))
				{
					dir = String::substring(dir, 3);
				}

				StringTokenizer tokenizer(dir, "/", builder, 1);
				vector<string> tokenVec;
				while (tokenizer.hasMoreTokens()) {
					tokenVec.push_back(tokenizer.nextToken());
				}
				dir = "/./";
				if (tokenVec.empty()) { operandStack.push(dir); return true; }
				for (int i = 0; i < tokenVec.size()-1; ++i) {
					dir = dir + tokenVec[i] +"/";
				}
				operandStack.push(dir);
				return true;
			}
			else if ("$lambda" == str) {
				vector<string> store;
				for (int i = 0; i < operandNum; ++i) {
					store.push_back(operandStack.pop());
					if (store[i][0] != store[i][store[i].size() - 1] || '\'' != store[i][0])
					{
						store[i] = "\'" + store[i] + "\'";
					}
				}
				wiz::load_data::UserType ut;
				wiz::load_data::LoadData::LoadDataFromString(store[0].substr(1, store[0].size() - 2), ut);
				
				
				if (operandNum >= 1) {
					vector<wiz::load_data::UserType*> param = ut.GetUserTypeItem("Event")[0]->GetUserTypeItem("$parameter");
					// in order.
					string mainStr = "Main = { $call = { id = NONE } } ";
					string eventStr = "Event = { id = NONE $call = { id = " +
						ut.GetUserTypeItem("Event")[0]->GetItem("id")[0].Get(0) + " ";

					// if opeandNum != ut[0].GetUserType("$parameter").size() then push error?
					for (int i = 0; i < param[0]->GetItemListSize(); ++i) {
						eventStr += param[0]->GetItemList(i).Get(0) + " = { \'" + store[i + 1].substr(1, store[i + 1].size() - 2) + "\' } ";
					}

					eventStr += " }  $return = { $return_value = { } } } ";

					eventStr += store[0].substr(1, store[0].size() - 2);

					ut.Remove();
					wiz::load_data::LoadData::LoadDataFromString(eventStr, ut);

					string result = excute_module(mainStr, &ut, ExcuteData(), 0);
					
					{
						wiz::load_data::UserType ut;
						wiz::load_data::LoadData::LoadDataFromString(result.substr(1, result.size() - 2), ut);
		
						if (0 == ut.GetUserTypeListSize()) {
							string param = ut.ToString();
							if (wiz::String::startsWith(param, "$parameter.")) { // 
								param = param.substr(11);
								result = string("\'Event = { id = identity ") + "$parameter = { " + param + " } $return = { $parameter." + param + " } }\'";
							}
						}
					}

 					operandStack.push(result);
				}

				return true;
			}
			
			else if("$to_float_from_integer" == str) { // integer, floating point number -> floating point number(long double)
				string value = operandStack.pop();
				if (wiz::load_data::Utility::IsInteger(value)) {
					long double x = stoll(value);
					operandStack.push(wiz::_toString(x));
				}
				else {
					operandStack.push("it is not integer");
				}
			}
			else if ("$to_integer_from_float" == str) { // integer, floating point number -> floating point number(long double)
				string value = operandStack.pop();
				if (wiz::load_data::Utility::IsDouble(value)) {
					long long x = stold(value);
					operandStack.push(wiz::_toString(x));
				}
				else {
					operandStack.push("it is not floating number");
				}
			}
			//floor, ceiling, round,
			else if ("$floor" == str) {
				string value = operandStack.pop();
				if (wiz::load_data::Utility::IsDouble) {
					long double x = floor(stold(value));
					operandStack.push(wiz::_toString(x));
				}
				else {
					operandStack.push("only double can");
				}
			}
			else if ("$ceiling" == str) {
				string value = operandStack.pop();
				if (wiz::load_data::Utility::IsDouble) {
					long double x = ceil(stold(value));
					operandStack.push(wiz::_toString(x));
				}
				else {
					operandStack.push("only double can");
				}
			}
			else if ("$round" == str) {
				string value = operandStack.pop();
				if (wiz::load_data::Utility::IsDouble) {
					long double x = round(stold(value));
					operandStack.push(wiz::_toString(x));
				}
				else {
					operandStack.push("only double can");
				}
			}
			//contains,
			else if ("$contains" == str) {
				string str = operandStack.pop();
				string chk_str = operandStack.pop();

				operandStack.push(string::npos != str.find(chk_str) ? "TRUE" : "FALSE");
			}
			//starts_with, ends_with,
			else if ("$starts_with" == str) {
				string str = operandStack.pop();
				string chk_str = operandStack.pop();

				operandStack.push(wiz::String::startsWith(str, chk_str) ? "TRUE" : "FALSE");
			}
			else if ("$ends_with" == str) {
				string str = operandStack.pop();
				string chk_str = operandStack.pop();

				operandStack.push(wiz::String::endsWith(str, chk_str) ? "TRUE" : "FALSE");
			}
			//string - length,
			else if ("$string_length" == str) {
				string str = operandStack.pop();
				
				operandStack.push(wiz::_toString(str.size()));
			}
			//substring ?
			else if ("$substring" == str) {
				string str = operandStack.pop();
				long long begin = stoll(operandStack.pop());
				long long end = stoll(operandStack.pop());

				operandStack.push(wiz::String::substring(str, begin, end-1));
			}

			// todo - Is~ ?? others ??
			else if ("$is_integer_type" == str) {
				operandStack.push(wiz::load_data::Utility::IsInteger(operandStack.pop())? "TRUE" : "FALSE");
			}
			else if ("$is_float_type" == str) {
				operandStack.push(wiz::load_data::Utility::IsDouble(operandStack.pop()) ? "TRUE" : "FALSE");
			}
			else if ("$is_pure_string_type" == str) {
				operandStack.push("STRING" == wiz::load_data::Utility::GetType(operandStack.pop())? "TRUE" : "FALSE");
			}
			else if ("$get_type" == str) {
				operandStack.push(wiz::load_data::Utility::GetType(operandStack.pop()));
			}

			else {
				if (wiz::String::startsWith(str, "$") && str.size() >= 2) {
					return false;
				}
				return true;
			}
			return true;
			// cf)
			// remove "
		}

		string ToBool3(wiz::load_data::UserType& global, const map<string, string>& parameters, const string& temp,
			 EventInfo info, StringBuilder* builder) /// has bug!
		{
			wiz::StringTokenizer tokenizer(temp, vector<string>{ "/" }, builder, 1);
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
						if (wiz::isWhitespace(tokenVec[i][j]) || tokenVec[i][j] == '{' || tokenVec[i][j] == '}' || tokenVec[i][j] == '=') {
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
						if (wiz::isWhitespace(tokenVec[i][j]) || tokenVec[i][j] == '{' || tokenVec[i][j] == '}' || tokenVec[i][j] == '=') {
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
		string ToBool4(wiz::load_data::UserType* now, wiz::load_data::UserType& global, const string& temp, const ExcuteData& excuteData, wiz::StringBuilder* builder)
		{ 
			string result = temp;
			//cout << "temp is " << temp << endl;
			/*set<string> utNames; // rename?
			{ // removal??
				UserType utTemp;
				LoadData::LoadDataFromString(result, utTemp);

				for (int i = 0; i < utTemp.GetUserTypeListSize(); ++i) {
					if (utTemp.GetUserTypeList(i)->GetName().empty() == false
						&& utTemp.GetUserTypeList(i)->GetName()[0] == '$'
						) {
						utNames.insert(utTemp.GetUserTypeList(i)->GetName());
					}
				}
			}*/
		//	cout << "result is " << result << endl;

			wiz::ArrayStack<string> resultStack;
			wiz::load_data::UserType ut;
			bool chk = false;
			int count_change = 0;

			bool flag_A = false;
			if (result.size() > 1 && result[0] == '/')
			{
				flag_A = true;
			}
			result = ToBool3(global, excuteData.info.parameters, result, excuteData.info, builder);
			if (result.empty()) { return ""; }
			if (!flag_A) {
				result = string(result.c_str() + 1);
			}
			wiz::load_data::LoadData::LoadDataFromString(result, ut);
			result = ut.ToString();
			if (result.empty()) { return result; }

			if (ut.empty()) {
				return "";
			}
			if (ut.GetUserTypeListSize() == 0 && ut.GetItemListSize() == 1) /// chk
			{
				if ('/' == result[0] && result.size() > 1)
				{
					string temp = Find(&global, result, builder);

					if (!temp.empty()) {
						result = move(temp);
					}return result;
				}
				else if (wiz::String::startsWith(result, "$local.")) {
					string _temp = FindLocals(excuteData.info.locals, result);
					if (!_temp.empty()) {
						result = move(_temp);
					}return result;
				}
				else if (wiz::String::startsWith(result, "$parameter.")) {
					string _temp = FindParameters(excuteData.info.parameters, result);
					if (!_temp.empty()) {
						result = move(_temp);
					}return result;
				}
				
			}
		
			vector<string> tokenVec;
			{
				wiz::StringTokenizer tokenizer(result, { " ", "\n", "\t", "\r" }, builder, 1); // , "{", "=", "}" }); //
				//wiz::StringTokenizer tokenizer2(result, { " ", "\n", "\t", "\r" } ); //
				
				//vector<string> tokenVec2;

				while (tokenizer.hasMoreTokens()) {
					tokenVec.push_back(tokenizer.nextToken());
				}

				for (int i = tokenVec.size() - 1; i >= 0; --i)
				{
					string before = tokenVec[i];
					if ('/' == tokenVec[i][0] && tokenVec[i].size() > 1)
					{
						string _temp = Find(&global, tokenVec[i], builder);

						if ("" != _temp) {
							tokenVec[i] = move(_temp);
						}
					}
					else if (wiz::String::startsWith(tokenVec[i], "$local.")) { // && length?
						string _temp = FindLocals(excuteData.info.locals, tokenVec[i]);
						if (!_temp.empty()) {
							tokenVec[i] = move(_temp);
						}
					}
					else if (wiz::String::startsWith(tokenVec[i], "$parameter.")) { // && length?
						string _temp = FindParameters(excuteData.info.parameters, tokenVec[i]);
						if (!_temp.empty()) {
							tokenVec[i] = move(_temp);
						}
					}
				}

				//while (tokenizer2.hasMoreTokens()) {
				//	tokenVec2.push_back(tokenizer2.nextToken());
				//}
			//	cout << "result is " << result << endl;
				//result = "";

				//int j = tokenVec.size() - 1;
				//for (int i = tokenVec2.size() - 1; i >= 0 && j >= 0; --i) {
				//	if (tokenVec2[i] == "{" || tokenVec2[i] == "}" || tokenVec2[i] == "=") { 
				//		continue;
				//	}
				//	else {
				//		tokenVec2[i] = tokenVec[j];
				//		--j;
				//	}
				//}
			//	result = "";
			//	for (int i = 0; i < tokenVec.size(); ++i) {
			//		if (i != 0) { result = result + " "; }
			//		
			//		result += tokenVec[i];
				//}
			}
			//cout << "result is " << result << endl;
			//
			wiz::ArrayStack<string> operandStack;
			wiz::ArrayStack<string> operatorStack; 
			//wiz::StringTokenizer tokenizer(result, { " ", "\n", "\t", "\r" }, builder, 1);
			//vector<string> tokenVec;

			//while (tokenizer.hasMoreTokens()) {
		//		tokenVec.push_back(tokenizer.nextToken());
			//}

			for (int i = tokenVec.size() - 1; i >= 0; --i) {
				// todo - chk first? functions in Event
				if ( //String::startsWith(tokenVec[i], "$parameter.") ||
					//tokenVec[i] == "$parameter" ||
					//tokenVec[i] == "$local" ||
					 //String::startsWith(tokenVec[i], "$local.") ||
					//"$return" == tokenVec[i] ||
					'$' != tokenVec[i][0] || ('$' == tokenVec[i][0] && tokenVec[i].size() == 1)
					) {
					operandStack.push(tokenVec[i]);
				}
				else
				{
					// cout << tokenVec[i] << endl;
					operandStack.pop(); // =
					operandStack.pop(); // {
					operatorStack.push(tokenVec[i]);

					if (false == operation(now, global, tokenVec[i], operandStack, excuteData, builder)) // chk!!
					{
						// chk removal here?
						cout << " false " << endl;
						_getch();
						//
						operatorStack.pop();
						operandStack.push("{");
						operandStack.push("=");
						operandStack.push(tokenVec[i]);
						continue;
					}

					operandStack[operandStack.size() - 2] = operandStack[operandStack.size() - 1];
					operandStack.pop(); // } 
				}
			}

			// ex) A = { B = 1 $C = { 3 } } D = { E }  $F = { G }
			// =>  A = { B = 1 $C = 3  }  D = E $F = G 
			// =>  A = { B = 1 $C = { 3 } } D = E  $F = { G } : ToDo!

			vector<string> strVec;
			stack<int> chkBrace;

			chkBrace.push(0);

			for (int i = operandStack.size() - 1; i >= 0; --i)
			{
				if (operandStack[i] == "}") {
					chkBrace.top()++;
					if (chkBrace.top() == 2 && !(i + 4 <= operandStack.size() - 1 && operandStack[i+3] == "=" && operandStack[i+4][0] == '$' && operandStack[i+4].size() > 1))
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

			//cout << "result is " << result << endl;
			//result = "";
			//builder->Clear();
			//for (int i = 0; i < strVec.size(); ++i) {
			//	if (i != 0) { 
					//	result = result + " "; 
			//		builder->Append(" ", 1);
			//	}
				// result = result + strVec[i] + " "; // add space!
			//	builder->Append(strVec[i].c_str(), strVec[i].size());
			//	builder->Append(" ", 1);
			//}
			//result = string(builder->Str(), builder->size());
			// todo!  $C = 3 => $C = { 3 } 
			{
				//StringTokenizer tokenizer(result, builder, 1);
				//result = "";
				builder->Clear();

				//while (tokenizer.hasMoreTokens()) {
				for (int i = 0; i < strVec.size(); ++i) {
					const string temp = strVec[i]; // tokenizer.nextToken();
					/*
					// chk!! @$paramter - removal? @$. (for regex)??
					if (temp.size() >= 3 && String::startsWith(temp, "$.")) { // cf) @$. ?
						//result = result + temp + " ";
						builder->Append(temp.c_str(), temp.size());
						builder->Append(" ", 1);
					}
					else if (temp.size() >= 12 && String::startsWith(temp, "$parameter.") || (temp.size()) >= 13 && String::startsWith(temp, "@$parameter.")) {
						//result = result + temp + " ";
						builder->Append(temp.c_str(), temp.size());
						builder->Append(" ", 1);
					}
					else if (temp.size() >= 8 && String::startsWith(temp, "$local.") || (temp.size()>=9 && String::startsWith(temp, "@$local."))) {
						//result = result + temp + " ";
						builder->Append(temp.c_str(), temp.size());
						builder->Append(" ", 1);
					}
					else if (
						(temp.size() >= 3 && temp[0] == '@' && temp[1] == '$')) { // chk - removal??
						++i; // tokenizer.nextToken(); // = 
						++i;
						string temp2 = strVec[i]; //tokenizer.nextToken(); // " ~~ "
						//result = result + temp + " = { " + temp2 + " } ";

						builder->Append(temp.c_str() + 1, temp.size());
						builder->Append(" = { ", 5); 
						
						builder->Append(temp2.c_str(), temp2.size());
						builder->Append(" } ", 3);
					}
					else 
					*/
					{
						//result = result + temp + " ";
						builder->Append(temp.c_str(), temp.size());
						builder->Append(" ", 1);
					}
				}
			}
			result = string(builder->Str(), builder->Size());
			if (!result.empty()) {
				result.erase(result.begin() + result.size() - 1);
			}
	/*

			{ // removal?? -why?? - reason?
				UserType ut;
				LoadData::LoadDataFromString(result, ut);

				for (int i = 0; i < ut.GetItemListSize(); ++i) {
					if (utNames.find(ut.GetItemList(i).GetName()) != utNames.end()) {
						UserType temp(ut.GetItemList(i).GetName());
						temp.AddItem("", ut.GetItemList(i).Get(0));
						ut.AddUserTypeItem(move(temp));
						ut.RemoveItemList(i);
						--i;
					}
				}
				
				result = ut.ToString();
			}

			{ // chk.. - removal?
				for (auto x = excuteData.info.parameters.rbegin(); x != excuteData.info.parameters.rend(); ++x) {
					string temp;
					Utility::ChangeStr(result, { "$parameter." + x->first }, { x->second }, temp);
					result = move(temp);
				}
				for (auto x = excuteData.info.locals.rbegin(); x != excuteData.info.locals.rend(); ++x) {
					string temp;
					Utility::ChangeStr(result, { "$local." + x->first }, { x->second }, temp);
					result = move(temp);
				}
			}
		*/
		//	cout << "result is " << result << endl;
		//	cout << endl;
			return result;
		}

	}
}


#endif // LOAD_DATA_H_INCLUDED
