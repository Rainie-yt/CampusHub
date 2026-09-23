#include"ClubManager.h"
#include<iostream>
#include"src/common/InputUtils.h"
ClubManager::ClubManager(string id,string name,string pwd,string maj,string cid):Student(id,name,pwd,maj),clubId(cid)
{
    role="ClubManager";
}
void ClubManager::showMenu(){
    int choice=readIntChoice();
    while(true){
    cout<<endl;
    cout<<"欢迎您，"<<userName<<"！"<<endl;
    cout<<"角色：负责人"<<endl;
    cout<<endl;
    cout<<"========社团负责人菜单========"<<endl;
    cout<<"1.个人信息管理"<<endl;
    cout<<"2.浏览社团"<<endl;
    cout<<"3.活动报名"<<endl;
    cout<<"4.我的报名活动"<<endl;
    cout<<"5.兴趣推荐"<<endl;
    cout<<"6.社团管理"<<endl;
    cout<<"0.退出登录"<<endl;
    cout<<endl;
    cout<<"请选择：";
        switch(choice)
        {
        case 1:{
            showInfoMenu();
            break;
        }
        case 2:{
        //社团模块
        break;
        }
        case 3:{
            //活动模块
            break;
        }
        case 4:{
            showMyActivityMenu();
            break;
        }
        case 5:{
            //活动模块
            break;
        }
        case 6:{
            clubManageMenu();
            break;
        }
        case 0:{
            cout<<"退出登录"<<endl;
            return;
        }
        default:{
            cout<<"无效的选择，请重新输入！"<<endl;
        }
        }
}
}
void ClubManager::clubManageMenu(){
    int choice=readIntChoice();
    while(true)
    {
        cout<<"========社团管理========"<<endl;
        cout<<"1.查看我的社团"<<endl;
        cout<<"2.修改社团信息"<<endl;
        cout<<"3.管理社团成员"<<endl;
        cout<<"4.活动管理"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        switch(choice)
        {
        case 1:
            showClubInfo();
            break;
        case 2:
            modifyClubInfo();
            break;
        case 3:
            manageMembers();
            break;
        case 4:
            activityManageMenu();
            break;
        case 0:
            return;
        default:
            cout<<"输入错误！"<<endl;    
        }
    }

}
void ClubManager::activityManageMenu()
{
    int choice=readIntChoice();
    while(true)
    {
        cout<<"========活动管理========"<<endl;
        cout<<"1.发布活动"<<endl;
        cout<<"2.修改活动"<<endl;
        cout<<"3.删除活动"<<endl;
        cout<<"4.查看报名情况"<<endl;
        cout<<"5.管理报名成员"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        switch(choice)
        {
        case 1:
            publishActivity();
            break;
        case 2:{
            modifyActivity();
            break;
        }
        case 3:{
            deleteActivity();
            break;
        }    
        case 4:
            showActivityRegistration();
            break;
        case 5:{
            manageRegistration();
            break;
        }
        case 0:
            return;
        default:{
            cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
    }
}
void ClubManager::showClubInfo(){
    cout<<endl;
    cout<<"========我的社团========"<<endl;
    cout<<"社团编号："<<clubId<<endl;
    //需要对接社团模块，根据clubId获取社团名称、简介和成员数量
}
void ClubManager::modifyClubInfo(){
    //对接社团模块，修改社团名称、简介和招新信息
}
void ClubManager::manageMembers(){
    //对接社团模块，传递clubId实现查看成员、删除成员、审核加入申请
}
void ClubManager::publishActivity(){
    //对接活动模块，传递clubId创建活动名称、时间、地点、描述
}
void ClubManager::modifyActivity(){
    //对接活动模块，根据clubId查询本社团活动，选择具体的activityId后修改活动信息
}
void ClubManager::deleteActivity(){
    //对接活动模块，根据clubId查询本社团活动，选择具体activityId后删除活动
}
void ClubManager::showActivityRegistration(){
    //对接活动模块，根据activityId查询报名学生列表
}
void ClubManager::manageRegistration(){
    //对接活动模块，实现审核报名，通过、拒绝申请、查看报名名单
}
