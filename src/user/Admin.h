#pragma once
#include"User.h"
/*
类名：Admin（系统管理员）
继承关系：User → Admin
    - 公有继承自用户基类 User，继承了用户Id、用户名、密码、角色，
      以及 UserManager、ClubService、ActivityManager 三个模块的指针；
    - 与 Student 是同一层的兄弟类，管理员不需要专业和兴趣标签，所以不从 Student 派生。
多态：重写基类的纯虚函数 showMenu()，登录后 main 通过 User* 指针调用，
      实际执行的是管理员菜单。
数据成员：没有新增数据成员，全部使用从 User 继承来的成员。
主要职责：用户管理（查看、删除、冻结/解冻、修改角色）、社团管理（查看、审核、删除）、
          活动管理（查看、审核、删除）、数据统计（用户、社团、活动数量和社团活跃度排行）。
*/
class Admin:public User{
    public:
    //构造函数。参数：id 用户Id，name 用户名，pwd 密码；角色固定为 Admin
    Admin(string id,string name,string pwd);
    //功能：管理员主菜单，循环处理选择，选 0 退出登录（重写基类纯虚函数）
    void showMenu();

    //一级子菜单
    //功能：用户管理菜单：查看、删除、冻结/解冻、修改角色
    void userManageMenu();
    //功能：社团管理菜单：查看、审核、删除
    void clubManageMenu();
    //功能：活动管理菜单：查看、审核、删除
    void activityManageMenu();
    //功能：数据统计菜单：用户、社团、活动数量统计和社团活跃度排行
    void systemStatisticsMenu();

    //用户管理
    //功能：显示所有用户（Id、用户名、角色、状态）
    void showAllUsers();
    //功能：删除用户，同时把他移出社团、撤销加入申请、取消未结束活动的报名
    void deleteUser();
    //功能：冻结或解冻用户账号，冻结后该用户无法登录
    void freezeUser();
    //功能：修改用户角色（普通学生 / 社团负责人 / 系统管理员）
    void modifyUserRole();
    //功能：显示用户列表并输入用户Id。参数：prompt 输入提示；返回：用户Id，选了自己或找不到时返回空字符串
    string selectUser(const string& prompt);

    //社团管理
    //功能：审核社团成立申请，通过后把申请人改为社团负责人
    void auditClub();
    //功能：删除违规社团，同时删除该社团的活动，负责人改回普通学生
    void deleteClub();
    //功能：查看所有社团
    void viewClubInfo();

    //活动管理
    //功能：审核待审核的活动（先显示详情再选择通过或拒绝）
    void auditActivity();
    //功能：删除违规活动，报名记录一起删除
    void removeActivity();
    //功能：查看所有活动，每个活动一行（编号、名称和类型、所属社团、审核状态）
    void viewActivityInfo();

    //功能：返回管理员的用户Id
    string getAdminId()const;
};
