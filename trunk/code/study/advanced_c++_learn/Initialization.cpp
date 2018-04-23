#include<iostream>
#include<vector>
using namespace std;

struct DeviceModel 
{
	const char *model_name;
	int   name_len;
};


static const DeviceModel mSpecialDeviceList[] =
{
  {"Mi A1",      5},
  {"MI MAX 2",   8},
  {"TC 51",      5},
  {"TC 56",      5}
};

int main() {
	int values[]={1,4,5};


	cout << values[0] << endl;
    
	class C {
		public:
			string text;
			int id;
	};

    C c1={"hello",7};

	cout << c1.text << endl;

	struct S {
		public:
			string text;
			int id;
	};

    S s1={"hello",7};

	cout << s1.text << endl;

	struct R {
		public:
			string text;
			int id;
	} r1 = {"hello", 7};


	cout << r1.text << endl;

    vector<string> strings;
    strings.push_back("One");
    strings.push_back("Two");
    strings.push_back("Three");
    strings.push_back("Four");

	cout << "length of mSpecialDeviceList : " <<sizeof(mSpecialDeviceList)/sizeof(DeviceModel) << endl;
	cout << "size   of mSpecialDeviceList : "<<sizeof(mSpecialDeviceList)<<endl;
	cout << "size   of DeviceModel: "<<sizeof(DeviceModel)<<endl;

	return 0;
	
}
