#include <iostream>
#include <memory>
#include <assert.h>
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
    shared_ptr<int> sp(new int(10));  
   assert(sp.use_count() == 1);  
   //create a weak_ptr from shared_ptr  
   weak_ptr<int> wp(sp);  
   //not increase the use count  
   assert(sp.use_count() == 1);  
   //judge wp is invalid  
   //expired() is equivalent with use_count() == 0  
   if(!wp.expired()){  
      shared_ptr<int> sp2 = wp.lock();//get a shared_ptr  
      *sp2 = 100;  
      assert(wp.use_count() == 2);  
      cout << *sp2 << endl;  
   }//out of scope,sp2 destruct automatically,use_count()--;  
   assert(wp.use_count() == 1);  
   sp.reset();//shared_ptr is invalid  
   assert(wp.expired());  
   assert(!wp.lock());  
    return 0;
}
