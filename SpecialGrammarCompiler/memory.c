#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "memory.h"
#include "executor.h"
#include "tree.h"

/*
* Data Structures
*/

// Frame structure to hold variable lookup tables
struct frame {
	char** iLookup; // Integer variable names
	int* iValues;   // Integer variable values
	int iLen;       // Number of integer variables
	
	char** rLookup; // Record variable names
	int** rValues;  // Record variable values (arrays of integers)
	int rLen;       // Number of record variables
};

static struct frame *callStack[20]; // Stack of frames for function calls
static int fp;                       // Frame pointer

static char* funcNames[20];          // Function names
static struct nodeFunction* funcBodies[20]; // Function bodies
static int fLen;                     // Number of functions

// Counter for garbage collection
int canReach;

/*
* Helper Functions
*/

// Search for an integer variable in the current frame
static int searchInteger(char* iden) {
	int location = -1;
	for (int i = 0; i < callStack[fp]->iLen; i++) {
		if (strcmp(callStack[fp]->iLookup[i], iden) == 0) {
			location = i;
		}
	}
	return location;
}

// Search for a record variable in the current frame
static int searchRecord(char* iden) {
	int location = -1;
	for (int i = 0; i < callStack[fp]->rLen; i++) {
		if (strcmp(callStack[fp]->rLookup[i], iden) == 0) {
			location = i;
		}
	}
	return location;
}

// Search for a function by name
static struct nodeFunction* searchFunction(char* iden) {
	int location = -1;
	for (int i = 0; i < fLen; i++) {
		if (strcmp(funcNames[i], iden) == 0) {
			location = i;
		}
	}
	return funcBodies[location];
}

/*
* Memory Functions
*/

// Push a function call onto the call stack, execute it, and then pop it off
void pushExecutePop(char* name, char** args) {
	// Get the function definition
	struct nodeFunction* function = searchFunction(name);
	
	// Push new frame
	fp++;
	callStack[fp] = (struct frame*) calloc(2, sizeof(struct frame));
	callStack[fp]->iLookup = malloc(0);
	callStack[fp]->iValues = malloc(0);
	callStack[fp]->iLen = 0;
	callStack[fp]->rLookup = malloc(0);
	callStack[fp]->rValues = malloc(0);
	callStack[fp]->rLen = 0;

	// Create formal parameters
	for (int i = 0; i < 2; i++) {
		if (args[i] != NULL) {
			fp--;
			int* value = callStack[fp]->rValues[searchRecord(args[i])];
			fp++;
			callStack[fp]->rLen++;
			callStack[fp]->rLookup = realloc(callStack[fp]->rLookup, callStack[fp]->rLen * sizeof(char*));
			callStack[fp]->rLookup[callStack[fp]->rLen - 1] = function->params[i];
			callStack[fp]->rValues = realloc(callStack[fp]->rValues, callStack[fp]->rLen * sizeof(int*));
			callStack[fp]->rValues[callStack[fp]->rLen - 1] = value;
		}
	}

	// Execute the function body
	executeDeclSeq(function->ds);
	executeStmtSeq(function->ss);
	
	// Perform cleanup
	int i;
	for (i = 0; i < callStack[fp]->rLen; i++) {
		int* res = callStack[fp]->rValues[i];
		if (res[0] != 0) {
			res[1]--;
			if (res[1] == 0) {
				canReach--;
				printf("gc:%d\n", canReach);
			}
		}
	}
	
	// Pop frame
	fp--;
}

// Free up memory from the last frame
void freeUpNode() {
	int i;
	for (i = 0; i < callStack[0]->rLen; i++) {
		int* res = callStack[0]->rValues[i];
		if (res[0] != 0) {
			res[1]--;
			if (res[1] == 0) {
				canReach--;
				printf("gc:%d\n", canReach);
			}
		}
	}

	int j;
	int countTo = canReach;
	for (j = 0; j < countTo; j++) {
		if (canReach != 0) {
			canReach--;
			printf("gc:%d\n", canReach);
		}
	}
}

// Add function to function table
void functionAdd(char* name, struct nodeFunction* f) {
	funcNames[fLen] = name;
	funcBodies[fLen] = f;
	fLen++;
}

