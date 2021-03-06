#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define pi 3.14159265

double logb2(int x);
void fft(double *data, int left, int right, int N, double *X_real, double *X_imag);


typedef struct chunkInfo{
    unsigned char chunkId[4];
    int *chunkSize;
    short *format, *chanels;
    int *sampler, *byter;
    short *blockal, *bitpers;
}chunkInfo;

typedef struct chunkData{
    char chunkId[4];
    int *chunkSize;
    unsigned char *data;
    char *dataS;
}chunkData;

int main(int argc, char *argv[]){
    FILE *entrada, *salida;
    char WAVE[4];
    chunkInfo info;
    chunkData data;

    if(argc < 3)
        printf("No se ha indicado alguno de los archivos");
    else{
        entrada = fopen(argv[1], "rb");
        salida = fopen(argv[2], "wb");
        if(entrada != NULL){
            for(int i=0; i<4; i++)
                fputc(fgetc(entrada), salida);
            for(int i=0; i<4; i++)
                fgetc(entrada);
            fread(WAVE, sizeof(char), 4, entrada);

            fread(info.chunkId, sizeof(char), 4, entrada);
            info.chunkSize= (int*)malloc(sizeof(int));
            fread(info.chunkSize, sizeof(int), 1, entrada);
            info.format = (short*)malloc(sizeof(short));
            fread(info.format, sizeof(short), 1, entrada);
            info.chanels = (short*)malloc(sizeof(short));
            fread(info.chanels, sizeof(short), 1, entrada);
            info.sampler = (int*)malloc(sizeof(int));
            fread(info.sampler, sizeof(int), 1, entrada);
            info.byter = (int*)malloc(sizeof(int));
            fread(info.byter, sizeof(int), 1, entrada);
            info.blockal = (short*)malloc(sizeof(short));
            fread(info.blockal, sizeof(short), 1, entrada);
            info.bitpers = (short*)malloc(sizeof(short));
            fread(info.bitpers, sizeof(short), 1, entrada);
            short bitPer = *info.bitpers;

            fread(data.chunkId, sizeof( char), 4, entrada);
            data.chunkSize = (int*)malloc(sizeof(int));
            fread(data.chunkSize, sizeof(int), 1, entrada);
            int dataChunk = *data.chunkSize;
            if(bitPer == 8){
                data.data = (unsigned char*)malloc(sizeof(char)*dataChunk);
                fread(data.data, sizeof(char), dataChunk, entrada);
            }
            else{
                data.dataS = (char*)malloc(sizeof(char)*dataChunk);
                fread(data.dataS, sizeof(char), dataChunk, entrada);
            }
            
            //Decoding the samples
            int dChunk = dataChunk/2;
            int finalSize = 0;
            double *muestras;
            double *X_real, *X_imag;
            int numBits = 0;
            if(bitPer == 8){
                numBits = ceil(logb2(dataChunk));
                finalSize = pow(2, numBits);
                muestras = (double*)malloc(sizeof(double)*finalSize);
                for(int i=0; i<finalSize; i++){
                    if(i < dataChunk) muestras[i] = (data.data[i]-128)/128.0;
                    else muestras[i] = 0.0;
                }
            }
            else{
                numBits = ceil(logb2(dChunk));
                finalSize = pow(2, numBits);
                muestras = (double*)malloc(sizeof(double)*finalSize);
                for(int i=0, b=0; i<finalSize; i++, b+=2){
                    if(i < dChunk)
                        muestras[i] = ((unsigned char)data.dataS[b] | data.dataS[b+1]<<8)/32767.0;
                    else
                        muestras[i] = 0.0;
                }
            }
            
            //Bit reversal
            int checkList[finalSize];
            for(int i=0; i<finalSize; i++)
                checkList[i] = -1;
            for(int j=0; j<finalSize; j++){ 
                unsigned int reverse = 0; 
                if(checkList[j] == -1){
                    for(int i=0; i<numBits; i++){
                        if((j & (1<<i))) 
                            reverse |= 1<<((numBits-1)-i);   
                    }
                    double temp = muestras[j];
                    muestras[j] = muestras[reverse];
                    muestras[reverse] = temp;
                    checkList[j] = 1; checkList[reverse] = 1;
                }
            }
            
            //FFT
            X_real = (double*)malloc(sizeof(double)*finalSize);
            X_imag = (double*)malloc(sizeof(double)*finalSize);
            fft(muestras, 0, finalSize-1, finalSize, X_real, X_imag);
    
            short numChannels = 2;
            int sampRate = *info.sampler;
            short blockAlign = numChannels*(bitPer/8);
            int byter = numChannels*sampRate*(bitPer/8);

            int subSize1 = *info.chunkSize;
            int subSize2 = finalSize*numChannels*(bitPer/8);
            int chunkSize = subSize1+subSize2+20;

            info.chanels = &numChannels;
            info.byter = &byter;
            info.blockal = &blockAlign;
            int *chkS = &chunkSize;
            
            fwrite(chkS, sizeof(int), 1, salida);
            fwrite(WAVE, sizeof(char), 4, salida);
            fwrite(info.chunkId, sizeof(char), 4, salida);
            fwrite(info.chunkSize, sizeof(int), 1, salida);
            fwrite(info.format, sizeof(short), 1, salida);
            fwrite(info.chanels, sizeof(short), 1, salida);
            fwrite(info.sampler, sizeof(int), 1, salida);
            fwrite(info.byter, sizeof(int), 1, salida);
            fwrite(info.blockal, sizeof(short), 1, salida);
            fwrite(info.bitpers, sizeof(short), 1, salida);

            *data.chunkSize = subSize2;
            fwrite(data.chunkId, sizeof(int), 1, salida);
            fwrite(data.chunkSize, sizeof(int), 1, salida);
            if(bitPer == 8){
                for(int i=0; i<finalSize; i++){
                    fputc((unsigned char)(X_real[i]*128/finalSize)+128, salida);
                    if(i<100) printf("%d: %.6f, %x\n", i, X_real[i], (unsigned char)(X_real[i]*128/finalSize)+128);
                    fputc((unsigned char)(X_imag[i]*128/finalSize)+128, salida);
                }
            }
            else{
                for(int i=0; i<dataChunk/2; i++){
                    unsigned int aux = (unsigned int)(X_real[i]*32767.0/dChunk);
                    unsigned char less = (unsigned char)aux;
                    char more = (char)(aux>>8);
                    fputc(less, salida);
                    fputc(more, salida);
                    aux = (unsigned int)(X_imag[i]*32767.0/dChunk);
                    less = (unsigned char)aux;
                    more = (char)(aux>>8);
                    fputc(less, salida);
                    fputc(more, salida);
                }
            }

            while(!feof(entrada))
                fputc(fgetc(entrada), salida);
            fclose(entrada);
            fclose(salida);
        }
        else
            printf("Error al abrir archivo");
    }
    return 0;
}

