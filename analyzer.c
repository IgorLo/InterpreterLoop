#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <mem.h>
#include <ctype.h>

//Мои штуки
#include "constants.h"

//-------------------------------------------------------------

#define NUM_VAR 100 //Ограничение на количество переменных; Если менять, то нужно ниже переопределять массив VARIABLES
#define LOOP_NEST 25 //Ограничение на вложенность циклов
#define START_POSITION_LOOP 0 //Индекс начала отсчета итераций в LOOP


/*
 * Объявление переменных
 */
char *program;
jmp_buf e_buf; //Буфер среды функции longjmp()

char token[80]; //Внешнее представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы

//Списк команд
struct command {
    char name[10];
    int token_int;
} tableCommand[] = {
        "LOOP", LOOP,
        "DO", DO,
        "END", END};

//Заранее объявленные переменные
int variables[NUM_VAR] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct loop_stack {
    int var; //Начальное значение цикла/счетчик цикла
    int target; //Конечное значение цикла
    char *location; // Программа цикла
} loops[LOOP_NEST]; //Стек для LOOP'ов
int loop_tos; //Индекс начала стека

//-------------------------------------------------------------


/*
 * Объявление функций
 */
int getToken(); //Считывает очередную лексему
int isWhite(char); //Пробел/табуляция или нет
int isDelim(char); //Разделитель или нет

int getIntCommand(char *); //Ищет внутренне представление команды
void assignment(); //Присваивает значение переменной
void performOperation(int *); //Считает значение выражения
int getVar(int); //Получение значения переменной
void getExp(int *); //Получения значения очередной переменной в выражении
void primitive(int *);
void arith(char, int *, int *); //Выполнение оперцаий +-

void executeLoop(); //Считать LOOP
void executeEnd(); //Произвести ОДНУ итерацию LOOP
void loop_push(struct loop_stack); //Положить LOOP в стек
struct loop_stack loop_pop(); //Изъять LOOP из стека

//-------------------------------------------------------------


//Начало работы анализатора
void start(char *p) {
    program = p;

    if (setjmp(e_buf)) //Инициализация буфера нелокальных переходов
        exit(1);

    loop_tos = 0; //Инициализация указателя стека LOOP'ов

    do {
        token_type = getToken();

        //Проверка на присваивание
        if (token_type == VARIABLE) {
            assignment();
        }

        //Проверка на команду
        if (token_type == COMMAND) {
            switch (token_int) {
                case LOOP:
                    executeLoop();
                    break;
                case END:
                    executeEnd();
                    break;
                default:
                    break;
            }
        }

    } while (token_int != FINISHED);
}

void sError(int error) {
    static char *e[] = {
            "Syntax error",
            "This is not an expression",
            "Expectation was the symbol of equality",
            "Not variable",
            "The nesting level of the loop is too large",
            "END does not match LOOP",
            "Requires DO operator",


    };
    printf("%s\n", e[error]);
    longjmp(e_buf, 1); //Возврат в точку сохранения
}

int getToken() {
    char *temp; //Указатель на лексему

    token_type = 0;
    token_int = 0;
    temp = token;

    while (isWhite(*program))
        program++; //Пропускаем пробелы

    //Проверка закончился ли файл интерпретируемой программы
    if (*program == '\0') {
        *token = '\0';
        token_int = FINISHED;
        return (token_type = DELIMITER);
    }

    //Проверка на конец строки программы
    if (*program == '\n') {
        program++;
        token_int = EOL;
        *token = '\n';
        temp++;
        *temp = 0;
        return (token_type = DELIMITER);
    }

    //Проверка на разделитель
    if (strchr("=+-:", *program)) {
        *temp = *program;

        //Если нашел :, то ищем =
        if (strchr(":", *program)) {
            program++;
            temp++;
            if (strchr("=", *program)) {
                *temp = *program;
                program++;
                temp++;
                *temp = 0;
            } else
                sError(2);
        } else {
            program++;
            temp++;
            *temp = 0;
        }
        return (token_type = DELIMITER);
    }

    //Проверка на число
    if (isdigit(*program)) {
        while (!isDelim(*program)) {
            *temp++ = *program++;
        }
        *temp = 0;
        return (token_type = NUMBER);
    }

    //Переменная или команда?
    if (isalpha(*program)) {

        if (strchr("x", *program)) {
            program++;
        }

        while (!isDelim(*program))
            *temp++ = *program++;
        *temp = 0;
        token_int = getIntCommand(token); //Получение внутреннего представления команды
        if (!token_int)
            token_type = VARIABLE;
        else
            token_type = COMMAND;
        return token_type;
    }
    sError(0);
}

int isWhite(char c) {
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}

int isDelim(char c) {
    if (strchr(" :+-=", c) || c == '\r' || c == '\n')
        return 1;
    return 0;
}

int getIntCommand(char *t) {

    //Поиск лексемы в таблице операторов
    for (int i = 0; *tableCommand[i].name; i++) {
        if (!strcmp(tableCommand[i].name, t))
            return tableCommand[i].token_int;
    }
    return 0; //Незнакомый оператор
}

void assignment() {
    int var, value;

    var = atoi(token);

    if (var > NUM_VAR) {
        sError(3);
        return;
    }

    getToken(); //Считываем символ равенства
    if (token[0] != ':' && token[1] != '=') {
        sError(2);
        return;
    }

    getExp(&value); //Считать присваемое значение

    //Присвоить значение
    variables[var] = value;

}

void getExp(int *result) {
    getToken();
    if (!*token) {
        sError(1);
        return;
    }
    performOperation(result);
}

void performOperation(int *result) {
    char operation;
    int hold;

    primitive(result);
    while ((operation = *token) == '+' || operation == '-') {
        getToken();
        primitive(&hold);
        arith(operation, result, &hold);
    }

}

void primitive(int *result) {
    switch (token_type) {
        case VARIABLE:
            *result = getVar(atoi(token));
            getToken();
            return;
        case NUMBER:
            *result = atoi(token);
            getToken();
            return;
        default:
            sError(0);
    }
}

void arith(char o, int *r, int *h) {
    switch (o) {
        case '-':
            *r = *r - *h;
            break;
        case '+':
            *r = *r + *h;
            break;
        default:
            break;
    }
}

int getVar(int v) {
    if (v > NUM_VAR) {
        sError(3); //Это не переменная
        return 0;
    }
    return variables[v];
}

void loop_push(struct loop_stack i) {
    if (loop_tos > LOOP_NEST)
        sError(4);

    loops[loop_tos] = i;
    loop_tos++;
}

struct loop_stack loop_pop() {
    loop_tos--;
    if (loop_tos < 0) sError(5);
    return (loops[loop_tos]);
}

void executeLoop() {
    struct loop_stack i;

    getToken(); //Чтение управляющей переменной
    if (atoi(token) > NUM_VAR) {
        sError(3);
        return;
    }

    i.var = START_POSITION_LOOP;
    i.target = variables[atoi(token)];

    getToken();
    if (token_int != DO)
        sError(6);

    i.location = program;
    loop_push(i);
}

void executeEnd() {
    struct loop_stack i;

    i = loop_pop(); //Чтение информации о цикле

    i.var++; //Увеличение переменной цикла
    if (i.var > i.target)
        return; //Конец
    loop_push(i); //В противном случае запомнить информацию
    program = i.location; //Цикл
}