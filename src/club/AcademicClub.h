#pragma once
#include"club.h"
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
