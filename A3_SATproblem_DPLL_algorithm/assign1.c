////////////////////////////////////////////////////////////////////////
// Solution to assignment #1 for ECE1733.
// This program implements the Quine-McCluskey method for 2-level
// minimization. 
////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/*** HEADER FILES *****************************************************/
/**********************************************************************/

#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common_types.h"
#include "blif_common.h"
#include "cubical_function_representation.h"

/**********************************************************************/
/*** DATA STRUCTURES DECLARATIONS *************************************/
/**********************************************************************/

/**********************************************************************/
/*** DEFINE STATEMENTS ************************************************/
/**********************************************************************/

/**********************************************************************/
/*** GLOBAL VARIABLES *************************************************/
/**********************************************************************/

/**********************************************************************/
/*** FUNCTION DECLARATIONS ********************************************/
/**********************************************************************/


int cube_cost(t_blif_cube *cube, int num_inputs);
int function_cost(t_blif_cubical_function *f);
int cover_cost(t_blif_cube **cover, int num_cubes, int num_inputs);

//void simplify_function(t_blif_cubical_function *f);
void SAT(t_blif_logic_circuit *circuit);


/**********************************************************************/
/*** BODY *************************************************************/
/**********************************************************************/


/**********************************************************************/
/*** COST FUNCTIONS ***************************************************/
/**********************************************************************/


int cube_cost(t_blif_cube *cube, int num_inputs)
/* Wires and inverters are free, everything else is #inputs+1*/
{
	int index;
	int cost = 0;

	for (index = 0; index < num_inputs; index++)
	{
		if (read_cube_variable(cube->signal_status, index) != LITERAL_DC)
		{
			cost++;
		}
	}
	if (cost > 1)
	{
		cost++;
	}
	return cost;
}


int function_cost(t_blif_cubical_function *f)
{
	int cost = 0;
	int index;
	
	if (f->cube_count > 0)
	{
		for(index = 0; index < f->cube_count; index++)
		{
			cost += cube_cost(f->set_of_cubes[index], f->input_count);
		}
		if (f->cube_count > 1)
		{
			cost += (f->cube_count+1);
		}
	}

	return cost;
}


int cover_cost(t_blif_cube **cover, int num_cubes, int num_inputs)
{
	int result = 0;
	int index;

	for (index = 0; index < num_cubes; index++)
	{
		result += cube_cost(cover[index], num_inputs);
	}
	if (num_cubes > 1)
	{
		result += num_cubes+1;
	}
	return result;
}

/**********************************************************************/
/*** SAT CODE *********************************************************/
/**********************************************************************/

