#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* memcpy */

#include "sudoku.h"
#include "sudoku_solutions.h"
#include "dlx.h"

void initialize_sudoku(Sudoku *sudoku, const char *setup) {
	/* The fact that we're allocating space for many row controllers
	 * at the same time means we can't use the function `free_dlx` to free
	 * this structure. Such a shame... */
	Control *columns = malloc(324*sizeof(Control));
	Node *nodes = malloc(729*4*sizeof(Node));
	Control *master = malloc(sizeof(Control));
	int i, col, row, n;
	Node *rightmost = NULL, *current_node, *j;

	master->right = master;
	master->left = master;
	for(i = 0; i < 324; i++) {
		add_control(master, columns + i, i);
	}

	for(row = 0; row < 9; row++) {
		for(col = 0; col < 9; col++) {
			for(n = 1; n <= 9; n++) {
				rightmost = NULL;
				/* Ouf. Ok, don't despair. Here we consider the option of
				 * putting the number n in row number `row` and column `col`.
				 * We add the row for this option by adding the 4 constraint
				 * nodes */

				/* First that there can only be one number in this case */
				rightmost = add_node(nodes + node_for(row, col, n),
				                     columns + case_constraint(col, row), rightmost);

				/* Then that there can only be one n in this row */
				rightmost = add_node(nodes + node_for(row, col, n) + 1,
				                     columns + row_constraint(n, row), rightmost);

				/* Same but for columns */
				rightmost = add_node(nodes + node_for(row, col, n) + 2,
				                     columns + column_constraint(n, col), rightmost);

				/* And finally, that there can be only one n in this 3x3 square */
				add_node(nodes + node_for(row, col, n) + 3,
				         columns + square_constraint(n, row, col), rightmost);
			}
		}
	}

	/* Now we setup the initial conditions */
	sudoku->iteration = 0;
	for(i = 0; i < 81; i++) {
		if(setup[i] > '0' && setup[i] <= '9') {
			current_node = nodes + node_for(i/9, i%9, setup[i] - '0');
			cover_column(current_node->control);
			j = current_node->right;
			while(j != current_node) {
				cover_column(j->control);
				j = j->right;
			}
			sudoku->solutions[sudoku->iteration] = current_node;
			sudoku->iteration++;
		}
	}

	memcpy(sudoku->setup, setup, 81);
	sudoku->master = master;
	sudoku->columns = columns;
	sudoku->nodes = nodes;
}

void fill_sudoku(Sudoku *sudoku, char *to_fill) {
	Node *current_node;
	int i;

	sudoku->iteration = 0;
	for(i = 0; i < 81; i++) {
		if(to_fill[i] > '0' && to_fill[i] <= '9') {
			current_node = sudoku->nodes + node_for(i/9, i%9, to_fill[i] - '0');
			cover_row(current_node);
			sudoku->solutions[sudoku->iteration] = current_node;
			sudoku->iteration++;
		}
	}

	memcpy(sudoku->setup, to_fill, 81);
}

struct sudoku_solution * solve_sudoku(Sudoku *sudoku, int verbosity) {
	struct sudoku_solution *ret = NULL;
	char *c;
	if(verbosity <= 0) 
		solve_dlx(sudoku->master, sudoku->iteration, sudoku->solutions,
	      	      NULL, NULL,
	              print_solution_sudoku, NULL);
	if(verbosity == 1) 
		solve_dlx(sudoku->master, sudoku->iteration, sudoku->solutions,
	              print_column_choice,
	              print_row_choice,
	              print_solution_sudoku, NULL);
	if(verbosity >= 2) {
		ret = malloc(sizeof(struct sudoku_solution));
		memcpy(ret->puzzle, sudoku->setup, 81);
		ret->puzzle[81] = '\0';
		ret->already_filled = 0;
		for(c = ret->puzzle; *c != '\0'; c++) {
			if(*c > '0' && *c <= '9') (ret->already_filled)++;
		}
		ret->first_step = NULL;
		solve_dlx(sudoku->master, sudoku->iteration, sudoku->solutions,
	              record_column_choice,
	              record_row_choice,
	              record_solution_sudoku, (void*) ret);
	}
	return ret;
}


