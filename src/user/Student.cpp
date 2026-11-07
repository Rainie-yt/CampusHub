#include"Student.h"
#include<iostream>
#include<algorithm>
#include"../common/InputUtils.h"
#include"UserManager.h"
#include"../club/ClubService.h"
#include"../activity/ActivityManager.h"
//社团列表的一行：编号、名称、类型
static void printClubRow(const ClubBrief& club){
    cout<<club.clubId<<"  "<<club.clubName<<"（"<<club.type<<"）"<<endl;
}
Student::Student(string id,string name,string pwd,string maj):User(id,name,pwd,"Student"),major(maj)
{
}
string Student::getMajor()const{
    return major;
}
void Student::showMenu(){
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：普通学生"<<endl;
        cout<<endl;
        cout<<"========学生菜单========"<<endl;
        cout<<"1.个人信息管理"<<endl;
        cout<<"2.社团中心"<<endl;
        cout<<"3.活动报名"<<endl;
        cout<<"4.我的报名活动"<<endl;
        cout<<"5.兴趣推荐"<<endl;
        cout<<"0.退出登录"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                showInfoMenu();
                break;
                //包括：1.查看个人信息；2.修改密码；3.兴趣标签管理
            case 2:
                clubMenu();
                break;
            case 3:
                joinActivityMenu();
                break;
            case 4:
                showMyActivityMenu();
                break;
            case 5:
                recommendClubs();
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
    while(true){
        cout<<endl;
        cout<<"========个人信息管理========"<<endl;
        showInfo();
        cout<<"专业："<<major<<endl;
        cout<<"兴趣标签：";
        showInterests();
        cout<<endl;
        cout<<"1.修改密码"<<endl;
        cout<<"2.管理兴趣标签"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                {
                    string newpwd;
                    newpwd=readField("请输入新密码：");
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
    while(true){
        cout<<endl;
        cout<<"========兴趣标签管理========"<<endl;
        cout<<"当前兴趣标签：";
        showInterests();
        cout<<endl;
        cout<<"1.添加兴趣标签"<<endl;
        cout<<"2.删除兴趣标签"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                {
                    string newInterest;
                    newInterest=readField("请输入要添加的兴趣标签：");
                    //单个标签，逗号和空格都不允许
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
    }
    else{
        cout<<"兴趣标签不存在！"<<endl;
    }
}
void Student::showInterests()const{
    if(interests.empty()){
        cout<<"无"<<endl;
        return;
    }
    //标签之间用顿号隔开，最后一个后面不加
    for(size_t i=0;i<interests.size();i++){
        if(i>0){
            cout<<"、";
        }
        cout<<interests[i];
    }
    cout<<endl;
}
vector<string> Student::getInterests()const{
    return interests;
}
void Student::joinActivityMenu(){
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    cout<<endl;
    activityManager->showApprovedActivities();
    if(activityManager->getApprovedActivities().empty()){
        return;
    }
    cout<<endl;
    cout<<"请输入要报名的活动编号（输入0返回）：";
    string activityId;
    cin>>activityId;
    if(activityId=="0"){
        return;
    }
        Activity* act=activityManager->findActivity(activityId);
    if(act==nullptr||!act->isApproved()){
        cout<<"未找到该活动，或活动尚未通过审核！"<<endl;
        return;
    }
    //先显示详情（含地点等），确认后再报名
    cout<<endl;
    cout<<"========活动详情========"<<endl;
    act->showInfo();
    cout<<endl;
    cout<<"确认报名该活动吗？1.确认  0.取消"<<endl;
    cout<<"请选择：";
    if(readIntChoice()!=1){
        cout<<"已取消。"<<endl;
        return;
    }
    //报名结果（成功、候补、时间冲突等）由活动板块自己输出
    activityManager->joinActivity(userId,activityId);
}
void Student::showMyActivityMenu()
{
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    while(true){
        cout<<endl;
        activityManager->showMyActivities(userId);
        cout<<endl;
        cout<<"我的积分："<<activityManager->getUserPoints(userId)<<"（参加的活动结束后计入）"<<endl;
        cout<<endl;
        cout<<"1.取消报名"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        if(choice==0){
            return;
        }
        if(choice!=1){
            cout<<"无效的选择，请重新输入！"<<endl;
            continue;
        }
        cout<<"请输入要取消报名的活动编号：";
        string activityId;
        cin>>activityId;
        //取消正式名额后，候补的第一个人会自动递补
        if(activityManager->cancelActivity(userId,activityId)){
            cout<<"已成功取消报名！"<<endl;
        }
        else{
            cout<<"取消失败：你没有报名该活动，或活动已结束。"<<endl;
        }
    }
}
void Student::recommendClubs()const{
    if(clubService==nullptr){
        cout<<"社团数据未加载，无法推荐社团！"<<endl;
        return;
    }
    if(interests.empty()){
        cout<<"你还没有兴趣标签，请先在个人信息管理中添加！"<<endl;
        return;
    }
    cout<<endl;
    cout<<"========为你推荐社团========"<<endl;
    vector<ClubBrief> clubs=clubService->getApprovedClubBriefs();
    bool found=false;
    for(const ClubBrief& club:clubs){
        //名称、简介、分类资料合在一起比对
        string text=club.clubName+" "+club.description+" "+club.extraInfo;
        for(const string& interest:interests){
            if(text.find(interest)!=string::npos){
                cout<<"推荐社团："<<club.clubId<<"  "<<club.clubName<<endl;
                found=true;
                break;
            }
        }
    }
    if(!found){
        cout<<"暂时没有匹配你兴趣的社团，你可以尝试添加更多兴趣标签！"<<endl;
    }
    //再推荐活动：按兴趣标签和活动标签的匹配度排序，由活动模块计算
    if(activityManager!=nullptr){
        cout<<endl;
        cout<<"========为你推荐活动========"<<endl;
        //recommend()只返回结果、不输出，按匹配度从高到低排好序，自己按统一格式显示前5个
        vector<pair<const Activity*,double>> acts=activityManager->recommend(interests,userId);
        if(acts.empty()){
            cout<<"暂时没有匹配你兴趣的活动，你可以尝试添加更多兴趣标签！"<<endl;
        }
        for(size_t i=0;i<acts.size()&&i<5;i++){
            cout<<"推荐活动："<<acts[i].first->getActivityId()<<"  "<<acts[i].first->getName()<<endl;
        }
    }
}
void Student::clubMenu(){
    if(clubService==nullptr){
        cout<<"社团数据未加载，无法进入社团菜单！"<<endl;
        return;
    }
    while(true){
        cout<<endl;
        cout<<"========社团中心========"<<endl;
        cout<<"1.浏览社团"<<endl;
        cout<<"2.搜索社团"<<endl;
        cout<<"3.查看社团详情"<<endl;
        cout<<"4.申请加入社团"<<endl;
        cout<<"5.我的加入申请"<<endl;
        cout<<"6.我加入的社团"<<endl;
        cout<<"7.退出社团"<<endl;
        cout<<"8.申请成立社团"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                browseClubs();
                break;
            case 2:{
                cout<<"请输入社团名称关键词：";
                string keyword=readLine();
                searchClubs(keyword);
                break;
            }
            case 3:{
                string clubId;
                cout<<"请输入社团编号：";
                cin>>clubId;
                showClubDetail(clubId);
                break;
            }
            case 4:{
                string clubId;
                cout<<"请输入要加入的社团编号：";
                cin>>clubId;
                //userId用当前登录用户自己的，不让用户输入
                if(clubService->applyToJoin(userId,clubId)){
                    saveClubData();
                    cout<<"加入申请已提交，请等待社团负责人审核！"<<endl;
                }
                else{
                    cout<<"申请失败：社团不存在或未通过审核，或你已是成员、已申请过。"<<endl;
                }
                break;
            }
            case 5:
                showMyApplicationList();
                break;
            case 6:
                showMyClubList();
                break;
            case 7:{
                string clubId;
                cout<<"请输入要退出的社团编号：";
                cin>>clubId;
                if(clubService->leaveClub(userId,clubId)){
                    saveClubData();
                    cout<<"已成功退出社团！"<<endl;
                }
                else{
                    cout<<"退出社团失败：社团不存在或你不是该社团成员。"<<endl;
                }
                break;
            }
            case 8:
                applyCreateClub();
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
void Student::applyCreateClub(){
    cout<<endl;
    cout<<"========申请成立社团========"<<endl;
    cout<<"1.体育社团  2.学术社团  3.艺术社团  4.普通社团"<<endl;
    cout<<"请选择社团类型：";
    int typeChoice=readIntChoice();
    string type;
    string extraInfo;
    //类型名称必须和社团模块读取文件时用的写法一致
    switch(typeChoice){
        case 1:
            type="体育社团";
            cout<<"请输入运动项目：";
            extraInfo=readLine();
            break;
        case 2:
            type="学术社团";
            cout<<"请输入学术领域：";
            extraInfo=readLine();
            break;
        case 3:
            type="艺术社团";
            cout<<"请输入艺术类别：";
            extraInfo=readLine();
            break;
        case 4:
            type="普通社团";
            break;
        default:
            cout<<"无效的选择，已取消申请。"<<endl;
            return;
    }
    cout<<"请输入社团名称：";
    string name=readLine();
    cout<<"请输入社团简介：";
    string intro=readLine();
    string newId=clubService->applyClub(userId,type,name,intro,extraInfo);
    if(newId.empty()){
        cout<<"申请失败：资料不合法，或你已经负责一个社团。"<<endl;
        return;
    }
    saveClubData();
    cout<<"申请已提交，社团编号为："<<newId<<"，请等待管理员审核！"<<endl;
}
void Student::browseClubs()const{
    vector<ClubBrief> clubs=clubService->getApprovedClubBriefs();
    if(clubs.empty()){
        cout<<"暂无审核通过的社团。"<<endl;
        return;
    }
    cout<<endl;
    cout<<"========社团列表========"<<endl;
    for(const ClubBrief& club:clubs){
        printClubRow(club);
    }
    cout<<endl;
    cout<<"共"<<clubs.size()<<"个社团"<<endl;
}
void Student::searchClubs(const string& keyword)const{
    cout<<endl;
    cout<<"========搜索结果========"<<endl;
    bool found=false;
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        if(club.clubName.find(keyword)!=string::npos){
            printClubRow(club);
            found=true;
        }
    }
    if(!found){
        cout<<"没有找到符合条件的社团。"<<endl;
    }
}
void Student::showClubDetail(const string& clubId)const{
    //学生只能查看已审核通过的社团
    if(!clubService->isClubApproved(clubId)){
        if(clubService->clubExists(clubId)){
            cout<<"该社团尚未通过审核，暂不能查看。"<<endl;
        }
        else{
            cout<<"未找到该社团！"<<endl;
        }
        return;
    }
    cout<<endl;
    cout<<"========社团详情========"<<endl;
    printClubDetail(clubId);
}
void Student::printClubDetail(const string& clubId)const{
    //只输出资料，不带标题，学生详情和负责人"我的社团"共用
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        if(club.clubId!=clubId){
            continue;
        }
        cout<<"编号："<<club.clubId<<endl;
        cout<<"名称："<<club.clubName<<endl;
        cout<<"类型："<<club.type<<endl;
        //分类资料的叫法和申请成立社团时一致
        if(!club.extraInfo.empty()){
            if(club.type=="体育社团"){
                cout<<"运动项目：";
            }
            else if(club.type=="学术社团"){
                cout<<"学术领域：";
            }
            else if(club.type=="艺术社团"){
                cout<<"艺术类别：";
            }
            else{
                cout<<"分类资料：";
            }
            cout<<club.extraInfo<<endl;
        }
        cout<<"简介："<<club.description<<endl;
        cout<<"负责人："<<club.leaderId;
        string leaderName=userManager!=nullptr?userManager->getUserNameById(club.leaderId):"";
        if(!leaderName.empty()){
            cout<<" "<<leaderName;
        }
        cout<<endl;
        cout<<"成员人数："<<club.memberCount<<"（不含负责人）"<<endl;
        return;
    }
}
void Student::showMyClubList()const{
    cout<<endl;
    cout<<"========我加入的社团========"<<endl;
    bool found=false;
    //成员只会在已通过的社团里，逐个查成员名单
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        vector<string> ids=clubService->getClubMemberIds(club.clubId);
        if(find(ids.begin(),ids.end(),userId)!=ids.end()){
            printClubRow(club);
            found=true;
        }
    }
    if(!found){
        cout<<"你尚未加入任何社团。"<<endl;
    }
}
void Student::showMyApplicationList()const{
    cout<<endl;
    cout<<"========我的加入申请========"<<endl;
    bool found=false;
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        vector<string> ids=clubService->getClubApplicantIds(club.clubId);
        if(find(ids.begin(),ids.end(),userId)!=ids.end()){
            printClubRow(club);
            found=true;
        }
    }
    if(!found){
        cout<<"暂无待审核的加入申请。"<<endl;
    }
}
