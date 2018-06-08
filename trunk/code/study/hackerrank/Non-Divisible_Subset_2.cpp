
#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);

string get_file_string(string path_to_file){
    std::ifstream ifs(path_to_file);
    return string((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));
}


// Complete the nonDivisibleSubset function below.
int nonDivisibleSubset(int k, vector<int> S) 
{
	int *arr = new int[k]();
    int result = 0;
	for(auto val: S)
	{
		arr[val%k]++;
	}

    if(arr[0] >= 1)result += 1;

	for(int i = 1; i <= k/2; i++)
	{
	   int tmp = arr[i];
	   if((k%2==0)&& (i==k/2)&&arr[i]!=0)
	   {
	   	  result += 1;
		  break;
	   }
	   if(arr[k - i] > tmp)tmp = arr[k - i];
	   result += tmp;
	}

	return result;
}

int main()
{
    ofstream fout(getenv("OUTPUT_PATH"));

    string nk_temp;
    getline(cin, nk_temp);

    vector<string> nk = split_string(nk_temp);

    int n = stoi(nk[0]);

    int k = stoi(nk[1]);

    string S_temp_temp;
	S_temp_temp = get_file_string("testcase.txt");
    //getline(cin, S_temp_temp);

    vector<string> S_temp = split_string(S_temp_temp);

    vector<int> S(n);

    for (int i = 0; i < n; i++) {
        int S_item = stoi(S_temp[i]);

        S[i] = S_item;
		std::cout<<S_item<<endl;
    }

    int result = nonDivisibleSubset(k, S);

    fout << result << "\n";

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
