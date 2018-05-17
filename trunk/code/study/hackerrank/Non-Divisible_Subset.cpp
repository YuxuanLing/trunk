
#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);

bool findMaxNonZeroIndex(int &idx, vector<int> v)
{
    int i, maxVal = 0;
    for(i = 0; i < v.size(); i++)
    {
        if(v[i] > maxVal)
        {
            maxVal = v[i];
            idx = i;
        }
    }
    
    if(maxVal == 0)return false;
    return true;
}
// Complete the nonDivisibleSubset function below.
int nonDivisibleSubset(int k, vector<int> S) {
    vector<int> s;
    for(auto val: S)
    {
        s.push_back((val%k) + k);
    }
    
    while(1)
    {
        int i, j, n = s.size(), idx;
        vector<int> rec(n,0);
        for(i = 0; i < n - 1; i++)
        {
            for(j = i+1 ; j < n ; j++)
            {
                if((s[i] + s[j]) % k == 0)
                {
                    rec[i]++;
                    rec[j]++;
                }                
            }
        }
        
        if(findMaxNonZeroIndex(idx, rec))
        //if(0)
        {
            vector<int>::iterator it = s.begin();
            advance(it,idx);
            //cout<<idx<<endl;
            s.erase(it);
        }else
        {
            //vector<int>::iterator it = S.begin();
            //advance(it,3);
            //S.erase(it);
            break;
        }
        
    }

    return s.size();
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
