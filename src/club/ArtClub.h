#pragma once
#include"club.h"
class ArtClub :public Club
{
private:
	string artCategory;//艺术类别
public:
	ArtClub(const string& id, const string& name, const string& intro, const string& leader, const string& category);
	string getArtCategory()const;
	string getType()const;
	Club* clone()const;
	string getExtraInfo()const;
};