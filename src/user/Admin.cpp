#include"Admin.h"
#include"../common/TextUtils.h"
#include<iostream>
#include"../common/InputUtils.h"
#include"../club/ClubService.h"
#include"../common.h"
#include"UserManager.h"
#include"../activity/ActivityManager.h"
/*
函数名：Admin（构造函数）
功能：调用父类 User 的构造函数设置账号信息，角色固定为 Admin
参数：id 用户Id；name 用户名；pwd 密码
*/
Admin::Admin(string id,string name,string pwd):User(id,name,pwd,"Admin")
{
}
/*
函数名：showMenu
功能：管理员主菜单，根据选择进入用户管理、社团管理、活动管理、数据统计四个子菜单
实现说明：
    用 while(true) 循环显示菜单，子菜单返回后重新显示主菜单，选 0 时 return 退出登录；
    readIntChoice() 遇到非数字输入会返回 -1，进入 default 提示重新输入。
    本函数重写了基类 User 的纯虚函数，main 通过 User* 调用时执行的是这个版本（多态）。
*/
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
/*
函数名：userManageMenu
功能：用户管理子菜单，四个选项分别调用查看、删除、冻结/解冻、修改角色函数
*/
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
/*
函数名：showAllUsers
功能：显示所有用户，具体的列表输出由 UserManager::showAllUsers() 完成
*/
void Admin::showAllUsers(){
    if(userManager!=nullptr){
        userManager->showAllUsers();
    }
}
/*
函数名：selectUser
功能：先显示用户列表，再输入要操作的用户Id，并检查是否合法
参数：prompt 输入提示语（如"请输入要删除的用户Id："）
返回值：合法的用户Id；输入的是自己的Id或用户不存在时返回空字符串
实现说明：删除、冻结、修改角色三个功能开头都要做这一步，所以提取成公共函数；
          用 getUserRole() 是否返回空字符串来判断该用户是否存在
*/
string Admin::selectUser(const string& prompt){
    if(userManager==nullptr){
        return "";
    }
    userManager->showAllUsers();
    cout<<endl;
    cout<<prompt;
    string targetId;
    cin>>targetId;
    //管理员不能删除、冻结自己或修改自己的角色
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
/*
函数名：deleteUser
功能：删除一个用户，并清理他在社团模块和活动模块中的数据
实现步骤：
    1. 用 selectUser() 选出要删除的用户；管理员账号不能删除；
    2. 社团负责人、或正在申请成立社团的用户不能直接删除，否则社团会没有负责人，
       要先由管理员处理其社团；
    3. 确认后调用 UserManager::deleteUser() 从 users.txt 中删除；
    4. 遍历所有已通过的社团：把他从成员名单中移除，并撤销他的加入申请，分别计数；
       有改动才保存社团文件；
    5. 遍历所有活动，取消他的报名和候补。cancelActivity() 对已结束的活动会返回 false，
       所以已结束活动的记录会保留；空出的名额由候补第一位自动递补。
*/
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
    //待审核社团的申请人也算，用 findAnyClubByLeader() 一起查出来（不论社团审核状态）
    if(role==ROLE_CLUB_MANAGER||(clubService!=nullptr&&!clubService->findAnyClubByLeader(targetId).empty())){
        cout<<"该用户是社团负责人或正在申请成立社团，请先处理其社团后再删除用户！"<<endl;
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
            //leaveClub() 返回 true 说明他原来是该社团成员
            if(clubService->leaveClub(targetId,club.clubId)){
                leftClubs++;
            }
            //拒绝申请要以负责人身份操作，所以传入该社团的负责人Id
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
/*
函数名：freezeUser
功能：冻结或解冻用户账号，冻结后该用户登录时会被拒绝
实现步骤：
    1. 用 selectUser() 选出用户，查出他当前的状态；
    2. 已冻结的账号询问是否解冻，正常的账号询问是否冻结（同一个菜单项完成两种操作）；
    3. 管理员账号不能被冻结，但如果数据中已经是冻结状态，仍然允许解冻；
    4. 确认后调用 UserManager::setUserStatus() 修改状态并写回 users.txt。
*/
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
    //三目运算符：原来冻结就改为正常，原来正常就改为冻结
    string newStatus=frozen?USER_NORMAL:USER_FROZEN;
    if(!userManager->setUserStatus(targetId,newStatus)){
        cout<<"操作失败！"<<endl;
        return;
    }
    cout<<(frozen?"已解冻该账号！":"已冻结该账号！")<<endl;
}
/*
函数名：modifyUserRole
功能：修改用户角色（普通学生 / 社团负责人 / 系统管理员）
实现步骤：
    1. 用 selectUser() 选出用户，记下他原来的角色；
    2. 循环读入新角色，输错重新选，选 0 取消；新旧角色相同时直接提示并返回；
    3. 查出该用户负责的已通过社团 clubId：
       - 负责着已通过的社团，不能改成其他角色，否则社团会没有负责人；
       - 没有已通过的社团，不能设为社团负责人，否则进入社团管理时无社团可管；
    4. 调用 UserManager::changeRole() 修改并写回文件。
       已登录的用户对象不会改变，所以该用户要重新登录后新角色才生效。
*/
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
    //负责着已通过的社团，不能改成别的角色
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
/*
函数名：clubManageMenu
功能：社团管理子菜单：查看社团信息、审核社团申请、删除违规社团
说明：这三个功能都要调用社团模块，所以进入菜单前先检查 clubService 是否已设置
*/
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
/*
函数名：viewClubInfo
功能：显示所有社团（包括待审核、已通过、已拒绝），由社团模块的 showAllClubs() 输出
*/
void Admin::viewClubInfo(){
    cout<<endl;
    clubService->showAllClubs();
}
/*
函数名：auditClub
功能：审核学生提交的社团成立申请
实现步骤：
    1. 显示所有社团，输入要审核的社团编号；
    2. 循环读入选择：1 通过、2 拒绝、0 取消，输错重新选；
    3. 调用社团模块的 auditClub() 修改审核状态，社团不存在或已审核过时返回 false；
    4. 成功后保存社团文件；
    5. 如果是通过，再把申请人的角色改为 ClubManager，申请人重新登录后就能使用社团管理功能。
*/
void Admin::auditClub(){
    cout<<endl;
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
/*
函数名：deleteClub
功能：删除违规社团，并处理与之相关的活动和负责人角色
实现步骤：
    1. 显示所有社团，输入要删除的社团编号；
    2. 删除前先记下负责人Id，因为社团删掉之后就查不到了；
    3. 调用社团模块的 deleteClub()，社团不存在或还有成员时删除失败；
    4. 删除成功后保存文件，并删除该社团的所有活动：
       先把活动编号存到 activityIds 中，再逐个删除，
       避免一边遍历活动列表一边删除元素导致遍历出错；
    5. 如果原负责人现在没有其他已通过的社团，就把他的角色改回普通学生。
*/
void Admin::deleteClub()
{
    cout<<endl;
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
函数名：activityManageMenu
功能：活动管理子菜单：查看活动信息、审核活动、删除违规活动
说明：三个功能都要调用活动模块，每个函数开头会各自检查 activityManager 是否已设置
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
/*
函数名：viewActivityInfo
功能：显示所有活动（包括待审核、已通过、已拒绝），每个活动一行
实现步骤：
    1. 从活动模块取出全部活动，没有活动时提示"暂无活动"；
    2. 计算名称列的宽度：取所有"名称（类型）"中最长的显示宽度再加 2，最少 20，
       这样名称再长也能和其他行对齐；
    3. 逐行输出编号、名称和类型、所属社团、审核状态，用 padText() 按显示宽度补空格；
    4. 最后空一行输出活动总数。
关键点：中文字符在控制台占 2 列，用 string 的 size() 算宽度会对不齐，
        所以用 TextUtils.h 中的 displayWidth() 计算。
*/
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
    //每个活动一行：编号、名称（类型）、所属社团、审核状态
    for(const Activity* act:acts){
        //按显示宽度补空格，让各列对齐
        cout<<padText(act->getActivityId(),6)<<padText(act->getName()+"（"+act->getTypeName()+"）",nameW)
            <<padText(act->getClubId(),6)<<Activity::statusText(act->getStatus())<<endl;
    }
    cout<<endl;
    cout<<"共"<<acts.size()<<"个活动"<<endl;
}
/*
函数名：auditActivity
功能：审核社团负责人发布的活动
实现步骤：
    1. 显示待审核活动，没有待审核活动时直接返回；
    2. 输入活动编号，检查活动是否存在、是否还是待审核状态；
    3. 显示活动详情，方便管理员判断是否通过；
    4. 循环读入选择：1 通过、2 拒绝、0 取消；
    5. 调用活动模块的 auditActivity() 修改状态，第二个参数 choice==1 为 true 表示通过。
*/
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
/*
函数名：removeActivity
功能：删除违规活动，报名记录一起删除
实现步骤：
    1. 调用 viewActivityInfo() 显示所有活动，没有活动时直接返回；
    2. 输入活动编号，检查活动是否存在；
    3. 确认后调用活动模块的 removeActivity() 删除。
说明：管理员可以删除任何社团的活动，所以不需要像负责人那样检查所属社团。
*/
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
/*
函数名：systemStatisticsMenu
功能：数据统计子菜单
    1 用户数量统计：总人数和各角色人数，由 UserManager::showUserStatistics() 输出；
    2 社团数量统计：社团总数；
    3 活动数量统计：活动总数、已通过数、待审核数；
    4 社团活跃度排行：由社团模块根据活动模块的数据计算并排序。
说明：每项统计前先检查对应模块的指针是否为空，避免访问空指针导致程序崩溃
*/
void Admin::systemStatisticsMenu()
{
    while(true)
    {
        cout<<endl;
        cout<<"========数据统计========"<<endl;
        cout<<"1.用户数量统计"<<endl;
        cout<<"2.社团数量统计"<<endl;
        cout<<"3.活动数量统计"<<endl;
        cout<<"4.社团活跃度排行"<<endl;
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
                    //size() 分别统计三个活动列表的长度
                    cout<<"活动总数："<<activityManager->getAllActivities().size()<<endl;
                    cout<<"已通过："<<activityManager->getApprovedActivities().size()<<endl;
                    cout<<"待审核："<<activityManager->getPendingActivities().size()<<endl;
                }
                break;

            case 4:
                //活跃度用活动模块的真实数据计算，由社团模块排序和显示
                //getActivityRank() 的参数是引用，所以用 *activityManager 传入对象本身
                if(clubService!=nullptr&&activityManager!=nullptr){
                    cout<<endl;
                    clubService->showActivityRank(clubService->getActivityRank(*activityManager));
                }
                break;

            case 0:
                return;

            default:
                cout << "无效的选择，请重新输入！" << endl;
        }
    }
}
/*
函数名：getAdminId
功能：返回管理员的用户Id
*/
string Admin::getAdminId() const
{
    return userId;
}
