// deep copy of a donut
Donut* DeepCopyDonut(Donut* original) {
    if (original == NULL) {
        exit(EXIT_FAILURE);
    }
    Donut* copy = (Donut*)malloc(sizeof(Donut));
    if (copy == NULL) exit(EXIT_FAILURE);

    copy->data = (DonutMetaData*)malloc(sizeof(DonutMetaData));
    if (copy->data == NULL) {
        free(copy);
        exit(EXIT_FAILURE);
    }
    memcpy(copy->data, original->data, sizeof(DonutMetaData));

    int thic = copy->data->THIC;
    int rows = copy->data->ROWS;
    int cols = copy->data->COLS;

    copy->donut = (int***)malloc(thic * sizeof(int**));
    if (copy->donut == NULL) {
        free(copy->data);
        free(copy);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thic; i++) {
        copy->donut[i] = (int**)malloc(rows * sizeof(int*));
        if (copy->donut[i] == NULL) {
            for (int x = 0; x < i; x++) {
                for (int y = 0; y < rows; y++) free(copy->donut[x][y]);
                free(copy->donut[x]);
            }
            free(copy->donut);
            free(copy->data);
            free(copy);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < rows; j++) {
            copy->donut[i][j] = (int*)malloc(cols * sizeof(int));
            if (copy->donut[i][j] == NULL) {
                exit(EXIT_FAILURE); 
            }
            memcpy(copy->donut[i][j], original->donut[i][j], cols * sizeof(int));
        }
    }
    return copy;
}