#include"AcademicClub.h"
// 先用公共资料初始化 Club，再保存研究方向。
AcademicClub::AcademicClub(const string& id, const string& name, const string& intro, const string& leader, const string& direction):Club(id,name,intro,leader)
{
	researchDirection = direction;
}
// 返回本社团登记的研究方向。
string AcademicClub:: getResearchDirection()const
{
	return researchDirection;
}
// 返回“学术社团”，用于列表显示与类型识别。
string AcademicClub::getType()const
{
	return"学术社团";
}
// 复制当前对象，保留研究方向和基类成员资料。
Club* AcademicClub::clone()const
{
	return new AcademicClub(*this);
}
// 向统一的社团资料接口提供研究方向。
string AcademicClub::getExtraInfo()const
{
	return researchDirection;
}