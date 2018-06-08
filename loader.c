#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <mem.h>

#include "main.h"

#define SIZE 5000
#define SIZE_LEXEM 100
/*
 * Объявление переменных
 */
struct variable {
    char name[SIZE_LEXEM];
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
        printf("Use format: <execute file>.exe <text program file>.txt <names of out variables>.txt");
        exit(1);
    }

    char *file_program = argv[1];
    char *file_variables = argv[2];

    if (!(pointer_program = (char *) malloc(SIZE))) {
        printf("Error allocating memory!");
        exit(1);
    }
    //Загрузка программы
    if (!readFile(file_program)) {
        printf("Ne udalos load program"); //TODO
        exit(1);
    }

    execute(pointer_program, file_variables);

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
        if (i == k*SIZE){
            k++;
            pointer_program = (char*) realloc(pointer_program, (size_t) (k * SIZE));
            point = pointer_program;
            point+=i;
        }
    } while (!feof(file));
    *(point - 1) = 0;
    fclose(file);
    return 1;
}