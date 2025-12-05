#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

typedef struct {
    int COLS;
    int ROWS;
    int THIC;
    double centerX;
    double centerY;
    double centerZ;
    double radiusX1;
    double radiusY1;
    double radiusZ1;
    double radiusX2;
    double radiusY2;
    double radiusZ2;
} DonutMetaData;

typedef struct {
    DonutMetaData* data;
    int ***donut;
} Donut;

typedef struct {
    int **viewport;
} Frame;

void RotationAboutX(int x, int y, int z, int deg, double cx, double cy, double cz, int result[3]){
    double rads = (deg * M_PI) / 180.0;
    double localY = y - cy;
    double localZ = z - cz;

    double rotY = localY * cos(rads) - localZ * sin(rads);
    double rotZ = localY * sin(rads) + localZ * cos(rads);

    result[0] = x;
    result[1] = round(rotY + cy);
    result[2] = round(rotZ + cz);
}

Donut* InitialiseEmptyDonut(int ROWS, int COLS, int THIC){
    
    Donut* model = (Donut*)malloc(sizeof(Donut));
    if (model == NULL) {
        perror("Failed to allocate memory for Donut struct");
        return NULL;
    }
    model->data = (DonutMetaData*)malloc(sizeof(DonutMetaData));
    if (model->data == NULL) {
        perror("Failed to allocate memory for Donut metadata");
        free(model);
        return NULL;
    }
    *model->data = (DonutMetaData){
        COLS,
        ROWS,
        THIC,
        (COLS - 1) / 2.0,
        (ROWS - 1) / 2.0,
        (THIC - 1) / 2.0,
        COLS / 2.0,
        ROWS / 2.0,
        THIC / 2.0,
        COLS / 7.0,
        ROWS / 7.0,
        THIC / 7.0
    };
    //matrix creation
    model->donut = (int***)malloc(model->data->THIC * sizeof(int**));
    if (model->donut == NULL) { 
        perror("Failed to allocate memory for thicc pointers");
        free(model->data);
        free(model);
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<model->data->THIC; i++){
        model->donut[i] = (int**)malloc(model->data->ROWS * sizeof(int*));
        if (model->donut[i] == NULL) {
            perror("Failed to allocate memory for row pointers");
            for (int j = 0; j < i; j++) {
                free(model->donut[j]);
            }
            free(model->donut);
            free(model->data);
            free(model);
            exit(EXIT_FAILURE);
        }
        for(int k=0; k<model->data->ROWS; k++){
            model->donut[i][k] = (int*)calloc(model->data->COLS, sizeof(int));
            if (model->donut[i][k] == NULL) {
                perror("Failed to allocate memory for col pointers");
                for (int z = 0; z <= i; z++) {
                    for (int y = 0; y < k; y++) {
                        free(model->donut[z][y]);
                    }
                    free(model->donut[z]);
                }
                free(model->donut);
                free(model->data);
                free(model);
                exit(EXIT_FAILURE);
            }
        }
    }
    return model;
}

