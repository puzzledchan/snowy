#include <memory>
#include <iostream>

class B ;
class A {
public: 
std::weak_ptr<B> ptrB;
 A() {
    a = 1;
    std::cout <<"A constructor" <<std::endl;
 }
 void set(std::shared_ptr<B> b);
 ~A() {
    a = 1;
    std::cout <<"A destructor" <<std::endl;
 }
private:
int a;
};

class B: public std::enable_shared_from_this<B>{
public :

 B() {
    b = 2;
    std::cout <<"B constructor" <<std::endl;
 }
 void set() {
    std::cout <<"before set" <<std::endl;
    std::shared_ptr<B> ptrB = shared_from_this();
    std::cout <<"after set" <<std::endl;
    a.set(ptrB);
              std::cout <<ptrB.use_count() <<std::endl;  
 }
 int getB() {
    return b;
 }
 ~B() {
    b = 2;
    std::cout<<"beforessss" <<std::endl;
                  std::cout <<"before destructor" << shared_from_this().use_count() <<std::endl;  
    std::cout <<"B destructor" <<std::endl;
 }
  A a;
 int b;
};

void A::set(std::shared_ptr<B> b) {
          std::cout <<b.use_count() <<std::endl;   
        ptrB = b;
        std::cout <<ptrB.use_count() <<std::endl; 
}
int main () {

    std::shared_ptr<B> s(new B);
            std::cout <<s.use_count() <<std::endl; 
    s->set();
                   std::cout<<"before end" <<s.use_count() <<std::endl;    
    return 0;
}