double logb2(int x){
    return log(x)/log(2);
}

void fft(double *data, int left, int right, int N, double *X_real, double *X_imag){
    if(N > 2){
        fft(data, left, N/2-1+left, N/2, X_real, X_imag);
        fft(data, N/2+left, right, N/2, X_real, X_imag);
        double angle;
        double real_sinusoid;
        double imag_sinusoid;
        for(int i=left, iSecHalf=i+N/2, k=0; k<N/2; i++, iSecHalf++, k++){
            angle = (2*pi*k)/N;
            real_sinusoid = cos(angle);
            imag_sinusoid = -sin(angle);
            double auxReal = X_real[i];
            double auxImag = X_imag[i];
            X_real[i] = X_real[i] + X_real[iSecHalf]*real_sinusoid;
            X_imag[i] = X_imag[i] + X_imag[iSecHalf]*imag_sinusoid;
            X_real[iSecHalf] = auxReal - X_real[iSecHalf]*real_sinusoid;
            X_imag[iSecHalf] = auxImag - X_imag[iSecHalf]*imag_sinusoid;
        }
    }
    else{
        //Sinusoid
        int k=0;
        double angle = (2*pi*k)/N;
        double real_sinusoid = cos(angle);
        double imag_sinusoid = -sin(angle);
        //New left
        X_real[left] = data[left] + data[right]*real_sinusoid;
        X_imag[left] = data[left] + data[right]*imag_sinusoid;
        //New right
        X_real[right] = data[left] - data[right]*real_sinusoid;
        X_imag[right] = data[left] - data[right]*imag_sinusoid;
    }
}





