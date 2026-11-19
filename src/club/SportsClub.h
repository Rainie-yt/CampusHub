#pragma once
#include"club.h"
/*
体育社团，继承 Club。
共有资料交给基类管理，sport 保存运动项目。
getExtraInfo 提供运动项目，clone 保留体育社团的实际类型。
*/
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
