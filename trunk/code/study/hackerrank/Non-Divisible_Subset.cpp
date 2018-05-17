#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);
bool isIncluded(int i, vector<int> v)
{
    for(auto val: v)
    {
	        if(i == val)return true;
	    }
    
    return false;
}


// Complete the nonDivisibleSubset function below.
int nonDivisibleSubset(int k, vector<int> S) {
    int i,j, n = S.size();
    vector<int> result, exclueded;
    for(i = 0; i < n - 1; i++)
    for(j = i + 1; j < n; j++)
	{
	  if((S[i] + S[j])%k != 0)
	  {
	     if(!isIncluded(S[i],result)  &&!isIncluded(S[i],
	  						   exclueded))
	  \
	                     result.push_back(S[i]);
	                 if(!isIncluded(S[j],
	  						   result)
	  					   &&
	  					   !isIncluded(S[j],
	  						   exclueded))
	  \
	                     result.push_back(S[j]);
	              }else
	  {
	                  if(!isIncluded(S[i],
	  							result))exclueded.push_back(S[i]);
	                  if(!isIncluded(S[j],
	  							result))exclueded.push_back(S[j]);
	              }
	        }
	
	
	cout <<"before erase duplicate" << endl;
    for(auto val: result)
	{
		cout << val << " ";
	}
    cout << endl;
	sort(result.begin(),result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    cout <<"after erase duplicate" << endl;
    for(auto val: result)
	{
		cout << val << " ";
	}
    
    return result.size();
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
    getline(cin, S_temp_temp);

    vector<string> S_temp = split_string(S_temp_temp);

    vector<int> S(n);

    for (int i = 0; i < n; i++) {
	        int S_item = stoi(S_temp[i]);
	
	        S[i] = S_item;
	    }

    int result = nonDivisibleSubset(k, S);

    fout << result << "\n";

    fout.close();

    return 0;
}

vector<string> split_string(string input_string) {
    string::iterator new_end = unique(input_string.begin(), input_string.end(),
			[] (const char &x, const char &y) {
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

    splits.push_back(input_string.substr(i, min(pos, input_string.length()) - i
				+ 1));

    return splits;
}

