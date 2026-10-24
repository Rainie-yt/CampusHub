#pragma once
#include<string>
using namespace std;
class UserManager;
class ClubService;
//前向声明
class User
/*
    用户基类
    作用：保存所有用户共有的信息，包括
    用户ID、用户名、密码、用户角色
*/
{
    protected:
    string userId; //用户ID
    string userName; //用户名
    string password; //密码
    string role; //用户角色:Student:普通学生；ClubManager:拥有社团管理权限的学生；Admin:系统管理员
    UserManager* userManager=nullptr;
    //记录是谁创建了自己，方便回调写文件
    ClubService* clubService=nullptr;
    //社团数据，由main创建后注入
    void saveClubData();
    //把社团、成员、申请三个文件一起写回
    public:
    User(string id,string name,string pwd,string rol);
    string getUserId()const;
    string getUserName()const;
    string getRole()const;
    string getPassword()const;
    void setUserManager(UserManager* um);
    //供UserManager创建对象后“注入”自己
    void setClubService(ClubService* cs);
    //登录后由main注入社团数据
    void changePassword(string newpwd);
    //修改密码
    virtual void showInfo()const;
    //查看个人信息
    virtual void showMenu()=0;
    //虚函数，不同角色显示不同信息
    virtual ~User();
};