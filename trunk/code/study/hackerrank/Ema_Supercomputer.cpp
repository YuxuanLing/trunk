 #include <bits/stdc++.h>

using namespace std;

//#define DEBUG 1

vector<string> split_string(string);
vector<string> string2vector_char(string input_string);
vector<string> split_string(std::string str,std::string pattern);


string get_file_string(string path_to_file){
    std::ifstream ifs(path_to_file);
    return string((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));
}

#ifdef DEBUG_LOCALLY

void print_grid(char *grid, int x_stride, int y_stride)
{
	for(int y = 0 ; y < y_stride; y++)
	{
		for(int x = 0; x < x_stride; x++)
		{
			cout << grid[y*x_stride + x];
		}
		cout << endl;
	}	
}

#endif

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

bool canPutPlus(char *grid, int x_stride, int y_stride, int cross_len, vector<pair<int, int>> &pos)
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


bool canPutPlus(char *grid, int x_stride, int y_stride, int cross_len)
{
	bool result = false;
	int k = (cross_len - 1)/2; 
	for(int y = k; y < y_stride - k; y++)
		for(int x = k; x < x_stride -k; x++)
		{
			bool posIsOK = canPutPlusAtPos(grid, x_stride, y_stride, cross_len, x, y);
			if(posIsOK)
			{
				return true;
			}			
		}
	
	return result;
}

void update_grid(char *src, char *des, int x_stride, int y_stride, int cross_len, pair<int, int> plus_pos)
{
	int k = (cross_len - 1)/2 , i = 0;
	memcpy(des, src, x_stride*y_stride);
	int x = plus_pos.first, y = plus_pos.second;
	
	for(i = x - k; i <= x + k; i++)
	{
		des[y*x_stride + i] = 'B';
	}

	for(i = y - k; i <= y + k; i++)
	{
		des[i*x_stride + x] = 'B';
	}
	
}

bool findMaxTwoPluses(char *grid, int x_stride, int y_stride,int cross_len, vector<pair<int, int>> poses, int &maxPlus)
{
	char updated_grid[y_stride*x_stride];
	int k = (cross_len - 1)/2, mul_first, mul_second , two_pluses = 0;
	bool max_found = false;
	vector<pair<int, int>>::iterator it_pos = poses.begin();
	
	maxPlus = 0;
	mul_first = k*4 + 1;
	
	while(it_pos != poses.end())
	{
		update_grid(grid, updated_grid,  x_stride,  y_stride,  cross_len, (*it_pos));
		//print_grid(updated_grid, x_stride, y_stride);
		for(int c_len = cross_len; c_len >= 1; c_len -= 2)
		{
			if(canPutPlus(updated_grid, x_stride, y_stride,  c_len))
			{
				mul_second = (c_len - 1)*2 + 1;
				two_pluses = mul_first*mul_second;
				if(two_pluses >= maxPlus) maxPlus = two_pluses;
				max_found = true;
				break;				
			}
		}
		
		it_pos++;
	}
	
	return max_found;
	
}



bool findMaxTwoPluses(char *grid, int x_stride, int y_stride,int cross_len_idx, vector<int> cross_lens, vector<pair<int, int>> *plus_pos, int &maxPlus)
{
	char updated_grid[y_stride*x_stride];
	int cross_len = cross_lens[cross_len_idx], cross_idx_max = cross_lens.size();
	int k = (cross_len - 1)/2, mul_first, mul_second , two_pluses = 0;
	bool max_found = false;
	vector<pair<int, int>>::iterator it_pos = plus_pos[cross_len_idx].begin();
	
	maxPlus = 0;
	mul_first = k*4 + 1;
	
	while(it_pos != plus_pos[cross_len_idx].end())
	{
		update_grid(grid, updated_grid,  x_stride,  y_stride,  cross_len, (*it_pos));

		for(int idx = cross_len_idx; idx < cross_idx_max; idx++)
		{
			int c_len = cross_lens[idx];
			bool local_max_found = false;
			for(vector<pair<int,int>>::iterator it = plus_pos[idx].begin(); it != plus_pos[idx].end(); it++)
			{
				if(canPutPlusAtPos(updated_grid, x_stride, y_stride, c_len, (*it).first, (*it).second))
				{
					mul_second = (c_len - 1)*2 + 1;
					two_pluses = mul_first*mul_second;
					local_max_found = true;
					if(two_pluses >= maxPlus)
					{
					  maxPlus = two_pluses;
					  max_found = true;
					}
					break;				
				}				
			}
			if(local_max_found)break;			
		}
		
		it_pos++;
	}
	
	return max_found;
	
}



