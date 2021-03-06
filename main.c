#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <mem.h>
#include <ctype.h>

//Типы лексем
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда

//Внутренние представления лексем
#define LOOP 10
#define DO 11
#define IF 12
#define THEN 13
#define ELSE 14
#define END 15
#define EOL 16
#define FINISHED 17
#define ENDIF 18

/*
 * Объявление переменных
 */
#define NESTING 30 //Количество вложений LOOP
#define SIZE_LEXEM 100 //Длина лексемы
#define START_POSITION_LOOP 1 //Индекс начало отсчета LOOP

char *program;
struct lexem {
    char name[SIZE_LEXEM];
    int id;
    int type;
} token;
struct command {
    char name[6];
    int id;
} commands[] = {
        "LOOP", LOOP,
        "DO", DO,
        "IF", IF,
        "THEN", THEN,
        "ELSE", ELSE,
        "END", END,
        "ENDIF", ENDIF};
struct loop_stack {
    int source;
    int target;
    char *body_cycle;
} loops[NESTING]; //Данные LOOP
int loop_index; //Индекс начала стека LOOP'ов
int count_var = 0; //Количество переменных
struct variable {
    char name[SIZE_LEXEM];
    int value;
} *pointer_variables; //Указатель на начало памяти хренения переменных
char *outFile; //Указатель на строку с путём к выходному файлу

//-------------------------------------------------------------


/*
 * Объявление функций
 */
void printError(char *); //Печатает ошибку
int isDelim(char), isWhite(char);
void readToken(); //Читает очередную лексему
int getIdCommand(char *); //Получает ID команды
void assignment(); //Присваивает значение переменной
struct variable *findVariable(char *); //Ищет переменную по имени
struct variable *addVariable(char *); //Добавляет переменную
void calcExpression(int *); //Считает выражение
void addOrSub(int *);
void multOrDiv(int *);
void unary(int *);
void parentheses(int *);
void arithmetic(char, int *, int *);
void executeLoop(), executeEnd(), executeIf();
void writeResult(char*);
void executeToElseOrEndif();
void skipToElseOrEndif();
void executeToken();
void endProgram();
void skipToEndif();

//-------------------------------------------------------------



void execute(char *to_program, char *file) {
    program = to_program;
    outFile = file;

    loop_index = 0;

    while (token.id != FINISHED){
        readToken();
        executeToken();
    }
    endProgram();

}

void executeToken() {
    //readToken();

    if (token.id == FINISHED){
        endProgram();
    }

    if (token.type == VARIABLE) {
        assignment();
    }

    if (token.type == COMMAND) {
        switch (token.id) {
            case LOOP:
                executeLoop();
                break;
            case IF:
                executeIf();
                break;
            case END:
                executeEnd();
            default:
                break;
        }
    }
}

void writeResult(char *file_name) {
    FILE *file_in, *file_out;
    file_in = fopen(file_name, "r");
    file_out = fopen("../result.txt", "w");
    char name[SIZE_LEXEM];
    char *t = name;
    do {
        *t = (char) getc(file_in);
        if (*t == '\n' || feof(file_in)) {
            *t = '\0';
            struct variable *temp_variable = findVariable(name);
            if (temp_variable == NULL){
                t++;
                continue;
            }
            fprintf(file_out, "%s = %d\n", temp_variable->name, temp_variable->value);
            t = name;
        } else{
            t++;
        }

    } while (!feof(file_in));
}

void putBack() {
    char *t;
    t = token.name;
    for (; *t; t++) program--;
}

void executeIf() {
    int x, y, cond;
    char operation;
    calcExpression(&x); //Получаем левое выражение
    if (!strchr("=<>", *token.name)) {
        printError("Syntax error: \"=, <, >\" expected."); //Недопустимый оператор
        return;
    }
    operation = *token.name;
    calcExpression(&y);  //Получаем правое выражение
    putBack();
    //Определяем результат
    cond = 0;
    switch (operation) {
        case '=':
            if (x == y) cond = 1;
            break;
        case '<':
            if (x < y) cond = 1;
            break;
        case '>':
            if (x > y) cond = 1;
            break;
        default:
            break;
    }
    if (cond) {  //Если значение IF "истина"
        if (token.id != THEN) {
            printError("Syntax error: THEN expected.");
        }
        executeToElseOrEndif();
    } else {
        skipToElseOrEndif();
//        do {
//            readToken();
//            if (token.id == FINISHED)
//                printError("Syntax error: ELSE or END expected."); //Ожидался ELSE или END
//        } while (token.id != ELSE && token.id != END);
//        if (token.id == ELSE)
//            putBack();
    }
}

