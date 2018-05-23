#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

//Мои штуки
#include "analyzer.h"

//-------------------------------------------------------------
#define PROG_SIZE 10000 //Память под программу в байтах

/*
 * Объявление переменных
 */
char *program;

//-------------------------------------------------------------


/*
 * Объявление функций
 */
int loadProgram(char*, char*); //Считывает программу

//-------------------------------------------------------------

int main(int argc, char *argv[]) {
    char *p_buf; //Указатель начала буфера программы
    char *file_name = argv[1]; //Имя файла программы

    if (argc != 2) {
        printf("Используйте формат: <исполняемый файл>.exe <файл программы>.txt");
        exit(1);
    }

    //Выделение памяти для программы
    if (!(p_buf = (char *) malloc(PROG_SIZE))) {
        printf("Error allocating memory!");
        exit(1);
    }

    //Загрузка программы
    if (!loadProgram(p_buf, file_name))
        exit(1);

    program = p_buf;

    start(program); //Запуск анализатора и интерпретатора; там все в кучу =)

    return 0;
}

int loadProgram(char *p, char *fname) {
    FILE *file;

    if (!(file = fopen(fname, "r"))) //Открываем только на чтение
        return 0;

    //Считываем текст программы в память
    int i = 0;
    do {
        *p = (char) getc(file);
        p++;
        i++;
    } while (!feof(file) && i < PROG_SIZE);

    *(p - 1) = '\0'; //Символ конца программы
    fclose(file);
    return 1;
}