/* Unfill sudoku is different depending on whether solving short-circuited
 * or not (i.e. whether DLX_EXHAUSTIVE is set). If DLX_EHAUSTIVE is set, 
 * solving yields back the same sudoku we started with, but if we short-circuited
 * solving returns a solved sudoku, and we have to work backwards from the
 * solution */

#ifdef DLX_EXHAUSTIVE
void unfill_sudoku(Sudoku *sudoku) {
	Node *current_node;
	int i;

	for(i = 80; i >= 0; i--) {
		if(sudoku->setup[i] > '0' && sudoku->setup[i] <= '9') {
			current_node = sudoku->nodes + node_for(i/9, i%9, sudoku->setup[i] - '0');
			uncover_row(current_node);
		}
	}

	sudoku->iteration = 0;
	memcpy(sudoku->setup, ZERO_SUDOKU, 81);
}

#else
void unfill_sudoku(Sudoku *sudoku) {
	Node *current_node;
	int i;

	for(i = 80; i >= 0; i--) {
		current_node = sudoku->solutions[i];
		uncover_row(current_node);
	}

	sudoku->iteration = 0;
	memcpy(sudoku->setup, ZERO_SUDOKU, 81);
}

#endif

int case_constraint(int col, int row) {
	/* We will use the first 81 rows for this constraint, listing left to right,
	 * top to bottom */
	return (row*9) + col;
}

int row_constraint(int n, int row) {
	/* 81 rows have already been used so we add a padding of 81 to this number.
	 * We list the rows from top to bottom, each one having 9 constraints : one
	 * for each number that has to be in it */
	return 81 + (row*9) + (n-1);
}

int column_constraint(int n, int col) {
	/* analogous to the row constraint, but here we have to pad by 2*81 = 162 */
	return 162 + (col*9) + (n-1);
}

int square_constraint(int n, int row, int col) {
	/* Used to the dance by now? Anyway, this one requires relying on integer
	 * division to figure out which square we're talking about */
	return 243 + 9*(3*(row/3) + (col/3)) + (n-1);
}

int node_for(int row, int col, int n) {
	return 4*(row*81 + col*9 + (n-1));
}

void free_sudoku(Sudoku* sudoku) {
	free(sudoku->columns);
	free(sudoku->master);
	free(sudoku->nodes);
	free(sudoku);
}

void print_solution_sudoku(Node *acc[], int iteration, void * not_used) {
	char sudoku[81];
	Node *current;
	int i, pos;
	
	for(i = 0; i < 81; i++) {
		/* figure out where acc[i] is in the sudoku board */
		current = acc[i];
		while(current->control->name >= 81) {
			current = current->right;
		}
		pos = current->control->name;

		sudoku[pos] = (current->right->control->name % 9) + 1;
	}

	/* print the sudoku board (no bells and whistles yet, sorry) */
	for(i = 0; i < 81; i++) {
		if(i % 9 == 0) {
			printf("\n");
		}
		printf("%d ", sudoku[i]);
	}

	printf("\n =================== \n\n");
}

void record_solution_sudoku(Node *acc[], int iteration, void * sol) {
	char *sudoku = ((struct sudoku_solution*) sol)->solved;
	Node *current;
	int i, pos;
	
	for(i = 0; i < 81; i++) {
		/* figure out where acc[i] is in the sudoku board */
		current = acc[i];
		while(current->control->name >= 81) {
			current = current->right;
		}
		pos = current->control->name;

		sudoku[pos] = (current->right->control->name % 9) + 1 + '0';
	}
	sudoku[81] = '\0';
}