void SAT(t_blif_logic_circuit *cir)
{
	int i, j, ii, jj, k, n;
	int input_n, function_n, n_remove, new_exp, pass, infered, count, note, new_function, it, error;
	int cubset[50] = {0};
	int remove[150];
	int con[20][20] = {{0}};
	int var[20], function[50];

	t_blif_cube *expand[75];
	
	new_exp = 0;
	pass = 0;
	for (i = 0; i < 60; i++)	remove[i] = -1;
	for (i = 0; i < 20; i++)	var[i] = -1;
	for (i = 0; i < 20; i++)	function[i] = -1;
	input_n = cir->list_of_functions[0]->input_count;
	function_n = cir->function_count;
	for (i = 0; i < function_n; i++) function[i] = 0;

	//************************************Count #of cubesets for each function**************************//
	for (i = 0; i < function_n; i++)
	{
		cubset[i] = cir->list_of_functions[i]->cube_count;
	}
	//************************************Initialize "remove" array*************************************//
	n_remove = 0;
	for (i = 0; i < function_n; i++)
	{
		for (j = 0; j < cubset[i]; j++)
		{
			remove[n_remove] = 0;
			n_remove++;
		}
	}
//	printf("n_remove %d\n",n_remove);
//	for (i = 0; i < function_n; i++)	printf("cubset  %d\n", cubset[i]);
//	for (i = 0; remove[i] >= 0; i++)	printf("remove  %d\n", remove[i]);
	//************************************Solve SAT problem********************************************//
	//Can we infer sth?
//	it = 12;
	error = 0;
	new_function = 0;
	while(1)
	{
//		it--;
		infered = 0;
		for (i = 0; i < function_n; i++)
		{
			n = 0;
			for (jj = 0; jj < function_n; jj++)
			{
				for (k = 0; k < cubset[jj]; k++)
				{
					if (jj == i)
						break;
					n++;
				}
				if (jj == i)
					break;
			}
			//printf("n  %d\n",n);
			count = 0;
			for (jj = 0; jj < cubset[i]; jj++)
				if (remove[n+jj] == 0)
					count++;
			if (count == 0)
			{
				error = 1;
				break;
			}
			//printf("count  %d\n",count);
			if (count == 1 && function[i] == 0)
			{
				for (ii = 0; ii < cubset[i]; ii++)
				{
					if (remove[n+ii] == 0)
					{
						if (i < function_n - new_function)
						{
							for (j = 0; j < input_n; j++)
							{
								if (read_cube_variable (cir->list_of_functions[i]->set_of_cubes[ii]->signal_status, j) == 3)
									continue;
								//Which vars infered that?
								for (jj = 0; jj < cubset[i]; jj++)
								{
									if (remove[n+jj] == 1)
									{
										for (k = 0; k < input_n; k++)
										{
											if (read_cube_variable (cir->list_of_functions[i]->set_of_cubes[jj]->signal_status, k) == 3)
												continue;
											con[j][k] = 1;
										}
									}
								}
								if (var[j] < 0 || (var[j] == read_cube_variable (cir->list_of_functions[i]->set_of_cubes[ii]->signal_status, j) - 1))
								{
									var[j] = read_cube_variable (cir->list_of_functions[i]->set_of_cubes[ii]->signal_status, j) - 1;
									infered = 1;
								}
								else		//conflict happens!
								{
									function_n++;
									new_function++;
									for (jj = 0; jj < input_n; jj++)	//find out which vars resulted that conflict
									{
										if (con[j][jj] == 1)			//expand functions and cubset array
										{
											expand[new_exp] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
											for (k = 0; k < input_n; k++)
												write_cube_variable(expand[new_exp]->signal_status, k, LITERAL_DC);
											if (var[jj] == 0)
												write_cube_variable(expand[new_exp]->signal_status, jj, LITERAL_1);
											else if(var[jj] == 1)
												write_cube_variable(expand[new_exp]->signal_status, jj, LITERAL_0);
											new_exp++;
											cubset[function_n-1]++;
										}
									}
									//for (k = 0; k < input_n; k++)	printf("exp %d  ",read_cube_variable(expand[1]->signal_status,k));
									for (jj = 0; jj < cubset[function_n-1]; jj++)	//expand "remove" array
									{
										remove[n_remove] = 0;
										n_remove++;
									}
									for (jj = 0; jj < 20; jj++)	var[jj] = -1;		//initialize var array, function array, remove array
									for (jj = 0; jj < n_remove; jj++)	remove[jj] = 0;
									for (jj = 0; jj < function_n; jj++)	function[jj] = 0;
									for (jj = 0; jj < input_n; jj++)				//finish inference => initialize con array
										for (k = 0; k < input_n; k++)
											con[jj][k] = 0;
									pass = 1;
									infered = 0;
									break;
								}
							}
						}
						else
						{
							note = 0;
							for (j = function_n - new_function; j < function_n; j++)
							{
								for (jj = 0; jj < cubset[j]; jj++)
								{
									if (j == i)	break;
									note++;
								}
								if (j == i)	break;
							}
							for (j = 0; j < input_n; j++)
							{
								if (read_cube_variable (expand[note+ii]->signal_status, j) == 3)
									continue;
								//Which vars infered that?
								for (jj = 0; jj < cubset[i]; jj++)
								{
									if (remove[n+jj] == 1)
									{
										for (k = 0; k < input_n; k++)
										{
											if (read_cube_variable (expand[note+jj]->signal_status, k) == 3)
												continue;
											con[j][k] = 1;
										}
									}
								}
								if (var[j] < 0 || (var[j] == read_cube_variable (expand[note+ii]->signal_status, j) - 1))
								{
									var[j] = read_cube_variable (expand[note+ii]->signal_status, j) - 1;
									infered = 1;
								}
								else		//conflict happens!
								{
									function_n++;
									new_function++;
									for (jj = 0; jj < input_n; jj++)	//find out which vars resulted that conflict
									{
										if (con[j][jj] == 1)			//expand functions and cubset array
										{
											expand[new_exp] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
											for (k = 0; k < input_n; k++)
												write_cube_variable(expand[new_exp]->signal_status, k, LITERAL_DC);
											if (var[jj] == 0)
												write_cube_variable(expand[new_exp]->signal_status, jj, LITERAL_1);
											else if(var[jj] == 1)
												write_cube_variable(expand[new_exp]->signal_status, jj, LITERAL_0);
											new_exp++;
											cubset[function_n-1]++;
										}
									}
									//for (k = 0; k < input_n; k++)	printf("exp %d  ",read_cube_variable(expand[1]->signal_status,k));
									for (jj = 0; jj < cubset[function_n-1]; jj++)	//expand "remove" array
									{
										remove[n_remove] = 0;
										n_remove++;
									}
									for (jj = 0; jj < 20; jj++)	var[jj] = -1;		//initialize var array, function array, remove array
									for (jj = 0; jj < n_remove; jj++)	remove[jj] = 0;
									for (jj = 0; jj < function_n; jj++)	function[jj] = 0;
									for (jj = 0; jj < input_n; jj++)				//finish inference => initialize con array
										for (k = 0; k < input_n; k++)
											con[jj][k] = 0;
									pass = 1;
									infered = 0;
									break;
								}
							}
						}
					}
					if (pass == 1)
						break;
				}
			}
			if (pass == 1)
			{
				i = -1;
				pass = 0;
			}
		}
		if (error == 1)
			break;
		for (jj = 0; jj < input_n; jj++)		//finish inference => initialize con array
			for (k = 0; k < input_n; k++)
				con[jj][k] = 0;
		//If we can't infer anything, initialize first unknown var to 0
		//printf("infered  %d\n",infered);
		if (infered == 0)
		{
			for (i = 0; i < input_n; i++)
			{
				if (var[i] < 0)
				{
					var[i] = 0;
					break;
				}
			}
		}
		//for (i = 0; i < input_n; i++)	printf("%d  ", var[i]);
		//printf("\n");
		//See which function is covered. If some functions are not covered, see if we can remove some sets of cubes.
		for (i = 0; i < input_n; i++)
		{
			if (var[i] >= 0)
			{
				for (j = 0; j < function_n - new_function; j++)
				{
					if(function[j] == 1)
						continue;
					for (ii = 0; ii < cubset[j]; ii++)
					{
						if (read_cube_variable(cir->list_of_functions[j]->set_of_cubes[ii]->signal_status, i)-1 == var[i]) //function being covered
							function[j] = 1;
						else if (read_cube_variable(cir->list_of_functions[j]->set_of_cubes[ii]->signal_status, i) != 3) //remove sth
						{
							n = 0;
							for (jj = 0; jj < function_n - new_function; jj++)
							{
								for (k = 0; k < cubset[jj]; k++)
								{
									if (jj == j)
										break;
									n++;
								}
								if (jj == j)
									break;
							}
							remove[n+ii] = 1;
						}
					}
				}
				//printf("done!\n");
				//printf("fun_n %d\n",function_n);
				//printf("f_n - new  %d\n",function_n - new_function);
				for (j = function_n - new_function; j < function_n; j++)
				{
					if (function[j] == 1)
						continue;
					n = 0;
					for (jj = function_n - new_function; jj < function_n; jj++)
					{
						for (k = 0; k < cubset[jj]; k++)
						{
							if (jj == j) break;
							n++;
						}
						if (jj == j) break;
					}
					//printf("nn %d  cubst[j] %d\n",n, cubset[j]);
					for (ii = 0; ii < cubset[j]; ii++)
					{
						//printf("ii %d\n",ii);
						if (read_cube_variable(expand[n + ii]->signal_status, i)-1 == var[i])	//function being covered
						{
							function[j] = 1;
						}
						else if (read_cube_variable(expand[n + ii]->signal_status, i) != 3)		//remove sth
						{
							note = 0;
							for (jj = 0; jj < function_n; jj++)
							{
								for (k = 0; k < cubset[jj]; k++)
								{
									if (jj == j)
										break;
									note++;
								}
								if (jj == j)
									break;
							}
							remove[note+ii] = 1;
						}
						//printf("check\n");
					}
				}
			}
		}
		//for (i = 0; i < function_n; i++)	printf("f %d  ", function[i]);
		//printf("\n");
		//for (i = 0; i < function_n; i++)	printf("c %d  ", cubset[i]);
		//printf("\n");
		//for (i = 0; i < n_remove; i++)	printf("%d  ", remove[i]);
		//printf("\n");
		note = 0;
		for (i = 0; i < function_n; i++)		//if all functions being covered, exit
		{
			if (function[i] == 1)
				note++;
		}
		if (note == function_n)
			break;
	}

//	for (i = 0; i < function_n; i++)	printf("%d  ", function[i]);
//	printf("\n");
//	for (i = 0; i < function_n; i++)	printf("%d  ", cubset[i]);
//	printf("\n");
//	for (i = 0; i < n_remove; i++)	printf("%d  ", remove[i]);
//	printf("\n");
	if (error == 0)
	{
		printf("Final SAT Result:\n");
		for (i = 0; i < input_n; i++)	
		{
			if (var[i] == -1)
			{
				printf("%d  ",0);
				continue;
			}
			printf("%d  ", var[i]);
		}
		printf("\n");
		printf("Problem is SAT!\n");
		printf("\n\n");
	}
	else
	{
		printf("\nProblem is UNSAT!\n\n");
	}
	printf("END\n");
}


