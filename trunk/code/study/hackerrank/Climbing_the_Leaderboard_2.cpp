#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);

string get_file_string(string path_to_file){
    std::ifstream ifs(path_to_file);
    return string((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));
}


int findRank(vector<int> s)
{
	int *rank = new int[s.size()](), i, j,size =s.size();
	int current_rank = 1, rt;
	if(s.size()<=1)return 1;
	for(i = 0; i < size - 1;)
	{
		rank[i] = current_rank;
		for(j = i+1; j < size;)
		{
			if(s[j] != s[i])
			{
				rank[j] = current_rank;
				current_rank++;
				i = j;
				break;				
			}else
			{
				rank[j] = current_rank;
                j++;				
			}
		}		
		i = j;		
	}
	rt = rank[size - 1];
	return  rt;
}


// Complete the climbingLeaderboard function below.
vector<int> climbingLeaderboard(vector<int> scores, vector<int> alice) {
	vector<int> result;
	
    int *rank = new int[scores.size()](), i, j,size =scores.size();
	int current_rank = 1;
	if(scores.size()<=1)rank[0] = current_rank;
	for(i = 0; i < size - 1;)
	{
		rank[i] = current_rank;
		for(j = i+1; j < size;)
		{
			if(scores[j] != scores[i])
			{
				current_rank++;
				rank[j] = current_rank;
				i = j;
				break;				
			}else
			{
				rank[j] = current_rank;
                j++;				
			}
		}		
		i = j;		
	}
	
	for(i = 0; i < size; i++)
	{
		cout<<"rank["<<i<<"]:  "<< rank[i]<<endl;
	}

    int start_search_idx = scores.size() - 1;
	for(int i = 0 ; i < alice.size(); i++)
	{
		int alice_score = alice[i], alice_current_rank = 0;

		for(j = start_search_idx; j >= 0;)
		{			
			if(alice_score < scores[j])
			{
                start_search_idx = j;
				alice_current_rank = rank[j]+1;
				result.push_back(alice_current_rank);
				break;				
			}else if(alice_score == scores[j])
			{
                start_search_idx = j;
				alice_current_rank = rank[j];
				result.push_back(alice_current_rank);
				break;									
			}else if(alice_score > scores[j])
			{				
				j--;
			}				
		}
		
		if(j < 0)result.push_back(1);
        //std::cout<<"rank = " << rank << endl;		
	}
	
	
	return result;

}

int main()
{
    ofstream fout(getenv("OUTPUT_PATH"));

    int scores_count;
	cout<<"input scores count" << endl;
    cin >> scores_count;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string scores_temp_temp;
	//scores_temp_temp = get_file_string("./testcase_climbing.txt");
    getline(cin, scores_temp_temp);
    //cout<<scores_temp_temp<<endl;
    vector<string> scores_temp = split_string(scores_temp_temp);

    vector<int> scores(scores_count);
    cout<<scores_count<<endl;
    for (int i = 0; i < scores_count; i++) {
        int scores_item = stoi(scores_temp[i]);
        scores[i] = scores_item;
    }

    int alice_count;
	cout<<"input alice count" << endl;
    cin >> alice_count;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string alice_temp_temp;
    //alice_temp_temp = get_file_string("./testcase_climbing_allice.txt");
    getline(cin, alice_temp_temp);

    vector<string> alice_temp = split_string(alice_temp_temp);

    vector<int> alice(alice_count);

    for (int i = 0; i < alice_count; i++) {
        int alice_item = stoi(alice_temp[i]);
        alice[i] = alice_item;
    }

    vector<int> result = climbingLeaderboard(scores, alice);

    for (int i = 0; i < result.size(); i++) {
        fout << result[i];

        if (i != result.size() - 1) {
            fout << "\n";
        }
    }

    fout << "\n";

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
