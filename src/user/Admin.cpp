#include"Admin.h"
#include<iostream>
#include"../common/InputUtils.h"
#include"../club/ClubService.h"
#include"../common.h"
#include"UserManager.h"
Admin::Admin(string id,string name,string pwd):User(id,name,pwd,"Admin")
{
}
void Admin::showMenu(){
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：系统管理员"<<endl;
        cout<<endl;
        cout<<"========管理员菜单========"<<endl;
        cout<<"1.用户管理"<<endl;
        cout<<"2.社团审核"<<endl;
        cout<<"3.活动管理"<<endl;
        cout<<"4.数据统计"<<endl;
        cout<<"0.退出登录"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:{
                userManageMenu();
                break;
            }
            case 2:{
                clubManageMenu();
                break;
            }
            case 3:{
                activityManageMenu();
                break;
            }
            case 4:{
                systemStatisticsMenu();
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
//用户管理子菜单
void Admin::userManageMenu(){
    while(true){
        cout<<endl;
        cout<<"========用户管理========"<<endl;
        cout<<"1.查看所有用户"<<endl;
        cout<<"2.删除用户"<<endl;
        cout<<"3.冻结用户账号"<<endl;
        cout<<"4.修改用户角色"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:{
                showAllUsers();
                break;
            }
            case 2:{
                deleteUser();
                break;
            }
            case 3:{
                freezeUser();
                break;
            }
            case 4:{
                modifyUserRole();
                break;
            }
            case 0:{
                return;
            }
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void Admin::showAllUsers(){
    //用户模块内部功能，调用UserManager.showUsers(),显示用户ID、用户名、用户角色
}
void Admin::deleteUser(){
    //用户模块内部功能，调用UserManager.deleteUser(userId)，根据用户ID删除账号
}
void Admin::freezeUser(){
    //用户和模块内部功能，调用UserManager.freezeUser(userId),修改用户状态：正常、冻结
}
void Admin::modifyUserRole(){
    //用户模块内部功能，调用UserManager.changeRole()，修改角色
}
//社团管理：需要调用社团模块
void Admin::clubManageMenu(){
    if(clubService==nullptr){
        cout<<"社团数据未加载！"<<endl;
        return;
    }
    while(true){
        cout<<endl;
        cout<<"========社团管理========"<<endl;
        cout<<"1.查看社团信息"<<endl;
        cout<<"2.审核社团申请"<<endl;
        cout<<"3.删除违规社团"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:{
                viewClubInfo();
                break;
            }
            case 2:{
                auditClub();
                break;
            }
            case 3:{
                deleteClub();
                break;
            }
            case 0:{
                return;
            }
            default:{
                cout<<"无效的选择，请重新输入！"<<endl;
            }
        }
    }
}
void Admin::viewClubInfo(){
    clubService->showAllClubs();
}
void Admin::auditClub(){
    clubService->showAllClubs();
    string clubId;
    cout<<endl;
    cout<<"请输入要审核的社团编号：";
    cin>>clubId;
    int choice;
    //输错就重新选，选0可以随时取消
    while(true){
        cout<<"1.通过  2.拒绝  0.取消"<<endl;
        cout<<"请选择：";
        choice=readIntChoice();
        if(choice==0){
            cout<<"已取消审核。"<<endl;
            return;
        }
        if(choice==1||choice==2){
            break;
        }
        cout<<"无效的选择，请重新输入！"<<endl;
    }
    bool passed=(choice==1);
    if(!clubService->auditClub(clubId,passed)){
        cout<<"审核失败：社团不存在或已经审核过。"<<endl;
        return;
    }
    saveClubData();
    if(!passed){
        cout<<"已拒绝该社团的申请。"<<endl;
        return;
    }
    //通过后把申请人改成负责人
    string leaderId=clubService->getClubLeaderId(clubId);
    if(userManager!=nullptr&&userManager->changeRole(leaderId,ROLE_CLUB_MANAGER)){
        cout<<"审核通过，"<<leaderId<<"已成为社团负责人（重新登录后生效）。"<<endl;
    }
    else{
        cout<<"社团已通过，但修改负责人角色失败，请检查用户数据。"<<endl;
    }
}
void Admin::deleteClub()
{
    clubService->showAllClubs();
    string clubId;
    cout<<endl;
    cout<<"请输入要删除的社团编号：";
    cin>>clubId;
    if(clubService->deleteClub(clubId)){
        saveClubData();
        cout<<"社团已删除。"<<endl;
    }
    else{
        cout<<"删除失败：社团不存在，或社团还有成员。"<<endl;
    }
}
/*
    活动管理

    需要调用活动模块

*/
void Admin::activityManageMenu()
{
    while(true)
    {
        cout<<endl;
        cout<<"========活动管理========"<<endl;
        cout<<"1.查看活动信息"<<endl;
        cout<<"2.审核活动"<<endl;
        cout<<"3.删除违规活动"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice)
        {
            case 1:
                viewActivityInfo();
                break;
            case 2:
                auditActivity();
                break;
            case 3:
                removeActivity();
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void Admin::viewActivityInfo()
{
    cout<<"进入查看活动信息..."<<endl;
    /*
        对接活动模块
        调用：ActivitySystem.showAllActivity()
        获取：
        活动编号
        活动名称
        发布社团
        活动状态
    */
}
void Admin::auditActivity()
{
    cout<<"进入活动审核..."<<endl;
    /*
        对接活动模块
        调用：
        ActivitySystem.auditActivity(activityId)
        审核活动：
        通过
        驳回
    */

}
void Admin::removeActivity()
{
    cout<<"进入删除活动..."<<endl;
    /*
        【对接活动模块】
        调用：
        ActivitySystem.deleteActivity(activityId)
        删除违规活动
    */
}
void Admin::systemStatisticsMenu()
{
    while(true)
    {
        cout<<endl;
        cout<<"========系统统计========"<<endl;
        cout<<"1.用户数量统计"<<endl;
        cout<<"2.社团数量统计"<<endl;
        cout<<"3.活动数量统计"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch (choice)
        {
            case 1:
                // 对接用户模块
                // 获取系统用户数量
                break;

            case 2:
                if(clubService!=nullptr){
                    cout<<"社团总数："<<clubService->getClubCount()<<endl;
                }
                break;

            case 3:
                // 对接活动模块
                // 获取系统活动数量
                break;

            case 0:
                return;

            default:
                cout << "无效的选择，请重新输入！" << endl;
        }
    }
}
string Admin::getAdminId() const
{
    return userId;
}