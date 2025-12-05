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

typedef struct {
    int size;
    Frame *frames;
} FrameList;

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
        return NULL;
    }
    for(int i=0; i<model->data->THIC; i++){
        model->donut[i] = (int**)malloc(model->data->ROWS * sizeof(int*));
        if (model->donut[i] == NULL) {
            perror("Failed to allocate memory for row pointers");
            for (int z = 0; z < i; z++) {
                for (int y = 0; y < model->data->ROWS; y++) {
                        free(model->donut[z][y]);
                    }
                free(model->donut[z]);
            }
            free(model->donut);
            free(model->data);
            free(model);
            return NULL;
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
                return NULL;
            }
        }
    }
    return model;
}

void InitialiseDonut(Donut* model){
    double maxWallThickness = 0.15; 

    for(int i=0; i<model->data->THIC; i++){
        double zNormal;
        if (model->data->THIC > 1) {
            zNormal = (i - model->data->centerZ) / model->data->centerZ; 
        } else {
            zNormal = 0;
        }

        // Circular Cross-Section (max at z=0)
        double chonkiness = sqrt(1.0 - (zNormal * zNormal));
        double currentThickness = maxWallThickness * chonkiness;
        double innerThreshold = 0.8 - currentThickness; //0.8 was ideally 1 for ellipse but had to chose lower number for chonkiness
        double outerThreshold = 0.8 + currentThickness;

        for(int j=0; j<model->data->ROWS; j++){
            for(int k=0; k<model->data->COLS; k++){
                double normX1 = (k - model->data->centerX) / model->data->radiusX1;
                double normX2 = (k - model->data->centerX) / model->data->radiusX2;
                double normY1 = (j - model->data->centerY) / model->data->radiusY1;
                double normY2 = (j - model->data->centerY) / model->data->radiusY2;

                // ((x-h)/rx)^2 + ((y-k)/ry)^2 = 1
                double eDist1 = pow(normX1, 2) + pow(normY1, 2);
                double eDist2 = pow(normX2, 2) + pow(normY2, 2);

                if(eDist2 >= innerThreshold && eDist1 <= outerThreshold){
                    model->donut[i][j][k] = i < model->data->THIC/2 ? 1 : 2;
                }
            }
        }
    }
}

void freeDonutModel(Donut* model){
    for (int z = 0; z < model->data->THIC; z++) {
        for (int y = 0; y < model->data->ROWS; y++) {
            free(model->donut[z][y]);
        }
        free(model->donut[z]);
    }
    free(model->donut);
    free(model->data);
    free(model);
}

void freeFrameBuffer(FrameList* frameList, Donut* model){
    for(int i=0; i<frameList->size; i++){
        for(int j=0; j<model->data->ROWS; j++) free(frameList->frames[i].viewport[j]);
        free(frameList->frames[i].viewport);
    }
    free(frameList->frames);
    free(frameList);
}

Frame* generateXRotationFrames(Donut* model, int frameBufferSize){
    int ROTATIONS = frameBufferSize;

    Frame* Frames = (Frame*)malloc((ROTATIONS)*sizeof(Frame));
    if (Frames == NULL) { 
        perror("Failed to create frame buffer");
        return NULL;
    }
    for(int r=0; r<=ROTATIONS; r++){
        Frames[r].viewport = (int**)malloc(model->data->ROWS * sizeof(int*));
        if (Frames[r].viewport == NULL){
            perror("Failed to initialize viewport Row for frame");
            for(int j=0; j<r; j++) free(Frames[j].viewport);
            free(Frames);
            return NULL;
        }
        for(int j=0; j<model->data->ROWS; j++){
            Frames[r].viewport[j] = (int*)malloc(model->data->COLS * sizeof(int));
            if (Frames[r].viewport[j] == NULL){
                perror("Failed to initialize viewport Column for frame");
                for(int j=0; j<=r; j++){
                    for(int k=0; k<j; k++) free(Frames[j].viewport[k]);
                    free(Frames[j].viewport);
                }
                free(Frames);
                return NULL;
            }
        }
    }

    //filling first frame
    for(int j=0; j<model->data->ROWS; j++){
        for(int k=0; k<model->data->COLS; k++){
            if(model->donut[0][j][k]) Frames[0].viewport[j][k] = 1;
        }
    }

    double** zBuffer = (double**)malloc(model->data->ROWS * sizeof(double*));
    if(zBuffer == NULL){
        perror("Failed to initialize zBuffer rows");
        return NULL;
    }
    for(int j=0; j<model->data->ROWS; j++){
        zBuffer[j] = (double*)malloc(model->data->COLS * sizeof(double));
        if(zBuffer[j] == NULL){
        perror("Failed to initialize zBuffer cols");
        for(int k=0; k<j; k++) free(zBuffer[k]);
        free(zBuffer);
        return NULL;
    }
    }

    for(int r=1; r<=ROTATIONS; r++){
        for(int j=0; j<model->data->ROWS; j++){
            for(int k=0; k<model->data->COLS; k++){
                zBuffer[j][k] = -10000.0; 
            }
        }
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
                        if (z > zBuffer[y][x]) {
                            zBuffer[y][x] = z;
                            Frames[r].viewport[y][x] = model->donut[i][j][k];
                        }
                    }
                }
            }
        }
    }
    for(int j=0; j<model->data->ROWS; j++) free(zBuffer[j]);
    free(zBuffer);
    return Frames;
}

void animate(Donut* model, Frame* frames, int frameBufferSize){
    while(1){
        for(int i=0; i<=frameBufferSize; i++){
            for(int j=0; j<model->data->ROWS; j++){
                for(int k=0; k<model->data->COLS; k++){
                    if(frames[i].viewport[j][k] == 2) printf("\033[36m.\033[0m");
                    else if(frames[i].viewport[j][k] == 1) printf(".");
                    else printf(" ");
                }
                printf("\n");
            }
            usleep(10000);
            system("clear");
        }
    }
}

int main(){
    int ROWS = 25;
    int COLS = 50;
    int THIC = 9;

    Donut* model = InitialiseEmptyDonut(ROWS, COLS, THIC);
    if(model == NULL){
        perror("Could not initialise model.");
        return 1;
    }

    InitialiseDonut(model);

    FrameList* frameList = (FrameList*)malloc(sizeof(FrameList));
    if(frameList == NULL){
        perror("Could not initialise frameList buffer.");
        freeDonutModel(model);
        return 1;
    }
    frameList->size = 361;
    frameList->frames = generateXRotationFrames(model, frameList->size);

    animate(model, frameList->frames, frameList->size);

    freeFrameBuffer(frameList, model);
    freeDonutModel(model);
    return 0;
}