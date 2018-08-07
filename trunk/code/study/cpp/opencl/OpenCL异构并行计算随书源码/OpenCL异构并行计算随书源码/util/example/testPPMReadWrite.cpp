#include <stdio.h>
#include "../inc/yyfnutil.h"

int main(){
    const char *fileName = "lena.ppm";//"ref.ppm";
    
    unsigned char *data = NULL;
    
    unsigned int width, height, channels;
    
    readPPMFile(fileName, &data, &width, &height, &channels);
/*    
    printf("width = %u\n", width);
    printf("height = %u\n", height);
    printf("channels = %u\n", channels);
    
    for(size_t i = 0; i < height; i++){
        for(size_t j = 0; j < width; j++){
            printf("[%d][%d] ", i, j);
            
            for(size_t k = 0; k < channels; k++){
                printf("%d ", data[channels*(i*width+j)+k]);
            }
            
            printf("\n");
        }
    }
    
    writePPMFile("t.ppm",data, width, height, channels);
  */  
    free(data);
}
