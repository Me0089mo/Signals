#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define pi 3.14159265

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
    chunkInfo info;
    chunkData data;

    if(argc < 3)
        printf("No se ha indicado alguno de los archivos");
    else{
        entrada = fopen(argv[1], "rb");
        salida = fopen(argv[2], "wb");
        if(entrada != NULL){
            for(int i=0; i<34; i++)
                fputc(fgetc(entrada), salida);
            info.bitpers = (short*)malloc(sizeof(short));
            fread(info.bitpers, sizeof(short), 1, entrada);
            short bitPer = *info.bitpers;
            fwrite(info.bitpers, sizeof(short), 1, salida);

            for(int i=0; i<4; i++)
                fputc(fgetc(entrada), salida);
            data.chunkSize = (int*)malloc(sizeof(int));
            fread(data.chunkSize, sizeof(int), 1, entrada);
            int dataChunk = *data.chunkSize;
            fwrite(data.chunkSize, sizeof(int), 1, salida);
            if(bitPer == 8){
                data.data = (unsigned char*)malloc(sizeof(char)*dataChunk);
                fread(data.data, sizeof(char), dataChunk, entrada);
            }
            else{
                data.dataS = (char*)malloc(sizeof(char)*dataChunk);
                fread(data.dataS, sizeof(char), dataChunk, entrada);
            }

        int dChunk = dataChunk/2;
        double *sumR, *sumI;
        if(bitPer == 8){
            double X_real[dChunk];
            double X_imag[dChunk];
            sumR = (double*)malloc(sizeof(double)*dChunk);
            sumI = (double*)malloc(sizeof(double)*dChunk);
            for(int i=0, j=0; i<dataChunk; i+=2, j++){
                X_real[j] = (data.data[i]-128)/128.0;
                X_imag[j] = (data.data[i+1]-128)/128.0;
            }
            for(int i=0; i<dChunk; i++){
                sumR[i] = 0; sumI[i] = 0;
                for(int j=0; j<dChunk; j++){
                    double angle = (2*pi*i*j)/dChunk;
                    sumR[i] += X_real[j]*cos(angle) - X_imag[j]*sin(angle);
                    sumI[i] += X_imag[j]*cos(angle) + X_real[j]*sin(angle);
                }
                if(sumR[i] > 1.0) sumR[i] = 1.0;
                if(sumI[i] > 1.0) sumI[i] = 1.0;
            }
        }
        else{
            double X_real[dChunk/2];
            double X_imag[dChunk/2];
            sumR = (double*)malloc(sizeof(double)*dChunk/2);
            sumI = (double*)malloc(sizeof(double)*dChunk/2);
            for(int n=0, b=0; n<dChunk/2; n++, b+=4){
                X_real[n] = ((unsigned char)data.dataS[b] | data.dataS[b+1]<<8)/32767.0;
                X_imag[n] = ((unsigned char)data.dataS[b+2] | data.dataS[b+3]<<8)/32767.0;
            }
            for(int i=0; i<dChunk/2; i++){
                sumR[i] = 0; sumI[i] = 0;
                for(int j=0; j<dChunk/2; j++){
                    double angle = (2*pi*i*j)/(dChunk/2);
                    sumR[i] += X_real[j]*cos(angle) - X_imag[j]*sin(angle);
                    sumI[i] += X_imag[j]*cos(angle) + X_real[j]*sin(angle);
                }
                if(sumR[i] > 1.0) sumR[i] = 1.0;
                if(sumI[i] > 1.0) sumI[i] = 1.0;
            }
        }
       
        if(bitPer == 8){
            for(int i=0, aux=0; i<dChunk; i++, aux+=2){
                fputc((unsigned char)(sumR[i]*128+128), salida);
                fputc((unsigned char)(sumI[i]*128+128), salida);
            }
        }
        else{
            for(int i=0; i<dChunk/2; i++){
                unsigned int aux = (unsigned int)(sumR[i]*32767.0);
                unsigned char less = (unsigned char)aux;
                char more = (char)(aux>>8);
                fputc(less, salida);
                fputc(more, salida);
                aux = (unsigned int)(sumI[i]*32767.0);
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
