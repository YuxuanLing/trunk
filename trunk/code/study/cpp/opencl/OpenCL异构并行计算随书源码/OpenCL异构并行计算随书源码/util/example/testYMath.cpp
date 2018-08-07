#include "../inc/yyfnutil.h"

int main(){
    float *rf;
    size_t len = 10000;
    rf = (float*) malloc(len*sizeof(float));
    
    CPURandom rand(13);
    
    rand.nextGaussianSequence(rf, len);
    
    float r = mean(rf, len);
    printf("%f\n", r);
    
    float m = mse(rf, len);
    printf("%f\n", m);
    
    int ir = roundToMultiple(5, 2);
    printf("%d\n", ir);
    
    free(rf);
    
    return 0;
}
