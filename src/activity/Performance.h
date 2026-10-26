#pragma once
#include "Activity.h"

// 演出：特有属性——节目类型；参加可得 8 分
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
