#pragma once
#include "Activity.h"

// ============================================================
//  Competition：比赛，抽象基类 Activity 的派生类（公有继承）
//
//  继承关系：Activity（基类）<- Competition。共有属性（编号、名称、时间、名单等）都由基类保存，
//            本类只增加特有属性 prize（奖项设置）。
//  重写的虚函数：
//    getType()    返回类型标识 "Competition"，用于写文件和读文件时还原对象
//    getTypeName() 返回中文类型名 "比赛"
//    getExtra()   返回特有字段 奖项设置，写入文件的最后一列
//    calcPoints() 参加该类活动得 10 分（多态：积分规则由子类决定）
//    showInfo()   显示详情：先调用基类 showBaseInfo()，再显示奖项设置
// ============================================================
class Competition : public Activity {
private:
    std::string prize;   // 奖项设置
public:
    Competition(const std::string& aid, const std::string& cid, const std::string& name,
                const std::vector<std::string>& tagList,
                const std::string& st, const std::string& et,
                const std::string& loc, int maxp,
                const std::string& prizeInfo,
                const std::string& stat = "pending");

    const std::string& getPrize() const { return prize; }

    std::string getType() const override { return "Competition"; }
    std::string getTypeName() const override { return "比赛"; }
    std::string getExtra() const override { return prize; }
    int calcPoints() const override { return 10; }
    void showInfo() const override;
};
