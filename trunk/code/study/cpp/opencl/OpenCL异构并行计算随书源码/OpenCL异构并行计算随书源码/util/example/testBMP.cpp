#include "../inc/yyfnutil.h"

int main(int argc, char *argv[]){
    Pixels pixels;
    BMPHeader header;
    BMPInfo info;
    
    readBMPFile("frog3.bmp", &pixels, &header, &info);
    
    writeBMPFile("fuck.bmp", pixels, header, info);
    
    freePixelsHost(&pixels);

    return 0;
}
