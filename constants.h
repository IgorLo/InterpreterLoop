#ifndef INTERPRETER_LOOP_CONSTANTS_H
#define INTERPRETER_LOOP_CONSTANTS_H

//Типы лексем
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда

//Внутренние представления лексем
#define LOOP 10
#define DO 11
#define END 12 //Конец LOOP
#define EOL 13 //Конец строки
#define FINISHED 14 //Конец программы

#endif //INTERPRETER_LOOP_CONSTANTS_H
