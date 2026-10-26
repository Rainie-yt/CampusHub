#pragma once
#include "Activity.h"

// 比赛：特有属性——奖项设置；参加可得 10 分
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
