#ifndef SUDOKU_SOLUTIONS_H
#define SUDOKU_SOLUTIONS_H

struct solution_choice {
	int digit;
	int row;
	int column;
	struct solution_choice *next_choice;
	struct solution_step *continuation;
};

struct sudoku_solution{
	char puzzle[82];
	char solved[82];
	int already_filled;
	struct solution_step *first_step;

};

struct solution_step {
	enum {
		CELL,
		SQUARE,
		ROW,
		COLUMN
	} constraint_type;
	
	union {
		struct {
			int row;
			int column;
		} cell;
		struct {
			int row;
			int digit;
		} row;
		struct {
			int digit;
			int column;
		} column;
		struct {
			int digit;
			int square;
		} square;
	} constraint_parameters;

	int possible_choices;
	struct solution_choice *first_choice;
};

void free_sudoku_solution(struct sudoku_solution* solution);
void free_solution_step(struct solution_step* step);
void free_solution_choice(struct solution_choice* choice);
void print_solution_json(struct sudoku_solution *solution);
void print_step_json(struct solution_step *step);
void print_choices_json(struct solution_choice *choice);
void print_constraint_header(struct solution_step *step);

#endif

