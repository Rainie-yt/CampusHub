#include"SportsClub.h"
//先调用基类构造函数初始化共有资料，再保存运动项目
SportsClub::SportsClub(const string& id, const string& name, const string& intro, const string& leader, const string& sportName)
	:Club(id,name,intro,leader)
{
	sport = sportName;
}
// 返回本社团登记的运动项目。
string SportsClub::getSport()const
{
	return sport;
}
// 返回“体育社团”，用于列表显示与类型识别。
string SportsClub::getType()const
{
	return"体育社团";
}
// 复制当前对象，保留运动项目和基类成员资料。
Club* SportsClub::clone()const
{
	return new SportsClub(*this);
}
// 向统一的社团资料接口提供运动项目。
string SportsClub::getExtraInfo()const
{
	return sport;
}