// Initialize memory
void memory_init() {
	fp = 0;
	fLen = 0;
	
	callStack[fp] = (struct frame*) calloc(1, sizeof(struct frame));
	callStack[fp]->iLookup = malloc(0);
	callStack[fp]->iValues = malloc(0);
	callStack[fp]->iLen = 0;
	callStack[fp]->rLookup = malloc(0);
	callStack[fp]->rValues = malloc(0);
	callStack[fp]->rLen = 0;
}

// Handle integer or record declaration
void declare(char* iden, int type) {
	if (type == INTEGER) {
		callStack[fp]->iLen++;
		callStack[fp]->iLookup = realloc(callStack[fp]->iLookup, callStack[fp]->iLen * sizeof(char*));
		callStack[fp]->iLookup[callStack[fp]->iLen - 1] = iden;
		callStack[fp]->iValues = realloc(callStack[fp]->iValues, callStack[fp]->iLen*sizeof(int));
		callStack[fp]->iValues[callStack[fp]->iLen-1] = 0;
	} else {
		callStack[fp]->rLen++;
		callStack[fp]->rLookup = realloc(callStack[fp]->rLookup, callStack[fp]->rLen*sizeof(char*));
		callStack[fp]->rLookup[callStack[fp]->rLen-1] = iden;
		callStack[fp]->rValues = realloc(callStack[fp]->rValues, callStack[fp]->rLen*sizeof(int*));
		callStack[fp]->rValues[callStack[fp]->rLen-1] = calloc(1, sizeof(int));
	}
}

void store(char* iden, int value) {
    // Check if the identifier is an integer variable
    int location = searchInteger(iden);
    if (location == -1) {
        // If not found, it may be a record variable, store at index 0
        storeRec(iden, 0, value);
    } else {
        // Store the value in the integer variable
        callStack[fp]->iValues[location] = value;
    }
}

int recall(char* iden) {
    // Check if the identifier is an integer variable
    int location = searchInteger(iden);
    if (location == -1) {
        // If not found, it may be a record variable, recall from index 0
        return recallRec(iden, 0);
    } else {
        // Return the value of the integer variable
        return callStack[fp]->iValues[location];
    }
}

void storeRec(char* iden, int index, int value) {
    // Find the index of the record variable
    int location = searchRecord(iden);
    // Check if the index is within bounds
    if (index + 1 <= callStack[fp]->rValues[location][0]) {
        // Store the value at the specified index in the record variable
        callStack[fp]->rValues[location][index + 1] = value;
    } else {
        // Error: index out of bounds
        printf("Runtime Error: write to index %d outside of array %s bounds!\n", index, iden);
        exit(0);
    }
}

int recallRec(char* iden, int index) {
    // Find the index of the record variable
    int location = searchRecord(iden);
    // Check if the index is within bounds
    if (index + 1 <= callStack[fp]->rValues[location][0]) {
        // Return the value at the specified index in the record variable
        return callStack[fp]->rValues[location][index + 1];
    } else {
        // Error: index out of bounds
        printf("Runtime Error: read from index %d outside of array %s bounds!\n", index, iden);
        exit(0);
    }
    return 0;
}

// Assign the contents of one record variable to another
void record(char* lhs, char* rhs) {
    // Find the indices of the record variables
    int locLhs = searchRecord(lhs);
    int locRhs = searchRecord(rhs);
    
    // Assign the contents of the rhs record to the lhs record
    callStack[fp]->rValues[locLhs] = callStack[fp]->rValues[locRhs];

    // Decrease the reference count of lhs rValues and check for garbage collection
    if (callStack[fp]->rValues[locLhs][1] != 0) {
        callStack[fp]->rValues[locLhs][1]--;
        if (callStack[fp]->rValues[locLhs][1] == 0) {
            canReach--;
            printf("gc:%d\n", canReach);
        }
    }
    
    // Increase the reference count of rhs rValues
    if (callStack[fp]->rValues[locRhs][1] != 0) {
        callStack[fp]->rValues[locRhs][1]++;
    }
}

// Allocate memory for a new record with the specified size
void allocateRecord(char* iden, int size) {
    // Find the index of the record variable
    int location = searchRecord(iden);
    // Allocate memory for the record with the specified size
    callStack[fp]->rValues[location] = calloc(size + 1, sizeof(int));
    // Store the size of the record in the first element
    callStack[fp]->rValues[location][0] = size;
    
    // Increment the garbage collection counter
    canReach++;
    printf("gc:%d\n", canReach);
}