void skipToElseOrEndif() {
    readToken();
    int count = 0;
    do {
        if (token.id == FINISHED){
            printError("ENDIF expected.");
        }
        if (token.id == IF)
            count++;
        if (token.id == ENDIF)
            count--;
        if (count < 0)
            printError("Syntax error: ENDIF doesn't match closing construction");
        readToken();
    } while ((token.id != ENDIF || count!= 0) && (token.id != ELSE || count != 0));
    if (token.id == ENDIF)
        return;
    do {
        readToken();
        executeToken();
    } while (token.id != ENDIF);
}

void executeToElseOrEndif() {
    readToken();
    do {
        if (token.id == FINISHED){
            printError("ENDIF expected.");
        }
        executeToken();
        readToken();
    } while (token.id != ENDIF && token.id != ELSE);
    if (token.id == ELSE){
        skipToEndif();
    }
}

void skipToEndif() {
    int count = 0;
    while (token.id != ENDIF){
        if (token.id == IF)
            count++;
        if (token.id == ENDIF)
            count--;
        if (count < 0)
            printError("Syntax error: ENDIF doesn't match closing construction");
        readToken();
    }
}

void loop_push(struct loop_stack i) {
    if (loop_index > NESTING)
        printError("The nesting level of the loop is too large.");

    loops[loop_index] = i;
    loop_index++;
}

struct loop_stack loop_pop() {
    loop_index--;
    if (loop_index < 0) printError("END does not match closing construction.");
    return (loops[loop_index]);
}

void executeLoop() {
    struct loop_stack i;

    //readToken(); //Чтение переменной-счётчика

    i.source = START_POSITION_LOOP;

    int loop_iterations = 0;

    calcExpression(&loop_iterations);

    i.target = loop_iterations;

    if (i.target < i.source){
        int counter = 0;

        do {

            switch (token.id){
                case FINISHED:
                    printError("Syntax error.");
                    break;
                case IF:
                    counter++;
                    break;
                case LOOP:
                    counter++;
                    break;
                case END:
                    counter--;
                    break;
                default:
                    break;
            }
            readToken();

        } while (token.id != END || counter != 0);

        return;
    }

    //readToken();
    if (token.id != DO)
        printError("Expected \"DO\" in LOOP construction");
    i.body_cycle = program;
    loop_push(i);

}

void executeEnd() {
    struct loop_stack i;

    i = loop_pop(); //Чтение информации о цикле

    i.source++; //Увеличение переменной цикла
    if (i.source > i.target)
        return; //Конец
    loop_push(i); //В противном случае запомнить информацию
    program = i.body_cycle; //Цикл
}

//Выводит ошибку и завершает программу
void printError(char *err) {
    printf(err);
    exit(1);
}

int isWhite(char c) {
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}

int isDelim(char c) {
    if (strchr(" :+-=/%*()", c) || c == '\r' || c == '\n')
        return 1;
    return 0;
}

