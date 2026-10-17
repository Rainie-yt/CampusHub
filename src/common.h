#pragma once
#include<string>
using namespace std;

/*
    common.h —— 用户、社团、活动三个模块共用的定义

    注意：这里不重复定义Club、Activity这些已经在
    各自模块里写成完整class的对象（比如活动模块的
    Activity类），只放大家都要用、但不属于任何一个
    模块的东西：统一常量 + 一个轻量的用户信息结构体。
*/

// ------------------ 角色常量 ------------------
const string ROLE_STUDENT      = "Student";
const string ROLE_CLUB_MANAGER = "ClubManager";
const string ROLE_ADMIN        = "Admin";

// ------------------ 审核状态常量 ------------------
const string STATUS_PENDING  = "pending";
const string STATUS_APPROVED = "approved";
const string STATUS_REJECTED = "rejected";

// ------------------ 跨模块用户信息 ------------------
struct UserBrief {
    string userId;
    string userName;
    string role;
};
