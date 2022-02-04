#include <stdlib.h>
#include <stdio.h>
#include "sudoku_solutions.h"

void free_sudoku_solution(struct sudoku_solution* solution) {
	if(solution->first_step != NULL)
		free_solution_step(solution->first_step);
	free(solution);
}

void free_solution_step(struct solution_step* step) {
	if(step->first_choice != NULL)
		free_solution_choice(step->first_choice);
	free(step);
}

void free_solution_choice(struct solution_choice* choice) {
	if(choice->continuation != NULL) 
		free(choice->continuation);
	if(choice->next_choice != NULL)
		free(choice->next_choice);
	free(choice);
}

void print_solution_json(struct sudoku_solution *solution) {
	printf("{ \"puzzle\" : \"%s\",\n", solution->puzzle);
	printf("{ \"solution\" : \"%s\",\n", solution->solved);
	printf("  \"steps\" : ");
	print_step_json(solution->first_step);
	printf("}\n");
}

void print_step_json(struct solution_step *step) {
	struct solution_step *current_step = step;
	struct solution_choice *current_choice = NULL;
	printf("[");
	while(current_step != NULL
	      && current_step->first_choice != NULL
	      && current_step->first_choice->next_choice == NULL) {
		print_constraint_header(current_step);
		printf("\"available_choices\": %d, \n", step->possible_choices);
		printf("\"choice\": ");
		current_choice = current_step->first_choice;
		printf("{\"digit\": %d, \"row\": %d, \"column\": %d} \n",
		       current_choice->digit, current_choice->row, current_choice->column);
		printf("},\n");
		if(current_choice->continuation == NULL) {
			printf("\"FILLED\"");
			current_step = NULL;
		} else {
		current_step = current_choice->continuation;
		}
	}
	if(current_step == NULL) {
		/* We need none of the below */
	}
	else if(current_step->first_choice == NULL) {
		print_constraint_header(current_step);
		printf("\"available_choices\": 0, \n");
		printf("\"choice\": \"BACKTRACK\"");
		printf("}\n");
	} else { /* more than one branch */
		print_constraint_header(current_step);
		printf("\"available_choices\": %d, \n", step->possible_choices);
		printf("\"branches\": [ ");
		current_choice = current_step->first_choice;
		while(current_choice != NULL) {
			printf("{\"branch_choice\": ");
			printf("{\"digit\": %d, \"row\": %d, \"column\": %d}, \n",
			       current_choice->digit, current_choice->row, current_choice->column);
			printf("\"further_steps\": ");
			print_step_json(current_choice->continuation);
			printf("}");
			if(current_choice->next_choice != NULL)
				printf(",\n");
			current_choice = current_choice->next_choice;
		}
		printf("]");
		printf("}\n"); /* close constraint (opened in "print_constraint_header") */
	}
	printf("]\n");
}

void print_constraint_header(struct solution_step *step) {
	switch(step->constraint_type) {
		case CELL:
			printf("{\"constraint\": \"CELL\", \"row\": %d, \"column\": %d, \n",
				   step->constraint_parameters.cell.row,
				   step->constraint_parameters.cell.column);
			break;
		case ROW:
			printf("{\"constraint\": \"ROW\", \"row\": %d, \"digit\": %d, \n",
			       step->constraint_parameters.row.row,
			       step->constraint_parameters.row.digit);
			break;
		case COLUMN:
			printf("{\"constraint\": \"COLUMN\", \"column\": %d, \"digit\": %d, \n",
			       step->constraint_parameters.column.column,
			       step->constraint_parameters.column.digit);
			break;
		case SQUARE:
			printf("{\"constraint\": \"SQUARE\", \"square\": %d, \"digit\": %d, \n",
			       step->constraint_parameters.square.square,
			       step->constraint_parameters.square.digit);
			break;
	}
}

void print_choices_json(struct solution_choice *choice) {
	struct solution_choice *current_choice = choice;
	printf("[ ");
	while(current_choice != NULL) {
		if(current_choice != choice) 
			printf(",\n");
		printf("{ \"digit\": %d, \"row\": %d, \"column\": %d",
		       current_choice->digit, current_choice->row, current_choice->column);
		if(current_choice->continuation != NULL) {
			printf(", \n \"continuation\": ");
			print_step_json(current_choice->continuation);
		}
		printf("}");
		current_choice = current_choice->next_choice;
	}
	printf("\n] \n");
}
	









	