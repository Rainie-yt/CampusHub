#include<windows.h>
#undef STATUS_PENDING
//windows.h里也定义了STATUS_PENDING，会和common.h里的常量重名，这里取消掉
#include<iostream>
#include"src/user/UserManager.h"
#include"src/common/InputUtils.h"
#include"src/club/ClubService.h"
#include"src/activity/ActivityManager.h"
#include"src/common.h"
using namespace std;
/*
    显示系统主菜单

    主菜单只负责系统级功能：

    1. 用户登录
    2. 用户注册
    0. 退出系统

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
int main(){
    /*
        创建用户管理对象。

        UserManager 负责：
        - 登录
        - 注册
        - 读取 users.txt
        - 创建用户对象
    */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    UserManager userManager;
    //全程只用这一个ClubService，所有角色共用
    ClubService clubService;
    if(clubService.loadClubs(CLUB_FILE)){
        //成员和申请都要先有社团，所以放在后面读
        clubService.loadMembers(MEMBER_FILE);
        clubService.loadApplications(APPLICATION_FILE);
    }
    else{
        cout<<"提示：未读取到社团数据，将从空列表开始。"<<endl;
    }
    //活动数据，文件不存在时从空列表开始，修改后自动保存
    ActivityManager activityManager;
    activityManager.load();
    //系统主循环
    while(true){
        //显示主菜单
        showMainMenu();
        int choice=readIntChoice();
        //替换原来的cin>>choice，防止非法输入引发死循环
        switch(choice){
            //用户登录
            case 1:{
                /*
                    login() 登录成功后：

                    如果是 Student：
                        返回 Student*

                    如果是 ClubManager：
                        返回 ClubManager*

                    如果是 Admin：
                        返回 Admin*

                    但是这里统一使用 User* 接收。

                    展现程序的多态。
                */
                User* currentUser=userManager.login();
                //登录成功
                if(currentUser!=nullptr){
                    currentUser->setClubService(&clubService);
                    currentUser->setActivityManager(&activityManager);
                    currentUser->showMenu();
                    //调用虚函数showMenu()，实际调用哪个版本由currentUser实际指向的对象决定
                    /*
                        用户退出角色菜单以后，
                        删除当前用户对象。

                        User 析构函数必须是 virtual，
                        这样才能正确释放派生类对象。
                    */
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