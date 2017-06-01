#include <iostream>
#include <memory>
#include<boost/shared_ptr.hpp>
#include<boost/weak_ptr.hpp>

using namespace std;

class B;
class A
{
public:
    shared_ptr<B> pb_;
    //weak_ptr<B> pb_;
    ~A()
    {
        cout<<"A delete\n";
    }
};
class B
{
public:
    shared_ptr<A> pa_;
    ~B()
    {
        cout<<"B delete\n";
    }
};
 
void fun()
{
    shared_ptr<B> pb(new B());
    shared_ptr<A> pa(new A());
    pb->pa_ = pa;
//    pa->pb_ = pb;
    cout<<"pb count = "<<pb.use_count()<<endl;
    cout<<"pb->pa_ count = "<<pb->pa_.use_count()<<endl;
    cout<<"pa count = "<<pa.use_count()<<endl;
    cout<<"pa->pb_ count = "<<pa->pb_.use_count()<<endl;
}
 
int main()
{
    fun();
    return 0;
}
