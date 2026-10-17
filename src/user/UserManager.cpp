#include"UserManager.h"
#include"Student.h"
#include"ClubManager.h"
#include"Admin.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;
//UserManager构造函数，设置用户数据文件的位置
UserManager::UserManager(){
    userFile="data/users.txt";
}
//字符串分割函数
vector<string> UserManager::split(const string&str,char delimiter){
    vector<string> result;
    stringstream ss(str);
    string item;
    while(getline(ss,item,delimiter)){
        //按照delimiter依次读取字符串
        result.push_back(item);
    }
    return result;
}
//将字符串转换为vector<string>
vector<string> UserManager::parseInterests(const string& interestData){
    vector<string> interests;
    if(interestData.empty()||interestData=="None"){
        return interests;
    }
    //如果没有兴趣标签，直接返回空列表
    interests=split(interestData,',');
    return interests;
    //使用逗号分割兴趣标签
}
//根据读取到的一行用户数据创建具体用户对象
User* UserManager::createUser(const vector<string>& fields){
    //正常情况下应该至少有6个字段
    if(fields.size()<6){
        return nullptr;
    }
    //读取用户基本信息
    string id=fields[0];
    string name=fields[1];
    string pwd=fields[2];
    string role=fields[3];
    string major=fields[4];
    //判断用户角色，不同角色创建不同的派生类对象
    if(role=="Student"){
        Student* student=new Student(id,name,pwd,major);
        student->setUserManager(this);
        //给创建的对象绑定UserManager指针
        vector<string> interests=parseInterests(fields[5]);
        //读取该学生保存的兴趣标签
        //将兴趣标签加入Student对象
        for(const string& interest:interests){
            student->loadInterest(interest);
        }
        return student;
    }
    //社团负责人
    if(role=="ClubManager"){
        /*
            当前 users.txt 中还没有保存 clubId。

            因此暂时传入空字符串。

            等社团模块确定最终的数据结构后，
            再统一处理 clubId 的来源。
        */
       ClubManager* manager=new ClubManager(id,name,pwd,major,"");
       manager->setUserManager(this);
       //读取兴趣标签
       vector<string> interests=parseInterests(fields[5]);
       //将兴趣标签加入ClubManager对象，ClubManager继承Student,所以可直接使用addInterest()
       for(const string& interest:interests){
        manager->loadInterest(interest);
       }
       return manager;
    }
    if(role=="Admin"){
        /*
        Admin直接继承User,不需要major和interests
        */
       Admin* admin=new Admin(id,name,pwd);
       admin->setUserManager(this);
       return admin;
    }
    //如果role不是系统支持的角色，则无法创建对应对象
    return nullptr;
}
bool UserManager::updateUser(User* user){
    //读全部行，找到该用户那行用最新数据重新拼，整体重写文件
    ifstream inFile(userFile);
    if(!inFile.is_open()){
        cout<<"无法打开用户数据文件！"<<endl;
        return false;
    }
    vector<string> lines;
    string line;
    bool found=false;
    while(getline(inFile,line)){
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        if(line.empty()){
            continue;
        }
        vector<string> fields=split(line,'|');
        if(!fields.empty()&&fields[0]==user->getUserId()){
            //找到目标用户，用最新数据重新拼一行
            string newLine=user->getUserId()+"|"+user->getUserName()+"|"+user->getPassword()+"|"+user->getRole()+"|";
            Student* student=dynamic_cast<Student*>(user);
            if(student!=nullptr){
                newLine+=student->getMajor()+"|";
                vector<string> interests=student->getInterests();
                if(interests.empty()){
                    newLine+="None";
                }
                else{
                    for(size_t i=0;i<interests.size();i++){
                        newLine+=interests[i];
                        if(i!=interests.size()-1){
                            newLine+=",";
                        }
                    }
                }
            }
            else{
                newLine+="None|None";
            }
            lines.push_back(newLine);
            found=true;
        }
        else{
            lines.push_back(line);
        }
    }
    inFile.close();
    if(!found){
        cout<<"未找到该用户，更新失败！"<<endl;
        return false;
    }
    ofstream outFile(userFile);
    if(!outFile.is_open()){
        cout<<"无法写入用户数据文件！"<<endl;
        return false;
    }
    for(const string& l:lines){
        outFile<<l<<endl;
    }
    outFile.close();
    return true;
}
/*
检查用户id是否已经存在，如果存在，则不能重复注册
*/
bool UserManager::userExists(const string& userId){
    ifstream file(userFile);
    //打开用户数据文件
    if(!file.is_open()){
        return false;
    }
    string line;
    while(getline(file,line)){
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        //去掉Windows换行残留的\r，避免最后一个字段比较出错
        if(line.empty()){
            continue;
        }
        //跳过空行
        vector<string> fields=split(line,'|');
        //按“|”分割当前行
        if(!fields.empty()){
            //比较用户id
            if(fields[0]==userId){
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}
/*
    用户登录函数

    登录流程：

        1. 输入用户 ID
        2. 输入密码
        3. 打开 users.txt
        4. 逐行读取用户数据
        5. 找到对应 userId
        6. 检查密码
        7. 创建对应角色对象
        8. 返回 User*

    登录失败返回 nullptr。
*/
User* UserManager::login(){
    string userId;
    cout<<endl;
    cout<<"========用户登录========"<<endl;
    cout<<"请输入用户ID：";
    cin>>userId;

    ifstream file(userFile);
    if(!file.is_open()){
        cout<<"无法打开用户数据文件！"<<endl;
        return nullptr;
    }

    //第一遍：只查ID存不存在，不问密码
    string line;
    vector<string> fields;
    bool found=false;
    while(getline(file,line)){
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        if(line.empty()){
            continue;
        }
        vector<string> currentFields=split(line,'|');
        if(currentFields.size()<6){
            continue;
        }
        if(currentFields[0]==userId){
            fields=currentFields;
            found=true;
            break;
        }
    }
    file.close();

    //ID不存在，直接结束，不问密码
    if(!found){
        cout<<"用户ID不存在！"<<endl;
        return nullptr;
    }

    //ID存在，再问密码
    string password;
    cout<<"请输入密码：";
    cin>>password;

    if(fields[2]!=password){
        cout<<"密码错误！"<<endl;
        return nullptr;
    }

    cout<<"登录成功！"<<endl;
    return createUser(fields);
}
/*
    用户注册函数

    当前注册流程：

        1. 输入用户 ID
        2. 检查 ID 是否重复
        3. 输入用户名
        4. 输入密码
        5. 输入专业
        6. 输入兴趣
        7. 将信息写入 users.txt

    当前注册默认创建 Student。

    ClubManager 和 Admin 的角色变更属于后续权限管理功能，
    不在普通用户注册时直接开放。
*/
bool UserManager::registerUser(){
    string userId;
    string userName;
    string password;
    string major;
    string interestData;
    cout<<endl;
    cout<<"========用户注册========"<<endl;
    cout<<"请输入用户ID：";
    cin>>userId;
    if(userExists(userId)){
        cout<<"该用户ID已存在，请重新注册！"<<endl;
        return false;
    }
    cout<<"请输入用户名：";
    cin>>userName;
    cout<<"请输入密码：";
    cin>>password;
    cout<<"请输入专业：";
    cin>>major;
    cout<<"请输入兴趣标签（多个兴趣用英文逗号分隔）：";
    cin>>interestData;
     /*
        注册用户默认角色为 Student。

        用户不能在注册时自己选择 Admin，
        否则任何人都可以注册成为管理员。

        ClubManager 的角色后续可以由管理员修改，
        或由社团模块进行相关权限设置。
    */
   string role="Student";
   /*
        以追加模式打开文件。

        ios::app 表示：
        新数据添加在文件末尾，
        不会覆盖原有用户数据。
    */
   ofstream file(userFile,ios::app);
   if(!file.is_open()){
    cout<<"无法打开用户数据文件！"<<endl;
    return false;
   }
   //写入格式：userId|userName|password|role|major|interests
   file<<userId<<"|"<<userName<<"|"<<password<<"|"<<role<<"|"<<major<<"|"<<interestData<<endl;
   file.close();
   cout<<"注册成功！"<<endl;
   return true;
}
string UserManager::getUserNameById(const string& userId){
    ifstream file(userFile);
    if(!file.is_open()){
      return "";
    }
    string line;
    while(getline(file,line)){
      if(!line.empty()&&line.back()=='\r'){
        line.pop_back();
      }
      if(line.empty()){
        continue;
      }
      vector<string> fields=split(line,'|');
      if(!fields.empty()&&fields[0]==userId){
        file.close();
        return fields[1];
      }
    }
    file.close();
    return "";
    //遍历完都没找到，说明这个ID不存在
  }