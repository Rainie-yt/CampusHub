/*
文件名：TextUtils.h（文本对齐工具函数）
功能：计算字符串在控制台上的显示宽度，并按宽度补空格，用来让列表各列对齐。
说明：程序中的字符串是 UTF-8 编码，一个中文字符占 3 个字节，但在控制台上只占 2 格，
      所以不能直接用 string 的 size() 计算宽度，否则含中文的列会对不齐。
      函数写在头文件中，声明为 inline，避免多个 .cpp 包含时重复定义。
*/
#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>

/*
函数名：displayWidth
功能：计算字符串在控制台上占几格：英文、数字占 1 格，中文和全角符号占 2 格
参数：s 要计算的字符串
返回值：显示宽度（格数）
实现说明：
    UTF-8 中每个字符的第一个字节决定了这个字符占几个字节：
        小于 0x80        → 1 字节（英文、数字），占 1 格；
        0xC0 ～ 0xDF     → 2 字节（拉丁字母等），占 1 格；
        0xE0 ～ 0xEF     → 3 字节（中文、全角符号），占 2 格；
        0xF0 及以上      → 4 字节（表情符号等），占 2 格。
    用 i 指向当前字符的第一个字节，每次按字节数往后跳，同时累加宽度。
关键点：c 要用 unsigned char，否则大于 0x7F 的字节会被当成负数，比较结果出错。
*/
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
            //4字节字符（表情符号等）占2格
            width+=2;
            i+=4;
        }
    }
    return width;
}

/*
函数名：padText
功能：在字符串右边补空格，补到指定的显示宽度，用来让列表各列对齐
参数：s 要显示的内容；width 这一列的显示宽度
返回值：补好空格的字符串
说明：内容本身已经达到或超过宽度时，补两个空格，与下一列隔开
*/
inline std::string padText(const std::string& s,int width){
    int w=displayWidth(s);
    if(w>=width){
        return s+"  ";
    }
    //string(n,' ') 构造由 n 个空格组成的字符串
    return s+std::string(width-w,' ');
}

#endif
