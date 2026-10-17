#pragma once
#include"club.h"
class SportsClub :public Club
{
private:
	string sport;//运动项目，例如篮球、羽毛球
public:
	SportsClub(const string& id, const string& name, const string& intro, const string& leader, const string& sportName);
	string getSport()const;
	string getType()const;
	Club* clone()const;
	string getExtraInfo()const;
};
