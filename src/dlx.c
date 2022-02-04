#include "dlx.h"
#include "dlx_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* Construct the dancing floor from a m by n matrix of 1's or 0's 
   The matrix is a simple array, listing the entries from left to right
   top to bottom. The labels array is an array of length n to label the
   columns of the matrix */
Control *from_matrix(const int *matrix, int m, int n, const int *labels) {
	int i, j;
	Control *master = malloc(sizeof(Control));
	Control *current_control = NULL;
	Node *rightmost = NULL, *new_node = NULL;

	master->right = master;
	master->left = master;

	/* Make the column headers */
	for(i = 0; i < n; i++) {
		Control * current_control = malloc(sizeof(Control));
		add_control(master, current_control, labels[i]);
	}

	/* add the rows */
	for(i = 0; i < m; i++) {
		current_control = master;
		rightmost = NULL;
		for(j = 0; j < n; j++) {
			current_control = current_control->right;
			if(matrix[i*n + j]) {
				new_node = malloc(sizeof(Node));
				rightmost = add_node(new_node, current_control, rightmost);
			}
		}
	}

	return master;
}

void add_control(Control *master, Control *new_control, int label) {
	new_control->left = master->left;
	master->left = new_control;
	new_control->left->right = new_control;
	new_control->right = master;

	new_control->node.up = &(new_control->node);
	new_control->node.down = &(new_control->node);
	new_control->size = 0;
	new_control->name = label;
}

Node *add_node(Node *new_node, Control* control, Node *rightmost) {
	if(rightmost == NULL) {
		/* link to the sides */
		new_node->right = new_node;
		new_node->left = new_node;
	}
	else {
		/* link to the sides */
		new_node->right = rightmost->right;
		new_node->left = rightmost;
		new_node->right->left = new_node;
		rightmost->right = new_node;
	}
				
	/* link up and down */
	control->node.up->down = new_node;
	new_node->up = control->node.up;
	control->node.up = new_node;
	new_node->down = &(control->node);

	new_node->control = control;
	control->size = control->size + 1;
	
	return new_node;
}

void free_dlx(Control * master) {
	Control *current_row = master->right;
	Control *next_row;
	Node *current_node, *next_node;

	while(current_row != master) {
		current_node = current_row->node.down;
		while(current_node != &(current_row->node)) {
			next_node = current_node->down;
			free(current_node);
			current_node = next_node;
		}
		next_row = current_row->right;
		free(current_row);
		current_row = next_row;
	}
	free(master);
}

/* It is the caller's responsability to make sure the memory for
 * acc is allocated and there's enough place for it to hold the full
 * solution. (it should have size n, if the max number of recursion
 * levels is n) */
int solve_dlx(Control *master, int iteration, Node *acc[],
               void (*column_chosen_callback)(const Control *, int, void *),
               void (*row_chosen_callback)(const Node *, int, void *),
               void (*solution_callback)(Node * [], int, void *), 
               void * callback_data) {
	Control *column = NULL;
	Node *row = NULL;
	Node *j = NULL;
	
	if(master->right == master) {
		if(solution_callback != NULL)
			solution_callback(acc, iteration, callback_data);
		return 1;
	}

	column = choose_column(master);
	if(column_chosen_callback != NULL)
		column_chosen_callback(column, iteration, callback_data);
	cover_column(column);
	row = column->node.down;
	while(row != &(column->node)) {
		if(row_chosen_callback != NULL)
			row_chosen_callback(row, iteration, callback_data);
		acc[iteration] = row;
		j = row->right;
		while(j != row) {
			cover_column(j->control);
			j = j->right;
		}

#ifdef DLX_EXHAUSTIVE
		solve_dlx(master, iteration + 1, acc, 
		          column_chosen_callback,
		          row_chosen_callback,
		          solution_callback, callback_data);
#else
		if(solve_dlx(master, iteration + 1, acc, 
		             column_chosen_callback,
		             row_chosen_callback,
		             solution_callback, callback_data))
			return 1;
#endif

		/* Now that the recursive search has returned, 
		   put things back to normal */
		j = row->left;
		while(j != row) {
			uncover_column(j->control);
			j = j->left;
		}

		row = row->down;
	}

	uncover_column(column);
	return 0;
}

void print_solution(Node *acc[], int iteration) {
	int i;
	Node *row, *j;
	
	for(i = 0; i < iteration; i++) {
		row = acc[i];
		j = row;
		do {
			printf("%d ", j->control->name);
			j = j->right;
		} while(j != row);
		printf("\n");
	}
	printf("================= \n\n");
}

Control *choose_column(Control *master) {
	Control *ret = master->right;
	Control *j = ret;
	int s = INT_MAX;

	while(j != master) {
		if(j->size < s){
			ret = j;
			s = j->size;
		}
		j = j->right;
	}
	return ret;
}

void cover_row(Node *row) {
	Node *j;
	
	cover_column(row->control);
	j = row->right;
	while(j != row) {
		cover_column(j->control);
		j = j->right;
	}
}

void uncover_row(Node *row) {
	Node *j;

	j = row->left;
	while(j != row) {
		uncover_column(j->control);
		j = j->left;
	}
	uncover_column(row->control);
}

void cover_column(Control* column) {
	Node *i = NULL, *j = NULL;
	
	/* Let's dance */
	column->left->right = column->right;
	column->right->left = column->left;

	i = column->node.down;
	while(i != &(column->node)) {
		j = i->right;
		while(j != i) {
			j->up->down = j->down;
			j->down->up = j->up;
			j->control->size -= 1;
			j = j->right;
		}
		i = i->down;
	}
	
}

void uncover_column(Control* column) {
	Node *i = NULL, *j = NULL;
	i = column->node.up;
	while(i != &(column->node)) {
		j = i->left;
		while(j != i) {
			j->control->size += 1;
			j->up->down = j;
			j->down->up = j;
			j = j->left;
		}
		i = i->up;
	}
	column->left->right = column;
	column->right->left = column;
}



				