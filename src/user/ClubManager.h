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
    void memberListMenu();
    //成员列表：按序号选择成员
    void memberActionMenu(const string& memberId);
    //对选中的成员进行操作：查看信息，移除成员
    void applicationMenu();
    //加入申请：按序号选择申请
    void applicantActionMenu(const string& applicantId);
    //对选中的申请操作：查看申请人信息、通过、拒绝
    string nameOf(const string& id);
    //按ID获取用户名，用于名单显示
    void publishActivity();
    void modifyActivity();
    void deleteActivity();
    void showActivityRegistration();
    void manageRegistration();
    string getClubId()const;
};