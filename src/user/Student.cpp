#include"Student.h"
#include<iostream>
#include<algorithm>
#include"../common/InputUtils.h"
#include"UserManager.h"
#include"../club/ClubService.h"
#include"../activity/ActivityManager.h"
/*
函数名：printClubRow（本文件内部使用的辅助函数）
功能：按"编号  名称（类型）"的格式输出社团列表中的一行
参数：club 社团模块提供的社团简要资料
说明：浏览、搜索、我加入的社团、我的加入申请四个列表共用，保证格式一致
*/
static void printClubRow(const ClubBrief& club){
    cout<<club.clubId<<"  "<<club.clubName<<"（"<<club.type<<"）"<<endl;
}
/*
函数名：Student（构造函数）
功能：调用基类 User 的构造函数设置账号信息，角色固定为 "Student"，并保存专业
参数：id 用户Id；name 用户名；pwd 密码；maj 专业
说明：兴趣标签不在构造函数中设置，由 UserManager 创建对象后调用 loadInterest() 逐个加入
*/
Student::Student(string id,string name,string pwd,string maj):User(id,name,pwd,"Student"),major(maj)
{
}
/*
函数名：getMajor
功能：返回专业，UserManager::updateUser() 写回文件时使用
*/
string Student::getMajor()const{
    return major;
}
/*
函数名：showMenu
功能：学生主菜单，循环显示菜单并根据选择调用对应功能，选 0 退出登录
说明：重写基类 User 的纯虚函数 showMenu()，体现多态
*/
void Student::showMenu(){
    while(true){
        cout<<endl;
        cout<<"欢迎您，"<<userName<<"！"<<endl;
        cout<<"角色：普通学生"<<endl;
        cout<<endl;
        cout<<"========学生菜单========"<<endl;
        cout<<"1.个人信息管理"<<endl;
        cout<<"2.社团中心"<<endl;
        cout<<"3.活动中心"<<endl;
        cout<<"4.兴趣推荐"<<endl;
        cout<<"0.退出登录"<<endl;
        cout<<endl;
        cout<<"请选择：";
        //readIntChoice() 会过滤非数字输入，避免 cin 出错后死循环
        int choice=readIntChoice();
        switch(choice){
            case 1:
                //个人信息管理：查看个人信息、修改密码、管理兴趣标签
                showInfoMenu();
                break;
            case 2:
                clubMenu();
                break;
            case 3:
                activityMenu();
                break;
            case 4:
                showRecommendations();
                break;
            case 0:
                cout<<"退出登录"<<endl;
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
/*
函数名：showInfoMenu
功能：个人信息管理菜单，显示个人信息，可修改密码或进入兴趣标签管理
实现步骤：
    1. 调用基类的 showInfo() 显示Id、用户名、角色，再补充显示专业和兴趣标签；
    2. 选 1：用 readField() 读入新密码（不允许 | 和空格），调用 changePassword() 修改并写回文件；
    3. 选 2、3：分别调用 modifyName()、modifyMajor() 修改用户名和专业；
    4. 选 4：进入 modifyInterest() 兴趣标签管理；
    5. 选 0：返回上一级菜单。
*/
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
        cout<<"2.修改用户名"<<endl;
        cout<<"3.修改专业"<<endl;
        cout<<"4.管理兴趣标签"<<endl;
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
                    modifyName();
                }
                break;
            case 3:
                {
                    modifyMajor();
                }
                break;
            case 4:
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
/*
函数名：modifyName
功能：修改当前用户的姓名，并写回 users.txt
实现步骤：
    1. 用 readField() 读取新姓名，允许空格和逗号，不允许文件分隔符 |；
    2. 保存原姓名，再修改对象中的 userName；
    3. 调用 updateUser() 写回文件，失败时恢复原姓名。
说明：目前项目用 userName 保存姓名，这里不修改用户Id，
      因此不会影响已经加入的社团和活动报名。
*/
void Student::modifyName(){
    string newName=readField("请输入新的用户名：",true,true);
    if(newName==userName){
        cout<<"用户名无变化。"<<endl;
        return;
    }

    string oldName=userName;
    userName=newName;
    if(userManager==nullptr||!userManager->updateUser(this)){
        userName=oldName;
        cout<<"用户名修改失败，原用户名未更改！"<<endl;
        return;
    }
    cout<<"用户名修改成功！"<<endl;
}

/*
函数名：modifyMajor
功能：修改当前用户的专业，并写回 users.txt
实现步骤：
    1. 用 readField() 读取新专业，允许空格和逗号；
    2. 保存原专业，再修改对象中的 major；
    3. 调用 updateUser() 保存，失败时恢复原专业。
关键点：只有文件保存成功才算修改成功，避免内存和文件中的资料不一致。
*/
void Student::modifyMajor(){
    string newMajor=readField("请输入新专业：",true,true);
    if(newMajor==major){
        cout<<"专业无变化。"<<endl;
        return;
    }

    string oldMajor=major;
    major=newMajor;
    if(userManager==nullptr||!userManager->updateUser(this)){
        major=oldMajor;
        cout<<"专业修改失败，原专业未更改！"<<endl;
        return;
    }
    cout<<"专业修改成功！"<<endl;
}
/*
函数名：modifyInterest
功能：兴趣标签管理菜单，可添加或删除兴趣标签
实现说明：
    添加时用 readField() 读入，逗号和空格都不允许，保证一次只加一个标签，
    否则写回文件后再读取时会被拆成多个标签；
    删除时输入的内容只用来比较，不写入文件，直接用 cin 读入。
*/
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
/*
函数名：addInterest
功能：添加一个兴趣标签，并立即写回 users.txt
参数：interest 要添加的兴趣标签
实现步骤：
    1. 用 std::find 在 interests 中查找，已存在则提示并返回，避免重复；
    2. 不存在则加入列表末尾；
    3. 调用 UserManager::updateUser() 写回文件。
*/
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
    //写回失败要撤销刚才的添加，不能提示成功
    if(userManager==nullptr||!userManager->updateUser(this)){
        interests.pop_back();
        cout<<"兴趣标签添加失败，未保存，请稍后重试！"<<endl;
        return;
    }
    cout<<"兴趣标签添加成功！"<<endl;
}
/*
函数名：loadInterest
功能：从 users.txt 恢复已有的兴趣标签
参数：interest 文件中读到的一个兴趣标签
说明：与 addInterest() 不同，只负责恢复数据，不检查重复、不提示、不写文件，
      由 UserManager::createUser() 创建对象时调用
*/
void Student::loadInterest(string interest){
    interests.push_back(interest);
}
/*
函数名：removeInterest
功能：删除一个兴趣标签，并立即写回 users.txt
参数：interest 要删除的兴趣标签
实现步骤：
    1. 用 std::find 找到该标签的位置（迭代器）；
    2. 找到则用 erase 删除，并调用 updateUser() 写回文件；
    3. 找不到则提示"兴趣标签不存在"。
*/
void Student::removeInterest(string interest){
    auto it=std::find(interests.begin(),interests.end(),interest);
    if(it!=interests.end()){
        size_t index=it-interests.begin();
        interests.erase(it);
        //写回失败要把标签放回原位置，不能提示成功
        if(userManager==nullptr||!userManager->updateUser(this)){
            interests.insert(interests.begin()+index,interest);
            cout<<"兴趣标签删除失败，未保存，请稍后重试！"<<endl;
            return;
        }
        cout<<"兴趣标签删除成功！"<<endl;
    }
    else{
        cout<<"兴趣标签不存在！"<<endl;
    }
}
/*
函数名：showInterests
功能：显示兴趣标签，标签之间用顿号隔开，没有标签时显示"无"
实现说明：从第二个标签开始，在前面先输出一个顿号，这样最后一个标签后面不会多出顿号
*/
void Student::showInterests()const{
    if(interests.empty()){
        cout<<"无"<<endl;
        return;
    }
    for(size_t i=0;i<interests.size();i++){
        if(i>0){
            cout<<"、";
        }
        cout<<interests[i];
    }
    cout<<endl;
}
/*
函数名：getInterests
功能：返回兴趣标签列表的副本，写回文件和推荐时使用
*/
vector<string> Student::getInterests()const{
    return interests;
}
/*
函数名：activityMenu
功能：活动中心菜单，学生和社团负责人共用
实现步骤：
    1. 检查活动管理器是否已设置，没有加载时提示并返回；
    2. 循环显示选项：1 浏览并报名，2 我的报名活动，0 返回；
    3. 调用原有函数处理报名、查看积分和取消报名，处理完后回到活动中心。
说明：这里只组织菜单，不直接修改报名数据，具体检查和保存仍由活动模块处理。
*/
void Student::activityMenu(){
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    while(true){
        cout<<endl;
        cout<<"========活动中心========"<<endl;
        cout<<"1.浏览并报名"<<endl;
        cout<<"2.我的报名活动"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();
        switch(choice){
            case 1:
                joinActivityMenu();
                break;
            case 2:
                showMyActivityMenu();
                break;
            case 0:
                return;
            default:
                cout<<"无效的选择，请重新输入！"<<endl;
        }
    }
}
/*
函数名：joinActivityMenu
功能：活动报名。列出可报名的活动，输入编号后显示详情，确认后报名
实现步骤：
    1. 调用活动模块的 showApprovedActivities() 列出已通过且未结束的活动；
    2. 遍历已通过的活动，判断有没有还没结束的，一个都没有就直接返回，不再询问编号；
    3. 输入活动编号（输入 0 返回），用 findActivity() 查找，不存在或未通过审核则提示并返回；
    4. 调用 showInfo() 显示活动详情（地点、主讲人等列表里没有的信息），让用户确认；
    5. 确认后调用 joinActivity()，报名成功、名额已满转入候补、时间冲突、
       活动已结束等结果由活动模块自己判断并输出。
*/
void Student::joinActivityMenu(){
    if(activityManager==nullptr){
        cout<<"活动数据未加载！"<<endl;
        return;
    }
    cout<<endl;
    activityManager->showApprovedActivities();
    //与活动模块列表的规则保持一致：只看已通过且还没结束的活动
    bool hasOpen=false;
    string current=activityManager->now();
    for(const Activity* act:activityManager->getApprovedActivities()){
        //hasEnded() 比较活动结束时间和当前时间（都是"yyyy-mm-dd hh:mm"格式的字符串）
        if(!act->hasEnded(current)){
            hasOpen=true;
            break;
        }
    }
    if(!hasOpen){
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
    //userId 用当前登录用户自己的，不让用户输入
    activityManager->joinActivity(userId,activityId);
}
/*
函数名：showMyActivityMenu
功能：我的报名活动。显示已报名、候补中的活动和积分，可以取消报名
实现步骤：
    1. 调用 showMyActivities() 列出本人报名或候补的活动及状态；
    2. 调用 getUserPoints() 显示积分（参加的活动结束后才计入）；
    3. 选 1 后输入活动编号，调用 cancelActivity() 取消报名，
       取消的是正式名额时，活动模块会让候补名单的第一位自动递补；
    4. 选 0 返回上一级菜单。
*/
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
        //没有报名该活动或活动已结束时返回 false
        if(activityManager->cancelActivity(userId,activityId)){
            cout<<"已成功取消报名！"<<endl;
        }
        else{
            cout<<"取消失败：你没有报名该活动、活动已结束或保存失败。"<<endl;
        }
    }
}
/*
函数名：showRecommendations
功能：根据当前用户的兴趣，分别显示社团和活动推荐结果
实现步骤：
    1. 没有兴趣标签时，提示用户先添加兴趣，再返回；
    2. 社团数据可用时，用兴趣匹配社团名称、简介和分类资料，匹配任意一个就推荐；
    3. 活动数据可用时，调用 recommend() 取得已按匹配度排序的活动，显示前 5 个。
说明：两种推荐分别检查管理器。一种数据不可用时提示原因，另一种推荐仍可以继续。
      社团使用关键词匹配，活动使用标签匹配度，具体计算规则不在这里重复实现。
*/
void Student::showRecommendations()const{
    if(interests.empty()){
        cout<<"你还没有兴趣标签，请先在个人信息管理中添加！"<<endl;
        return;
    }

    cout<<endl;
    cout<<"========为你推荐社团========"<<endl;
    if(clubService!=nullptr){
        vector<ClubBrief> clubs=clubService->getApprovedClubBriefs();
        bool found=false;
        for(const ClubBrief& club:clubs){
            //任意一处包含兴趣标签就推荐，找到后跳出内层循环，避免同一社团重复显示
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
    }else{
        cout<<"社团数据未加载，无法推荐社团！"<<endl;
    }

    cout<<endl;
    cout<<"========为你推荐活动========"<<endl;
    if(activityManager!=nullptr){
        //pair 的 first 是活动指针，second 是匹配度；结果已由活动模块排好序
        vector<pair<const Activity*,double>> acts=activityManager->recommend(interests,userId);
        if(acts.empty()){
            cout<<"暂时没有匹配你兴趣的活动，你可以尝试添加更多兴趣标签！"<<endl;
        }
        for(size_t i=0;i<acts.size()&&i<5;i++){
            cout<<"推荐活动："<<acts[i].first->getActivityId()<<"  "<<acts[i].first->getName()<<endl;
        }
    }else{
        cout<<"活动数据未加载，无法推荐活动！"<<endl;
    }
}
/*
函数名：clubMenu
功能：社团中心菜单，学生和负责人共用
实现说明：
    申请加入、退出社团时，userId 用当前登录用户自己的，不让用户输入，防止替别人操作；
    社团模块修改数据后不会自动保存，操作成功后调用 saveClubData() 写回三个文件。
*/
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
                searchClubMenu();
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
                if(persistChange([&](){
                    return clubService->applyToJoin(userId,clubId)
                        && saveClubData();
                })){
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
                if(persistChange([&](){
                    return clubService->leaveClub(userId,clubId)
                        && saveClubData();
                })){
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
/*
函数名：applyCreateClub
功能：申请成立社团，申请人提交后成为该社团的负责人（待管理员审核）
实现步骤：
    1. 选择社团类型，体育、学术、艺术社团还要填写对应的分类资料，
       对应社团模块的三个派生类 SportsClub、AcademicClub、ArtClub；
    2. 输入社团名称和简介；
    3. 调用 applyClub()，社团模块会检查资料是否合法，
       以及申请人是否已经负责一个待审核或已通过的社团（一人只能负责一个社团）；
    4. 成功后返回新社团编号，保存文件并提示等待管理员审核。
*/
void Student::applyCreateClub(){
    cout<<endl;
    cout<<"========申请成立社团========"<<endl;
    cout<<"1.体育社团  2.学术社团  3.艺术社团  4.普通社团"<<endl;
    cout<<"请选择社团类型：";
    int typeChoice=readIntChoice();
    string type;
    string extraInfo;
    //类型名称必须和社团模块读写文件时用的写法完全一致
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
    //申请失败时返回空字符串
    string newId;
    bool saved=persistChange([&](){
        newId=clubService->applyClub(userId,type,name,intro,extraInfo);
        return !newId.empty() && saveClubData();
    });
    if(newId.empty()){
        cout<<"申请失败：资料不合法，或你已经负责一个社团。"<<endl;
        return;
    }
    if(saved){
        cout<<"申请已提交，社团编号为："<<newId<<"，请等待管理员审核！"<<endl;
    }
}
/*
函数名：browseClubs
功能：浏览所有已审核通过的社团，每个社团一行，最后显示总数
说明：社团资料通过社团模块的 getApprovedClubBriefs() 获取，只包含已通过的社团
*/
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
/*
函数名：searchClubMenu
功能：社团查询子菜单，支持按名称、类型和负责人编号查询
实现步骤：
    1. 循环显示查询方式，选 0 返回社团中心；
    2. 按名称查询时调用原来的 searchClubs()；
    3. 按类型、负责人编号查询时调用社团模块已有的函数。
说明：三种查询都只显示已审核通过的社团。
      类型按完整名称匹配，负责人按用户Id查询，不是按姓名查询。
*/
void Student::searchClubMenu()const{
    if(clubService==nullptr){
        cout<<"社团数据未加载！"<<endl;
        return;
    }

    while(true){
        cout<<endl;
        cout<<"========社团查询========"<<endl;
        cout<<"1.按名称查询"<<endl;
        cout<<"2.按类型查询"<<endl;
        cout<<"3.按负责人编号查询"<<endl;
        cout<<"0.返回上一级菜单"<<endl;
        cout<<endl;
        cout<<"请选择：";
        int choice=readIntChoice();

        switch(choice){
            case 1:{
                cout<<"请输入社团名称关键词：";
                string keyword=readLine();
                searchClubs(keyword);
                break;
            }
            case 2:{
                cout<<"请输入社团类型（体育社团、学术社团、艺术社团、普通社团）：";
                string type=readLine();
                clubService->searchApprovedClubsByType(type);
                break;
            }
            case 3:{
                cout<<"请输入负责人编号：";
                string leaderId;
                cin>>leaderId;
                clubService->searchApprovedClubByLeader(leaderId);
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
函数名：searchClubs
功能：按名称关键词搜索已通过的社团，结果格式与浏览一致
参数：keyword 社团名称关键词
实现说明：逐个检查社团名称是否包含关键词，找到就输出，
          全部检查后仍没有匹配结果时显示提示。
*/
void Student::searchClubs(const string& keyword)const{
    cout<<endl;
    cout<<"========搜索结果========"<<endl;
    bool found=false;
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        if(club.clubName.find(keyword)!=string::npos){
            //如果社团名称包含搜索关键词，就显示该社团
            printClubRow(club);
            found=true;
        }
    }
    if(!found){
        cout<<"没有找到符合条件的社团。"<<endl;
    }
}
/*
函数名：showClubDetail
功能：显示社团详情
参数：clubId 社团编号
实现说明：学生只能查看已审核通过的社团；待审核或被拒绝的社团提示不能查看，
          不存在的编号提示未找到；具体资料由 printClubDetail() 输出
*/
void Student::showClubDetail(const string& clubId)const{
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
/*
函数名：printClubDetail
功能：输出一个已通过社团的完整资料，不带标题
参数：clubId 社团编号
实现步骤：
    1. 在 getApprovedClubBriefs() 返回的列表中找到该社团；
    2. 输出编号、名称、类型；分类资料按社团类型显示为"运动项目""学术领域""艺术类别"，
       叫法与申请成立社团时一致；
    3. 负责人除了编号，还通过 UserManager 查出姓名一起显示；
    4. 成员人数来自社团模块，不含负责人本人。
说明：学生的"社团详情"和负责人的"我的社团"共用本函数，标题由调用者输出
*/
void Student::printClubDetail(const string& clubId)const{
    for(const ClubBrief& club:clubService->getApprovedClubBriefs()){
        if(club.clubId!=clubId){
            continue;
        }
        cout<<"编号："<<club.clubId<<endl;
        cout<<"名称："<<club.clubName<<endl;
        cout<<"类型："<<club.type<<endl;
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
        //getUserNameById() 找不到时返回空字符串，此时只显示编号
        string leaderName=userManager!=nullptr?userManager->getUserNameById(club.leaderId):"";
        if(!leaderName.empty()){
            cout<<" "<<leaderName;
        }
        cout<<endl;
        cout<<"成员人数："<<club.memberCount<<"（不含负责人）"<<endl;
        return;
    }
}
/*
函数名：showMyClubList
功能：显示当前用户加入的社团，每个社团一行
实现说明：成员只会在已通过的社团里，所以逐个查已通过社团的成员名单，
          用 std::find 判断自己的 userId 是否在名单中
*/
void Student::showMyClubList()const{
    cout<<endl;
    cout<<"========我加入的社团========"<<endl;
    bool found=false;
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
/*
函数名：showMyApplicationList
功能：显示当前用户提交的、还在待审核的加入申请，每个社团一行
实现说明：做法与 showMyClubList() 相同，查的是每个社团的申请人名单
*/
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
