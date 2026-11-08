#pragma once
#include"User.h"
#include<vector>
/*
类名：Student（普通学生）
继承关系：User → Student → ClubManager
    - 公有继承自用户基类 User，继承了用户Id、用户名、密码、角色，
      以及 UserManager、ClubService、ActivityManager 三个模块的指针；
    - 派生出社团负责人类 ClubManager，负责人可以使用学生的全部功能，
      再额外拥有"社团管理"菜单。
多态：重写基类的纯虚函数 showMenu()，登录后 main 通过 User* 指针调用，
      实际执行的是学生菜单。
数据成员：major 专业；interests 兴趣标签列表。
主要职责：个人信息与兴趣标签管理、社团中心（浏览、搜索、申请加入、申请成立社团）、
          活动报名与取消、按兴趣推荐社团和活动。
*/
class Student:public User{
    protected:
    string major;              //专业
    vector<string> interests;  //兴趣标签，用于推荐社团和活动；写回文件时用逗号连接

    public:
    //构造函数。参数：id 用户Id，name 用户名，pwd 密码，maj 专业；角色固定为 Student
    Student(string id,string name,string pwd,string maj);
    //功能：返回专业，写回文件时使用
    string getMajor()const;
    //功能：显示学生主菜单并循环处理选择，选 0 退出登录（重写基类纯虚函数）
    void showMenu();
    //功能：个人信息管理菜单，显示个人信息，可修改密码、管理兴趣标签
    void showInfoMenu();
    //功能：我的报名活动菜单，显示已报名和候补的活动、积分，可取消报名
    void showMyActivityMenu();
    //功能：活动报名，浏览已通过且未结束的活动，查看详情后确认报名
    void joinActivityMenu();
    //功能：显示我加入的社团，每个社团一行
    void showMyClubList()const;
    //功能：显示我提交的、还在待审核的加入申请，每个社团一行
    void showMyApplicationList()const;
    //功能：添加一个兴趣标签并写回文件。参数：interest 要添加的标签
    void addInterest(string interest);
    //功能：从 users.txt 恢复兴趣标签，只加入列表、不提示、不写文件。参数：interest 标签
    void loadInterest(string interest);
    //功能：删除一个兴趣标签并写回文件。参数：interest 要删除的标签
    void removeInterest(string interest);
    //功能：兴趣标签管理菜单（添加、删除）
    void modifyInterest();
    //功能：显示兴趣标签，标签之间用顿号隔开，没有时显示"无"
    void showInterests()const;
    //功能：返回兴趣标签列表，写回文件时使用
    vector<string> getInterests()const;
    //功能：按兴趣标签推荐社团和活动
    void recommendClubs()const;
    //功能：社团中心菜单：浏览、搜索、查看详情、申请加入、我的申请、我的社团、退出、申请成立
    void clubMenu();
    //功能：申请成立社团，选择类型并填写资料，提交后等待管理员审核
    void applyCreateClub();
    //功能：浏览已审核通过的社团，每个社团一行
    void browseClubs()const;
    //功能：按名称关键词搜索已通过的社团。参数：keyword 关键词
    void searchClubs(const string& keyword)const;
    //功能：显示社团详情（只能查看已通过的社团）。参数：clubId 社团编号
    void showClubDetail(const string& clubId)const;

    protected:
    //功能：只输出社团资料、不带标题，学生的社团详情和负责人的"我的社团"共用。参数：clubId 社团编号
    void printClubDetail(const string& clubId)const;
};
