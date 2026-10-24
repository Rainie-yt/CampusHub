#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <iostream>
#include <limits>
#include <string>

using namespace std;

// 统一读取整数菜单选项
// 如果用户输入非数字内容，则清除错误状态并重新输入
inline int readIntChoice()
{
    int choice;

    while (!(cin >> choice))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "输入无效，请输入数字：";
    }

    return choice;
}

inline string readLine(){
    string line;
    getline(cin>>ws,line);
    return line;
}

#endif