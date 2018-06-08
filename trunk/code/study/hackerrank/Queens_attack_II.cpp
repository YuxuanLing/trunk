#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);

string get_file_string(string path_to_file){
    std::ifstream ifs(path_to_file);
    return string((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));
}

// Complete the queensAttack function below.
int queensAttack(int n, int k, int r_q, int c_q, vector<vector<int>> obstacles) 
{
	int result = 0;
	int dirRec[8][2]={0}, x, y;
	//init dirRec
	dirRec[0][0] = c_q;
	dirRec[0][1] = n + 1;
	dirRec[4][0] = c_q;
	dirRec[4][1] = 0;
	
	if(r_q > c_q) {
	   dirRec[1][0] = n + 1 - r_q + c_q;
	   dirRec[1][1] = n + 1;

	   dirRec[5][0] = 0;
	   dirRec[5][1] = r_q - c_q;	   
	}else {
	   dirRec[1][0] = n + 1;
	   dirRec[1][1] = n + 1 - c_q + r_q;		

	   dirRec[5][0] = c_q - r_q;
	   dirRec[5][1] = 0;	   
	}
		
	dirRec[2][0] = n + 1;
	dirRec[2][1] = r_q;
	dirRec[6][0] = 0;
	dirRec[6][1] = r_q;
	
	
	if(n + 1> c_q + r_q ) {
	   dirRec[3][0] = r_q + c_q;
	   dirRec[3][1] = 0;	
	   
	   dirRec[7][0] = 0;
	   dirRec[7][1] = r_q + c_q;
	}else {
	   dirRec[3][0] = n + 1;
	   dirRec[3][1] = r_q + c_q - n - 1;			
   		
 	   dirRec[7][0] = r_q + c_q - n - 1;
	   dirRec[7][1] = n + 1;			
   			
	}
	
	
	int q_y = r_q, q_x = c_q;
	//for(vector<vector<int>>::iterator it = obstacles.begin(); it != obstacles.end(); it++)
    
	for(int i = 0; i < k; i++)
	{
		vector<int> attack = obstacles[i];
		int x = attack[1], y = attack[0];
		cout << "q_x = "<<q_x<<", "<<"q_y = "<<q_y<<endl;
		cout << "x = "<<x<<", "<<"y = "<<y<<endl;
		if(x == q_x && y > q_y)
		{
			//dir 0
			if(y < dirRec[0][1]){
				dirRec[0][0] = x;
				dirRec[0][1] = y;
			}
            cout << "dirRec[0]"<<x<<", "<<y<<endl;				
			continue;
		}
		
		if(x == q_x && y < q_y)
		{
			//dir 4
			if(y > dirRec[4][1]){
				dirRec[4][0] = x;
				dirRec[4][1] = y;
			}
            cout << "dirRec[4]"<<x<<", "<<y<<endl;				
			continue;
		}
		
		if(x > q_x && y > q_y && (y - q_y) == (x - q_x))
		{
			//dir 1
			if((x < dirRec[1][0]) && (y < dirRec[1][1])){
				dirRec[1][0] = x;
				dirRec[1][1] = y;
			}
            cout << "dirRec[1]"<<x<<", "<<y<<endl;					
			continue;
		}
		
		if(x < q_x && y < q_y && (q_y - y) == (q_x - x))
		{
			//dir 5
			if((x > dirRec[5][0]) && (y > dirRec[5][1])){
				dirRec[5][0] = x;
				dirRec[5][1] = y;
			}
            cout << "dirRec[5]"<<x<<", "<<y<<endl;					
			continue;
		}
		
		if(y == q_y && x > q_x)
		{
			//dir 2
			if(x < dirRec[2][0]){
				dirRec[2][0] = x;
				dirRec[2][1] = y;
			}
            cout << "dirRec[2]"<<x<<", "<<y<<endl;					
			continue;
		}

		if(y == q_y && x < q_x)
		{
			//dir 6
			if(x > dirRec[6][0]){
				dirRec[6][0] = x;
				dirRec[6][1] = y;
			}
            cout << "dirRec[6]"<<x<<", "<<y<<endl;					
			continue;
		}

		if(x > q_x && y < q_y && (q_y - y) == (x - q_x))
		{
			//dir 3
			if((x < dirRec[3][0]) && (y > dirRec[3][1])){
				dirRec[3][0] = x;
				dirRec[3][1] = y;
			}
            cout << "dirRec[3]"<<x<<", "<<y<<endl;					
			continue;
		}
		
		if(x < q_x && y > q_y && (y - q_y) == (q_x - x))
		{
			//dir 7
			if((x > dirRec[7][0]) && (y < dirRec[7][1])){
				dirRec[7][0] = x;
				dirRec[7][1] = y;
			}
            cout << "dirRec[7]"<<x<<", "<<y<<endl;					
			continue;
		}
		
	}
	
	cout << " direct results:" << endl;
	for(int i = 0 ; i < 8 ;i++)
	{
		cout<<dirRec[i][0]<<"  "<<dirRec[i][1]<<endl;
	}
	
	result = 0;
	result += dirRec[0][1] - q_y - 1;
	result += q_y - dirRec[4][1] - 1;

	result += dirRec[1][0] - q_x - 1;
	assert(dirRec[1][0] - q_x == dirRec[1][1] - q_y);
	result += q_x - dirRec[5][0] - 1;
	assert(q_x - dirRec[5][0]  == q_y - dirRec[5][1]);

	
	result += dirRec[2][0] - q_x - 1;
	result += q_x - dirRec[6][0] - 1;
	
	result += dirRec[3][0] - q_x - 1;
	assert(dirRec[3][0] - q_x == -dirRec[3][1] + q_y);
	result +=  q_x - dirRec[7][0] - 1;
	assert(-dirRec[7][0] + q_x == dirRec[7][1] - q_y);
	
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

    string r_qC_q_temp;
    getline(cin, r_qC_q_temp);

    vector<string> r_qC_q = split_string(r_qC_q_temp);

    int r_q = stoi(r_qC_q[0]);

    int c_q = stoi(r_qC_q[1]);

	string prevent_temp_temp;
    //prevent_temp_temp = get_file_string("./testcase_climbing_queenattack.txt");
	//vector<string> obstacles_temp = split_string(prevent_temp_temp);

    vector<vector<int>> obstacles(k);
	   
    for (int i = 0; i < k; i++) {
        obstacles[i].resize(2);

        for (int j = 0; j < 2; j++) {
			//int tmp = stoi(obstacles_temp[i*2 + j]);
            //obstacles[i][j] = tmp;
            cin >> obstacles[i][j];
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int result = queensAttack(n, k, r_q, c_q, obstacles);
	cout << result << "\n";

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