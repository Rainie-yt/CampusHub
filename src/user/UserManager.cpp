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
#include"../common/FileStore.h"
#include"../common/DataFile.h"
#include<algorithm>
#include"../club/ClubService.h"
#include"../activity/ActivityManager.h"
#include<set>
using namespace std;
/*
函数名：UserManager（构造函数）
功能：设置用户数据文件的位置
说明：路径是相对路径，程序要在项目根目录下运行才能找到 data 文件夹
*/
UserManager::UserManager(){
    userFile="data/users.txt";
}

/*
函数名：validateData
功能：启动时检查账号文件，避免错误账号参与登录或历史注销Id处理
返回值：文件不存在或所有记录通过检查返回 true；读取失败或账号记录有错返回 false。
实现步骤：
    1. 读取账号文件，检查文件头是否含有当前账号读取方式不支持的 BOM；
    2. 每行按 | 拆分，检查字段数量、Id、密码、角色和已有的状态字段；
    3. 用 set 保存读过的Id，发现重复时报告文件名和行号。
说明：兼容没有状态列的旧六字段记录。本函数只做检查，不修复或改写文件，
      main 收到 false 后停止启动，防止错误数据在后续操作中被覆盖。
*/
bool UserManager::validateData(){
    string text;
    if(!DataFile::read(userFile,text)) return false;
    // 其他账号读取入口按原字段读取，不默默接受会改变Id含义的BOM。
    if(text.compare(0,3,"\xEF\xBB\xBF")==0)
        return DataFile::error(userFile,1,"请使用不带BOM的UTF-8保存账号文件");
    istringstream input(text);
    string line;
    size_t number=0;
    set<string> ids;
    while(DataFile::nextLine(input,line,number)){
        if(line.empty()) continue;
        const vector<string> fields=split(line,'|');
        if(fields.size()<6 || fields[0].empty() || containsSpace(fields[0])
            || fields[1].empty() || fields[2].empty() || containsSpace(fields[2])
            || (fields[3]!="Student" && fields[3]!="ClubManager" && fields[3]!="Admin")
            || (fields.size()>=7 && fields[6]!=USER_NORMAL && fields[6]!=USER_FROZEN))
            return DataFile::error(userFile,number,"账号字段不完整或Id、密码、角色、状态不合法");
        // insert 返回的 second 为 false，说明这个Id之前已经出现过。
        if(!ids.insert(fields[0]).second)
            return DataFile::error(userFile,number,"用户Id重复");
    }
    return true;
}
/*
函数名：split
功能：按分隔符把一个字符串切成若干段
参数：str 要分割的字符串；delimiter 分隔符（如 '|' 或 ','）
返回值：分割后的各段，按原顺序存放在 vector 中
实现说明：把字符串放进 stringstream，再用 getline 的第三个参数指定分隔符，
          每次读到分隔符就停下，得到一段
*/
vector<string> UserManager::split(const string&str,char delimiter){
    vector<string> result;
    stringstream ss(str);
    string item;
    //按照 delimiter 依次读取字符串
    while(getline(ss,item,delimiter)){
        result.push_back(item);
    }
    return result;
}
/*
函数名：parseInterests
功能：把文件中保存的兴趣字符串转换成 vector<string>
参数：interestData 兴趣字符串，如 "机器人,人工智能"；没有兴趣时文件中保存的是 "None"
返回值：兴趣标签列表；没有兴趣标签时返回空列表
*/
vector<string> UserManager::parseInterests(const string& interestData){
    vector<string> interests;
    //如果没有兴趣标签，直接返回空列表
    if(interestData.empty()||interestData=="None"){
        return interests;
    }
    //使用英文逗号分割兴趣标签
    interests=split(interestData,',');
    return interests;
}
/*
函数名：createUser
功能：根据 users.txt 中一行拆分出的字段，创建对应角色的用户对象
参数：fields 一行数据按 | 拆分后的字段（id、name、pwd、role、major、interests、status）
返回值：用户对象指针（User*）；字段不足或角色不认识时返回 nullptr
实现步骤：
    1. 字段少于 6 个说明这一行数据不完整，返回 nullptr；
    2. 取出用户Id、用户名、密码、角色、专业；
    3. 根据角色用 new 创建 Student、ClubManager 或 Admin 对象，
       并调用 setUserManager(this) 绑定本对象，之后修改密码等操作才能写回文件；
    4. Student 和 ClubManager 还要用 loadInterest() 恢复兴趣标签；
    5. 返回基类指针 User*，main 用它调用 showMenu() 时会执行对应派生类的版本（多态）。
关键点：对象是用 new 创建的，由 main 在退出登录后用 delete 释放。
*/
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
        //给创建的对象绑定 UserManager 指针
        student->setUserManager(this);
        //读取该学生保存的兴趣标签，逐个加入 Student 对象
        vector<string> interests=parseInterests(fields[5]);
        for(const string& interest:interests){
            student->loadInterest(interest);
        }
        return student;
    }
    //社团负责人
    if(role=="ClubManager"){
        //users.txt 中没有保存负责人的社团编号，所以这里传入空字符串，
        //登录后在 ClubManager::showMenu() 中通过社团模块按负责人Id查出
        ClubManager* manager=new ClubManager(id,name,pwd,major,"");
        manager->setUserManager(this);
        //ClubManager 继承自 Student，所以可以直接使用 loadInterest()
        vector<string> interests=parseInterests(fields[5]);
        for(const string& interest:interests){
            manager->loadInterest(interest);
        }
        return manager;
    }
    if(role=="Admin"){
        //Admin 直接继承 User，不需要 major 和 interests
        Admin* admin=new Admin(id,name,pwd);
        admin->setUserManager(this);
        return admin;
    }
    //如果 role 不是系统支持的角色，则无法创建对应对象
    return nullptr;
}
/*
函数名：updateUser
功能：把一个用户的最新数据（密码、兴趣标签等）写回 users.txt
参数：user 要保存的用户对象
返回值：成功返回 true；文件打不开或找不到该用户返回 false
实现步骤：
    1. 逐行读入 users.txt，去掉行尾的 \r，跳过空行；
    2. 找到 Id 相同的那一行，用对象中的最新数据重新拼成一行：
       - 前四个字段：用户Id、用户名、密码、角色；
       - 用 dynamic_cast 判断 user 是否为 Student（ClubManager 也是 Student），
         是的话再拼上专业和兴趣标签（没有标签写 None），
         否则（管理员）写 None|None；
       - 原来第 7 个及以后的字段（账号状态）原样保留，否则改密码会把冻结状态丢掉；
    3. 其他行原样保存；
    4. 把所有行整体覆盖写回文件。
关键点：文件不能只改中间一行，所以采用"全部读入 → 修改 → 全部写回"的方法。
*/
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
        //去掉 Windows 换行残留的 \r
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
            //dynamic_cast 转换失败返回 nullptr，说明这个用户不是学生（是管理员）
            Student* student=dynamic_cast<Student*>(user);
            if(student!=nullptr){
                newLine+=student->getMajor()+"|";
                vector<string> interests=student->getInterests();
                if(interests.empty()){
                    newLine+="None";
                }
                else{
                    //兴趣标签之间用逗号连接，最后一个后面不加逗号
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
            //保留第7个字段（账号状态），否则改密码会把冻结状态丢掉
            for(size_t i=6;i<fields.size();i++){
                newLine+="|"+fields[i];
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
    return writeLines(lines);
}

/*
函数名：userExists
功能：检查用户Id是否已经存在，存在则不能重复注册
参数：userId 要检查的用户Id
返回值：存在返回 true；不存在或文件打不开返回 false
实现说明：逐行读取 users.txt，按 | 分割后比较第一个字段
*/
bool UserManager::userExists(const string& userId){
    //打开用户数据文件
    ifstream file(userFile);
    if(!file.is_open()){
        return false;
    }
    string line;
    while(getline(file,line)){
        //去掉 Windows 换行残留的 \r，避免最后一个字段比较出错
        if(!line.empty()&&line.back()=='\r'){
            line.pop_back();
        }
        //跳过空行
        if(line.empty()){
            continue;
        }
        //按"|"分割当前行，比较用户Id
        vector<string> fields=split(line,'|');
        if(!fields.empty()){
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
函数名：login
功能：用户登录
返回值：登录成功返回对应角色的用户对象指针；失败返回 nullptr
实现步骤：
    1. 输入用户Id；
    2. 第一遍读文件，只查这个 Id 是否存在，不存在就直接提示并结束，不再询问密码；
    3. Id 存在，再输入密码并比较（fields[2] 是密码）；
    4. 密码正确后再检查账号是否被冻结，
       放在密码之后判断，是为了不让别人只输入 Id 就能试探出账号状态；
    5. 全部通过后调用 createUser() 创建对应角色的对象并返回。
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
        //字段不足的行数据不完整，跳过
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
函数名：registerUser
功能：注册新用户，注册的用户角色固定为普通学生
返回值：注册成功返回 true；Id 已存在或文件打不开返回 false
实现步骤：
    1. 输入用户Id，检查是否已经存在；
    2. 输入用户名、密码、专业，都用 readField() 读入：
       所有字段都不能含有 |（文件的分隔符）；
       用户名和专业允许空格和逗号，用户Id和密码不允许空格；
    3. 输入兴趣标签：先把中文逗号统一换成英文逗号，
       再检查逗号不能出现在开头、结尾或连续出现（否则会产生空标签），不合法就重新输入；
    4. 角色固定为 Student，用户不能在注册时自己选择管理员，
       否则任何人都可以注册成为管理员；社团负责人由管理员审核社团后设置；
    5. 以追加模式打开 users.txt，写入一行新用户数据，状态为 normal。
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
    vector<string> retiredIds;
    if(!readRetiredIds(retiredIds)) return false;
    if(find(retiredIds.begin(),retiredIds.end(),userId)!=retiredIds.end()){
        cout<<"该用户Id已注销，不能再次注册，请使用新的Id！"<<endl;
        return false;
    }
    string existingText;
    bool usersExist=false;
    if(!FileStore::read(userFile,existingText,usersExist)){
        cout<<"无法读取用户数据，注册已取消！"<<endl;
        return false;
    }
    if(userExists(userId)){
        cout<<"该用户Id已存在，请重新注册！"<<endl;
        return false;
    }
    //用户名和专业允许空格、逗号，只拦 |
    userName=readField("请输入用户名：",true,true);
    password=readField("请输入密码：");
    major=readField("请输入专业：",true,true);
    while(true){
        interestData=readField("请输入兴趣标签（多个兴趣用逗号分隔）：",true,false);
        //把中文逗号统一换成英文逗号，用户两种都能用
        size_t pos;
        while((pos=interestData.find("，"))!=string::npos){
            //中文逗号在 UTF-8 里占 3 个字节，所以替换长度是 3
            interestData.replace(pos,3,",");
        }
        //逗号是分隔符，但不能出现在开头、结尾或连着两个
        if(interestData.front()==','||interestData.back()==','||interestData.find(",,")!=string::npos){
            cout<<"兴趣标签不能为空，请重新输入！"<<endl;
            continue;
        }
        break;
    }
    //注册用户默认角色为 Student
    string role="Student";
    if(!existingText.empty()&&existingText.back()!='\n') existingText+='\n';
    existingText+=userId+"|"+userName+"|"+password+"|"+role+"|"+major+"|"+interestData+"|"+USER_NORMAL+"\n";
    if(!FileStore::write(userFile,existingText)){
        cout<<"无法写入用户数据文件，注册失败！"<<endl;
        return false;
    }
    cout<<"注册成功！"<<endl;
    return true;
}
/*
函数名：getUserNameById
功能：按用户Id查用户名，用于成员列表、删除确认等需要显示姓名的地方
参数：userId 用户Id
返回值：用户名（fields[1]）；找不到或文件打不开时返回空字符串
*/
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
        if(fields.size()>=2&&fields[0]==userId){
            file.close();
            return fields[1];
        }
    }
    file.close();
    //遍历完都没找到，说明这个ID不存在
    return "";
}
/*
函数名：changeRole
功能：修改指定用户的角色并写回文件
参数：userId 用户Id；newRole 新角色（Student / ClubManager / Admin）
返回值：成功返回 true；文件打不开或找不到该用户返回 false
实现步骤：
    1. 逐行读入 users.txt；
    2. 找到该用户那一行，把第 4 个字段（fields[3]，角色）改成 newRole，
       再用 | 把所有字段重新拼回一行，其他字段（包括状态）保持不变；
    3. 把所有行整体覆盖写回文件。
说明：供管理员审核社团（申请人变为负责人）、删除社团（负责人改回学生）、修改角色时使用
*/
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
            //第4个字段是角色，改完重新用|拼回去
            fields[3]=newRole;
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
    return writeLines(lines);
}

/*
函数名：showUserInfoById
功能：按用户Id显示用户的完整信息（不显示密码），负责人查看成员和申请人信息时使用
参数：userId 用户Id
返回值：找到并显示返回 true；找不到返回 false
实现说明：文件中兴趣标签用英文逗号分隔，显示时把逗号换成顿号，
          与学生"个人信息"中的显示方式保持一致
*/
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
            //不显示密码 fields[2]
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
/*
函数名：readLines
功能：读出 users.txt 的所有非空行
参数：lines 用来存放读出的行（引用参数，函数内直接往里添加）
返回值：成功返回 true；文件打不开返回 false
说明：管理员的几个功能都要先读全部数据，所以把读文件提取成公共函数
*/
bool UserManager::readLines(vector<string>& lines){
    string text;
    bool exists=false;
    if(!FileStore::read(userFile,text,exists)||!exists){
        cout<<"无法打开用户数据文件！"<<endl;
        return false;
    }
    istringstream file(text);
    string line;
    while(getline(file,line)){
        if(!line.empty()&&line.back()=='\r') line.pop_back();
        if(!line.empty()) lines.push_back(line);
    }
    return true;
}
/*
函数名：writeLines
功能：用 lines 中的内容整体覆盖写回 users.txt
参数：lines 所有用户行
返回值：成功返回 true；文件打不开返回 false
说明：ofstream 默认以覆盖方式打开文件，原内容会被清空后重新写入
*/
bool UserManager::writeLines(const vector<string>& lines){
    ostringstream text;
    for(const string& line:lines) text<<line<<'\n';
    if(!FileStore::write(userFile,text.str())){
        cout<<"无法写入用户数据文件！"<<endl;
        return false;
    }
    return true;
}
/*
函数名：statusOf
功能：取一行用户数据的账号状态
参数：fields 一行数据拆分后的字段
返回值：第 7 个字段是 frozen 时返回 frozen，否则（包括旧数据没有第 7 个字段）返回 normal
*/
string UserManager::statusOf(const vector<string>& fields){
    if(fields.size()>=7&&fields[6]==USER_FROZEN){
        return USER_FROZEN;
    }
    return USER_NORMAL;
}
/*
函数名：roleName
功能：把文件中保存的英文角色转换成中文，用于显示
参数：role 英文角色
返回值：中文角色名；不认识的角色原样返回
*/
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
/*
函数名：showAllUsers
功能：显示所有用户，每个用户一行：用户Id、用户名、角色，被冻结的账号在最后显示"已冻结"
实现步骤：
    1. 用 readLines() 读出全部用户行；
    2. 每行按 | 拆分，字段不足 6 个的跳过；
    3. 用 padText() 按显示宽度补空格，让各列对齐；
    4. 最后空一行显示用户总数。
*/
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
        //正常账号不显示状态，只有冻结的账号才标出来
        if(statusOf(fields)==USER_FROZEN){
            cout<<"已冻结";
        }
        cout<<endl;
        count++;
    }
    cout<<endl;
    cout<<"共"<<count<<"个用户"<<endl;
}
/*
函数名：showUserStatistics
功能：统计并显示用户总数、各角色人数和已冻结账号数
实现说明：遍历所有用户行，按第 4 个字段（角色）分别计数，再用 statusOf() 统计冻结账号；
          冻结账号也算在对应角色的人数中
*/
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
/*
函数名：getUserRole
功能：按用户Id查角色
参数：userId 用户Id
返回值：角色字符串（fields[3]）；找不到返回空字符串
说明：管理员模块也用它来判断用户是否存在（返回空字符串说明不存在）
*/
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
/*
函数名：getUserStatus
功能：按用户Id查账号状态
参数：userId 用户Id
返回值：normal 或 frozen；找不到返回空字符串
*/
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
/*
函数名：setUserStatus
功能：修改账号状态（冻结或解冻）并写回文件
参数：userId 用户Id；status 新状态（USER_NORMAL 或 USER_FROZEN）
返回值：成功返回 true；找不到该用户或写文件失败返回 false
实现步骤：
    1. 读出全部用户行；
    2. 找到该用户那一行，旧数据只有 6 个字段时先补上 normal，再把第 7 个字段改成 status；
    3. 用 | 重新拼回一行。循环变量用引用 string&，修改的就是 lines 中的那一行；
    4. 用 writeLines() 整体写回文件。
*/
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
/*
函数名：deleteUser
功能：从 users.txt 中删除指定用户
参数：userId 要删除的用户Id
返回值：成功返回 true；找不到该用户或写文件失败返回 false
实现说明：把除了该用户以外的所有行存进 kept，再用 kept 覆盖写回文件，相当于删掉那一行。
          该用户在社团、活动中的数据由调用者（Admin::deleteUser）处理。
*/
bool UserManager::deleteUser(const string& userId){
    vector<string> lines,retired;
    if(!readLines(lines)||!readRetiredIds(retired)) return false;
    vector<string> kept;
    bool found=false;
    for(const string& line:lines){
        vector<string> fields=split(line,'|');
        if(!fields.empty()&&fields[0]==userId) found=true;
        else kept.push_back(line);
    }
    if(!found) return false;
    if(find(retired.begin(),retired.end(),userId)==retired.end()) retired.push_back(userId);
    ostringstream userText, retiredText;
    for(const string& line:kept) userText<<line<<'\n';
    for(const string& id:retired) retiredText<<id<<'\n';
    FileStore::Writes writes;
    writes.push_back(make_pair(retiredIdFile,retiredText.str()));
    writes.push_back(make_pair(userFile,userText.str()));
    return FileStore::write(writes);
}

