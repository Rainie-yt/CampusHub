#include"SportsClub.h"
//先调用基类构造含函数初始化共有资料，再保存运动项目
SportsClub::SportsClub(const string& id, const string& name, const string& intro, const string& leader, const string& sportName)
	:Club(id,name,intro,leader)
{
	sport = sportName;
}
string SportsClub::getSport()const
{
	return sport;
}
string SportsClub::getType()const
{
	return"体育社团";
}
Club* SportsClub::clone()const
{
	return new SportsClub(*this);
}
string SportsClub::getExtraInfo()const
{
	return sport;
}