//登录、注册、读取用户文件、创建用户对象
#ifndef USERMANAGER_H
#define USERMANAGER_H
#include<string>
#include<vector>
#include"User.h"
using namespace std;
/*
    UserManager:用户管理类
    主要负责：
    1.用户登录
    2.用户注册
    3.从users.txt中读取用户信息
    4.根据用户角色创建对应的用户对象
    5.检查用户账号是否已经存在
    注意：
    UserManager不负责具体角色的菜单功能
    例如：
    Student的菜单由Student::showMenu()负责；
    ClubManager的菜单由ClubManager::showMenu()负责；
    Admin的菜单由Admin::showMenu()负责。
*/
class UserManager{
    public:
    UserManager();
    User* login();
    //用户登录
    //登录成功：返回对应的User对象
    //登录失败：返回nullptr
    bool registerUser();
    //用户注册
    //注册成功返回true，失败返回false
    bool updateUser(User* user);
    //把user最新数据写回users.txt
    bool changeRole(const string& userId,const string& newRole);
    //修改指定用户的角色并写回文件，供管理员审核社团时使用
    string getUserNameById(const string& userId);
    //按ID查用户名，找不到返回空字符串
    bool showUserInfoById(const string& userId);
    //按ID显示用户完整信息（不含密码），找不到返回false
    private:
    //用户数据文件路径
    string userFile;
    vector<string> split(const string& str,char delimiter);
    /*
        字符串分割函数
        例如： "U001|张三|123456|Student|计算机科学|机器人,人工智能"
        调用split(str,'|')
        得到：U001 张三 123456 Student 计算机科学 机器人，人工智能
    */
    vector<string> parseInterests(const string& interestData);
    /*
        将兴趣字符串转换为vector<string>
        例如：“机器人，人工智能”转换成 interests[0] = "机器人" interests[1] = "人工智能"
    */
    User* createUser(const vector<string>& fields);
    /*
        根据users.txt中的一行数据创建对应的用户对象
        如果role==Student:创建Student对象
        如果role==ClubManager:创建ClubManager对象
        如果role==Admin:创建Admin对象
        返回User*，利用多态保存不同的派生类对象
    */
    bool userExists(const string& userId);
};
#endif