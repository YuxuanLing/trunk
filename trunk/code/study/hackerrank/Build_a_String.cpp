#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);

bool isSubStr( string str1 , string str2)
{
	if(str1.size()>=str2.size())
	{
		string::size_type len = str2.size();
		for(string::size_type i=0 ; i<str1.size()-len+1 ; i++)
		{
			string tmpstr = str1.substr(i,len);
			if(tmpstr==str2)
				return true;
		}
		return false;
	}
	return false;
}

bool isSubStr(string str1, string str2, string::size_type &idx)
{
	idx = str1.find(str2);
	
	if(idx == string::npos)return false;
	else return true;
	
}

bool useCopySubStr(string s, int idx, int a, int b, int &substrlen)
{
	bool ret = false;
	int strlen = s.length();
	int min_substr = (b + a - 1)/a;
	int max_substr = min(strlen - idx, idx);
	string pre_str = s.substr(0, idx);
	
	for(int sub_len = max_substr; sub_len >= min_substr; sub_len--)
	{
		string sub_str = s.substr(idx, sub_len);
		if(isSubStr(pre_str, sub_str))
		{
			ret = true;
			substrlen = sub_len;
			return ret;
		}
		
	}
		
	return ret;
}

/*
 * Complete the buildString function below.
 */
int buildString(int a, int b, string s) {
    /*
     * Write your code here.
     */
	 int ret = 0, str_len = s.length(), sub_len = 0;
	 
	 for(int idx = 0; idx < str_len;)
	 {
		 if(useCopySubStr(s, idx, a, b ,sub_len))
		 {
			 ret += b;
			 idx += sub_len;
			 
			 cout << "use copy : " << s.substr(0, idx) << endl;
			 cout << "score : " << ret << "  sub length : " << sub_len <<endl;
		 }else
		 {
			 ret += a;
			 idx++;
			 cout << "use add  : " << s.substr(0, idx) << endl;
			 cout << "score : " << ret << "  sub length : " << 1 <<endl;
		}		 
	 }

	 return ret;
}

int main()
{
    ofstream fout(getenv("OUTPUT_PATH"));

    int t;
    cin >> t;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int t_itr = 0; t_itr < t; t_itr++) {
        string nab_temp;
        getline(cin, nab_temp);

        vector<string> nab = split_string(nab_temp);

        int n = stoi(nab[0]);

        int a = stoi(nab[1]);

        int b = stoi(nab[2]);

        string s;
        getline(cin, s);

        int result = buildString(a, b, s);
		
		cout << "testcase " << t_itr <<" : " << result << endl;
    }

    fout.close();

    return 0;
}

vector<string> split_string(string input_string) {
    string::iterator new_end = unique(input_string.begin(), input_string.end(), [] (const char &x, const char &y) {
        return x == y and x == ' ';
    });

    input_string.erase(new_end, input_string.end());

    while (input_string[input_string.length() - 1] == ' ') {
        input_string.pop_back();
    }

    vector<string> splits;
    char delimiter = ' ';

    size_t i = 0;
    size_t pos = input_string.find(delimiter);

    while (pos != string::npos) {
        splits.push_back(input_string.substr(i, pos - i));

        i = pos + 1;
        pos = input_string.find(delimiter, i);
    }

    splits.push_back(input_string.substr(i, min(pos, input_string.length()) - i + 1));

    return splits;
}
