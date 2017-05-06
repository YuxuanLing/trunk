#ifdef __cplusplus
/**
 * @file
 * @author yyfn
 * @date 20100909
 *
 * @brief rnd lib for cpu
 **/

#pragma once
    #include <string.h>
    #include <math.h>
    #include <time.h>
    #include <stdlib.h>
 /**
  * @class
  * @brief class use for generate rand data
  *
  **/
class CPURandom{

	public :
		/**
		 * @brief constructor, use current time as rand seed
		 *
		 **/
		CPURandom(){
			srand(time(NULL));
		}
		/**
		 * @brief constructor, use seed as rand seed
		 *
		 **/
		CPURandom(int seed){
			srand(seed);
		}
		/**
		 * @brief generate one int data
		 *
		 **/
		inline int nextInt(){
			return rand();
		}
		
		inline void nextIntSequence(int *out, const int num){
			for(int i = 0; i < num;i++){
				out[i] = nextInt();
			}
		}

		/**
		 * @brief generate one int data between min and max
		 *
		 **/
		inline int nextInt(const int min, const int max){
			return min+rand()%(max - min);
		}
		/**
		 * @brief generate a sequence int data between min and max
		 *
		 **/
		inline void nextIntSequence(const  int min, const int max,\
				const int num, int *out){
			for(int i = 0; i < num; i++){
				out[i] = nextInt(min, max);
			}
		}
		/**
		 * @brief generate one float data between 0 and 1
		 *
		 **/
		inline float nextFloat(){
			return 1.0f*rand()/RAND_MAX;
		}
		/**
		 * @brief generate a sequence float data between 0 and 1
		 *
		 **/
		inline void nextFloatSequence(float *out, const int num){
			for(int i = 0; i < num; i++){
				out[i] = nextFloat();
			}
		}

		/**
		 * @brief generate one data between min and max
		 *
		 **/
		inline float nextFloat(const float min, const float max){
			return min+(max-min)*nextFloat();
		}
		/**
		 * @brief generate a sequence data between min and max
		 *
		 **/
		inline void nextFloatSequence(const float min, const float max,\
				float *out, const int num){
			for(int i = 0; i < num; i++){
				out[i] = nextFloat(min, max);
			}
		}
		/**
		 * @brief generate one data obey gaussian distribution
		 *
		 **/
		float nextGaussian(){
			static float v1, v2, s;
			static  int haveNextValue=0;
			float value;
			
			if(0 == haveNextValue){
				do {
				   v1 = 2.0f * rand()/RAND_MAX - 1; 
				   v2 = 2.0f * rand()/RAND_MAX - 1;
				 
				   s = v1 * v1 + v2 * v2;
				} while (s >= 1.0f || s == 0);
				
				value = v1*sqrt(-2 * log(s)/s);
			}else{
				value = v2*sqrt(-2*log(s)/s);     
			}

			 haveNextValue = 1-haveNextValue;
			 return value;
		}
		/**
         * @brief generate a sequence of data obeyed gaussian distribution
		 *
		 * @param num data length
		 * @param out data
		 **/
		void nextGaussianSequence(float *out, const int num){
			for(int i = 0; i < num; i++){
				out[i] = nextGaussian();
			}
		}
};

#endif
