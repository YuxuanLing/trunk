#pragma once

#ifndef CUDAC
#ifdef __CUDACC__
#define CUDAC __host__ __device__
#else
#define CUDAC
#endif
#endif

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

template <typename T>
class Complex{
  public:
    T real;
    T image;
};
//add
template <typename T>
CUDAC void operator+=(Complex<T>& a, const Complex<T>& b){
  a.real += b.real;
  a.image += b.image;
}

template<typename T>
CUDAC Complex<T> operator+(const Complex<T>& a, const Complex<T>& b){
  Complex<T> temp = a;
  temp += b;
  return temp;
}
//sub
template <typename T>
CUDAC void operator-=(Complex<T>& a, const Complex<T>& b){
  a.real -= b.real;
  a.image -= b.image;
}

template<typename T>
CUDAC Complex<T> operator-(Complex<T> a, const Complex<T>& b){
  a -= b;
  return a;
}
//multiply
template <typename T>
CUDAC void operator*=(Complex<T>& a, const Complex<T>& b){
  Complex<T> temp;
  temp.real = a.real*b.real - a.image*b.image;
  temp.image = a.real*b.image + a.image*b.real;

  a = temp;
}

template<typename T>
CUDAC Complex<T> operator*(Complex<T> a, const Complex<T>& b){
  a *= b;
  return a;
}
//¹²éî
template <typename T>
CUDAC Complex<T> conjugate(Complex<T>& a){
  a.image = -a.image;
  return a;
}
//divide
template <typename T>
CUDAC void operator/=(Complex<T>& a, Complex<T> b){
  T temp = b.real*b.real + b.image*b.image;
  temp = 1/temp;

  conjugate(b);
//cout<<b<<endl;
  b *= a;

  a.real = b.real*temp;
  a.image = b.image*temp;
}

template<typename T>
CUDAC Complex<T> operator/(Complex<T>& a, const Complex<T>& b){
  a /= b;
  return a;
}

//
template<typename T>
CUDAC T length(Complex<T>& c){
	return (T)sqrt(c.real*c.real + c.image*c.image);
}
//typedef struct Complex Complex;
template <typename T>
ostream& operator<<(ostream& os, Complex<T>& c){
  os<<c.real<<":"<<c.image;
  return os;
}

template <typename T>
istream& operator>>(istream& is, Complex<T>& c){
  is>>c.real>>c.image;
//check error
  if(!is){
    cerr<<"input error in line"<<__LINE__<<endl;
  }
  return is;
}
/*
int main(int argc, char *argv[]){
  Complex<double> dc;
  dc.real = 0.04;
  dc.image = 0.6;
  cin>>dc;
  cout<<dc<<endl;

  Complex<double> rc;
  rc.real = 0.5;
  rc.image = 0.8;

  dc += rc;
  dc = dc+rc;
  cout<<dc<<endl;

  dc -= rc;
  dc =dc-rc;
  cout<<dc<<endl;

  dc *= rc;
  dc = dc*rc;
  cout<<dc<<endl;

  dc /= rc;
//  dc = dc/rc;
  cout<<dc<<endl;

  return 0;
}
*/
