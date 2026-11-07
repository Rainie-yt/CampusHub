#include"Admin.h"
#include"../common/TextUtils.h"
#include<iostream>
#include"../common/InputUtils.h"
#include"../club/ClubService.h"
#include"../common.h"
#include"UserManager.h"
#include"../activity/ActivityManager.h"
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
        cout<<"2.社团管理"<<endl;
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
        cout<<"3.冻结/解冻用户账号"<<endl;
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
    if(userManager!=nullptr){
        userManager->showAllUsers();
    }
}
//先显示用户列表，再输入要操作的用户Id
//不能操作自己，找不到时返回空字符串
string Admin::selectUser(const string& prompt){
    if(userManager==nullptr){
        return "";
    }
    userManager->showAllUsers();
    cout<<endl;
    cout<<prompt;
    string targetId;
    cin>>targetId;
    if(targetId==userId){
        cout<<"不能对自己的账号进行此操作！"<<endl;
        return "";
    }
    if(userManager->getUserRole(targetId).empty()){
        cout<<"未找到该用户！"<<endl;
        return "";
    }
    return targetId;
}
void Admin::deleteUser(){
    string targetId=selectUser("请输入要删除的用户Id：");
    if(targetId.empty()){
        return;
    }
    string role=userManager->getUserRole(targetId);
    if(role==ROLE_ADMIN){
        cout<<"不能删除管理员账号！"<<endl;
        return;
    }
    //负责人要先删社团，否则社团会没有负责人
    if(role==ROLE_CLUB_MANAGER||(clubService!=nullptr&&!clubService->findClubByLeader(targetId).empty())){
        cout<<"该用户是社团负责人，请先删除其社团后再删除用户！"<<endl;
        return;
    }
    cout<<"确认删除 "<<targetId<<" "<<userManager->getUserNameById(targetId)<<" 吗？1.确认  0.取消"<<endl;
    cout<<"请选择：";
    if(readIntChoice()!=1){
        cout<<"已取消。"<<endl;
        return;
    }
    if(!userManager->deleteUser(targetId)){
        cout<<"删除失败！"<<endl;
        return;
    }
    //把他从所有社团的成员名单和加入申请里移除，分开计数
    int leftClubs=0;
    int cancelledApps=0;
    if(clubService!=nullptr){
        for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
            if(clubService->leaveClub(targetId,club.clubId)){
                leftClubs++;
            }
            //拒绝申请要以负责人身份操作
            if(clubService->rejectApplication(club.leaderId,club.clubId,targetId)){
                cancelledApps++;
            }
        }
        if(leftClubs>0||cancelledApps>0){
            saveClubData();
        }
    }
    cout<<"用户已删除！"<<endl;
    if(leftClubs>0){
        cout<<"已将其移出"<<leftClubs<<"个社团。"<<endl;
    }
    if(cancelledApps>0){
        cout<<"已撤销其"<<cancelledApps<<"条加入申请。"<<endl;
    }
    //取消他在未结束活动中的报名和候补，空出名额由候补自动递补
    //已结束活动的记录保留
    int cancelledActs=0;
    if(activityManager!=nullptr){
        for(const Activity* act:activityManager->getAllActivities()){
            if(activityManager->cancelActivity(targetId,act->getActivityId())){
                cancelledActs++;
            }
        }
    }
    if(cancelledActs>0){
        cout<<"已取消其"<<cancelledActs<<"个未结束活动的报名或候补。"<<endl;
    }
}
void Admin::freezeUser(){
    string targetId=selectUser("请输入要冻结或解冻的用户Id：");
    if(targetId.empty()){
        return;
    }
    string name=userManager->getUserNameById(targetId);
    //已冻结就问是否解冻，否则问是否冻结
    bool frozen=(userManager->getUserStatus(targetId)==USER_FROZEN);
    //管理员不能被冻结，但已冻结的仍可以解冻
    if(!frozen&&userManager->getUserRole(targetId)==ROLE_ADMIN){
        cout<<"不能冻结管理员账号！"<<endl;
        return;
    }
    if(frozen){
        cout<<"该账号目前已冻结，确认解冻 "<<targetId<<" "<<name<<" 吗？1.确认  0.取消"<<endl;
    }
    else{
        cout<<"确认冻结 "<<targetId<<" "<<name<<" 吗？冻结后该用户无法登录。1.确认  0.取消"<<endl;
    }
    cout<<"请选择：";
    if(readIntChoice()!=1){
        cout<<"已取消。"<<endl;
        return;
    }
    string newStatus=frozen?USER_NORMAL:USER_FROZEN;
    if(!userManager->setUserStatus(targetId,newStatus)){
        cout<<"操作失败！"<<endl;
        return;
    }
    cout<<(frozen?"已解冻该账号！":"已冻结该账号！")<<endl;
}
void Admin::modifyUserRole(){
    string targetId=selectUser("请输入要修改角色的用户Id：");
    if(targetId.empty()){
        return;
    }
    string oldRole=userManager->getUserRole(targetId);
    cout<<"1.普通学生  2.社团负责人  3.系统管理员  0.取消"<<endl;
    string newRole;
    //输错就重新选，选0取消
    while(true){
        cout<<"请选择新角色：";
        int choice=readIntChoice();
        if(choice==0){
            cout<<"已取消。"<<endl;
            return;
        }
        if(choice==1){
            newRole=ROLE_STUDENT;
            break;
        }
        if(choice==2){
            newRole=ROLE_CLUB_MANAGER;
            break;
        }
        if(choice==3){
            newRole=ROLE_ADMIN;
            break;
        }
        cout<<"无效的选择，请重新输入！"<<endl;
    }
    if(newRole==oldRole){
        cout<<"该用户已经是这个角色。"<<endl;
        return;
    }
    string clubId;
    if(clubService!=nullptr){
        clubId=clubService->findClubByLeader(targetId);
    }
    //负责着已跳过的社团，不能改成别的角色
    if(!clubId.empty()&&newRole!=ROLE_CLUB_MANAGER){
        cout<<"该用户是社团"<<clubId<<"的负责人，请先删除该社团再修改角色！"<<endl;
        return;
    }
    //没有已通过的社团，设成负责人也无社团可管
    if(clubId.empty()&&newRole==ROLE_CLUB_MANAGER){
        cout<<"该用户没有已通过审核的社团，不能设为社团负责人！"<<endl;
        return;
    }
    if(userManager->changeRole(targetId,newRole)){
        cout<<"角色修改成功！（该用户重新登录后生效）"<<endl;
    }
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
        cout<<"已拒绝该社团的申请！"<<endl;
        return;
    }
    //通过后把申请人改成负责人
    string leaderId=clubService->getClubLeaderId(clubId);
    if(userManager!=nullptr&&userManager->changeRole(leaderId,ROLE_CLUB_MANAGER)){
        cout<<"审核通过，"<<leaderId<<"已成为社团负责人！（重新登录后生效）"<<endl;
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
    //删除前先记下负责人，删掉之后就查不到了
    string leaderId=clubService->getClubLeaderId(clubId);
    if(clubService->deleteClub(clubId)){
        saveClubData();
        cout<<"社团已删除！"<<endl;
        //社团的活动一起删除，先记下编号再删，避免边遍历边删除
        if(activityManager!=nullptr){
            vector<string> activityIds;;
            for(const Activity* act:activityManager->getActivitiesByClub(clubId)){
                activityIds.push_back(act->getActivityId());
            }
            for(const string& id:activityIds){
                activityManager->removeActivity(id);
            }
            if(!activityIds.empty()){
                cout<<"已同时删除该社团的"<<activityIds.size()<<"个活动。"<<endl;
            }
        }
        //原来是负责人、且没有别的已通过社团，就改回普通学生
        if(userManager!=nullptr&&userManager->getUserRole(leaderId)==ROLE_CLUB_MANAGER&&clubService->findClubByLeader(leaderId).empty()){
            if(userManager->changeRole(leaderId,ROLE_STUDENT)){
                cout<<leaderId<<"已改回普通学生！（重新登录后生效）"<<endl;
            }
        }
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
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    vector<const Activity*> acts=activityManager->getAllActivities();
    cout<<endl;
    cout<<"========活动列表========"<<endl;
    if(acts.empty()){
        cout<<"暂无活动。"<<endl;
        return;
    }
    //名称列宽度取最长的"名称（类型）"再加2格，名称再长也能对齐
    int nameW=20;
    for(const Activity* act:acts){
        int w=displayWidth(act->getName()+"（"+act->getTypeName()+"）")+2;
        if(w>nameW){
            nameW=w;
        }
    }
    //每个活动一行：编号、名称、类型、所属社团、审核状态
    for(const Activity* act:acts){
        //按显示宽度补空格，让各列对齐
        cout<<padText(act->getActivityId(),6)<<padText(act->getName()+"（"+act->getTypeName()+"）",nameW)
            <<padText(act->getClubId(),6)<<Activity::statusText(act->getStatus())<<endl;
    }
    cout<<endl;
    cout<<"共"<<acts.size()<<"个活动"<<endl;
}
void Admin::auditActivity()
{
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    cout<<endl;
    activityManager->showPendingActivities();
    if(activityManager->getPendingActivities().empty()){
        return;
    }
    string activityId;
    cout<<endl;
    cout<<"请输入要审核的活动编号：";
    cin>>activityId;
    Activity* act=activityManager->findActivity(activityId);
    if(act==nullptr||act->getStatus()!=STATUS_PENDING){
        cout<<"审核失败：活动不存在或已经审核过。"<<endl;
        return;
    }
    cout<<endl;
    cout<<"========活动详情========"<<endl;
    act->showInfo();
    cout<<endl;
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
    activityManager->auditActivity(activityId,choice==1);
    cout<<(choice==1?"活动已通过审核！":"已拒绝该活动！")<<endl;
}
void Admin::removeActivity()
{
    viewActivityInfo();
    if(activityManager==nullptr||activityManager->getAllActivities().empty()){
        return;
    }
    string activityId;
    cout<<endl;
    cout<<"请输入要删除的活动编号：";
    cin>>activityId;
    if(activityManager->findActivity(activityId)==nullptr){
        cout<<"未找到该活动！"<<endl;
        return;
    }
    cout<<"确认删除该活动吗？报名记录会一起删除。1.确认  0.取消"<<endl;
    cout<<"请选择：";
    if(readIntChoice()!=1){
        cout<<"已取消。"<<endl;
        return;
    }   
    activityManager->removeActivity(activityId);
    cout<<"活动已删除！"<<endl;
}
void Admin::systemStatisticsMenu()
{
    while(true)
    {
        cout<<endl;
        cout<<"========数据统计========"<<endl;
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
                if(userManager!=nullptr){
                    cout<<endl;
                    cout<<"========用户统计========"<<endl;
                    userManager->showUserStatistics();
                }
                break;

            case 2:
                if(clubService!=nullptr){
                    cout<<endl;
                    cout<<"========社团统计========"<<endl;
                    cout<<"社团总数："<<clubService->getClubCount()<<endl;
                }
                break;

            case 3:
                if(activityManager!=nullptr){
                    cout<<endl;
                    cout<<"========活动统计========"<<endl;
                    cout<<"活动总数："<<activityManager->getAllActivities().size()<<endl;
                    cout<<"已通过："<<activityManager->getApprovedActivities().size()<<endl;
                    cout<<"待审核："<<activityManager->getPendingActivities().size()<<endl;
                }
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
