#pragma once
#include"club.h"
/*
学术社团，继承 Club。
researchDirection 保存研究方向，成员和申请沿用基类的名单。
通过 getExtraInfo 读取研究方向，复制时保留学术社团类型。
*/
class AcademicClub :public Club
{
private:
	string researchDirection;//研究方向
public:
	AcademicClub(const string& id, const string& name, const string& intro, const string& leader, const string& direction);
	string getResearchDirection()const;
	string getType()const;
	Club* clone()const;
	string getExtraInfo()const;
};
