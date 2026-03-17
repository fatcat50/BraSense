#ifndef TASKS_H
#define TASKS_H

void sdTask(void *pvParam);
void wsTask(void *pvParam);
void sensorTask(void *pvParam);   // 500 Hz Messschleife auf Core 1
void createTasks();

#endif