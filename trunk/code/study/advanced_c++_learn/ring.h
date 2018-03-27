#include<iostream>
using namespace std;

template<class T>
class ring {
	private:
		T *m_values;
		int m_size;
		int m_pos;
	public:
      class iterator;
	public:
	  ring(int size):m_pos(0), m_size(size),m_values(NULL) {
	  	m_values = new T[size];
	  }
      ~ring(){
		  delete [] m_values;
	  }
	  int size(){
	  	return m_size;
	  }
	  void add(T value){
	       m_values[m_pos] = value;
		   m_pos++;
		   if(m_pos == m_size){
		   		m_pos = 0;		   
		   }
	  }

	  T &get(int pos){
	  	return m_values[pos];
	  }
      
	  iterator begin() {
	  	return iterator(0,*this;
	  }
      iterator end() {
	  	return iterator(m_size, *this);
	  }

};	

template<class T>
class ring<T>::iterator {
public:
   void print(){
        cout << "Hello from iterator: " <<T()<< endl;
   }
};
   
