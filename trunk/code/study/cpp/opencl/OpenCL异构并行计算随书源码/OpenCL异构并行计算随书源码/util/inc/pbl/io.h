/**
 * @file
 * @author yyfn
 * @date 20100909
 **/
#pragma once
    #include <stdio.h>
    #include <stdlib.h>
    #include <vector>
    #include <iostream>
    #include <fstream>
    #include <string.h>

using namespace std;

/**
 * @class IO
 * @brief use for data input or output
 *
 **/
class IO{

	public :
	

	static std::string clLoadProgramSource(const char* filename){

		FILE* file = fopen(filename, "rb");
		if(NULL == file){       
			return std::string();
		}

		if(0 != fseek(file, 0, SEEK_END)){
			perror("fseek:");
			printf("%s, %d\n", __FILE__, __LINE__);
			exit(1);
		}
	 
		size_t size = ftell(file);

		if(0 != fseek(file, 0, SEEK_SET)){
			perror("fseek:");
			printf("%s, %d\n", __FILE__, __LINE__);
			exit(1);
		} 

		char* sourceString = (char *)malloc(size + 1); 
		if(1 != fread(sourceString, size, 1, file)){
		    fclose(file);
		    free(sourceString);
			perror("fread:");
			printf("%s, %d\n", __FILE__, __LINE__);
		    exit(1);
		}

		std::string s = std::string(sourceString, size);

		fclose(file);
		free(sourceString);

		return s;
	}

    /**
     * @brief Read file \fileName and return the data
     * @return true if reading the file succeeded, otherwise false
     * @param fileName name of the source file
     * @param data  uninitialized pointer, returned initialized and pointing to the data read
     * @param len  number of data elements in data, -1 on error
     **/
    template<typename T>
    static bool readFile(const char* fileName, T** data, size_t *len){
    // check input arguments
        if(NULL == fileName){
            printf("fileName pointer == NULL\n");
            exit(1);
        }
        
        if(NULL == len){
            printf("len pointer == NULL\n");
            exit(1);
        }
        
        if(NULL != *data){
            printf("data pointer != NULL\n");
            exit(1);
        }

    // intermediate storage for the data read
        vector<T>  dv;

    // open file for reading
        fstream fs(fileName, fstream::in);
    // check if filestream is valid
        if(!fs.good()){
            std::cerr << "readFile() : Opening file failed." << std::endl;
            return false;
        }

    // read all data elements 
        T token;
        while(fs.good()){
            fs >> token;   
            dv.push_back(token);
        }

    // the last element is read twice
        dv.pop_back();

    // check if reading result is consistent
        if( !fs.eof()){
            std::cerr << "WARNING : readData() : reading file might have failed." << std::endl;
		    exit(1);
        }

        fs.close();

// allocate storage for the data read
	    *data = (T*)malloc(sizeof(T)*dv.size());
// store signal size
        *len = static_cast<unsigned int>(dv.size());

// copy data
        memcpy(*data, &dv[0], sizeof(T) * dv.size());

        return true;
    }
    
    /**
     * @brief Write a data file
     * @return true if writing the file succeeded, otherwise false
     * @param fileName name of the file
     * @param data  data to write
     * @param len  number of data elements in data
     **/
    template<class T>
    static bool writeFile(const char* fileName, const T* data, size_t len){
        if(NULL == fileName){
            printf("fileName pointer == NULL\n");
            exit(1);
        }
    
        if(NULL == data){
            printf("data pointer == NULL\n");
            exit(1);
        }

    // open file for writing
        fstream fo( fileName, std::fstream::out);
    // check if filestream is valid
        if(!fo.good()){
            std::cerr << "writeFile() : Opening file failed." << std::endl;
            return false;
        }

    // write data
        for(size_t i = 0; (i < len) && (fo.good()); i++){
            fo << data[i] << ' ';
        }

    // Check if writing succeeded
        if(!fo.good()){
            std::cerr << "writeFile() : Writing file failed." << std::endl;
            return false;
        }

    // file ends with nl
        fo << std::endl;

        return true;
    }

    /**
     ** @brief load binary data from file \fileName
     ** @notes you must free return data manual
     ** @return data in file, length == size
     **/
    unsigned char* readRBFile(const char* fileName, size_t size){
        if(NULL == fileName){
            printf("fileName == NULL\n");
            exit(1);
        }
    
        FILE *fp = NULL;
    
        if ((fp = fopen(fileName, "rb")) == NULL){
            printf(" Error opening file '%s' !!!\n", fileName);
            exit(1);
        }

        unsigned char* data = (unsigned char*)malloc(size);
        size_t read = fread(data, 1, size, fp);
        fclose(fp);

        printf(" Read '%s', %ld bytes\n", fileName, read);

        return data;
    }

    /**
     * @brief print one data every line to screen. 
     *
     * @param num data size
     * @param out data to print
     **/
    template <typename T>
    static void println(const int num, T *out){
        if(NULL == out){
            printf("out == NULL\n");
            exit(1);
        }
        
        for(int i = 0; i < num; i++){
            std::cout << out[i] << std::endl;
        }
    }

    /**
     * @brief print \changLine data every line to screen
     *
     * @param num data size
     * @param changLine number data per line
     * @param out data to print
     **/
    template <typename T>
    static void println(const int num, const int changLine, T *out){
        if(NULL == out){
            printf("out == NULL\n");
            exit(1);
        }
        
        for(int i = 0; i < num; i++){
            std::cout << out[i] <<"    ";
            if(0 == (i+1)%changLine){
                std::cout << std::endl;
            }
        }
    }

};
