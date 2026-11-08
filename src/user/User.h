#pragma once
#include<string>
using namespace std;
//前向声明：这里只用到指针，不需要包含完整的头文件，可以减少头文件之间的相互依赖
class UserManager;
class ClubService;
class ActivityManager;
/*
类名：User（用户基类，抽象类）
继承关系：
    User 是用户模块所有角色的基类，派生关系为：
    User → Student（普通学生）→ ClubManager（社团负责人）
    User → Admin（系统管理员）
    User 含有纯虚函数 showMenu()，不能直接创建对象，只能创建派生类对象。
多态：
    main 登录成功后用 User* 指针接收 UserManager::login() 返回的对象，
    调用 showMenu() 时会根据对象的实际类型执行学生、负责人或管理员各自的菜单。
数据成员：
    userId、userName、password、role 为所有角色共有的账号信息；
    userManager、clubService、activityManager 是三个模块的指针，
    由 UserManager 和 main 在创建对象、登录成功后设置（注入），
    派生类通过它们读写用户文件、调用社团模块和活动模块的功能。
主要职责：保存账号信息，提供修改密码、显示个人信息等所有角色共用的功能。
*/
class User
{
    protected:
    string userId;    //用户Id，唯一标识一个用户
    string userName;  //用户名
    string password;  //密码
    string role;      //用户角色：Student 普通学生；ClubManager 社团负责人；Admin 系统管理员
    UserManager* userManager=nullptr;
    //创建本对象的用户管理器，修改密码、兴趣标签后通过它写回 users.txt
    ClubService* clubService=nullptr;
    //社团模块，由 main 创建后在登录成功时注入，所有角色共用同一个对象
    ActivityManager* activityManager=nullptr;
    //活动模块，由 main 创建后在登录成功时注入，所有角色共用同一个对象

    //功能：把社团、成员、加入申请三个文件一起写回，社团数据有变化后调用
    void saveClubData();

    public:
    //构造函数。参数：id 用户Id，name 用户名，pwd 密码，rol 角色
    User(string id,string name,string pwd,string rol);
    //功能：返回用户Id
    string getUserId()const;
    //功能：返回用户名
    string getUserName()const;
    //功能：返回角色（Student / ClubManager / Admin）
    string getRole()const;
    //功能：返回密码，供 UserManager 写回文件时使用
    string getPassword()const;
    //功能：设置用户管理器指针。参数：um 创建本对象的 UserManager
    void setUserManager(UserManager* um);
    //功能：设置社团模块指针。参数：cs main 中创建的 ClubService 对象
    void setClubService(ClubService* cs);
    //功能：设置活动模块指针。参数：am main 中创建的 ActivityManager 对象
    void setActivityManager(ActivityManager* am);
    //功能：修改密码并立即写回文件。参数：newpwd 新密码
    void changePassword(string newpwd);
    //功能：显示用户Id、用户名和中文角色名（虚函数，派生类可以扩展）
    virtual void showInfo()const;
    //功能：显示本角色的菜单并处理选择（纯虚函数，由各派生类实现，体现多态）
    virtual void showMenu()=0;
    //虚析构函数：通过 User* 删除派生类对象时，能正确调用派生类的析构函数
    virtual ~User();
};
