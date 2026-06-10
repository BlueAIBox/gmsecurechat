#ifndef SM3_H
#define SM3_H

#include <vector>
#include <string>

class SM3
{
public:
    static std::vector<unsigned char> hash(const std::vector<unsigned char>& data);
    static std::string hashString(const std::string& data);
};

#endif // SM3_H