bool UserManager::readRetiredIds(vector<string>& ids){
    string text;
    bool exists=false;
    if(!FileStore::read(retiredIdFile,text,exists)){
        cout<<"无法读取已注销Id名单，操作已取消！"<<endl;
        return false;
    }
    // 兼容旧项目：名单尚不存在时从空名单开始；删除用户时与 users.txt 一起提交。
    istringstream input(text);
    string id;
    while(getline(input,id)){
        if(!id.empty()&&id.back()=='\r') id.pop_back();
        if(id.empty()) continue;
        if(containsSpace(id)||id.find('|')!=string::npos){
            cout<<"已注销Id名单格式错误，操作已取消！"<<endl;
            return false;
        }
        ids.push_back(id);
    }
    return true;
}

bool UserManager::reserveHistoricalIds(const ClubService& clubs,const ActivityManager& activities){
    vector<string> retired;
    if(!readRetiredIds(retired)) return false;
    string text;
    bool exists=false;
    if(!FileStore::read(userFile,text,exists)) return false;
    istringstream input(text);
    string line;
    set<string> active,known;
    while(getline(input,line)){
        auto fields=split(line,'|');
        if(fields.size()>=6) active.insert(fields[0]);
    }
    for(const Activity* a:activities.getAllActivities()){
        for(const string& uid:a->getParticipants()) known.insert(uid);
        for(const string& uid:a->getWaitList()) known.insert(uid);
    }
    for(const ClubBrief& c:clubs.getApprovedClubBriefs()){
        known.insert(c.leaderId);
        for(const string& uid:clubs.getClubMemberIds(c.clubId)) known.insert(uid);
        for(const string& uid:clubs.getClubApplicantIds(c.clubId)) known.insert(uid);
    }
    bool changed=false;
    for(const string& uid:known){
        if(!uid.empty()&&!active.count(uid)&&find(retired.begin(),retired.end(),uid)==retired.end()){
            if(containsSpace(uid)||uid.find('|')!=string::npos) return false;
            retired.push_back(uid);
            changed=true;
        }
    }
    if(!changed) return true;
    ostringstream saved;
    for(const string& uid:retired) saved<<uid<<'\n';
    return FileStore::write(retiredIdFile,saved.str());
}
