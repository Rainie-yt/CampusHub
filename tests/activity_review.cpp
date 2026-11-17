/*
文件名：activity_review.cpp
功能：固定当前时间，检查发布、修改和审核在活动结束前后的处理结果。
说明：关闭自动保存，只在内存里构造测试活动，不读取或改写项目的业务数据。
      由 activity_review.py 编译运行，任意检查不通过就以失败结束。
*/
#include "../src/activity/ActivityManager.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

int passed=0;
/*
函数名：check
功能：判断一项检查是否通过，通过后累计数量，失败时抛出异常并结束测试
参数：ok 检查结果；name 本项检查的说明
*/
void check(bool ok,const char* name){
    if(!ok) throw std::runtime_error(name);
    std::cout<<"PASS "<<name<<"\n"; ++passed;
}
/*
函数名：act
功能：创建供测试使用的待审核讲座，统一社团、开始时间和人数上限
参数：end 结束时间，默认 2035-06-01 16:00
返回值：新建的活动对象，尚未加入管理器或分配编号。
*/
std::unique_ptr<Activity> act(const std::string& end="2035-06-01 16:00"){
    return ActivityManager::makeActivity("Lecture","","C001","Test",{"sport"},
        "2035-06-01 14:00",end,"room",1,"pending","speaker");
}
/*
函数名：modify
功能：把字符串临时当作键盘输入，测试交互式修改活动
参数：m 活动管理器；id 要修改的活动编号；input 模拟输入，每次回车用 \n 表示
返回值：modifyActivity 的执行结果
说明：调用后恢复 cin 原来的输入缓冲区，避免影响下一项测试。
*/
bool modify(ActivityManager& m,const std::string& id,const std::string& input){
    std::istringstream stream(input);
    std::streambuf* previous=std::cin.rdbuf(stream.rdbuf());
    bool result=m.modifyActivity(id,"C001");
    std::cin.rdbuf(previous);
    return result;
}
int main(){
    ActivityManager m;
    m.setAutoSave(false);
    m.setClock("2035-06-01 16:00");
    check(m.addActivity(act()).empty(),"new end equal to now rejected");
    m.setClock("2035-06-01 16:01");
    check(m.addActivity(act()).empty(),"new end before now rejected");
    m.setClock("2035-06-01 15:00");
    std::string id=m.addActivity(act());
    std::string reason;
    check(id=="3001","ongoing activity allowed and rejected additions consume no id");
    m.setClock("2035-06-01 15:59");
    check(m.auditActivity(id,true,&reason) && reason.empty(),"approve one minute before end");
    check(!m.auditActivity(id,false,&reason) && reason=="该活动不是待审核状态。","already reviewed distinguished");
    std::string pending=m.addActivity(act());
    m.setClock("2035-06-01 16:00");
    check(!m.auditActivity(pending,true,&reason) && reason.find("活动已结束")!=std::string::npos && m.getPendingActivities().size()==1,
          "approval at end refused without changing pending state");
    check(m.auditActivity(pending,false) && m.getPendingActivities().empty(),"bool interface rejects expired pending");
    check(!m.auditActivity("missing",true,&reason) && reason=="活动不存在。","missing activity distinguished");
    m.setClock("2035-06-01 13:00");
    std::string edit=m.addActivity(act());
    check(!modify(m,edit,"\n\n\n\n2035-06-01 11:00\n2035-06-01 13:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 16:00","modify end equal now rejected");
    check(!modify(m,edit,"\n\n\n\n2035-06-01 10:00\n2035-06-01 12:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 16:00","modify past end rejected");
    check(modify(m,edit,"\n\n\n\n\n2035-06-01 17:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 17:00","valid future modification accepted");
    m.setClock("2035-06-01 17:00");
    check(!modify(m,edit,"Changed\n"),"already ended activity remains historical");
    m.setClock("2035-06-01 13:00");
    std::string expires=m.addActivity(act());
    m.setClock("2035-06-01 16:01");
    check(m.auditActivity(expires,false,&reason) && reason.empty() &&
          m.findActivity(expires)->getStatus()=="rejected","activity expiring while pending can be rejected");
    // 已经有报名的活动过期后再拒绝，报名和候补都应该保留下来。
    m.setClock("2035-06-01 13:00");
    std::string booked=m.addActivity(act());
    check(m.auditActivity(booked) && m.tryJoin("S1",booked)==JoinResult::SUCCESS &&
          m.tryJoin("S2",booked)==JoinResult::WAITLISTED,"registration fixture");
    check(!modify(m,booked,"Changed\n") && m.findActivity(booked)->getName()=="Test" &&
          m.findActivity(booked)->isApproved() && m.findActivity(booked)->hasParticipant("S1") &&
          m.findActivity(booked)->inWaitList("S2"),"booked activity cannot be edited");
    // 模拟旧版本留下的带报名待审核活动，仍需兼容过期后拒绝并保留名单。
    m.findActivity(booked)->setStatus("pending");
    m.setClock("2035-06-01 16:00");
    check(m.auditActivity(booked,false) && m.findActivity(booked)->hasParticipant("S1") &&
          m.findActivity(booked)->inWaitList("S2") && m.getUserPoints("S1")==0,
          "expired rejection retains lists without promotion or points");
    std::cout<<"ALL PASSED "<<passed<<"\n";
}
