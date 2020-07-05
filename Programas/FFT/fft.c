#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define pi 3.14159265

double logb2(int x);
void fft(double *data, double *sinusoids, double angle, int left, int right, int N, double X_real, double X_imag);


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

            int dChunk = dataChunk/2;
            int finalSize = 0;
            double *sinusoids = (double *)malloc(sizeof(double)*2);
            double *muestras;
            double *X_real, *X_imag;
            if(bitPer == 8){
                finalSize = pow(2, ceil(logb2(dataChunk)));
                muestras = (double*)malloc(sizeof(double)*finalSize);
                X_real = (double*)malloc(sizeof(double)*finalSize);
                X_imag = (double*)malloc(sizeof(double)*finalSize);
                for(int i=0; i<finalSize; i++){
                    if(i < dataChunk) muestras[i] = (data.data[i]-128)/128.0;
                    else muestras[i] = 0;
                }
            }
            else{
                finalSize = pow(2, ceil(logb2(dChunk)));
                muestras = (double*)malloc(sizeof(double)*finalSize);
                X_real = (double*)malloc(sizeof(double)*finalSize);
                X_imag = (double*)malloc(sizeof(double)*finalSize);
                for(int i=0, b=0; i<finalSize || b < dataChunk; i++, b+=2){
                    if(i < dataChunk)
                        muestras[i] = ((unsigned char)data.dataS[b] | data.dataS[b+1]<<8)/32767.0;
                    else
                        muestras[i] = 0;
                }
            }
            for(int i=0; i<finalSize; i++){
                //Calculate sinusoid
                double angle;
                if(bitPer == 8) angle = (2*pi*i)/dataChunk;
                else angle = (2*pi*i)/dChunk;
                sinusoids[0] = muestras[i] * cos(angle);
                sinusoids[1] = muestras[i] * (-sin(angle));
                double realk=0, imgk=0;
                fft(muestras, sinusoids, angle, 0, finalSize-1, finalSize, realk, imgk);
                X_real[i] = realk;
                X_imag[i] = imgk;
            }
    
            printf("%d, %d", dataChunk, finalSize);
            short numChannels = 2;
            int sampRate = *info.sampler;
            short blockAlign = numChannels*(bitPer/8);
            int byter = numChannels*sampRate*(bitPer/8);

            int subSize1 = (int)(*info.chunkSize);
            int subSize2 = finalSize*numChannels;
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
                for(int i=0; i<dataChunk; i++){
                    fputc((unsigned char)(X_real[i]*128/dataChunk)+128, salida);
                    fputc((unsigned char)(X_imag[i]*128/dataChunk)+128, salida);
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

void fft(double *data, double *sinusoids, double angle, int left, int right, int N, double X_real, double X_imag){
    if(N > 2){
        fft(data, sinusoids, angle, left, N/2-1, N/2, X_real, X_imag);
        fft(data, sinusoids, angle, N/2, right-1, N/2, X_real, X_imag);
    }
    else{
        double ang1 = angle * left * 2;
        X_real += data[left]*cos(ang1);
        X_imag += data[left]*(-sin(ang1));
        double ang2 = angle * right * 2;
        X_real += data[right]*cos(ang2) * sinusoids[0];
        X_imag += data[right]*(-sin(ang2)) * sinusoids[1];
    }
}