void print_column_choice(const Control *column, int iteration, void * not_used) {
	int label = column->name;
	int options = column->size;
	if(label < 81) {
		printf("%d\tThere must be a number in row %d, column %d ",
		       iteration, label/9 +1, (label%9) + 1);
	}
	else if(label < 162) {
		printf("%d\tThere must be a %d in row %d ",
		       iteration, ((label-81) % 9) + 1, ((label-81)/9) + 1);
	}
	else if(label < 243) {
		printf("%d\tThere must be a %d in column %d ",
		       iteration, ((label-162) % 9) + 1, ((label-162)/9) + 1);
	}
	else {
		printf("%d\tThere must be a %d in square number %d ",
		       iteration, ((label-243) % 9) + 1, ((label-243)/9) + 1);
	}

	if(options == 0)
		printf("(No candidates, we must backtrack)\n");
	else if(options == 1)
		printf("(Only 1 candidate, our choice is forced)\n");
	else
		printf("(%d possible choices, we might have to backtrack here)\n", options);
}

void record_column_choice(const Control *column, int iteration, void * sln) {
	int label = column->name;
	int options = column->size;
	struct sudoku_solution *solution = (struct sudoku_solution*) sln;
	struct solution_step *current_step = solution->first_step;
	struct solution_choice *current_choice = NULL;
	struct solution_step *new_step = malloc(sizeof(struct solution_step));

	iteration -= solution->already_filled;
	if(iteration == 0) {
		solution->first_step = new_step;
	}
	else {
		while(iteration > 0) {
			current_choice = current_step->first_choice;
			current_step = current_choice->continuation;
			iteration--;
		}
		current_choice->continuation = new_step;
	}
	
	if(label < 81) {
		new_step->constraint_type = CELL;
		new_step->constraint_parameters.cell.row = label/9 +1;
		new_step->constraint_parameters.cell.column = (label%9) + 1;
	}
	else if(label < 162) {
		new_step->constraint_type = ROW;
		new_step->constraint_parameters.row.row = ((label-81)/9) + 1;
		new_step->constraint_parameters.row.digit = ((label-81) % 9) + 1;
	}
	else if(label < 243) {
		new_step->constraint_type = COLUMN;
		new_step->constraint_parameters.column.digit = ((label-162) % 9) + 1;
		new_step->constraint_parameters.column.column = ((label-162)/9) + 1;
	}
	else {
		new_step->constraint_type = SQUARE;
		new_step->constraint_parameters.square.digit = ((label-243) % 9) + 1;
		new_step->constraint_parameters.square.square = ((label-243)/9) + 1;
	}

	new_step->possible_choices = options;
	new_step->first_choice = NULL;
}

void print_row_choice(const Node *row, int iteration, void * not_used) {
	const Node *current = row;
	int pos, n;
	while(current->control->name >= 81) {
		current = current->right;
	}
	pos = current->control->name;
	n = (current->right->control->name % 9) + 1;
	printf("%d\t\tWe put a %d in row %d, column %d\n",
	       iteration, n, pos/9 + 1, (pos%9) + 1);
}

void record_row_choice(const Node *row, int iteration, void * sln) {
	const Node *current = row;
	int pos, n;
	struct sudoku_solution *solution = (struct sudoku_solution*) sln;
	struct solution_step *current_step = solution->first_step;
	struct solution_choice *current_choice = NULL;
	struct solution_choice *new_choice = malloc(sizeof(struct solution_choice));

	iteration -= solution->already_filled;
	while(iteration > 0) {
		current_choice = current_step->first_choice;
		current_step = current_choice->continuation;
		iteration--;
	}

	new_choice->next_choice = current_step->first_choice;
	current_step->first_choice = new_choice;
	
	while(current->control->name >= 81) {
		current = current->right;
	}
	pos = current->control->name;
	n = (current->right->control->name % 9) + 1;

	new_choice->digit = n;
	new_choice->row = (pos/9) + 1;
	new_choice->column = (pos%9) + 1;
	new_choice->continuation = NULL;
}



