#include"Student.h"
#include<iostream>
#include<algorithm>
#include"../common/InputUtils.h"
#include"UserManager.h"
#include"../club/ClubService.h"
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
                //活动报名，属于活动模块
                break;
            case 4:
                showMyActivityMenu();
                //活动模块负责提供我的报名活动信息
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
        for(const auto& interest:interests){
            cout<<interest<<" ";
        }
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
    }
    else{
        cout<<"兴趣标签不存在！"<<endl;
    }
}
void Student::showInterests()const{
    if(interests.empty()){
        cout<<"无兴趣标签"<<endl;
    }
    else{
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
void Student::recommendClubs()const{
    if(clubService==nullptr){
        cout<<"社团数据未加载，无法推荐社团！"<<endl;
        return;
    }
    if(interests.empty()){
        cout<<"您还没有兴趣标签，请先在个人信息管理中添加！"<<endl;
        return;
    }
    vector<ClubBrief> clubs=clubService->getApprovedClubBriefs();
    bool found=false;
    for(const ClubBrief& club:clubs){
        //名称、简介、分类资料合在一起比对
        string text=club.clubName+" "+club.description+" "+club.extraInfo;
        for(const string& interest:interests){
            if(text.find(interest)!=string::npos){
                cout<<"推荐社团："<<club.clubName<<"（编号："<<club.clubId<<"，匹配兴趣："<<interest<<"）"<<endl;
                found=true;
                break;
            }
        }
    }
    if(!found){
        cout<<"暂时没有匹配您兴趣的社团，您可以尝试添加更多兴趣标签！"<<endl;
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
                clubService->showApprovedClubs();
                break;
            case 2:{
                cout<<"请输入社团名称关键词：";
                string keyword=readLine();
                clubService->searchApprovedClubs(keyword);
                break;
            }
            case 3:{
                string clubId;
                cout<<"请输入社团编号：";
                cin>>clubId;
                clubService->showClubInfo(clubId);
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
                clubService->showMyApplications(userId);
                break;
            case 6:
                clubService->showMyClubs(userId);
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
            cout<<"无效的选择，申请失败！"<<endl;
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