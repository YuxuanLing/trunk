//
# include <iostream>
# include <vector>

using namespace std;
class Animal
{
public:
    virtual  void eat() const { cout << "I eat like a generic Animal." << endl; }
    virtual  ~Animal() {} 
};
 
class Wolf : public Animal
{
public:
    void eat() const { cout << "I eat like a wolf!" << endl; }
};
 
class Fish : public Animal
{
public:
    void eat() const { cout << "I eat like a fish!" << endl; }
};
 
class GoldFish : public Fish
{
public:
    void eat() const { cout << "I eat like a goldfish!" << endl; }
};
 
 
class OtherAnimal : public Animal
{
};

class Base  //abstract class
{
public:
   virtual ~Base()
   {
	   std::cout <<"Base Create\n"; 
   };//virtual, not pure
   virtual void Hiberarchy() const = 0;//pure virtual
   virtual Base *clone(){}
};

void Base::Hiberarchy() const //pure virtual also can have function body
{
   std::cout <<"Base::Hiberarchy\n";
}

class Derived : public Base
{
public:
   Derived()
   {
     std::cout <<"Derived Create\n";   
   }
   virtual void Hiberarchy() const
   {
       Base::Hiberarchy();
       std::cout <<"Derived::Hiberarchy\n";
   }
   virtual void foo(){}
   virtual Derived *clone()
   {
	   std::cout<<"Derived::clone \n";	   
   }
};


void too()
{
   Base* pb=new Derived();
   pb->Hiberarchy();
   pb->Base::Hiberarchy();
   pb->clone();
} 
 
int main()
{
    std::vector<Animal*> animals;
    animals.push_back( new Animal() );
    animals.push_back( new Wolf() );
    animals.push_back( new Fish() );
    animals.push_back( new GoldFish() );
    animals.push_back( new OtherAnimal() );
 
    for( std::vector<Animal*>::const_iterator it = animals.begin();
       it != animals.end(); ++it) 
    {
        (*it)->eat();
        delete *it;
    }
    too();
   return 0;
}
