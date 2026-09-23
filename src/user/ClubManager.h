#pragma once
#include"Student.h"
class ClubManager:public Student{
    private:
    string clubId;//管理的社团编号
    public:
    ClubManager(string id,string name,string pwd,string maj,string cid);
    void showMenu();
    //重写菜单，包含：Student的菜单+社团管理菜单
    void clubManageMenu();
    //社团管理入口
    void activityManageMenu();
    //活动管理入口
    //具体功能
    void showClubInfo();
    void modifyClubInfo();
    void manageMembers();
    void publishActivity();
    void modifyActivity();
    void deleteActivity();
    void showActivityRegistration();
    void manageRegistration();
    string getClubId()const;
};