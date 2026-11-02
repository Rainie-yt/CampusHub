//登录、注册、读取用户文件、创建用户对象
#ifndef USERMANAGER_H
#define USERMANAGER_H
#include<string>
#include<vector>
#include"User.h"
using namespace std;
//账号状态，存在users.txt每行第7个字段
//旧数据没有这一栏时按normal处理
const string USER_NORMAL="normal";
const string USER_FROZEN="frozen";
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
    //-------管理员用户管理-------
    void showAllUsers();
    //每个用户一行：用户Id、用户名、角色、状态
    void showUserStatistics();
    //用户总数及各角色、冻结账号数量
    string getUserRole(const string& userId);
    //查角色，找不到返回空字符串
    string getUserStatus(const string& userId);
    //查账号状态，找不到返回空字符串
    bool setUserStatus(const string& userId,const string& status);
    //冻结或解冻，写回文件
    bool deleteUser(const string& userId);
    //从users.txt删除该用户那一行，社团关系由调用者先处理
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
    bool readLines(vector<string>& lines);
    //读出users.txt所有非空行
    bool writeLines(const vector<string>& lines);
    //整体覆盖写回users.txt
    string statusOf(const vector<string>& fields);
    //取一行的状态，没有第7个字段时返回normal
    string roleName(const string& role);
    //角色转成中文显示
};
#endif