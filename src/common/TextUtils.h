#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>

//计算字符串在控制台上占几格：英文、数字占1格，中文和全角符号占2格
inline int displayWidth(const std::string& s){
    int width=0;
    size_t i=0;
    while(i<s.size()){
        unsigned char c=s[i];
        if(c<0x80){
            width+=1;
            i+=1;
        }
        else if(c<0xE0){
            //2字节字符（拉丁字母等）占1格
            width+=1;
            i+=2;
        }
        else if(c<0xF0){
            //3字节字符（中文、全角符号）占2格
            width+=2;
            i+=3;
        }
        else{
            width+=2;
            i+=4;
        }
    }
    return width;
}

//在右边补空格，补到指定的显示宽度，用来让列表各列对齐
//内容本身超过宽度时，补两个空格隔开下一列
inline std::string padText(const std::string& s,int width){
    int w=displayWidth(s);
    if(w>=width){
        return s+"  ";
    }
    return s+std::string(width-w,' ');
}

#endif