/**********************************************************************/
/*** MINIMIZATION CODE ************************************************/
/**********************************************************************/


//void simplify_function(t_blif_cubical_function *f)
/* This function simplifies the function f. The minimized set of cubes is
 * returned though a field in the input structure called set_of_cubes.
 * The number of cubes is stored in the field cube_count.
 */
//{
	/* PUT YOUR CODE HERE */
//}


/**********************************************************************/
/*** MAIN FUNCTION ****************************************************/
/**********************************************************************/


int main(int argc, char* argv[])
{
	t_blif_logic_circuit *circuit = NULL;

	if (argc != 2)
	{
		printf("Usage: %s <source BLIF file>\r\n", argv[0]);
		return 0;
	}
	printf("SAT problem program.\r\n");

	/* Read BLIF circuit. */
	printf("Reading file %s...\n",argv[1]);
	circuit = ReadBLIFCircuit(argv[1]);

	if (circuit != NULL)
	{
		int index;

		/* Minimize each function, one at a time. */
/*		printf("Minimizing logic functions\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			simplify_function(function);
		}*/

		/* SAT function */
		SAT (circuit);

		/* Print out synthesis report. */
/*		printf("Report:\r\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			/* Print function information. */
//			printf("Function %i: #inputs = %i; #cubes = %i; cost = %i\n", index+1, function->input_count, function->cube_count, function_cost(function)); 
//		}

		/* Finish. */
		printf("Done.\r\n");
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
	return 0;
}

