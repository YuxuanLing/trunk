 #include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string);
vector<string> string2vector_char(string input_string);

string get_file_string(string path_to_file){
    std::ifstream ifs(path_to_file);
    return string((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));
}

bool canPutPlusAtPos(char *grid, int x_stride, int y_stride, int cross_len, int pos_x, int pos_y)
{
	int k = (cross_len - 1)/2;
		
	for(int x = pos_x - k; x <= pos_x + k; x++)
	{
		if(x < 0 || x >= x_stride || grid[pos_y*x_stride + x] == 'B')
		{
			return false;
		}		
	}
	
	for(int y = pos_y - k; y <= pos_y + k; y++)
	{
		if(y < 0 || y >= y_stride || grid[y*x_stride + pos_x] == 'B')
		{
			return false;
		}		
	}
	
	return true;
}

bool canPutPlus(char *grid, int x_stride, int y_stride, int cross_len, vector<pair<int, int>> pos)
{
	bool result = false;
	int k = (cross_len - 1)/2; 
	for(int y = k; y < y_stride - k; y++)
		for(int x = k; x < x_stride -k; x++)
		{
			bool posIsOK = canPutPlusAtPos(grid, x_stride, y_stride, cross_len, x, y);
			if(posIsOK)
			{
				pos.push_back(make_pair(x,y));
				result |= posIsOK;
			}			
		}
	
	return result;
}


// Complete the twoPluses function below.
int twoPluses(vector<string> grid) {
	int ret = 0;

    return ret;
}

int main()
{
    ofstream fout(getenv("OUTPUT_PATH"));

    string nm_temp;
    getline(cin, nm_temp);

    vector<string> nm = split_string(nm_temp);

    int n = stoi(nm[0]);

    int m = stoi(nm[1]);

	string grid_string;
	grid_string = get_file_string("./testcase_ema.txt");
    vector<string> grid = string2vector_char(grid_string);

/*	
    vector<string> grid(n);

    for (int i = 0; i < n; i++) {
        string grid_item;
        getline(cin, grid_item);

        grid[i] = grid_item;
    }
*/
    for(vector<string>::iterator it = grid.begin(); it != grid.end();it++)
	{
		cout << *it << endl;
		
	}
	
	int result = twoPluses(grid);

    cout << result << "\n";

    fout.close();

    return 0;
}

vector<string> string2vector_char(string input_string) {
    vector<string> splits;
	
	size_t pos = 0;
    cout << input_string << endl;
    while (pos < input_string.length()) {
		string ch = input_string.substr(pos, pos+1);
        if(ch != " " && ch != "\n" && ch !="")splits.push_back(ch);

        pos++;
    }
    
	return splits;
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
