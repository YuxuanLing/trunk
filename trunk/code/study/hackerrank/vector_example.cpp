#include<vector>  
#include<iostream>  
  
using namespace std;  
  
int main(int argc,char* argv[])  
{  
    vector<int> v(10);  
    for(int i=0;i<10;i++)  
    {  
	        v[i]=i;  
	    }  
   
    v.erase (v.begin ()+2);//删除第2个元素(迭代器中)，从0计数。所以剩下013456789  
  
    vector<int>::iterator it;  
  
    for(it=v.begin ();it!=v.end ();it++)  
    {  
	   cout<<*it<<" ";  
	}  
  
    cout<<endl;  
      
    //v.erase (v.begin ()+ 1,v.begin()+5);//删除迭代器中第1到第5区间内所有元素，所以只剩06789了。  
    v.erase (v.begin ()+ 4,v.end());
	v.pop_back();
    for(it=v.begin ();it!=v.end ();it++)  
    {  
	        cout<<*it<<" ";  
	 }  
    cout<<endl;  
  
    v.clear ();//全部删除了。  
  
 
    cout<<v.size ()<<endl;  
    return 0;  
  
  
} 
