#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "snr.h"

#ifdef WIN32
# define bool int
# define true 1
# define false 0
# define __bool_true_false_are_defined 1
#endif

#ifndef max
# define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
# define min(a, b) ((a) < (b) ? (a) : (b))
#endif


float snr (
  const unsigned char * data1,
  const unsigned char * data2,
  const int width,
  const int height,
  const int stride)
{
  float psnr;
  float sumsqr = 0;

  for (int i = 0; i < height; i++)
  {
    #pragma unroll(8)
    #pragma ivdep
    #pragma vector aligned
    for (int j = 0; j < width; j++)
    {
      float ival = (float) (data1[i * width + j] - data2[i * stride + j]);
      sumsqr += (ival * ival);
    }
  }
  psnr = ((float) sumsqr) / (65025.0f * width * height);
  psnr = -10.0f * log10f(psnr);

  return psnr;
}


double bdr (
  const double ref_psnr[4],
  const double new_psnr[4],
  const double ref_bitrate[4],
  const double new_bitrate[4])
{
  double xl,xh;
  double diff;

  double E[4],F[4],G[4],H[4];
  double SUM[2];
  
  int i,j;
  
  double  DET0,DET1,DET2,DET3,DET;
  double  D0,D1,D2,D3;
  double  A,B,C,D;

  double X[2][4];
  double Y[2][4];

  for (i=0;i<4;i++){
    X[0][i] = ref_psnr[i];
    X[1][i] = new_psnr[i];
    Y[0][i] = log (ref_bitrate[i]);
    Y[1][i] = log (new_bitrate[i]);
  }

  xl=max(X[0][0],X[1][0]);
  xh=min(X[0][3],X[1][3]);
  
  for (j=0;j<2;j++){
    for (i=0;i<4;i++){      
      E[i]=X[j][i];            
      F[i]=E[i]*E[i];         
     
      G[i]=E[i]*E[i]*E[i];
      H[i]=Y[j][i];
    }

    DET0= E[1]*(F[2]*G[3]-F[3]*G[2])-E[2]*(F[1]*G[3]-F[3]*G[1])+E[3]*(F[1]*G[2]-F[2]*G[1]);
    DET1=-E[0]*(F[2]*G[3]-F[3]*G[2])+E[2]*(F[0]*G[3]-F[3]*G[0])-E[3]*(F[0]*G[2]-F[2]*G[0]);
    DET2= E[0]*(F[1]*G[3]-F[3]*G[1])-E[1]*(F[0]*G[3]-F[3]*G[0])+E[3]*(F[0]*G[1]-F[1]*G[0]);
    DET3=-E[0]*(F[1]*G[2]-F[2]*G[1])+E[1]*(F[0]*G[2]-F[2]*G[0])-E[2]*(F[0]*G[1]-F[1]*G[0]);
    DET=DET0+DET1+DET2+DET3;

     
    D0=H[0]*DET0+H[1]*DET1+H[2]*DET2+H[3]*DET3;
    
    
    D1=
      H[1]*(F[2]*G[3]-F[3]*G[2])-H[2]*(F[1]*G[3]-F[3]*G[1])+H[3]*(F[1]*G[2]-F[2]*G[1])-
      H[0]*(F[2]*G[3]-F[3]*G[2])+H[2]*(F[0]*G[3]-F[3]*G[0])-H[3]*(F[0]*G[2]-F[2]*G[0])+
      H[0]*(F[1]*G[3]-F[3]*G[1])-H[1]*(F[0]*G[3]-F[3]*G[0])+H[3]*(F[0]*G[1]-F[1]*G[0])-
      H[0]*(F[1]*G[2]-F[2]*G[1])+H[1]*(F[0]*G[2]-F[2]*G[0])-H[2]*(F[0]*G[1]-F[1]*G[0]);
 
    D2=
      E[1]*(H[2]*G[3]-H[3]*G[2])-E[2]*(H[1]*G[3]-H[3]*G[1])+E[3]*(H[1]*G[2]-H[2]*G[1])-
      E[0]*(H[2]*G[3]-H[3]*G[2])+E[2]*(H[0]*G[3]-H[3]*G[0])-E[3]*(H[0]*G[2]-H[2]*G[0])+
      E[0]*(H[1]*G[3]-H[3]*G[1])-E[1]*(H[0]*G[3]-H[3]*G[0])+E[3]*(H[0]*G[1]-H[1]*G[0])-
      E[0]*(H[1]*G[2]-H[2]*G[1])+E[1]*(H[0]*G[2]-H[2]*G[0])-E[2]*(H[0]*G[1]-H[1]*G[0]);
   
    D3=
      E[1]*(F[2]*H[3]-F[3]*H[2])-E[2]*(F[1]*H[3]-F[3]*H[1])+E[3]*(F[1]*H[2]-F[2]*H[1])-
      E[0]*(F[2]*H[3]-F[3]*H[2])+E[2]*(F[0]*H[3]-F[3]*H[0])-E[3]*(F[0]*H[2]-F[2]*H[0])+
      E[0]*(F[1]*H[3]-F[3]*H[1])-E[1]*(F[0]*H[3]-F[3]*H[0])+E[3]*(F[0]*H[1]-F[1]*H[0])-
      E[0]*(F[1]*H[2]-F[2]*H[1])+E[1]*(F[0]*H[2]-F[2]*H[0])-E[2]*(F[0]*H[1]-F[1]*H[0]);
   

    A=D0/DET;
    B=D1/DET;
    C=D2/DET;
    D=D3/DET;
   
   
    SUM[j]=A*(xh-xl)+B*(xh*xh-xl*xl)/2+C*(xh*xh*xh-xl*xl*xl)/3+D*(xh*xh*xh*xh-xl*xl*xl*xl)/4;
    
  } 
  
  diff=(SUM[1]-SUM[0])/(xh-xl);
  diff=(exp(diff)-1)*100;
  
  return diff;

}