// Complete the twoPluses function below.
int twoPluses(vector<string> grid) {
	int ret = 0;
	int y_stride = grid.size(), x_stride = grid[0].length();
	char g[y_stride*x_stride] , updated_grid[y_stride*x_stride];
    int min_stride = min(x_stride, y_stride), pluskinds = 0;
	vector<pair<int, int>> plus_pos[min_stride];
	vector<int> cross_lens;
#ifdef DEBUG_LOCALLY	
	cout<<"y_stride = " << y_stride << "  x_stride = " << x_stride << endl;
#endif	
	for(int y = 0 ; y < y_stride; y++)
	{
		for(int x = 0; x < x_stride; x++)
		{
			g[y*x_stride + x] = grid[y][x];
		}
	}
#ifdef DEBUG_LOCALLY
	print_grid(g,  x_stride,  y_stride);
#endif	
	for(int k = (min_stride - 1)/2; k >= 0; k-- )
	{
	   int cross_len = 2*k + 1;
	   if(canPutPlus(g, x_stride, y_stride, cross_len, plus_pos[pluskinds]))
	   {	   
	     pluskinds++;
		 cross_lens.push_back(cross_len);
	   }
	}
 
#ifdef DEBUG_LOCALLY 
	cout << "pluskinds = " << pluskinds << endl;
	
	for(int i = 0; i < pluskinds; i++)
	{
		cout<<"cross length : " << cross_lens[i]<<endl;
		for(vector<pair<int, int>>::iterator it = plus_pos[i].begin(); it != plus_pos[i].end(); it++)
		{
			cout<<"("<< (*it).first <<" , " << (*it).second <<")";			
		}
		cout<<endl;		
	}
#endif	
	
    int maxPlus = 0;
	for(int i = 0; i < pluskinds; i++)
	{	   
	   //bool found = findMaxTwoPluses(g, x_stride, y_stride, cross_lens[i], plus_pos[i], maxPlus);
	   bool found = findMaxTwoPluses(g, x_stride, y_stride, i, cross_lens, plus_pos, maxPlus);
	   if(found)
	   {
		   if(maxPlus > ret)ret = maxPlus;
		   cout <<"corss length = " << cross_lens[i] << " maxPlus = " << maxPlus << endl;		   
	   }else
	   {
		   cout <<"can not find two pluses" << endl;		   		   
	   }	
	}
	
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
    vector<string> grid = split_string(grid_string, "\r\n");

/*	
    vector<string> grid(n);

    for (int i = 0; i < n; i++) {
        string grid_item;
        getline(cin, grid_item);

        grid[i] = grid_item;
    }
*/  
 
#ifdef DEBUG_LOCALLY 
	int i = 0;
    for(vector<string>::iterator it = grid.begin(); it != grid.end();it++)
	{
		cout <<i <<"  :  "<< *it << endl;
		i++;		
	}
#endif	
	int result = twoPluses(grid);

    cout << result << "\n";

    fout.close();

    return 0;
}

vector<string> string2vector_char(string input_string) {
    vector<string> splits;
	
	size_t pos = 0;
    //cout << input_string << endl;
    while (pos < input_string.length()) {
		//cout << pos << endl;
		string ch = input_string.substr(pos, 1);
        if(ch != " " && ch != "\n" && ch !="" && ch != "\r" && !ch.empty())splits.push_back(ch);

        pos++;
    }
    
	return splits;
}


//字符串分割函数
vector<std::string> split_string(std::string str,std::string pattern)
{
    string::size_type pos;
    vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
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
