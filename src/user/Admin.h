#pragma once
#include"User.h"
class Admin:public User{
    public:
    Admin(string id,string name,string pwd);
    //管理员主菜单
    void showMenu();
    //子菜单
    void userManageMenu();
    //用户管理
    void clubManageMenu();
    //社团管理
    void activityManageMenu();
    //活动管理
    void systemStatisticsMenu();
    //系统统计
    //具体功能
    //用户管理
    void showAllUsers();
    void deleteUser();
    void freezeUser();
    void modifyUserRole();
    //社团管理
    void auditClub();
    void deleteClub();
    void viewClubInfo();
    //活动管理
    void auditActivity();
    void removeActivity();
    void viewActivityInfo();
    //提供管理员身份接口
    string getAdminId()const;
};