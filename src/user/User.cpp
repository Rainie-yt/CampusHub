#include<iostream>
#include"User.h"
#include"UserManager.h"
#include"../club/ClubService.h"
#include"../common.h"
#include"../common/FileStore.h"
#include"../activity/ActivityManager.h"
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
bool User::saveClubData(){
    if(clubService==nullptr) return false;
    if(FileStore::Batch::current()){
        return clubService->saveClubs(CLUB_FILE)
            && clubService->saveMembers(MEMBER_FILE)
            && clubService->saveApplications(APPLICATION_FILE);
    }
    FileStore::Batch batch;
    return clubService->saveClubs(CLUB_FILE)
        && clubService->saveMembers(MEMBER_FILE)
        && clubService->saveApplications(APPLICATION_FILE) && batch.finish();
}

/*
函数名：persistChange
功能：将一次跨模块操作的文件统一提交，失败时恢复社团和活动的内存数据
参数：change 执行业务修改并提交待保存内容的函数；成功返回 true，失败返回 false
返回值：业务操作和文件提交均成功时返回 true，否则返回 false。
实现步骤：
    1. 如果外层已有批量保存操作，直接执行 change，由外层统一提交和恢复；
    2. 备份社团和活动数据，再建立文件批次，执行 change；
    3. 操作成功后提交文件；操作或提交失败时，交换备份以恢复内存。
关键点：change 返回 false 时不调用 finish，批次中的待写内容随对象析构而丢弃。
*/
bool User::persistChange(const std::function<bool()>& change){
    if(FileStore::Batch::current()){
        return change();
    }
    ClubService oldClubs;
    ActivityManager oldActivities;
    if(clubService){
        clubService->copyStateTo(oldClubs);
    }
    if(activityManager){
        activityManager->copyStateTo(oldActivities);
    }
    FileStore::Batch batch;
    bool changed=change();
    if(changed && batch.finish()){
        return true;
    }
    if(clubService){
        clubService->swapState(oldClubs);
    }
    if(activityManager){
        activityManager->swapState(oldActivities);
    }
    if(changed){
        cout<<"保存失败，本次操作已撤销，原数据保持不变。请检查文件是否只读、被占用或磁盘空间不足。"<<endl;
    }
    return false;
}
/*
函数名：changePassword
功能：修改密码，并立即写回 users.txt
参数：newpwd 新密码（调用前已由 readField 检查过不含 | 和空格）
实现步骤：
    1. 修改对象中的 password；
    2. 调用 UserManager::updateUser() 把这个用户的最新数据写回文件，
       否则退出程序后修改会丢失；写回失败要把密码改回原值并提示失败，不能提示成功。
*/
bool User::changePassword(string newpwd)
{
    string oldpwd=password;
    password=newpwd;
    //只有写回文件成功才算修改成功；失败时把密码改回去，
    //否则本次运行里是新密码、文件里是旧密码，下次登录就对不上了
    if(userManager==nullptr||!userManager->updateUser(this)){
        password=oldpwd;
        cout<<"密码修改失败，密码未更改，请稍后重试！"<<endl;
        return false;
    }
    cout<<"密码修改成功！"<<endl;
    return true;
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
