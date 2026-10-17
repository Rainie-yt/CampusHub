#include"ArtClub.h"
ArtClub::ArtClub(const string& id, const string& name, const string& intro, const string& leader, const string& category):Club(id,name,intro,leader)
{
	artCategory = category;
}
string ArtClub::getArtCategory()const
{
	return artCategory;
}
string ArtClub::getType()const
{ 
	return"艺术社团";
}
Club* ArtClub::clone()const
{
	return new ArtClub(*this);
}
string ArtClub::getExtraInfo()const
{
	return artCategory;
}