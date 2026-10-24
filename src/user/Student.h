#pragma once
#include"User.h"
#include<vector>
class Student:public User{
    protected:
    string major; //专业
    vector<string> interests;//兴趣标签
    public:
    Student(string id,string name,string pwd,string maj);
    string getMajor()const;
    //写回文件需要读专业信息
    void showMenu();
    void showInfoMenu();
    //个人信息菜单
    void showMyActivityMenu();
    //我的报名活动
    void addInterest(string interest);
    void loadInterest(string interest);
    //程序从users.txt恢复已有兴趣，只加载不提示
    void removeInterest(string interest);
    void modifyInterest();
    void showInterests()const;
    vector<string> getInterests()const;
    void recommendClubs()const;
    void clubMenu();
    //社团中心：浏览、申请加入、申请成立等
    void applyCreateClub();
    //申请成立社团
};
