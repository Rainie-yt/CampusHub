#include"UserManager.h"
#include"Student.h"
#include"ClubManager.h"
#include"Admin.h"
#include"../common/InputUtils.h"
#include"../common/TextUtils.h"
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
            for(size_t i=6;i<fields.size();i++){
                newLine+="|"+fields[i];
            }
            //保留第7个字段（账号状态），否则改密码会把冻结状态丢掉
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
    cout<<"请输入用户Id：";
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
        cout<<"用户Id不存在！"<<endl;
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

    //密码对了才提示冻结，避免别人借此试探账号状态
    if(statusOf(fields)==USER_FROZEN){
        cout<<"该账号已被冻结，请联系管理员！"<<endl;
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
    userId=readField("请输入用户Id：");
    if(userExists(userId)){
        cout<<"该用户Id已存在，请重新注册！"<<endl;
        return false;
    }
    userName=readField("请输入用户名：",true,true);
    //用户名和专业允许空格、逗号，只拦|
    password=readField("请输入密码：");
    major=readField("请输入专业：",true,true);
    while(true){
        interestData=readField("请输入兴趣标签（多个兴趣用逗号分隔）：",true,false);
        //把中文逗号统一换成英文逗号，用户两种都能用
        size_t pos;
        while((pos=interestData.find("，"))!=string::npos){
            interestData.replace(pos,3,",");
            //中文逗号在UTF-8里占3个子节，所以替换长度是3
        }
    //逗号是分隔符，但不能出现在开头、结尾或连着两个
    if(interestData.front()==','||interestData.back()==','||interestData.find(",,")!=string::npos){
        cout<<"兴趣标签不能为空，请重新输入！"<<endl;
        continue;
    }
    break;
    }
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
    file<<userId<<"|"<<userName<<"|"<<password<<"|"<<role<<"|"<<major<<"|"<<interestData<<"|"<<USER_NORMAL<<endl;
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
bool UserManager::changeRole(const string& userId,const string& newRole){
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
        if(fields.size()>=4&&fields[0]==userId){
            fields[3]=newRole;
            //第4个字段是角色，改完重新用|拼回去
            line=fields[0];
            for(size_t i=1;i<fields.size();i++){
                line+="|"+fields[i];
            }
            found=true;
        }
        lines.push_back(line);
    }
    inFile.close();
    if(!found){
        cout<<"未找到该用户，修改角色失败！"<<endl;
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
bool UserManager::showUserInfoById(const string& userId){
    ifstream file(userFile);
    if(!file.is_open()){
        cout<<"无法打开用户数据文件！"<<endl;
        return false;
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
        if(fields.size()>=6&&fields[0]==userId){
            file.close();
            //不显示密码fields[2]
            cout<<"用户Id："<<fields[0]<<endl;
            cout<<"用户名："<<fields[1]<<endl;
            cout<<"用户角色："<<roleName(fields[3])<<endl;
            cout<<"专业："<<fields[4]<<endl;
            if(fields[5]=="None"){
                cout<<"兴趣标签：无"<<endl;
            }
            else{
                //文件里用逗号分隔，显示时换成顿号
                string tags=fields[5];
                size_t pos;
                while((pos=tags.find(','))!=string::npos){
                    tags.replace(pos,1,"、");
                }
                cout<<"兴趣标签："<<tags<<endl;
            }
            return true;
        }
    }
    file.close();
    cout<<"未找到该用户！"<<endl;
    return false;
}
bool UserManager::readLines(vector<string>& lines){
    ifstream file(userFile);
    if(!file.is_open()){
        cout<<"无法打开用户数据文件！"<<endl;
        return false;
    }
    string line;
    while(getline(file,line)){
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        if(!line.empty()){
            lines.push_back(line);
        }
    }
    file.close();
    return true;
}
bool UserManager::writeLines(const vector<string>& lines){
    ofstream file(userFile);
    if(!file.is_open()){
        cout<<"无法写入用户数据文件！"<<endl;
        return false;
    }
    for(const string& l:lines){
        file<<l<<endl;
    }
    file.close();
    return true;
}
string UserManager::statusOf(const vector<string>& fields){
    if(fields.size()>=7&&fields[6]==USER_FROZEN){
        return USER_FROZEN;
    }
    return USER_NORMAL;
}
string UserManager::roleName(const string& role){
    if(role=="Student"){
        return "普通学生";
    }
    if(role=="ClubManager"){
        return "社团负责人";
    }
    if(role=="Admin"){
        return "系统管理员";
    }
    return role;
}
void UserManager::showAllUsers(){
    vector<string> lines;
    if(!readLines(lines)){
        return;
    }
    cout<<endl;
    cout<<"========用户列表========"<<endl;
    int count=0;
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(fields.size()<6){
            continue;
        }
        //按显示宽度补空格，让各列对齐
        cout<<padText(fields[0],6)<<padText(fields[1],12)<<padText(roleName(fields[3]),12);
        if(statusOf(fields)==USER_FROZEN){
            cout<<"已冻结";
        }
        cout<<endl;
        count++;
    }
    cout<<endl;
    cout<<"共"<<count<<"个用户"<<endl;
}
void UserManager::showUserStatistics(){
    vector<string> lines;
    if(!readLines(lines)){
        return;
    }
    int total=0,students=0,managers=0,admins=0,frozen=0;
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(fields.size()<6){
            continue;
        }
        total++;
        if(fields[3]=="Student"){
            students++;
        }
        else if(fields[3]=="ClubManager"){
            managers++;
        }
        else if(fields[3]=="Admin"){
            admins++;
        }
        if(statusOf(fields)==USER_FROZEN){
            frozen++;
        }
    }
    cout<<"用户总数："<<total<<endl;
    cout<<"普通学生："<<students<<endl;
    cout<<"社团负责人："<<managers<<endl;
    cout<<"系统管理员："<<admins<<endl;
    cout<<"已冻结账号："<<frozen<<endl;
}
string UserManager::getUserRole(const string& userId){
    vector<string> lines;
    if(!readLines(lines)){
        return "";
    }
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(fields.size()>=6&&fields[0]==userId){
            return fields[3];
        }
    }
    return "";
}
string UserManager::getUserStatus(const string& userId){
    vector<string> lines;
    if(!readLines(lines)){
        return "";
    }
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(fields.size()>=6&&fields[0]==userId){
            return statusOf(fields);
        }
    }
    return "";
}
bool UserManager::setUserStatus(const string& userId,const string& status){
    vector<string> lines;
    if(!readLines(lines)){
        return false;
    }
    bool found=false;
    for(string& line:lines){
        vector<string> fields=split(line,'|');
        if(fields.size()>=6&&fields[0]==userId){
            //旧数据只有6个字段，补到7再改
            while(fields.size()<7){
                fields.push_back(USER_NORMAL);
            }
            fields[6]=status;
            line=fields[0];
            for(size_t i=1;i<fields.size();i++){
                line+="|"+fields[i];
            }
            found=true;
            break;
        }
    }
    if(!found){
        return false;
    }
    return writeLines(lines);
}
bool UserManager::deleteUser(const string& userId){
    vector<string> lines;
    if(!readLines(lines)){
        return false;
    }
    vector<string> kept;
    bool found=false;
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(!fields.empty()&&fields[0]==userId){
            found=true;
            //跳过这一行，相当于删除
            continue;
        }
        kept.push_back(line);
    }
    if(!found){
        return false;
    }
    return writeLines(kept);
}