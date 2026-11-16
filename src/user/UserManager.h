#ifndef USERMANAGER_H
#define USERMANAGER_H
#include<string>
#include<vector>
#include"User.h"
using namespace std;
//账号状态，存在 users.txt 每行第 7 个字段
//旧数据没有这一栏时按 normal 处理
const string USER_NORMAL="normal";   //正常，可以登录
const string USER_FROZEN="frozen";   //已冻结，不能登录
/*
类名：UserManager（用户管理类）
继承关系：不继承其他类，也没有派生类。它负责创建和管理 User 体系的对象，
          与 User 之间是"使用"关系，而不是继承关系。
数据成员：userFile 用户数据文件的路径（users.txt）。
文件格式：users.txt 每行一个用户，字段之间用 | 分隔：
          用户Id|用户名|密码|角色|专业|兴趣标签|状态
          兴趣标签之间用英文逗号分隔；状态为 normal 或 frozen，旧数据可以没有这一栏。
主要职责：
    1. 用户登录、注册；
    2. 从 users.txt 中读取用户信息，根据角色创建 Student / ClubManager / Admin 对象，
       以 User* 返回给 main（利用多态，main 不需要知道具体是哪种用户）；
    3. 把用户修改后的数据（密码、兴趣标签、角色、状态）写回 users.txt；
    4. 为管理员提供用户列表、用户统计、冻结/解冻、删除用户等功能。
注意：UserManager 不负责具体角色的菜单功能，
      各角色的菜单分别由 Student::showMenu()、ClubManager::showMenu()、Admin::showMenu() 负责。
*/
class UserManager{
    public:
    //构造函数，设置用户数据文件路径
    UserManager();
    // 启动时只读校验账号格式和重复Id；现有文件出错时返回false，不修改文件。
    // 文件不存在时允许首次注册创建，旧六字段记录仍受支持。
    bool validateData();
    //功能：用户登录。返回：登录成功返回对应的用户对象指针，失败返回 nullptr
    User* login();
    //功能：用户注册（只能注册普通学生）。返回：注册成功返回 true，失败返回 false
    bool registerUser();
    // 首次升级时保留历史数据中已不存在的用户Id，防止旧注销账号被重新注册。
    bool reserveHistoricalIds(const ClubService& clubs,const ActivityManager& activities);
    //功能：把用户的最新数据写回 users.txt。参数：user 要保存的用户对象；返回：是否成功
    bool updateUser(User* user);
    //功能：修改指定用户的角色并写回文件，供管理员审核社团、修改角色时使用。参数：userId 用户Id，newRole 新角色
    bool changeRole(const string& userId,const string& newRole);
    //功能：按用户Id查用户名。参数：userId 用户Id；返回：用户名，找不到返回空字符串
    string getUserNameById(const string& userId);
    //功能：按用户Id显示用户完整信息（不显示密码）。参数：userId 用户Id；返回：找不到返回 false
    bool showUserInfoById(const string& userId);

    //-------管理员用户管理-------
    //功能：显示所有用户，每个用户一行：用户Id、用户名、角色、状态
    void showAllUsers();
    //功能：统计用户总数、各角色人数和冻结账号数量
    void showUserStatistics();
    //功能：查用户角色。参数：userId 用户Id；返回：角色，找不到返回空字符串（也用来判断用户是否存在）
    string getUserRole(const string& userId);
    //功能：查账号状态。参数：userId 用户Id；返回：normal 或 frozen，找不到返回空字符串
    string getUserStatus(const string& userId);
    //功能：冻结或解冻账号并写回文件。参数：userId 用户Id，status 新状态
    bool setUserStatus(const string& userId,const string& status);
    //功能：从 users.txt 删除该用户那一行。参数：userId 用户Id。社团和活动中的数据由调用者处理
    bool deleteUser(const string& userId);

    private:
    string retiredIdFile="data/retired_user_ids.txt";
    bool readRetiredIds(vector<string>& ids);
    string userFile;  //用户数据文件路径

    //功能：字符串分割。参数：str 要分割的字符串，delimiter 分隔符；返回：分割后的各段
    //例如："U001|张三|123456|Student|计算机科学|机器人,人工智能" 调用 split(str,'|')
    //      得到：U001  张三  123456  Student  计算机科学  机器人,人工智能
    vector<string> split(const string& str,char delimiter);
    //功能：把兴趣字符串转换成 vector<string>。参数：interestData 用逗号分隔的兴趣字符串
    //例如："机器人,人工智能" 转换成 interests[0]="机器人"，interests[1]="人工智能"
    vector<string> parseInterests(const string& interestData);
    //功能：根据 users.txt 中一行拆分出的字段创建对应的用户对象
    //      role 为 Student 时创建 Student，ClubManager 时创建 ClubManager，Admin 时创建 Admin
    //参数：fields 一行数据拆分后的字段；返回：User* 指针，利用多态保存不同的派生类对象
    User* createUser(const vector<string>& fields);
    //功能：检查用户Id是否已经存在，注册时使用。参数：userId 用户Id
    bool userExists(const string& userId);
    //功能：读出 users.txt 的所有非空行。参数：lines 用来存放读出的行；返回：文件打不开时返回 false
    bool readLines(vector<string>& lines);
    //功能：用 lines 整体覆盖写回 users.txt。参数：lines 所有用户行；返回：是否成功
    bool writeLines(const vector<string>& lines);
    //功能：取一行数据的账号状态。参数：fields 拆分后的字段；返回：没有第 7 个字段时返回 normal
    string statusOf(const vector<string>& fields);
    //功能：把英文角色转换成中文显示。参数：role 英文角色；返回：中文角色名
    string roleName(const string& role);
};
#endif
