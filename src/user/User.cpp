#include<iostream>
#include"User.h"
#include"UserManager.h"
#include"../club/ClubService.h"
#include"../common.h"
using namespace std;
/*
函数名：User（构造函数）
功能：用初始化列表设置用户的四项基本信息
参数：id 用户Id；name 用户名；pwd 密码；rol 角色
*/
User::User(string id,string name,string pwd,string rol):userId(id),userName(name),password(pwd),role(rol)
{
}
/*
函数名：getUserId
功能：返回用户Id
*/
string User::getUserId()const
{
    return userId;
}
/*
函数名：getUserName
功能：返回用户名
*/
string User::getUserName()const
{
    return userName;
}
/*
函数名：getRole
功能：返回角色字符串（Student / ClubManager / Admin）
*/
string User::getRole()const
{
    return role;
}
/*
函数名：getPassword
功能：返回密码，UserManager::updateUser() 把用户数据写回文件时需要用到
*/
string User::getPassword()const{
    return password;
}
/*
函数名：setUserManager
功能：保存创建本对象的 UserManager 指针，之后修改密码、兴趣标签时通过它写回文件
参数：um 用户管理器指针
*/
void User::setUserManager(UserManager* um){
    userManager=um;
}
/*
函数名：setClubService
功能：保存社团模块指针，登录成功后由 main 调用
参数：cs main 中创建的社团模块对象
*/
void User::setClubService(ClubService* cs){
    clubService=cs;
}
/*
函数名：setActivityManager
功能：保存活动模块指针，登录成功后由 main 调用
参数：am main 中创建的活动模块对象
*/
void User::setActivityManager(ActivityManager* am){
    activityManager=am;
}
/*
函数名：saveClubData
功能：把社团、社团成员、加入申请三份数据写回各自的文件
实现说明：
    社团模块的增删改只修改内存中的数据，不会自动保存，
    所以用户模块在申请加入、退出社团、审核、删除等操作成功后调用本函数。
    三个文件路径定义在 common.h 中。
*/
void User::saveClubData(){
    if(clubService==nullptr){
        return;
    }
    clubService->saveClubs(CLUB_FILE);
    clubService->saveMembers(MEMBER_FILE);
    clubService->saveApplications(APPLICATION_FILE);
}
/*
函数名：changePassword
功能：修改密码，并立即写回 users.txt
参数：newpwd 新密码（调用前已由 readField 检查过不含 | 和空格）
实现步骤：
    1. 修改对象中的 password；
    2. 调用 UserManager::updateUser() 把这个用户的最新数据写回文件，
       否则退出程序后修改会丢失。
*/
void User::changePassword(string newpwd)
{
    password=newpwd;
    if(userManager!=nullptr){
        userManager->updateUser(this);
    }
    cout<<"密码修改成功！"<<endl;
}
/*
函数名：showInfo
功能：显示用户Id、用户名和角色
实现说明：文件中角色存的是英文（Student 等），显示时转换成中文，和用户列表的显示保持一致
*/
void User::showInfo()const
{
    cout<<"用户Id："<<userId<<endl;
    cout<<"用户名："<<userName<<endl;
    string roleText=role;
    if(role==ROLE_STUDENT){
        roleText="普通学生";
    }
    else if(role==ROLE_CLUB_MANAGER){
        roleText="社团负责人";
    }
    else if(role==ROLE_ADMIN){
        roleText="系统管理员";
    }
    cout<<"用户角色："<<roleText<<endl;
}
/*
函数名：~User（虚析构函数）
功能：基类析构函数声明为虚函数，main 用 delete 释放 User* 指向的派生类对象时才能正确析构
*/
User::~User()
{
}
