#ifndef DLX_H
#define DLX_H

typedef struct Node {
	struct Node *left, *right, *up, *down;
	struct Control *control;
} Node;

typedef struct Control {
	int size;
	int name;
	struct Control *left, *right;
	Node node;
} Control;

Control *from_matrix(const int*, int, int, const int*);
void add_control(Control* master, Control *, const int label);
Node *add_node(Node *new_node, Control* control, Node *rightmost);
void free_dlx(Control * master);
int solve_dlx(Control *master, int iteration, Node *acc[],
               void (*column_chosen_callback)(const Control *, int, void *),
               void (*row_chosen_callback)(const Node *, int, void *),
               void (*solution_callback)(Node * [], int, void *),
               void *callback_data);
void print_solution(Node *acc[], int iteration);
Control *choose_column(Control *master);
void cover_row(Node *row);
void uncover_row(Node *row);
void cover_column(Control*);
void uncover_column(Control*);


#endif