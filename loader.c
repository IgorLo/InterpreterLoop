#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#define TOTAL_MEMORY_SIZE 5000
#define LEXEME_MEMORY_SIZE 100

/*
 * Объявление переменных
 */

struct variable {
    char name[LEXEME_MEMORY_SIZE];
    int value;
};

char *pointer_program;

//-------------------------------------------------------------

/*
 * Объявление функций
 */
int readFile(char *);

//-------------------------------------------------------------

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Wrong format.\nUse: <executable file>.exe <out variables>.txt");
        exit(1);
    }

    char *filename_program = argv[1];
    char *filename_variables = argv[2];

    if (!(pointer_program = (char *) malloc(TOTAL_MEMORY_SIZE))) {
        printf("Unable to allocate the memory.");
        exit(1);
    }
    //Загрузка программы
    if (!readFile(filename_program)) {
        printf("Unable to read the program file.");
        exit(1);
    }

    execute(pointer_program, filename_variables);

    return 0;
}

int readFile(char *file_name) {
    FILE *file;
    if (!(file = fopen(file_name, "r")))
        return 0;
    char *point = pointer_program;
    int i = 0, k = 1;

    do {
        *point = (char) getc(file);
        point++;
        i++;
        if (i == k*TOTAL_MEMORY_SIZE){
            k++;
            pointer_program = (char*) realloc(pointer_program, (size_t) (k * TOTAL_MEMORY_SIZE));
            point = pointer_program;
            point += i;
        }
    } while (!feof(file));
    *(point - 1) = 0;
    fclose(file);
    return 1;
}