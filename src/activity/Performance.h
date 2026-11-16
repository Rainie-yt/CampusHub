#pragma once
#include "Activity.h"

// ============================================================
//  Performance：演出，抽象基类 Activity 的派生类（公有继承）
//
//  继承关系：Activity（基类）<- Performance。共有属性（编号、名称、时间、名单等）都由基类保存，
//            本类只增加特有属性 programType（节目类型）。
//  重写的虚函数：
//    getType()    返回类型标识 "Performance"，用于写文件和读文件时还原对象
//    getTypeName() 返回中文类型名 "演出"
//    getExtra()   返回特有字段 节目类型，写入文件的最后一列
//    calcPoints() 参加该类活动得 8 分（多态：积分规则由子类决定）
//    showInfo()   显示详情：先调用基类 showBaseInfo()，再显示节目类型
// ============================================================
class Performance : public Activity {
private:
    std::string programType;   // 节目类型，如 音乐会 / 话剧 / 舞蹈
public:
    Performance(const std::string& aid, const std::string& cid, const std::string& name,
                const std::vector<std::string>& tagList,
                const std::string& st, const std::string& et,
                const std::string& loc, int maxp,
                const std::string& program,
                const std::string& stat = "pending");

    const std::string& getProgramType() const { return programType; }

    std::string getType() const override { return "Performance"; }
    std::string getTypeName() const override { return "演出"; }
    std::string getExtra() const override { return programType; }
    int calcPoints() const override { return 8; }
    void showInfo() const override;
};
