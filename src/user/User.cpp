#include<iostream>
#include"User.h"
#include"UserManager.h"
using namespace std;
User::User(string id,string name,string pwd,string rol):userId(id),userName(name),password(pwd),role(rol)
{
}
string User::getUserId()const
{
    return userId;
}
string User::getUserName()const
{
    return userName;
}
string User::getRole()const
{
    return role;
}
string User::getPassword()const{
    return password;
}
void User::setUserManager(UserManager* um){
    userManager=um;
}
//修改密码
void User::changePassword(string newpwd)
{
    password=newpwd;
    if(userManager!=nullptr){
        userManager->updateUser(this);
        //改完密码后立刻写回文件，避免退出后失效
    }
    cout<<"密码修改成功！"<<endl;
}
void User::showInfo()const
{
    cout<<"用户Id："<<userId<<endl;
    cout<<"用户名："<<userName<<endl;
    cout<<"用户角色："<<role<<endl;
}
User::~User()
{
}