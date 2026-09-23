#include"Student.h"
#include<iostream>
#include<algorithm>
#include"src/common/InputUtils.h"
#include"UserManager.h"
Student::Student(string id,string name,string pwd,string maj):User(id,name,pwd,"Student"),major(maj)
{
}
string Student::getMajor()const{
    return major;
}
void Student::showMenu(){
    int choice=readIntChoice();
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：普通学生"<<endl;
        cout<<endl;
        cout<<"========学生菜单========"<<endl;
        cout<<"1.个人信息管理"<<endl;
        cout<<"2.浏览社团"<<endl;
        cout<<"3.活动报名"<<endl;
        cout<<"4.我的报名活动"<<endl;
        cout<<"5.兴趣推荐"<<endl;
        cout<<"0.退出登录"<<endl;
        cout<<endl;
        cout<<"请选择：";
        switch(choice){
            case 1:
                showInfoMenu();
                break;
                //包括：1.查看个人信息；2.修改密码；3.兴趣标签管理
            case 2:
                //浏览社团，属于社团模块
                break;
            case 3:
                //活动报名，属于活动模块
                break;
            case 4:
                showMyActivityMenu();
                //活动模块负责提供我的报名活动信息
                break;
            case 5:
                //兴趣推荐，活动模块负责提供活动标签
                break;
            case 0:
                cout<<"退出登录"<<endl;
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void Student::showInfoMenu(){
    int choice=readIntChoice();
    while(true){
    cout<<endl;
    cout<<"========个人信息管理========"<<endl;
    showInfo();
    cout<<"专业："<<major<<endl;
    cout<<"兴趣标签：";
    for(const auto& interest:interests){
        cout<<interest<<" ";
    }
    cout<<endl;
    cout<<"1.修改密码"<<endl;
    cout<<"2.管理兴趣标签"<<endl;
    cout<<"0.返回上级菜单"<<endl;
    cout<<endl;
    cout<<"请选择：";
    switch(choice){
        case 1:
            {
                string newpwd;
                cout<<"请输入新密码：";
                cin>>newpwd;
                changePassword(newpwd);
            }
            break;
        case 2:
            {
                modifyInterest();
            }
            break;
        case 0:
            return;
        default:
            cout<<"无效的选择，请重新输入！"<<endl;
    }
    }
}
void Student::modifyInterest(){
    int choice=readIntChoice();
    while(true){
        cout<<endl;
        cout<<"========兴趣标签管理========"<<endl;
        cout<<"当前兴趣标签：";
        showInterests();
        cout<<endl;
        cout<<"1.添加兴趣标签"<<endl;
        cout<<"2.删除兴趣标签"<<endl;
        cout<<"0.返回上级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        switch(choice){
            case 1:
                {
                    string newInterest;
                    cout<<"请输入要添加的兴趣标签：";
                    cin>>newInterest;
                    addInterest(newInterest);
                }
                break;
            case 2:
                {
                    string delInterest;
                    cout<<"请输入要删除的兴趣标签：";
                    cin>>delInterest;
                    removeInterest(delInterest);
                }
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void Student::addInterest(string interest)
{
    if(std::find(interests.begin(),
            interests.end(),
            interest)
       != interests.end())
    {
        cout<<"该兴趣标签已经存在！"<<endl;
        return;
    }
    interests.push_back(interest);
    if(userManager!=nullptr){
        userManager->updateUser(this);
        //改完兴趣标签立刻持久化
    }
    cout<<"兴趣标签添加成功！"<<endl;
}
void Student::loadInterest(string interest){
    interests.push_back(interest);
}
//从用户数据文件加载已有兴趣标签
//该函数只负责恢复数据，不向用户显示操作提示
void Student::removeInterest(string interest){
    auto it=std::find(interests.begin(),interests.end(),interest);
    if(it!=interests.end()){
        interests.erase(it);
        if(userManager!=nullptr){
            userManager->updateUser(this);
        }
        cout<<"兴趣标签删除成功！"<<endl;
    }else{
        cout<<"兴趣标签不存在！"<<endl;
    }
}
void Student::showInterests()const{
    if(interests.empty()){
        cout<<"无兴趣标签"<<endl;
    }else{
        for(const auto& interest:interests){
            cout<<interest<<" ";
        }
        cout<<endl;
    }
}
vector<string> Student::getInterests()const{
    return interests;
}
void Student::showMyActivityMenu()
{
    // TODO：后续对接活动模块
}