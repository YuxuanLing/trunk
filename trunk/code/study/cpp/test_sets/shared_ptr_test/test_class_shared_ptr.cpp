#include<gtest/gtest.h>
#include <iostream>
#include <memory>
#include<boost/shared_ptr.hpp>

using namespace std;

class A
{
public:
	A()
	{
		cout<<"Class A Creat"<<endl;
	}
	
	A(string str)
	{
		str_a = str;
		cout<<"Class A Creat"<<endl;
	}
	
	string& getStr()
	{
		return str_a;			    
	}
	
	~A()
	{
		cout<<"Class A Destroy"<<endl;		
	}
public:
	string str_a;	
};


void print_classA_str(A a)
{
	cout<<"A String : "<<a.str_a<<endl;	
}

class Test
{
public:
    //friend 
	Test(string s)
	{
		str = s;
		ptr_a = make_shared<A>(s);
		cout<<"Test "<<str<<" creat\n";
					       
	}
	
    Test(string s, shared_ptr<A> a)
	{
		str = s;
		ptr_a = a;
		cout<<"Test "<<str<<" creat\n";
	}
	
	~Test()
	{
		//ptr_a = 0;     //wrong , do not destroy here
		cout<<"Test "<<str<<" delete\n";				    
	}
	
	shared_ptr<A>& getClassA()
	{
		
		return ptr_a;
	}
	
	string& getStr()
	{
		return str;			    
	}
	
	void setStr(string s)
	{
		str = s;
				    
	}
	
	void print()
	{
		cout<<str<<endl;
				    
	}
private:
	string str;
	shared_ptr<A> ptr_a;

};


void test_func0()
{
	cout<<"test_func0  start !\n";
    shared_ptr<Test> ptest(new Test("123"));
    shared_ptr<Test> ptest2(new Test("456"));
    cout<<ptest2->getStr()<<endl;
    cout<<ptest2.use_count()<<endl;
    ptest = ptest2;//"456"引用次数加1，“123”销毁
    ptest->print();
    cout<<ptest2.use_count()<<endl;//2
    cout<<ptest.use_count()<<endl;//2
	//ptest2.reset();
	ptest2 = 0;
	cout<<ptest2.use_count()<<endl;//2
    cout<<ptest.use_count()<<endl;//2
    //ptest = 0;
	//cout<<ptest2.use_count()<<endl;//2
    //cout<<ptest.use_count()<<endl;//2
	//ptest.reset();
    //ptest2.reset();//此时“456”销毁
    cout<<"test_func0  done !\n";	
}

	shared_ptr<A>  share_ptr_a = make_shared<A>("Internal Class A");


void test_func1()
{
	cout<<"test_func0  start !\n";
  //{
    shared_ptr<Test> p1(new Test("123", share_ptr_a));
	cout<<"P1 Class A String:"<<p1->getClassA()->getStr()<<endl;
    cout<<"p1 use_cnt = "<<p1.use_count()<<endl;
    cout<<"a  use_cnt = "<<share_ptr_a.use_count()<<endl;
    p1 = 0;	
  //}
    shared_ptr<Test> p2(new Test("456", share_ptr_a));  
	cout<<"P2 Class A String:"<<p2->getClassA()->getStr()<<endl;
    cout<<"p2 use_cnt = "<<p2.use_count()<<endl;
	cout<<"a  use_cnt = "<<share_ptr_a.use_count()<<endl;	

	//cout<<"P2 Class A String:"<<p2->getClassA()->getStr()<<endl;
    cout<<"test_func0  stop !\n";	
}


TEST(func_test_class_shared_ptr_suit, func1)
{
	test_func1();
}


/*
int main()
{
	
	//test_func0();
	test_func1();
    return 0;
}
*/
