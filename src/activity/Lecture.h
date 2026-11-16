#pragma once
#include "Activity.h"

// ============================================================
//  Lecture：讲座，抽象基类 Activity 的派生类（公有继承）
//
//  继承关系：Activity（基类）<- Lecture。共有属性（编号、名称、时间、名单等）都由基类保存，
//            本类只增加特有属性 speaker（主讲人）。
//  重写的虚函数：
//    getType()    返回类型标识 "Lecture"，用于写文件和读文件时还原对象
//    getTypeName() 返回中文类型名 "讲座"
//    getExtra()   返回特有字段 主讲人，写入文件的最后一列
//    calcPoints() 参加该类活动得 5 分（多态：积分规则由子类决定）
//    showInfo()   显示详情：先调用基类 showBaseInfo()，再显示主讲人
// ============================================================
class Lecture : public Activity {
private:
    std::string speaker;   // 主讲人
public:
    Lecture(const std::string& aid, const std::string& cid, const std::string& name,
            const std::vector<std::string>& tagList,
            const std::string& st, const std::string& et,
            const std::string& loc, int maxp,
            const std::string& speakerName,
            const std::string& stat = "pending");

    const std::string& getSpeaker() const { return speaker; }

    std::string getType() const override { return "Lecture"; }
    std::string getTypeName() const override { return "讲座"; }
    std::string getExtra() const override { return speaker; }
    int calcPoints() const override { return 5; }
    void showInfo() const override;
};
