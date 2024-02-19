#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "tree.h"
#include "executor.h"
#include "scanner.h"
#include "memory.h"

/*
* Execute functions
*/

// Execute a procedure
void executeProcedure(struct nodeProcedure* p) {
	// Initialize memory for execution
	memory_init();
	
	// Execute function sequence if type is 1
	if (p->type == 1) {
		executeFuncSeq(p->fs);
	}
	
	// Execute declaration sequence
	executeDeclSeq(p->ds);
	
	// Execute statement sequence
	executeStmtSeq(p->ss);
	
	// Free the last frame
	freeUpNode();
}

// Execute function sequence
void executeFuncSeq(struct nodeFuncSeq* fs) {
	// Execute function
	executeFunction(fs->f);
	
	// If type is 1, continue executing function sequence
	if (fs->type == 1) {
		executeFuncSeq(fs->fs);
	}
}

// Execute a function
void executeFunction(struct nodeFunction* f) {
	// Add function to function table
	functionAdd(f->name, f);
}

// Execute declaration sequence
void executeDeclSeq(struct nodeDeclSeq* ds) {
	// Execute declaration
	executeDecl(ds->d);
	
	// If type is 1, continue executing declaration sequence
	if (ds->type == 1) {
		executeDeclSeq(ds->ds);
	}
}

// Execute a declaration
void executeDecl(struct nodeDecl* d) {
	// Execute integer declaration if type is 0, otherwise record declaration
	if (d->type == 0) {
		executeDeclInt(d->di);
	} else {
		executeDeclRec(d->dr);
	}
}

// Execute integer declaration
void executeDeclInt(struct nodeDeclInt* di) {
	// Declare integer variable
	declare(di->name, INTEGER);
}

// Execute record declaration
void executeDeclRec(struct nodeDeclRec* dr) {
    // Declare record variable
    declare(dr->name, RECORD);
}

// Execute statement sequence
void executeStmtSeq(struct nodeStmtSeq* ss) {
    // Execute statement
    executeStmt(ss->s);
	
	// If more statements, continue executing statement sequence
	if (ss->more == 1) {
		executeStmtSeq(ss->ss);
	}
}

// Execute a statement
void executeStmt(struct nodeStmt* s) {
	// Execute assignment if type is 0
	if (s->type == 0) {
		executeAssign(s->assign);
	} 
	// Execute if statement if type is 1
	else if (s->type == 1) {
		executeIf(s->ifStmt);
	} 
	// Execute loop if type is 2
	else if (s->type == 2) {
		executeLoop(s->loop);
	} 
	// Execute out statement if type is 3
	else if (s->type == 3) {
		executeOut(s->out);
	} 
	// Execute call statement otherwise
	else {
		executeCall(s->call);
	}
}

// Execute call statement
void executeCall(struct nodeCall* c) {
	// Push, execute, and pop the function call
	pushExecutePop(c->name, c->params);
}

// Execute assignment
void executeAssign(struct nodeAssign* a) {
	// Execute assignment based on type
	if (a->type == 0) {
		// Assign value to variable
		int rhs = executeExpr(a->expr);
		store(a->lhs, rhs);
	} else if (a->type == 1) {
		// Assign value to record field
		int index = executeIndex(a->index);
		int rhs = executeExpr(a->expr);
		storeRec(a->lhs, index, rhs);
	} else if (a->type == 2) {
		// Allocate memory for record
		int rhs = executeExpr(a->expr);
		allocateRecord(a->lhs, rhs);
	} else if (a->type == 3) {
		// Record operation
		record(a->lhs, a->rhs);
	}
}

// Execute index
int executeIndex(struct nodeIndex* index) {
	return executeExpr(index->expr);
}

// Execute out statement
void executeOut(struct nodeOut* out) {
	// Evaluate expression and print result
	int value = executeExpr(out->expr);
	printf("%d\n", value);
}

// Execute if statement
void executeIf(struct nodeIf* ifStmt) {
	// Execute statement sequence based on condition
	if (executeCond(ifStmt->cond)) {
		executeStmtSeq(ifStmt->ss1);
	} else if (ifStmt->type == 1) {
		executeStmtSeq(ifStmt->ss2);
	}
}

// Execute loop statement
void executeLoop(struct nodeLoop* loop) {
	// Execute loop while condition is true
	while (executeCond(loop->cond)) {
		// Execute statement sequence
		executeStmtSeq(loop->ss);
		
		// Handle cleanup after loop iteration
		canReach--;
		freeUpNode();
		printf("gc:0\n");
	}
}

// Execute condition
int executeCond(struct nodeCond* cond) {
	int result = 0;
	// Execute condition based on type
	if (cond->type == 0) {
		result = executeCmpr(cond->cmpr);
	} else if (cond->type == 1) {
		result = !executeCond(cond->cond);
	} else if (cond->type == 2) {
		result = executeCmpr(cond->cmpr) || executeCond(cond->cond);
	} else if (cond->type == 3) {
		result = executeCmpr(cond->cmpr) && executeCond(cond->cond);
	}
	return result;
}

// Execute comparison
int executeCmpr(struct nodeCmpr* cmpr) {
	int lhs = executeExpr(cmpr->expr1);
	int rhs = executeExpr(cmpr->expr2);
	return (cmpr->type == 0) ? (lhs == rhs) : (lhs < rhs);
}

// Execute expression
int executeExpr(struct nodeExpr* expr) {
	int value = executeTerm(expr->term);
	if (expr->type == 1) {
		value += executeExpr(expr->expr);
	} else if (expr->type == 2) {
		value -= executeExpr(expr->expr);
	}
	return value;
}

// Execute term
int executeTerm(struct nodeTerm* term) {
	int value = executeFactor(term->factor);
	if (term->type == 1) {
		value *= executeTerm(term->term);
	} else if (term->type == 2) {
		int denom = executeTerm(term->term);
		if (denom != 0) {
			value /= denom;
		} else {
			printf("Runtime Error: Division by zero!\n");
			exit(0);
		}
	}
	return value;
}

// Execute factor
int executeFactor(struct nodeFactor* factor) {
	int value = 0;
	if (factor->type == 0) {
		value = recall(factor->id);
	} else if (factor->type == 1) {
		int index = executeExpr(factor->expr);
		value = recallRec(factor->id, index);
	} else if (factor->type == 2) {
		value = factor->constant;
	} else if (factor->type == 3) {
		value = executeExpr(factor->expr);
	} else {
		if (currentToken() == EOS) {
			printf("Runtime Error: Data file used up!");
			exit(0);
		}
		value = getConst();
		nextToken();
	}
	return value;
}
