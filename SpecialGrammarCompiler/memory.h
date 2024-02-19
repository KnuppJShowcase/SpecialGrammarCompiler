#ifndef MEMORY_H
#define MEMORY_H

#include "tree.h"

void memory_init();

void memory_free();

int canReach;

void freeUpNode();

void functionAdd(char* name, struct nodeFunction* f);

void pushExecutePop(char* name, char** args);

void declare(char* iden, int type);

void store(char* iden, int value);

int recall(char* iden);

void storeRec(char* iden, int index, int value);

int recallRec(char* iden, int index);

void record(char* lhs, char* rhs);

void allocateRecord(char* iden, int size);

#endif