void readToken() {
    char *t = token.name;
    token.id = 0;
    token.type = 0;

    while (isWhite(*program))
        program++;

    //Проверка конца программы
    if (*program == 0) {
        *token.name = 0;
        token.id = FINISHED;
        token.type = DELIMITER;
        return;
    }

    //Проверка конца строки
    if (*program == '\n') {
        *t++ = *program++;
        *t = '\0';
        token.id = EOL;
        token.type = DELIMITER;
        return;
    }

    //Првоерка разделителя
    if (strchr(":=+-*/%()<>", *program)) {
        //Ищем знак присваивания

        if (*program == ':') {
            *t++ = *program++;
            if (*program == '=') {
                *t++ = *program++;
                *t = '\0';
            } else
                printError("\":=\" expected.");
        } else {
            *t++ = *program++;
            *t = '\0';
        }
        token.type = DELIMITER;
        return;
    }

    //Проверка на число
    if (isdigit(*program)) {
        while (!isDelim(*program))
            *t++ = *program++;
        *t = '\0';
        token.type = NUMBER;
        return;
    }

    //Проверка на букву
    if (isalpha(*program)) {
        while (!isDelim(*program))
            *t++ = *program++;
        *t = '\0';
        token.id = getIdCommand(token.name);
        if (!token.id)
            token.type = VARIABLE;
        else
            token.type = COMMAND;
        return;
    }

    printError("Syntax error! Unknown token.");
}

int getIdCommand(char *command) {
    for (int i = 0; *commands[i].name; i++) {
        if (!strcmp(commands[i].name, command))
            return commands[i].id;
    }
    return 0;
}

struct variable *findVariable(char *name) {
    int i = 1;
    struct variable *t = pointer_variables;
    while (i <= count_var) {
        if (!strcmp(name, t->name)) {
            return t;
        }
        i++;
        t++;
    }
    addVariable(name);
    return findVariable(name);
}

struct variable *addVariable(char *name) {
    count_var++;
    pointer_variables = (struct variable *) realloc(pointer_variables, sizeof(struct variable) * count_var);
    struct variable *buffer_index = pointer_variables;

    int i = 1;
    while (i < count_var) {
        buffer_index++;
        i++;
    }

    struct variable *result = buffer_index;
    strcpy(result->name, name);
    result->value = 0;

    return result;
}

void assignment() {
    int value;
    struct variable *var;
    if ((var = findVariable(token.name)) == NULL)
        var = addVariable(token.name);
    readToken(); //Считываем равно
    calcExpression(&value); //Вычисляем выражение
    var->value = value;
}

void calcExpression(int *result) {
    readToken();
    addOrSub(result);
}

void addOrSub(int *result) {
    char operation;
    multOrDiv(result);

    int hold;
    while ((operation = *token.name) == '+' || operation == '-') {
        readToken();
        multOrDiv(&hold);
        arithmetic(operation, result, &hold);
    }
}

void multOrDiv(int *result) {
    char operation;
    unary(result);

    int hold;
    while ((operation = *token.name) == '/' || operation == '%' || operation == '*') {
        readToken();
        unary(&hold);
        arithmetic(operation, result, &hold);
    }
}

void unary(int *result) {
    char operation;
    if (token.type == DELIMITER && (operation = *token.name) == '+' || operation == '-') {
        readToken();
    }
    parentheses(result);
    if (operation == '-')
        *result = -(*result);
}

void parentheses(int *result) {
    char operation = *token.name;
    if (token.type == DELIMITER && operation == '(') {
        readToken();
        addOrSub(result);
        if (*token.name != ')')
            printError("This is not an expression!"); //TODO
        readToken();
    } else {
        struct variable *temp = findVariable(token.name);
        switch (token.type) {
            case VARIABLE:
                if (temp == NULL || temp->value == NULL){
                    *result = 0;
                }
                *result = temp->value;
                readToken();
                return;
            case NUMBER:
                *result = atoi(token.name);
                readToken();
                return;
            default:
                printError("Syntax error!"); //TODO
        }
    }
}

void arithmetic(char operation, int *leftPart, int *rightPart) {
    int t;
    switch (operation) {
        case '-':
            *leftPart = *leftPart - *rightPart;
            break;
        case '+':
            *leftPart = *leftPart + *rightPart;
            break;
        case '*':
            *leftPart = *leftPart * *rightPart;
            break;
        case '/':
            *leftPart = (*leftPart) / (*rightPart);
            break;
        case '%':
            t = (*leftPart) / (*rightPart);
            *leftPart = *leftPart - (t * (*rightPart));
            break;
        default:
            break;
    }
}

void endProgram() {
    if (loop_index != 0){
        printError("Loop is not closed with \"END\"");
    }
    writeResult(outFile);
}