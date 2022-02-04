#ifndef SUDOKU_H
#define SUDOKU_H
#include "dlx.h"
#include "dlx_config.h"

#define ZERO_SUDOKU "000000000000000000000000000000000000000000000000000000000000000000000000000000000"

typedef struct {
	Control *master;
	Control *columns; /* 324 element array */
	Node *nodes; /* 2916 (729*4) element array */
	char setup[81];
	Node *solutions[81];
	int iteration;
} Sudoku;

void initialize_sudoku(Sudoku *sudoku, const char *setup);
void fill_sudoku(Sudoku *sudoku, char *to_fill);
struct sudoku_solution *solve_sudoku(Sudoku *, int);
void unfill_sudoku(Sudoku *sudoku);
int case_constraint(int col, int row);
int row_constraint(int n, int row);
int column_constraint(int n, int col);
int square_constraint(int n, int row, int col);
int node_for(int row, int col, int n);
void free_sudoku(Sudoku*);
void print_solution_sudoku(Node *acc[], int iteration, void *);
void record_solution_sudoku(Node *acc[], int iteration, void *);
void print_column_choice(const Control *column, int iteration, void *);
void print_row_choice(const Node *row, int iteration, void *);
void record_column_choice(const Control *column, int iteration, void *);
void record_row_choice(const Node *row, int iteration, void *);


#endif
