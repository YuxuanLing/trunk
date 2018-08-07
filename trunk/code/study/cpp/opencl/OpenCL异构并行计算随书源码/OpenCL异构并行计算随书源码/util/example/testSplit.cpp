#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "../inc/yyfnutil.h"

int main(){
    const std::string s = "we are hero";
    
    std::vector<std::string> vs;
    
    const std::string delim = " ";
    
    split(s, delim, &vs);

    size_t len = vs.size();
    printf("len = %ld\n", len);

    for(int i = 0; i < len; i++){
        std::cout<<vs[i]<<std::endl;
    }

}
