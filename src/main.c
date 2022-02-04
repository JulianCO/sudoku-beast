#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlx_config.h"
#include "dlx.h"
#include "sudoku.h"
#include "sudoku_solutions.h"

int main()
{
	char input[82];
	Sudoku *dance_floor = malloc(sizeof(Sudoku));
	int read_fail;
	struct sudoku_solution *solution;

	initialize_sudoku(dance_floor, ZERO_SUDOKU);
	while(!feof(stdin)) {
		read_fail = fscanf(stdin, "%s", input);
		if(read_fail == EOF)
			break;
		fill_sudoku(dance_floor, input);
		solution = solve_sudoku(dance_floor, 2);
		print_solution_json(solution);
		unfill_sudoku(dance_floor);
	}

	free_sudoku_solution(solution);
	free_sudoku(dance_floor);
	
	return (0);
}


/*
004500000062400000109060000005340100700000004003096200000070302000003640000008500
 */
