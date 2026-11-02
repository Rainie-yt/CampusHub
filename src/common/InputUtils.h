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

//读取要写进users.txt的内容，不合法就重新输入
//任何内容都不能有|；allowComma、allowSpace为false时也不能有逗号、空格
inline string readField(const  string& prompt,bool allowComma=false,bool allowSppace=false){
    while(true){
        cout<<prompt;
        string s=readLine();
        if(s.find('|')!=string::npos){
            cout<<"不能包含字符 |，请重新输入！"<<endl;
        }
        else if(!allowComma&&s.find(',')!=string::npos){
            cout<<"不能包含逗号，请重新输入！"<<endl;
        }
        else if(!allowComma&&s.find(' ')!=string::npos){
            cout<<"不能包含空格，请重新输入！"<<endl;
        }
        else{
            return s;
        }
    }
}

#endif