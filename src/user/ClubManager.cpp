#include"ClubManager.h"
#include<iostream>
#include"../common/InputUtils.h"
#include"../club/ClubService.h"
#include"UserManager.h"
#include"../activity/ActivityManager.h"
/*
函数名：ClubManager（构造函数）
功能：调用父类 Student 的构造函数设置账号信息和专业，保存社团编号，并把角色改为 ClubManager
参数：id 用户Id；name 用户名；pwd 密码；maj 专业；cid 社团编号
说明：Student 的构造函数会把角色设为 Student，所以这里要重新赋值为 ClubManager；
      创建对象时 cid 传入的是空字符串，真正的社团编号在 showMenu() 中查出
*/
ClubManager::ClubManager(string id,string name,string pwd,string maj,string cid):Student(id,name,pwd,maj),clubId(cid)
{
    role="ClubManager";
}
/*
函数名：showMenu
功能：负责人主菜单。前 5 项与学生菜单相同，直接调用从 Student 继承来的函数，第 6 项是社团管理
实现说明：
    users.txt 中没有保存负责人管理的社团编号，所以每次进入菜单时，
    调用社团模块的 findClubByLeader() 按负责人Id查出自己负责的已通过社团。
    本函数重写了 Student::showMenu()，main 通过 User* 调用时执行的是这个版本（多态）。
*/
void ClubManager::showMenu(){
    if(clubService!=nullptr){
        clubId=clubService->findClubByLeader(userId);
    }
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：社团负责人"<<endl;
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
            //1～5 是从 Student 继承来的功能，不需要重新编写
            case 1:{
                showInfoMenu();
                break;
            }
            case 2:{
                clubMenu();
                break;
            }
            case 3:{
                joinActivityMenu();
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
/*
函数名：clubManageMenu
功能：社团管理菜单：查看我的社团、修改社团信息、管理社团成员、活动管理
说明：clubId 为空说明该负责人没有已审核通过的社团，不能进入管理菜单
*/
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
/*
函数名：activityManageMenu
功能：活动管理菜单，五个选项分别调用本类的五个活动管理函数
*/
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
        cout<<"6.重新提交被驳回活动"<<endl;
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
            case 6:{
                string id=selectMyActivity("请输入要重新提交的活动编号：");
                if(!id.empty()){
                    if(activityManager->resubmitActivity(id,clubId)) cout<<"已重新提交，请等待管理员审核！"<<endl;
                    else cout<<"重新提交失败：只支持本社团未结束且被驳回的活动，或文件保存失败。"<<endl;
                }
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
/*
函数名：showClubInfo
功能：显示我负责的社团的详细资料
说明：调用从 Student 继承来的 printClubDetail()，与学生查看社团详情共用同一套显示代码
*/
void ClubManager::showClubInfo(){
    cout<<endl;
    cout<<"========我的社团========"<<endl;
    printClubDetail(clubId);
}
/*
函数名：modifyClubInfo
功能：修改社团名称和简介
实现说明：用 readLine() 读入整行，名称和简介中间可以有空格；
          社团模块会检查操作者是否为该社团负责人、名称是否为空，成功后保存文件
*/
void ClubManager::modifyClubInfo(){
    cout<<"请输入新的社团名称：";
    string newName=readLine();
    cout<<"请输入新的社团简介：";
    string newDescription=readLine();
    if(persistChange([&](){
        return clubService->modifyClubInfo(userId,clubId,newName,newDescription)
            && saveClubData();
    })){
        cout<<"社团信息修改成功！"<<endl;
    }
    else{
        cout<<"修改失败：名称不能为空，且名称和简介不能换行。"<<endl;
    }
}
/*
函数名：manageMembers
功能：成员管理菜单：1 进入成员列表，2 进入加入申请列表
*/
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
/*
函数名：memberListMenu
功能：显示成员列表（序号、用户Id、姓名），输入序号选择一个成员进行操作
实现步骤：
    1. 每次循环都重新向社团模块获取成员名单，这样移除成员后列表能立即更新；
    2. 名单为空时提示"暂无成员"并返回；
    3. 输入序号，0 返回；序号超出范围时提示无效并重新显示；
    4. 序号从 1 开始显示，而 vector 下标从 0 开始，所以用 memberIds[choice-1] 取出成员Id，
       进入 memberActionMenu()。
*/
void ClubManager::memberListMenu(){
    while(true){
        vector<string> memberIds=clubService->getClubMemberIds(clubId);
        cout<<endl;
        cout<<"========成员列表========"<<endl;
        if(memberIds.empty()){
            cout<<"暂无成员。"<<endl;
            return;
        }
        for(size_t i=0;i<memberIds.size();i++){
            cout<<i+1<<"."<<memberIds[i]<<"  "<<nameOf(memberIds[i])<<endl;
        }
        cout<<"0.返回"<<endl;
        cout<<endl;
        cout<<"请选择成员序号：";
        int choice=readIntChoice();
        if(choice==0){
            return;
        }
        //size() 是无符号数，先转成 int 再比较，避免负数比较出错
        if(choice<1||choice>(int)memberIds.size()){
            cout<<"无效的序号，请重新输入！"<<endl;
            continue;
        }
        memberActionMenu(memberIds[choice-1]);
    }
}
/*
函数名：memberActionMenu
功能：对选中的成员进行操作：1 查看完整信息，2 移除该成员
参数：memberId 选中成员的用户Id
实现说明：
    查看信息时调用 UserManager::showUserInfoById()，不显示密码；
    移除前要确认，移除成功后保存文件并直接返回成员列表，因为该成员已经不在社团里了。
*/
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
                    cout<<endl;
                    cout<<"========成员信息========"<<endl;
                    userManager->showUserInfoById(memberId);
                }
                break;
            case 2:{
                cout<<"确认移除 "<<memberId<<" "<<nameOf(memberId)<<" 吗？1.确认  0.取消"<<endl;
                cout<<"请选择：";
                if(readIntChoice()!=1){
                    cout<<"已取消。"<<endl;
                    break;
                }
                //社团模块会核对操作者 userId 是否为该社团负责人
                if(persistChange([&](){
                    return clubService->removeClubMember(userId,clubId,memberId)
                        && saveClubData();
                })){
                    cout<<"已移除该成员！"<<endl;
                    return;
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
/*
函数名：applicationMenu
功能：显示加入申请列表，输入序号选择一个申请人进行审核
实现说明：做法与 memberListMenu() 相同，名单换成社团的申请人名单；
          审核完一个申请后重新获取名单，全部处理完时提示"暂无待审核的申请"并返回
*/
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
            cout<<i+1<<"."<<applicantIds[i]<<"  "<<nameOf(applicantIds[i])<<endl;
        }
        cout<<"0.返回"<<endl;
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
/*
函数名：applicantActionMenu
功能：对选中的申请人操作：1 查看完整信息，2 通过申请，3 拒绝申请
参数：applicantId 申请人的用户Id
实现说明：通过后申请人成为社团成员，拒绝后从申请名单中删除；
          两种操作成功后都保存文件，并返回申请列表
*/
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
                    cout<<endl;
                    cout<<"========申请人信息========"<<endl;
                    userManager->showUserInfoById(applicantId);
                }
                break;
            case 2:
                if(persistChange([&](){
                    return clubService->approveApplication(userId,clubId,applicantId)
                        && saveClubData();
                })){
                    cout<<"已通过，"<<applicantId<<" 成为社团成员！"<<endl;
                    return;
                }
                cout<<"操作失败：该用户不在申请名单中。"<<endl;
                break;
            case 3:
                if(persistChange([&](){
                    return clubService->rejectApplication(userId,clubId,applicantId)
                        && saveClubData();
                })){
                    cout<<"已拒绝该申请！"<<endl;
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
/*
函数名：nameOf
功能：按用户Id查用户名，用于成员列表、申请列表中显示姓名
参数：id 用户Id
返回值：用户名；找不到或用户管理器未设置时返回空字符串
*/
string ClubManager::nameOf(const string& id){
    if(userManager==nullptr){
        return "";
    }
    return userManager->getUserNameById(id);
}
/*
函数名：selectMyActivity
功能：列出本社团的活动，输入活动编号并检查该活动是否属于本社团
参数：prompt 输入提示语（如"请输入要修改的活动编号："）
返回值：合法的活动编号；没有活动、编号不存在或不属于本社团时返回空字符串
实现说明：修改、删除、查看报名、管理报名四个功能开头都要做这一步，所以提取成公共函数；
          通过比较活动的所属社团编号和自己的 clubId，保证负责人只能操作本社团的活动
*/
string ClubManager::selectMyActivity(const string& prompt){
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return "";
    }
    cout<<endl;
    activityManager->showActivitiesByClub(clubId);
    if(activityManager->getActivitiesByClub(clubId).empty()){
        return "";
    }
    cout<<endl;
    cout<<prompt;
    string activityId;
    cin>>activityId;
    Activity* act=activityManager->findActivity(activityId);
    if(act==nullptr||act->getClubId()!=clubId){
        cout<<"未找到该活动，或该活动不属于本社团！"<<endl;
        return "";
    }
    return activityId;
}
/*
函数名：publishActivity
功能：发布本社团的新活动，发布后为待审核状态，管理员审核通过后学生才能看到
实现说明：
    活动模块的 publishActivity() 用 getline 逐行读入活动资料，
    而菜单用 readIntChoice() 读选项后，输入缓冲区里还留着一个换行符，
    不先清掉的话，活动模块第一次 getline 会直接读到空行，导致发布被取消。
    所以调用前先用 skipLine() 丢掉这个换行。
*/
void ClubManager::publishActivity(){
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    skipLine();
    cout<<endl;
    activityManager->publishActivity(clubId);
}
/*
函数名：modifyActivity
功能：修改本社团的活动（名称、标签、地点、人数上限等，直接回车表示不修改）
实现说明：先用 selectMyActivity() 选出本社团的活动；
          cin>> 读入编号后同样会留下换行，调用活动模块前先 skipLine()；
          已经有人报名的活动不能修改时间，由活动模块判断
*/
void ClubManager::modifyActivity(){
    string activityId=selectMyActivity("请输入要修改的活动编号：");
    if(activityId.empty()){
        return;
    }
    skipLine();
    cout<<endl;
    cout<<"========修改活动========"<<endl;
    activityManager->modifyActivity(activityId,clubId);
}
/*
函数名：deleteActivity
功能：删除本社团的活动，报名记录一起删除
实现说明：删除前要确认；调用活动模块时传入本社团编号，活动模块会再次核对权限
*/
void ClubManager::deleteActivity(){
    string activityId=selectMyActivity("请输入要删除的活动编号：");
    if(activityId.empty()){
        return;
    }
    cout<<"确认删除该活动吗？报名记录会一起删除。1.确认  0.取消"<<endl;
    cout<<"请选择：";
    if(readIntChoice()!=1){
        cout<<"已取消。"<<endl;
        return;
    }
    if(activityManager->deleteActivity(activityId,clubId)){
        cout<<"活动已删除！"<<endl;
    }
    else{
        cout<<"删除失败：该活动不存在、不属于本社团或保存失败。"<<endl;
    }
}
/*
函数名：showActivityRegistration
功能：查看本社团某个活动的报名名单和候补名单
说明：showRegistration() 只负责显示、不读输入，所以不需要 skipLine()
*/
void ClubManager::showActivityRegistration(){
    string activityId=selectMyActivity("请输入要查看报名情况的活动编号：");
    if(activityId.empty()){
        return;
    }
    cout<<endl;
    activityManager->showRegistration(activityId);
}
/*
函数名：manageRegistration
功能：管理本社团某个活动的报名成员，可以把某个用户移出名单
实现说明：活动模块的 manageRegistration() 用 getline 读入要移除的用户Id（直接回车退出），
          调用前先 skipLine() 清掉换行；移除正式名额后，候补第一位会自动递补
*/
void ClubManager::manageRegistration(){
    string activityId=selectMyActivity("请输入要管理报名的活动编号：");
    if(activityId.empty()){
        return;
    }
    skipLine();
    cout<<endl;
    activityManager->manageRegistration(activityId,clubId);
}