void InitialiseDonut(Donut* model){
    for(int i=0; i<2; i++){
        for(int j=0; j<model->data->ROWS; j++){
            for(int k=0; k<model->data->COLS; k++){
                double normX1 = (k - model->data->centerX) / model->data->radiusX1;
                double normX2 = (k - model->data->centerX) / model->data->radiusX2;
                double normY1 = (j - model->data->centerY) / model->data->radiusY1;
                double normY2 = (j - model->data->centerY) / model->data->radiusY2;
                // based on ((x-h)/rx)**2 + ((y-k)/ry)**2 = 1
                double eDist1 = pow(normX1, 2) + pow(normY1, 2);
                double eDist2 = pow(normX2, 2) + pow(normY2, 2);

                if(eDist2 >= (1 + (0.2 - i*0.1)) && eDist1 <= (1 - (0.2 - i*0.1))){
                    model->donut[i][j][k] = 1;
                }
            }
        }
    }
    for(int i=2; i<7; i++){
        for(int j=0; j<model->data->ROWS; j++){
            for(int k=0; k<model->data->COLS; k++){
                double normX1 = (k - model->data->centerX) / model->data->radiusX1;
                double normX2 = (k - model->data->centerX) / model->data->radiusX2;
                double normY1 = (j - model->data->centerY) / model->data->radiusY1;
                double normY2 = (j - model->data->centerY) / model->data->radiusY2;
                // based on ((x-h)/rx)**2 + ((y-k)/ry)**2 = 1
                double eDist1 = pow(normX1, 2) + pow(normY1, 2);
                double eDist2 = pow(normX2, 2) + pow(normY2, 2);

                if(eDist2 >= 1 && eDist1 <= 1){
                    model->donut[i][j][k] = 1;
                }
            }
        }
    }
    for(int i=7; i<model->data->THIC; i++){
        for(int j=0; j<model->data->ROWS; j++){
            for(int k=0; k<model->data->COLS; k++){
                double normX1 = (k - model->data->centerX) / model->data->radiusX1;
                double normX2 = (k - model->data->centerX) / model->data->radiusX2;
                double normY1 = (j - model->data->centerY) / model->data->radiusY1;
                double normY2 = (j - model->data->centerY) / model->data->radiusY2;
                // based on ((x-h)/rx)**2 + ((y-k)/ry)**2 = 1
                double eDist1 = pow(normX1, 2) + pow(normY1, 2);
                double eDist2 = pow(normX2, 2) + pow(normY2, 2);

                if(eDist2 >= (1 + (0.2 - abs(i - 8)*0.1)) && eDist1 <= (1 - (0.2 - abs(i - 8)*0.1))){
                    model->donut[i][j][k] = 1;
                }
            }
        }
    }
}

Frame* generateXRotationFrames(Donut* model){
    int ROTATIONS = 360;
    Frame* Frames = (Frame*)malloc((ROTATIONS + 1)*sizeof(Frame));
    if (Frames == NULL) { 
        perror("Failed to copy struct");
        free(model);
        free(Frames);
        exit(EXIT_FAILURE);
    }
    for(int r=0; r<=ROTATIONS; r++){
        Frames[r].viewport = (int**)malloc(model->data->ROWS * sizeof(int*));
        for(int j=0; j<model->data->ROWS; j++){
            Frames[r].viewport[j] = (int*)malloc(model->data->COLS * sizeof(int));
        }
    } 
    for(int j=0; j<model->data->ROWS; j++){
        for(int k=0; k<model->data->COLS; k++){
            if(model->donut[0][j][k]) Frames[0].viewport[j][k] = 1;
        }
    }

    for(int r=1; r<=ROTATIONS; r++){
        for(int i=0; i<model->data->THIC; i++){
            for(int j=0; j<model->data->ROWS; j++){
                for(int k=0; k<model->data->COLS; k++){
                    if(model->donut[i][j][k]){
                        int transformedPoint[3];
                        RotationAboutX(k, j, i, r, model->data->centerX, model->data->centerY, model->data->centerZ, transformedPoint);
                        int x = transformedPoint[0];
                        int y = transformedPoint[1];
                        int z = transformedPoint[2];
                        if( x < 0 || x >= model->data->COLS) continue;
                        if( y < 0 || y >= model->data->ROWS) continue;
                        if( z < 0 || z >= model->data->THIC) continue;
                        Frames[r].viewport[y][x] = 1;
                    }
                }
            }
        }
    }
    return Frames;
}

int main(){
    int ROWS = 25;
    int COLS = 50;
    int THIC = 9;

    Donut* model = InitialiseEmptyDonut(ROWS, COLS, THIC); 
    InitialiseDonut(model);

    Frame* frames = generateXRotationFrames(model);

    while(1){
        for(int i=0; i<=360; i++){
            for(int j=0; j<model->data->ROWS; j++){
                for(int k=0; k<model->data->COLS; k++){
                    if(frames[i].viewport[j][k]) printf(".");
                    else printf(" ");
                }
                printf("\n");
            }
            usleep(10000);
            system("clear");
        }
    }

    free(model);
    return 0;
}