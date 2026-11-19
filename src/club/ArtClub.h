#pragma once
#include"club.h"
/*
艺术社团，继承 Club。
artCategory 保存艺术类别，负责人和成员资料由基类维护。
getExtraInfo 提供分类资料，clone 用于复制完整的艺术社团。
*/
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