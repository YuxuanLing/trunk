#include "../inc/yyfnutil.h"

int main(){
    CPURandom rnd;
    
    size_t len = 10000;
    
    int *ri;
    ri = (int*) malloc(sizeof(int)*len);
    rnd.nextIntSequence(ri, len);
    rnd.nextIntSequence(0, 50, len, ri);
        for(int i = 0; i < len; i++){
        printf("i = %d value = %d\n", i, ri[i]);
    }

    float *rf;
    rf = (float*) malloc(sizeof(float)*len);
//    rnd.nextFloatSequence(len, rf);
    rnd.nextFloatSequence(0.6f, 1.0f, rf, len);
    for(int i = 0; i < len; i++){
        printf("i = %d value = %f\n", i, rf[i]);
    }
    
    free(rf);
    
    free(ri);

    return 0;
}
