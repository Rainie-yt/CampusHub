#include "../src/activity/ActivityManager.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

int passed=0;
void check(bool ok,const char* name){
    if(!ok) throw std::runtime_error(name);
    std::cout<<"PASS "<<name<<"\n"; ++passed;
}
std::unique_ptr<Activity> act(const std::string& end="2035-06-01 16:00"){
    return ActivityManager::makeActivity("Lecture","","C001","Test",{"sport"},
        "2035-06-01 14:00",end,"room",1,"pending","speaker");
}
bool modify(ActivityManager& m,const std::string& id,const std::string& input){
    std::istringstream stream(input);
    auto previous=std::cin.rdbuf(stream.rdbuf());
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
    auto id=m.addActivity(act());
    check(id=="3001","ongoing activity allowed and rejected additions consume no id");
    m.setClock("2035-06-01 15:59");
    check(m.tryAuditActivity(id,true)==AuditResult::SUCCESS,"approve one minute before end");
    check(m.tryAuditActivity(id,false)==AuditResult::NOT_PENDING,"already reviewed distinguished");
    auto pending=m.addActivity(act());
    m.setClock("2035-06-01 16:00");
    check(m.tryAuditActivity(pending,true)==AuditResult::ENDED && m.getPendingActivities().size()==1,
          "approval at end refused without changing pending state");
    check(m.auditActivity(pending,false) && m.getPendingActivities().empty(),"bool interface rejects expired pending");
    check(m.tryAuditActivity("missing")==AuditResult::NOT_FOUND,"missing activity distinguished");
    m.setClock("2035-06-01 13:00");
    auto edit=m.addActivity(act());
    check(!modify(m,edit,"\n\n\n\n2035-06-01 11:00\n2035-06-01 13:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 16:00","modify end equal now rejected");
    check(!modify(m,edit,"\n\n\n\n2035-06-01 10:00\n2035-06-01 12:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 16:00","modify past end rejected");
    check(modify(m,edit,"\n\n\n\n\n2035-06-01 17:00\n") &&
          m.findActivity(edit)->getEndTime()=="2035-06-01 17:00","valid future modification accepted");
    m.setClock("2035-06-01 17:00");
    check(!modify(m,edit,"Changed\n"),"already ended activity remains historical");
    m.setClock("2035-06-01 13:00");
    auto expires=m.addActivity(act());
    m.setClock("2035-06-01 16:01");
    check(m.tryAuditActivity(expires,false)==AuditResult::SUCCESS &&
          m.findActivity(expires)->getStatus()=="rejected","activity expiring while pending can be rejected");
    // 已有报名的活动重新送审并过期后，拒绝不能递补或删除历史名单。
    m.setClock("2035-06-01 13:00");
    auto booked=m.addActivity(act());
    check(m.auditActivity(booked) && m.tryJoin("S1",booked)==JoinResult::SUCCESS &&
          m.tryJoin("S2",booked)==JoinResult::WAITLISTED,"registration fixture");
    check(modify(m,booked,"Changed\n\n\n\n"),"edit retains registrations and resubmits");
    m.setClock("2035-06-01 16:00");
    check(m.auditActivity(booked,false) && m.findActivity(booked)->hasParticipant("S1") &&
          m.findActivity(booked)->inWaitList("S2") && m.getUserPoints("S1")==0,
          "expired rejection retains lists without promotion or points");
    std::cout<<"ALL PASSED "<<passed<<"\n";
}
