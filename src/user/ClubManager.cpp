#include"ClubManager.h"
#include<iostream>
#include"../common/InputUtils.h"
#include"../club/ClubService.h"
#include"UserManager.h"
ClubManager::ClubManager(string id,string name,string pwd,string maj,string cid):Student(id,name,pwd,maj),clubId(cid)
{
    role="ClubManager";
}
void ClubManager::showMenu(){
    //登录后按负责人编号查出自己管理的社团
    if(clubService!=nullptr){
        clubId=clubService->findClubByLeader(userId);
    }
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：负责人"<<endl;
        cout<<endl;
        cout<<"========社团负责人菜单========"<<endl;
        cout<<"1.个人信息管理"<<endl;
        cout<<"2.社团中心"<<endl;
        cout<<"3.活动报名"<<endl;
        cout<<"4.我的报名活动"<<endl;
        cout<<"5.兴趣推荐"<<endl;
        cout<<"6.社团管理"<<endl;
        cout<<"0.退出登录"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice)
        {
            case 1:{
                showInfoMenu();
                break;
            }
            case 2:{
                clubMenu();
                break;
            }
            case 3:{
                //活动模块
                cout<<"该功能等待活动模块完成。"<<endl;
                break;
            }
            case 4:{
                showMyActivityMenu();
                break;
            }
            case 5:{
                recommendClubs();
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
    if(clubService==nullptr){
        cout<<"社团数据未加载！"<<endl;
        return;
    }
    if(clubId.empty()){
        cout<<"你目前没有已通过审核的社团。"<<endl;
        return;
    }
    while(true)
    {
        cout<<endl;
        cout<<"========社团管理========"<<endl;
        cout<<"1.查看我的社团"<<endl;
        cout<<"2.修改社团信息"<<endl;
        cout<<"3.管理社团成员"<<endl;
        cout<<"4.活动管理"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
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
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }

}
void ClubManager::activityManageMenu()
{
    while(true)
    {
        cout<<endl;
        cout<<"========活动管理========"<<endl;
        cout<<"1.发布活动"<<endl;
        cout<<"2.修改活动"<<endl;
        cout<<"3.删除活动"<<endl;
        cout<<"4.查看报名情况"<<endl;
        cout<<"5.管理报名成员"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
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
    printClubDetail(clubId);
}
void ClubManager::modifyClubInfo(){
    cout<<"请输入新的社团名称：";
    string newName=readLine();
    cout<<"请输入新的社团简介：";
    string newDescription=readLine();
    if(clubService->modifyClubInfo(userId,clubId,newName,newDescription)){
        saveClubData();
        cout<<"社团信息修改成功！"<<endl;
    }
    else{
        cout<<"修改失败：名称不能为空，且名称和简介不能换行。"<<endl;
    }
}
void ClubManager::manageMembers(){
    while(true){
        cout<<endl;
        cout<<"========成员管理========"<<endl;
        cout<<"1.查看成员列表"<<endl;
        cout<<"2.查看加入申请"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                memberListMenu();
                break;
            case 2:
                applicationMenu();
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void ClubManager::memberListMenu(){
    while(true){
        //每次循环重新取名单，移除后能看到最新结果
        vector<string> memberIds=clubService->getClubMemberIds(clubId);
        cout<<endl;
        cout<<"========成员列表========"<<endl;
        if(memberIds.empty()){
            cout<<"暂无成员。"<<endl;
            return;
        }
        for(size_t i=0;i<memberIds.size();i++){
            cout<<i+1<<". "<<memberIds[i]<<" "<<nameOf(memberIds[i])<<endl;
        }
        cout<<"0. 返回"<<endl;
        cout<<endl;
        cout<<"请选择成员序号：";
        int choice=readIntChoice();
        if(choice==0){
            return;
        }
        if(choice<1||choice>(int)memberIds.size()){
            cout<<"无效的序号，请重新输入！"<<endl;
            continue;
        }
        memberActionMenu(memberIds[choice-1]);
    }
}
void ClubManager::memberActionMenu(const string& memberId){
    while(true){
        cout<<endl;
        cout<<"已选择成员："<<memberId<<" "<<nameOf(memberId)<<endl;
        cout<<"1.查看完整信息"<<endl;
        cout<<"2.移除该成员"<<endl;
        cout<<"0.返回成员列表"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                if(userManager!=nullptr){
                    userManager->showUserInfoById(memberId);
                }
                break;
            case 2:{
                cout<<"确认移除 "<<memberId<<" 吗？1.确认  0.取消"<<endl;
                cout<<"请选择：";
                if(readIntChoice()!=1){
                    cout<<"已取消。"<<endl;
                    break;
                }
                if(clubService->removeClubMember(userId,clubId,memberId)){
                    saveClubData();
                    cout<<"已移除该成员。"<<endl;
                    return;
                    //人已不在社团，直接回到名单
                }
                cout<<"移除失败：该用户不是本社团成员。"<<endl;
                break;
            }
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void ClubManager::applicationMenu(){
    while(true){
        vector<string> applicantIds=clubService->getClubApplicantIds(clubId);
        cout<<endl;
        cout<<"========加入申请========"<<endl;
        if(applicantIds.empty()){
            cout<<"暂无待审核的申请。"<<endl;
            return;
        }
        for(size_t i=0;i<applicantIds.size();i++){
            cout<<i+1<<". "<<applicantIds[i]<<" "<<nameOf(applicantIds[i])<<endl;
        }
        cout<<"0. 返回"<<endl;
        cout<<endl;
        cout<<"请选择申请序号：";
        int choice=readIntChoice();
        if(choice==0){
            return;
        }
        if(choice<1||choice>(int)applicantIds.size()){
            cout<<"无效的序号，请重新输入！"<<endl;
            continue;
        }
        applicantActionMenu(applicantIds[choice-1]);
    }
}
void ClubManager::applicantActionMenu(const string& applicantId){
    while(true){
        cout<<endl;
        cout<<"已选择申请人："<<applicantId<<" "<<nameOf(applicantId)<<endl;
        cout<<"1.查看完整信息"<<endl;
        cout<<"2.通过申请"<<endl;
        cout<<"3.拒绝申请"<<endl;
        cout<<"0.返回申请列表"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                if(userManager!=nullptr){
                    userManager->showUserInfoById(applicantId);
                }
                break;
            case 2:
                if(clubService->approveApplication(userId,clubId,applicantId)){
                    saveClubData();
                    cout<<"已通过，"<<applicantId<<" 成为社团成员。"<<endl;
                    return;
                }
                cout<<"操作失败：该用户不在申请名单中。"<<endl;
                break;
            case 3:
                if(clubService->rejectApplication(userId,clubId,applicantId)){
                    saveClubData();
                    cout<<"已拒绝该申请。"<<endl;
                    return;
                }
                cout<<"操作失败：该用户不在申请名单中。"<<endl;
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
string ClubManager::nameOf(const string& id){
    if(userManager==nullptr){
        return "";
    }
    return userManager->getUserNameById(id);
}
void ClubManager::publishActivity(){
    //对接活动模块，传递clubId创建活动名称、时间、地点、描述
    cout<<"该功能等待活动模块完成。"<<endl;
}
void ClubManager::modifyActivity(){
    //对接活动模块，根据clubId查询本社团活动，选择具体的activityId后修改活动信息
    cout<<"该功能等待活动模块完成。"<<endl;
}
void ClubManager::deleteActivity(){
    //对接活动模块，根据clubId查询本社团活动，选择具体activityId后删除活动
    cout<<"该功能等待活动模块完成。"<<endl;
}
void ClubManager::showActivityRegistration(){
    //对接活动模块，根据activityId查询报名学生列表
    cout<<"该功能等待活动模块完成。"<<endl;
}
void ClubManager::manageRegistration(){
    //对接活动模块，实现审核报名，通过、拒绝申请、查看报名名单
    cout<<"该功能等待活动模块完成。"<<endl;
}
