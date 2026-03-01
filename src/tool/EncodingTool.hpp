#ifndef ENCODINGTOOL_HPP
#define ENCODINGTOOL_HPP
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <string>

inline sf::String GBKToSFString(const std::string& gbkStr) {
    int wLen = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    wchar_t* wStr = new wchar_t[wLen];
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, wStr, wLen);

    sf::String sfStr(wStr);
    
    delete[] wStr;
    return sfStr;
}

inline sf::String UTF8ToSFString(const std::string& utf8Str) {
    int wLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    wchar_t* wStr = new wchar_t[wLen];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wStr, wLen);

    sf::String sfStr(wStr);
    delete[] wStr;
    return sfStr;
}

inline void SetConsoleGBK() {
    SetConsoleOutputCP(936);  
    SetConsoleCP(936);
}

#endif // ENCODINGTOOL_HPP