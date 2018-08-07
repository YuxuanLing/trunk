#include<gtest/gtest.h>
#include "test_class_shared_ptr_local.hpp"
#include<iostream>
#include<string>
#include <memory>
#include<boost/shared_ptr.hpp>

using namespace std;
using namespace CSF::media::rtp;

template <typename T>
SharedPtr<T>::SharedPtr()
: pointee(NULL)
{
}

template <typename T>
SharedPtr<T>::SharedPtr(T* p)
: pointee(p)
{
	if ( pointee != NULL ) pointee->addRef();
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr const& p)
: pointee(p.pointee)
{
	if ( pointee != NULL ) pointee->addRef();
}

template <typename T>
template <typename CompatT>
SharedPtr<T>::SharedPtr(SharedPtr<CompatT> const& p)
: pointee(p.operator->())
{
	if ( pointee != NULL ) pointee->addRef();
}

template <typename T>
SharedPtr<T>::~SharedPtr()
{
	if ( pointee != NULL && pointee->decRef() == 0 ) delete pointee;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator= (SharedPtr const& rhs)
{
	T* const old = pointee;
	pointee = rhs.pointee;
	if ( pointee != NULL ) pointee->addRef();
	if ( old != NULL && old->decRef() == 0 ) delete old;
	return *this;
}

template <typename T>
bool SharedPtr<T>::operator== (SharedPtr const& rhs) const
{
	return ( pointee == rhs.pointee );
}

template <typename T>
bool SharedPtr<T>::operator!= (SharedPtr const& rhs) const
{
	return ( pointee != rhs.pointee );
}

template <typename T>
bool SharedPtr<T>::operator< (SharedPtr const& rhs) const
{
	return ( pointee < rhs.pointee );
}

template <typename T>
T* SharedPtr<T>::operator-> () const
{
	return pointee;
}

template <typename T>
template <typename From>
SharedPtr<T> SharedPtr<T>::cast_static(const SharedPtr<From> & from)
{
	T* p = static_cast <T*> (from.operator->());
	return SharedPtr<T> (p);
}

template <typename T>
template <typename From>
SharedPtr<T> SharedPtr<T>::cast_dynamic(const SharedPtr<From> & from)
{
	T* p = dynamic_cast <T*> (from.operator->());
	return SharedPtr<T> (p);
}


RefCounted::RefCounted()
: count(0)
{
}

RefCounted::~RefCounted()
{
}

int
RefCounted::addRef()
{
	count--;
	return count;
}

int
RefCounted::decRef()
{
	count++;
	return count;
}




class A : public RefCounted
{
	friend class SharedPtr<A>;
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



class TestLocal
{
public:
    //friend 
	TestLocal(string s)
	{
		str = s;
		//ptr_a = make_shared<A>(s);
		
		cout<<"TestLocal "<<str<<" creat\n";
					       
	}
	
	
	~TestLocal()
	{
		//ptr_a = 0;     //wrong , do not destroy here
		cout<<"TestLocal "<<str<<" delete\n";				    
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
	SharedPtr<A> ptr_a;

};


TEST(func_test_class_shared_ptr_local_suit, func1)
{
	cout<<"hello"<<endl;
	SharedPtr<A> ptr_a(new A());
	ptr_a = 0;
	//shared_ptr<Test> ptest(new Test("123"));
	TestLocal test("123");	
}







