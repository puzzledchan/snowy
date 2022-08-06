#include <iostream>

class BBelong {
public:
    BBelong(){
        std::cout << "BBelong constructor" <<std::endl;
    }
    ~BBelong() {
        std::cout << "BBelong destructor" <<std::endl;  
    }
};

class Base {
public:
    Base(){
        std::cout << "Base constructor" <<std::endl;
    }
    ~Base() {
        std::cout << "Base destructor" <<std::endl;  
    }
private:
BBelong be;
};

class CBelong {
public:
    CBelong(){
        std::cout << "CBelong constructor" <<std::endl;
    }
    ~CBelong() {
        std::cout << "CBelong destructor" <<std::endl;  
    }
};

class CBase: public Base {
public:
    CBase(){
        std::cout << "CBase constructor" <<std::endl;
    }
    ~CBase() {
        std::cout << "CBase destructor" <<std::endl;  
    }
private:
    CBelong ce;
};

class DBelong {
public:
    DBelong(){
        std::cout << "DBelong constructor" <<std::endl;
    }
    ~DBelong() {
        std::cout << "DBelong destructor" <<std::endl;  
    }
};

class DBase: public Base {
public:
    DBase(){
        std::cout << "DBase constructor" <<std::endl;
    }
    ~DBase() {
        std::cout << "DBase destructor" <<std::endl;  
    }
private:
    DBelong de;
};

int main() {
    DBase d;
    return 0;
}
/**
 * @brief 
 * 构造顺序：类构造按照继承顺序， 类内构造按照 成员变量 -> 构造函数
 * 析构顺序：类析构反继承顺序，类内析构 析构函数->成员变量析构函数
 */