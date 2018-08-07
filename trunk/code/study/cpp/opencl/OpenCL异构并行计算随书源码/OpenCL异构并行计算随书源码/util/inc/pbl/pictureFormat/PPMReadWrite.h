/**
 * @file
 * @author yyfn
 * @date 20100909
 *
 * @brief wrap for accessing PPM file, no CUDA support
 **/
#pragma once
    #include <iostream>
    #include <fstream>
    #include <stdlib.h>
    #include <string.h>

const unsigned int PGMHeaderSize = 0x40;
/**
 * @brief Load PGM or PPM file
 * @note if data == NULL then the necessary memory is allocated in the 
 *   function and w and h are initialized to the size of the image
 * @return true if the file loading succeeded, otherwise false
 * @param file name of the file to load
 * @param data handle to the memory for the image file data
 * @param w width of the image
 * @param h height of the image
 * @param channels number of channels in image
**/
bool readPPMFile(const char* file, unsigned char** data, 
            unsigned int *w, unsigned int *h, unsigned int *channels){
            
    if(NULL == file || NULL == w || NULL == h || NULL == channels){
        printf("one argument == NULL");
        exit(1);
    }
    
    if(NULL != *data){
        printf("warnning: maybe you init data \n");
    }
    FILE* fp = NULL;

	// open the file for binary read
    if ((fp = fopen(file, "rb")) == NULL){
        printf("Failed to open file: %s\n", file);
        perror("");
        return false;
    }

    // check header
    char header[PGMHeaderSize];
    if ((fgets( header, PGMHeaderSize, fp) == NULL) && ferror(fp)){
        if (fp){
            fclose (fp);
        }
        perror("File is not a valid PPM image");
        *channels = 0;
        return false;
    }

    if (strncmp(header, "P5", 2) == 0){
        *channels = 1;
    }else if (strncmp(header, "P6", 2) == 0){
        *channels = 3;
    }else{
        perror("File is not a PPM or PGM image");
        *channels = 0;
        return false;
    }

    // parse header, read maxval, width and height
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int maxval = 0;
    unsigned int i = 0;
    while(i < 3){
        if ((fgets(header, PGMHeaderSize, fp) == NULL) && ferror(fp)){
            if (fp){
                fclose (fp);
            }
            perror("File is not a valid PPM or PGM image");
            return false;
        }
        if(header[0] == '#') continue;

            if(i == 0){
                i += sscanf(header, "%u %u %u", &width, &height, &maxval);
            }else if (i == 1){
                i += sscanf(header, "%u %u", &height, &maxval);
            }else if (i == 2){
                i += sscanf(header, "%u", &maxval);
            }
    }

    *data = (unsigned char*)malloc( sizeof(unsigned char) * width * height * *channels);
    *w = width;
    *h = height;

    // read and close file
    if (fread(*data, sizeof(unsigned char), width * height * *channels, fp) != width * height * *channels){
        fclose(fp);
        std::cerr << "loadPPM() : Invalid image." << std::endl;
        return false;
    }
    fclose(fp);

    return true;
}

/**
 * @brief Write PPM file
 *
 * @note Internal usage only
 * @param file  name of the image file
 * @param data  handle to the data read
 * @param w     width of the image
 * @param h     height of the image
**/
bool writePPMFile( const char* file, unsigned char *data, 
             unsigned int w, unsigned int h, unsigned int channels){
    if(NULL == data){
        printf("data == NULL\n");
        exit(1);
    }

    std::fstream fh( file, std::fstream::out | std::fstream::binary );
    if( fh.bad()){
        printf("Opening file %s failed.", file);
        perror("");
        return false;
    }

    if (channels == 1){
        fh << "P5\n";
    }else if (channels == 3) {
        fh << "P6\n";
    }else {
        perror("Invalid number of channels.");
        return false;
    }

    fh << w << "\n" << h << "\n" << 0xff << std::endl;

    for( unsigned int i = 0; (i < (w*h*channels)) && fh.good(); ++i){
        fh << data[i];
    }
    fh.flush();

    if(fh.bad()){
        perror("Writing data failed.");
        return false;
    } 
    fh.close();

    return true;
}

/**
 * @brief Load PPM image file (with unsigned char as data element type), padding 4th component
 * @return true if reading the file succeeded, otherwise false
 * @param file  name of the image file
 * @param data  handle to the data read
 * @param w     width of the image
 * @param h     height of the image
**/
bool readPPMFile4Bytes( const char* file, unsigned char** data, 
                unsigned int *w,unsigned int *h){
    unsigned char *idata = 0;
    unsigned int channels;
    
    if (readPPMFile( file, &idata, w, h, &channels)) {
        // pad 4th component
        int size = *w * *h;

        // keep the original pointer
        unsigned char* idata_orig = idata;
        *data = (unsigned char*) malloc( sizeof(unsigned char) * size * 4);
        unsigned char *ptr = *data;
        for(int i=0; i<size; i++){
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = 0;
        }
        free( idata_orig);
        return true;
    }else{
        free(idata);
        return false;
    }
}

/**
 * @brief Save PPM image file (with unsigned char as data element type, padded to 4 byte)
 *
 * @return true if writing the file succeeded, otherwise false
 * @param file  name of the image file
 * @param data  handle to the data read
 * @param w     width of the image
 * @param h     height of the image
**/
bool writePPMFile4Byte( const char* file, unsigned char *data, 
               unsigned int w, unsigned int h){
    // strip 4th component
    int size = w * h;
    unsigned char *ndata = (unsigned char*) malloc( sizeof(unsigned char) * size*3);
    unsigned char *ptr = ndata;
    for(int i=0; i<size; i++) {
        *ptr++ = *data++;
        *ptr++ = *data++;
        *ptr++ = *data++;
        data++;
    }
    
    return writePPMFile(file, ndata, w, h, 3);
}

