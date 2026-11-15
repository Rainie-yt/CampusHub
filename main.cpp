/*
文件名：main.cpp（程序入口）
功能：CampusHub 校园社团活动综合管理系统的主程序。
主要流程：
    1. 设置控制台为 UTF-8 编码，让中文正常显示和输入；
    2. 创建用户、社团、活动三个模块的对象，并从文件中读取社团和活动数据；
    3. 循环显示主菜单（登录、注册、退出）；
    4. 登录成功后，把社团模块和活动模块的地址交给当前用户，
       再通过 User* 调用 showMenu()，进入对应角色的菜单；
    5. 用户退出登录后释放用户对象，回到主菜单。
*/
#include<windows.h>
//windows.h里也定义了STATUS_PENDING，会和common.h里的常量重名，这里取消掉
#undef STATUS_PENDING
#include<iostream>
#include"src/user/UserManager.h"
#include"src/common/InputUtils.h"
#include"src/club/ClubService.h"
#include"src/activity/ActivityManager.h"
#include"src/common.h"
using namespace std;
/*
函数名：showMainMenu
功能：显示系统主菜单
说明：主菜单只负责系统级功能：1 用户登录，2 用户注册，0 退出系统；
      具体的用户功能由各角色自己的 showMenu() 负责。
*/
void showMainMenu(){
    cout<<endl;
    cout<<"================================"<<endl;
    cout<<"CampusHub 校园社团活动综合管理系统"<<endl;
    cout<<"================================"<<endl;
    cout<<"1.用户登录"<<endl;
    cout<<"2.用户注册"<<endl;
    cout<<"0.退出系统"<<endl;
    cout<<endl;
    cout<<"请选择：";
}
/*
函数名：main
功能：程序入口，创建各模块对象并运行系统主循环
实现步骤：
    1. 调用 SetConsoleOutputCP / SetConsoleCP 把控制台输出和输入都设为 UTF-8；
    2. 创建 UserManager（负责登录、注册、读取 users.txt、创建用户对象）；
    3. 创建 ClubService 并读取社团、成员、申请三份数据。
       成员和申请都属于某个社团，所以要先读社团再读这两份；
    4. 创建 ActivityManager 并读取活动数据；已有文件损坏或无法读取时停止启动，
       不进入业务菜单、不补写注销名单，避免不完整数据覆盖原文件；
    5. 进入主循环：
       - 选 1 登录：login() 根据角色返回 Student*、ClubManager* 或 Admin*，这里统一用 User* 接收；
         注入社团和活动模块后调用 showMenu()，退出登录后 delete 释放对象；
       - 选 2 注册；
       - 选 0 退出系统。
关键点：
    clubService 和 activityManager 在 main 中只创建一份，所有角色共用同一份数据，
    用户对象中保存的只是它们的地址（指针）。
*/
int main(){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    UserManager userManager;
    // 先完整校验所有数据；加载失败时禁止进入菜单，避免不完整数据被保存覆盖。
    ClubService clubService;
    ActivityManager activityManager;
    if(!userManager.validateData()
        || !clubService.loadClubs(CLUB_FILE)
        || !clubService.loadMembers(MEMBER_FILE)
        || !clubService.loadApplications(APPLICATION_FILE)
        || !activityManager.load()){
        cerr<<"数据加载失败，启动已取消；原数据文件未修改，请修复后重试。"<<endl;
        return 1;
    }
    // 所有业务数据通过校验后，才允许补充历史注销Id名单。
    if(!userManager.reserveHistoricalIds(clubService,activityManager)){
        cout<<"无法检查或保存历史注销Id，启动已取消，请先检查数据文件权限和格式。"<<endl;
        return 1;
    }
    //系统主循环
    while(true){
        //显示主菜单
        showMainMenu();
        //用 readIntChoice() 代替 cin>>choice，防止非法输入引发死循环
        int choice=readIntChoice();
        switch(choice){
            //用户登录
            case 1:{
                //login() 登录成功后返回 Student*、ClubManager* 或 Admin*，
                //这里统一使用基类指针 User* 接收，体现程序的多态
                User* currentUser=userManager.login();
                //登录成功
                if(currentUser!=nullptr){
                    //把社团模块和活动模块的地址交给当前用户
                    currentUser->setClubService(&clubService);
                    currentUser->setActivityManager(&activityManager);
                    //调用虚函数showMenu()，实际调用哪个版本由currentUser实际指向的对象决定
                    currentUser->showMenu();
                    //用户退出角色菜单以后，删除当前用户对象
                    //User 的析构函数是 virtual，这样才能正确释放派生类对象
                    delete currentUser;
                    currentUser=nullptr;
                }
                break;
            }
            //用户注册
            case 2:{
                userManager.registerUser();
                break;
            }
            //退出系统
            case 0:{
                cout<<endl;
                cout<<"感谢使用CampusHub！"<<endl;
                return 0;
            }
            //输入错误
            default:{
                cout<<"无效的选择，请重新输入！"<<endl;
                break;
            }
        }
    }
    return 0;
}
