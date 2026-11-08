/*
文件名：InputUtils.h（输入工具函数）
功能：把程序中反复用到的几种键盘输入方式统一写成函数，各个模块共用。
说明：函数都写在头文件中，所以声明为 inline，
      这样多个 .cpp 文件包含这个头文件时不会出现"函数重复定义"的链接错误。
*/
#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <iostream>
#include <limits>
#include <string>

using namespace std;

/*
函数名：readIntChoice
功能：统一读取整数菜单选项
返回值：用户输入的整数
实现步骤：
    1. 用 cin>>choice 读入整数；
    2. 如果用户输入了字母等非数字内容，cin 会进入错误状态，读取失败：
       先用 cin.clear() 清除错误状态，
       再用 cin.ignore() 丢掉这一行剩下的内容，
       提示后重新输入，直到读到整数为止。
关键点：如果不清除错误状态，cin 之后的读取会一直失败，菜单会陷入死循环。
*/
inline int readIntChoice()
{
    int choice;

    while (!(cin >> choice))
    {
        cin.clear();
        //numeric_limits<streamsize>::max() 表示不限个数，一直丢到换行符为止
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "输入无效，请输入数字：";
    }

    return choice;
}

/*
函数名：readLine
功能：读取一整行输入，内容中间可以有空格（如社团名称、社团简介）
返回值：读到的这一行
关键点：cin>>ws 会先跳过前面残留的空白字符和换行符，
        否则在 cin>> 之后直接调用 getline，会读到上一次输入剩下的换行，得到空字符串。
*/
inline string readLine(){
    string line;
    getline(cin>>ws,line);
    return line;
}

/*
函数名：readField
功能：读取要写进 users.txt 的内容，不合法就提示并重新输入
参数：prompt 输入提示语；
      allowComma 是否允许逗号，默认不允许；
      allowSpace 是否允许空格，默认不允许
返回值：合法的输入内容
实现说明：
    users.txt 用 | 分隔字段，所以任何内容都不能有 |，否则读文件时字段会错位；
    兴趣标签用逗号分隔，用户Id、密码等字段如果含逗号也会引起混乱，
    所以 allowComma 为 false 时，中文逗号和英文逗号都不允许；
    allowSpace 为 false 时不能含有空格（用户Id、密码）。
*/
inline string readField(const string& prompt,bool allowComma=false,bool allowSpace=false){
    while(true){
        cout<<prompt;
        string s=readLine();
        if(s.find('|')!=string::npos){
            cout<<"不能包含字符 |，请重新输入！"<<endl;
        }
        else if(!allowComma&&(s.find(',')!=string::npos||s.find("，")!=string::npos)){
            cout<<"不能包含逗号，请重新输入！"<<endl;
        }
        else if(!allowSpace&&s.find(' ')!=string::npos){
            cout<<"不能包含空格，请重新输入！"<<endl;
        }
        else{
            return s;
        }
    }
}

/*
函数名：skipLine
功能：丢掉 cin>> 或 readIntChoice() 之后输入缓冲区里剩下的换行
说明：活动模块的发布、修改等函数用 getline 读整行，
      调用前要先清掉这个换行，否则第一次 getline 会直接读到空行。
*/
inline void skipLine(){
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
}

#endif
