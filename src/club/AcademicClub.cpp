#include"AcademicClub.h"
AcademicClub::AcademicClub(const string& id, const string& name, const string& intro, const string& leader, const string& direction):Club(id,name,intro,leader)
{
	researchDirection = direction;
}
string AcademicClub:: getResearchDirection()const
{
	return researchDirection;
}
string AcademicClub::getType()const
{
	return"学术社团";
}
Club* AcademicClub::clone()const
{
	return new AcademicClub(*this);
}
string AcademicClub::getExtraInfo()const
{
	return researchDirection;
}