#if 0
int main(int argc,char **argv)
{
  FILE *fd;
  char snr_filename[100];
  float tmp;
  int i;

  double diff;

  double X[2][4];
  double Y[2][4];

  if (argc != 2) 
  {
    printf("Usage: %s <snr.txt> \n",argv[0]);
    printf("<snr.txt> gives snr values and bitrates\n");
    printf("Format:\n");
    printf("snrA0  snrA1  snrA2  snrA3 \n");
    printf("birtA0 bitrA1 bitrA2 bitrA3 \n");
    printf("snrB0  snrB1  snrB2  snrB3 \n");
    printf("birtB0 bitrB1 bitrB2 bitrB3 \n");


    exit(-1);
  }
  strcpy(snr_filename,argv[1]);
  
  if((fd=fopen(snr_filename,"r")) == NULL){
      printf("Error: Input file %s not found\n",snr_filename);
      exit(1);
  } 
  
  for(i=0;i<4;i++){
    fscanf(fd,"%f",&tmp);      // first 4 SNR values
    X[0][i]=tmp;
  }
  fscanf(fd,"%*[^\n]");         // new line 
  
  for(i=0;i<4;i++){             // first 4 bitrate values
    fscanf(fd,"%f,",&tmp);
    Y[0][i]=tmp;
  }
  fscanf(fd,"%*[^\n]");  

  for(i=0;i<4;i++){
    fscanf(fd,"%f,",&tmp);
    X[1][i]=tmp;

  }
  fscanf(fd,"%*[^\n]");
  
  for(i=0;i<4;i++){
    fscanf(fd,"%f,",&tmp);
    Y[1][i]=tmp;
  }
  fscanf(fd,"%*[^\n]");  

  diff = bdr (X[0], X[1], Y[0], Y[1]);
  printf("Percentage difference between the curves are = %f\n",diff);
 
  return 0;
}
#endif
