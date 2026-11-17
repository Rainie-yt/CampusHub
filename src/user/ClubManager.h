#pragma once
#include"Student.h"
/*
类名：ClubManager（社团负责人）
继承关系：User → Student → ClubManager
    - 公有继承自 Student，负责人本身也是学生，可以直接使用学生的全部功能
      （个人信息、社团中心、活动报名、兴趣推荐），这些函数不用重新编写；
    - 在学生功能的基础上，增加"社团管理"菜单，用来管理自己负责的社团、成员和活动。
多态：重写 showMenu()，菜单在学生菜单的基础上多了"5.社团管理"。
数据成员：clubId 负责的社团编号，登录后在 showMenu() 中通过社团模块按负责人查出。
主要职责：查看和修改社团信息、管理成员（查看、移除）、审核加入申请、
          发布/修改/删除本社团活动、查看和管理活动报名。
*/
class ClubManager:public Student{
    private:
    string clubId;  //负责的社团编号，只有已审核通过的社团才有

    public:
    //构造函数。参数：id 用户Id，name 用户名，pwd 密码，maj 专业，cid 社团编号；角色设为 ClubManager
    ClubManager(string id,string name,string pwd,string maj,string cid);
    //功能：负责人主菜单（学生菜单 + 社团管理），重写基类函数
    void showMenu();
    //功能：社团管理菜单：查看社团、修改信息、管理成员、活动管理
    void clubManageMenu();
    //功能：活动管理菜单：发布、修改、删除活动，查看和管理报名
    void activityManageMenu();
    //功能：显示我负责的社团的详细资料
    void showClubInfo();
    //功能：修改社团名称和简介
    void modifyClubInfoMenu();
    //功能：成员管理菜单：成员列表、加入申请
    void manageMembersMenu();
    //功能：成员列表，按序号选择成员
    void memberListMenu();
    //功能：对选中的成员操作：查看完整信息、移除。参数：memberId 成员的用户Id
    void memberActionMenu(const string& memberId);
    //功能：加入申请列表，按序号选择申请人
    void applicationMenu();
    //功能：对选中的申请操作：查看信息、通过、拒绝。参数：applicantId 申请人的用户Id
    void applicantActionMenu(const string& applicantId);
    //功能：按用户Id查用户名，用于名单显示。参数：id 用户Id；返回：用户名，找不到返回空字符串
    string nameOf(const string& id);
    //功能：发布本社团的新活动
    void publishActivityMenu();
    //功能：修改本社团的活动
    void modifyActivityMenu();
    //功能：删除本社团的活动
    void deleteActivityMenu();
    //功能：查看本社团某个活动的报名情况
    void showActivityRegistrationMenu();
    //功能：管理本社团某个活动的报名成员（移除）
    void manageRegistrationMenu();
    //功能：显示本社团活动并输入编号。prompt 为输入提示，approvedOnly 为 true 时只允许选择已通过的活动。
    //返回：符合要求的活动编号；没有可选活动、编号不存在或不符合要求时返回空字符串。
    string selectMyActivity(const string& prompt, bool approvedOnly=false);
};
