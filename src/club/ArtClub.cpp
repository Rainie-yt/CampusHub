#include"ArtClub.h"
// 先用公共资料初始化 Club，再保存艺术类别。
ArtClub::ArtClub(const string& id, const string& name, const string& intro, const string& leader, const string& category):Club(id,name,intro,leader)
{
	artCategory = category;
}
// 返回本社团登记的艺术类别。
string ArtClub::getArtCategory()const
{
	return artCategory;
}
// 返回“艺术社团”，用于列表显示与类型识别。
string ArtClub::getType()const
{ 
	return"艺术社团";
}
// 复制当前对象，保留艺术类别和基类成员资料。
Club* ArtClub::clone()const
{
	return new ArtClub(*this);
}
// 向统一的社团资料接口提供艺术类别。
string ArtClub::getExtraInfo()const
{
	return artCategory;
}