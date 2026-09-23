//防止非法输入使程序陷入死循环
#pragma once
#include<iostream>
#include<limits>
using namespace std;
/*
    安全读取一个整数选项。
    如果用户输入的不是数字，会清除cin的错误状态，
    丢弃这一行剩余的非法字符，并要求重新输入，
    避免cin一直处于失败状态导致死循环。
*/
inline int readIntChoice(){
    int choice;
    while(!(cin>>choice)){
        //输入非数字时cin会失败，循环拦截避免死循环
        cin.clear();
        //清除cin的错误状态
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        //丢弃这行残留的非法字符
        cout<<"输入无效，请输入数字：";
    }
    return choice;
}