#ifndef H_INC_STRINGUTIL
#define H_INC_STRINGUTIL
/**
 * @file
 * @author yyfn
 * @date 20100909
 *
 **/
#pragma once
     #include <ctype.h>
     #include <stdio.h>
     #include <string.h>
     #include <stdlib.h>
#ifdef __cplusplus
    #include <string>
    #include <vector>
#endif
/**
 * @brief delete right space of string 
 *
 * @param in input string
 */ 
inline void rtrim(char *in){
    if(NULL == in){
        printf("in == NULL\n");
        exit(1);
    }
    
    const int len = strlen(in);
    
    int i = len-1;
    
    while(isspace(in[i])){
        i--;
    }
    in[i+1] = '\0';
}
/**
 * @brief delete left space of string
 * @param in input string
 */
inline void ltrim(char *in){
    if(NULL == in){
        printf("in == NULL\n");
        exit(1);
    }
    
    const int len = strlen(in);
    int i = 0;
    
    while(isspace(in[i])){
        i++;
    }
    
    int k =0;
    
    for(int j = i; j < len+1; j++, k++){
        in[k] = in[j];
    }    
}
/**
 * @brief delete left and right space of string
 * @param in input string
 */
inline void trim(char *in){
    rtrim(in);
    ltrim(in);
}
/**
 int main(){
    const char *s = "we, are, hero";
    
    size_t len;
    
    const char *delim = ",";
    char *result = strSplit(s, delim, &len);
    
    if(NULL == result){
        printf("String %s don\'t has %s\n", s, delim);
        exit(1);
    }
    printf("len = %ld\n", len);
    size_t offset = 0;
    for(int i = 0; i < len; i++){
        printf("%s\n", result+offset);
        offset += strlen(result+offset)+1;
    }
    
    free(result);
}

**/

/**

 ** @brief 将字符串依据分隔符分割
 ** 
 ** @param source 待分割字符串
 ** @param delim 分割字符串
 ** @param segment 返回的字符串向量

 ** @notes 不支持中文
 **/
char* strSplit(const char* source, const char *delim, size_t *segment){
	char* temp = (char*) malloc(strlen(source)+1);
	strcpy(temp, source);
	
	char* p = strtok(temp, delim);
	
	if(NULL == p){
		free(temp);
		*segment = 0;
		return NULL;
	}
	
	size_t len = 0;
	
	while(NULL != p){
		len++;
		p = strtok(NULL, delim);
	}
	
	*segment = len;
	
	return temp;
}
#ifdef __cplusplus
/**
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
**/
/**
 ** @brief 将字符串依据分隔符分割
 ** 
 ** @param source 待分割字符串
 ** @param delim 分割字符串
 ** @param vs 返回的字符串向量
 ** @notes 不支持中文
**/
void split(const std::string& source, const std::string& delim, std::vector<std::string> *vs){
	char *temp;
	temp = (char*)malloc(source.size()+1);
	if(NULL == temp){
		fprintf(stderr, "malloc failed!\n");
		exit(1);
	}
	
	strcpy(temp, source.c_str());
	
	vs->clear();
	const char *del = delim.c_str();
	
	char *p = strtok(temp, del);
	while(NULL != p){
		trim(p);
		if('\0' != p[0])
		vs->push_back(std::string(p));
		p = strtok(NULL, del);
	}
	
	free(temp);
}
#endif

#endif
