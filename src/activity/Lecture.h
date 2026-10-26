#pragma once
#include "Activity.h"

// 讲座：特有属性——主讲人；参加可得 5 分
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
