#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define pi 3.14159265

int main(int argc, char *argv[]){
    FILE *entrada, *salida;
    short *bitpers = (short*)malloc(sizeof(short));;
    int *chunkSize = (int*)malloc(sizeof(int));
    unsigned char *data;
    char *dataS;

    if(argc < 3)
        printf("No se ha indicado alguno de los archivos");
    else{
        entrada = fopen(argv[1], "rb");
        salida = fopen(argv[2], "wb");
        if(entrada != NULL){
            for(int i=0; i<22; i++)
                fputc(fgetc(entrada), salida);
            
            fread(bitpers, sizeof(short), 1, entrada);
            fwrite(bitpers, sizeof(short), 1, salida);
            short bitPer = *bitpers;
            
            for(int i=0; i<4; i++)
                fputc(fgetc(entrada), salida);
            fread(chunkSize, sizeof(int), 1, entrada);
            int dataChunk = *chunkSize;
            if(bitPer == 8){
                data = (unsigned char*)malloc(sizeof(char)*dataChunk);
                fread(data, sizeof(char), dataChunk, entrada);
            }
            else{
                dataS = (char*)malloc(sizeof(char)*dataChunk);
                fread(dataS, sizeof(char), dataChunk, entrada);
            }
            
            //Starts algorithm
            int frecMuest = 44100;
            double frecuencias = {697, 770, 852, 941, 1209, 1336, 1477};
            int subChunk = 
            for(int i=0; i<; i++)
                frecuencias[i] = frecuencias[i]*2*pi/frecMuest; 
            
            if(bitPer == 8){
                
            }
            else{
                
            }
        }
        else
            printf("Error al abrir archivo");
        
    return 0;
}
