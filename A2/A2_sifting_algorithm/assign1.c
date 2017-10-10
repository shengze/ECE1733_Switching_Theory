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
#include <time.h>
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

void simplify_function(t_blif_cubical_function *f);
void ROBDD_package(t_blif_cubical_function *f);

void shifting_alg(t_blif_cubical_function *f);
int GetNumberofNodes(t_blif_cubical_function *f);

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
/*** MINIMIZATION CODE ************************************************/
/**********************************************************************/


void simplify_function(t_blif_cubical_function *f)
/* This function simplifies the function f. The minimized set of cubes is
 * returned though a field in the input structure called set_of_cubes.
 * The number of cubes is stored in the field cube_count.
 */
{
	/* PUT YOUR CODE HERE */
	//Show the list of input circuits.
	int testi, testj;

	int i, j, n, note, k;
	//========================================Expand initial circuit to 0s and 1s=========================================//
	int epn, epnforever, newmem, PIcount;

	int coverTc[50][50] = {{0}};
	int coverN[50] = {0};
	int coverNote[50] = {0};
	int covBr[50] = {0};
	int covBc[50] = {0};

	int i1, i2, nrc, PIn, finalCount;

	int custate[50] = {0};

	t_blif_cube **expansion;
	t_blif_cube *expaid[100];
	t_blif_cube *norepeat[100];
	t_blif_cube *PI[100];
	t_blif_cube *PIno[100];
	t_blif_cube *finalPI[50];

	expansion = (t_blif_cube **) malloc (f->cube_count * f->cube_count * sizeof(t_blif_cube *));
	n = 1;
	epnforever = 0;
	while (n)
	{
		n = 0;
		epn = 0;
		for(i = 0; i < f->cube_count; i++)
		{
			if (f->set_of_cubes[i]->is_DC) continue;
			expaid[epnforever] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
			memcpy(expaid[epnforever], f->set_of_cubes[i], sizeof(t_blif_cube));
			expansion[epn] = expaid[epnforever];
			epnforever++;
			for(j = 0; j < f->input_count; j++)
			{
				if (read_cube_variable(f->set_of_cubes[i]->signal_status, j) == 3)
				{
					n++;
					write_cube_variable(expansion[epn]->signal_status, j, LITERAL_0);
					epn++;

					expaid[epnforever] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
					memcpy(expaid[epnforever], f->set_of_cubes[i], sizeof(t_blif_cube));
					expansion[epn] = expaid[epnforever];
					epnforever++;

					write_cube_variable(expansion[epn]->signal_status, j, LITERAL_1);
					break;
				}
			}
			epn++;
		}
		for (j = 0; j < epn; j++) f->set_of_cubes[j] = expaid[epnforever - epn + j];
		f->cube_count = epn;
	}
	for (j = 0; j < epnforever - epn; j++) free(expaid[j]);

	//========================================Remove All the Repeated Items from 0/1 List=========================================//
	nrc = 0;
	norepeat[nrc] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
	memcpy(norepeat[nrc], f->set_of_cubes[0], sizeof(t_blif_cube));
	nrc++;
	for(i = 1; i < f->cube_count; i++)
	{
		n = 0;
		for (j = 0; j < i; j++)
		{
			for (i1 = 0; i1 < f->input_count; i1++)
			{
				if (read_cube_variable(f->set_of_cubes[i]->signal_status,i1) != read_cube_variable(f->set_of_cubes[j]->signal_status,i1))
				{
					n++;
					break;
				}
			}
		}
		if (n == i)
		{
			norepeat[nrc] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
			memcpy(norepeat[nrc], f->set_of_cubes[i], sizeof(t_blif_cube));
			nrc++;
		}
	}
	for (j = 0; j < nrc; j++) f->set_of_cubes[j] = norepeat[j];
	f->cube_count = nrc;
	for (j = epnforever - epn; j < epnforever; j++) free(expaid[j]);


	//====================================
	/*
	printf("The final simplified result is:\n\n");

	printf("Final Prime Implicants \t\t Outputs\n");

	for (testi = 0; testi < f->cube_count; testi++)
	{
		for (testj = 0; testj < f->input_count; testj++)
		{
			if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 2)	printf("%d", 1);
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 3)	printf("%c", '-');
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 1)	printf("%d", 0);
			printf("  ");
		}
		printf("\t\t\t ");
		if (!f->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n");*/
}


/**********************************************************************/
/*** BUILD ROBDD PACKAGE **********************************************/
/**********************************************************************/

void ROBDD_package(t_blif_cubical_function *f)
{
	int table[64][4] = {{0}};
	int level_count[6] = {0};
	int hnote[64][2] = {{0}};

	int i, j, k, n, ii, jj, ord, nc, ind, note, p, q;
	int input_n, order_n, tc, fk;
	int node1, node2, level1, level2;
	int testi, testj;
	
	input_n = f->input_count;

	//========================================Build Initial BDD graph=========================================//
	n = 0;
	order_n = 1;
	for (i = 0; i < f->cube_count; i++)
	{
		nc = 0;
		for (j = 0; j < input_n; j++)
		{
			k = read_cube_variable(f->set_of_cubes[i]->signal_status, j) - 1;
			if (nc > 0)
			{
				if (table[nc][k] != 0)
				{
					order_n = (table[nc][k] - (j+2))/(input_n - 1) + 1;
					ord = 0;
					for (jj = 0; jj < 64; jj ++)
					{
						if (table[jj][2] == j+2)
						{
							ord++;
							if (ord == order_n)
							{
								nc = jj;
								break;
							}
						}
						if (table[jj][2] == 0)
						{
							//nc = jj;
							break;
						}
					}
					continue;
				}
			}
			
			if (j == 0 && table[0][2] != 0)
			{
				if (table[0][k] != 0)
				{
					order_n = (table[0][k] - (j+2))/(input_n - 1) + 1;
					ord = 0;
					for (jj = 0; jj < 64; jj ++)
					{
						if (table[jj][2] == j+2)
						{
							ord++;
							if (ord == order_n)
							{
								nc = jj;
								break;
							}
						}
						if (table[jj][2] == 0)
						{
							//nc = jj;
							break;
						}
					}
					continue;
				}
				else
				{
					order_n = 1;
					for (ii = 0; ii < 64; ii++)
					{
						if (table[ii][2] == j+2)	order_n++;
						if (table[ii][2] == 0)	break;
					}
					table[0][k] = (input_n - 1)*(order_n - 1)+(j+2);
					continue;
				}
			}
			

			if (table[nc][2] == 0 || (j > 0 && nc == 0))
			{
				table[n][2] = j+1;
				level_count[j]++;
			}
			if (j == input_n - 1)
			{
				if (nc == 0)	table[n][k] = 1;
				else	table[nc][k] = 1;
			}
			else
			{
				order_n = 1;
				for (ii = 0; ii < 64; ii++)
				{
					if (table[ii][2] == j+2)	order_n++;
					if (table[ii][2] == 0)	break;
				}
				if (nc == 0)	table[n][k] = (input_n - 1)*(order_n - 1)+(j+2);
				else	table[nc][k] = (input_n - 1)*(order_n - 1)+(j+2);
			}
			if (nc == 0)	n++;
			nc = 0;
		}
	}

	//==============================================Get ROBDD===================================================//
	ind = 0;
	note = 1;
	tc = 0;
	fk = 0;
	for (i = 0; i < 6; i++)
	{
		tc += level_count[i];
	}
	for (i = 0; i < 64; i++)
	{
		if (table[i][2] == 0)
			break;
		if (table[i][3] == 1)
			continue;
		//==================================Row Reduce================
		while(note == 1)
		{
			for (ii = 0; ii < tc-1; ii++)
			{
				for (jj = ii+1; jj < tc; jj++)
				{
					if (table[ii][3] == 0 && table[jj][3] == 0)
					{
						k = 0;
						for (j = 0; j < 3; j++)
						{
							if (table[ii][j] == table[jj][j])
								k++;
						}
						if (k == 3)
						{
							fk = 1;
							level1 = table[ii][2];
							ord = 0;
							for (j = 0; j < ii+1; j++)
							{
								if (table[j][3] == 1)	continue;
								if (table[j][2] == level1)
								{
									ord++;
								}
							}
							node1 = (input_n - 1) * (ord - 1) + level1;

							ord = 0;
							for (j = 0; j < jj+1; j++)
							{
								if (table[j][3] == 1)	continue;
								if (table[j][2] == level1)
								{
									ord++;
								}
							}
							node2 = (input_n - 1) * (ord - 1) + level1;

							table[jj][3] = 1;
							level_count[level1-1]--;


							for (p = 0; p < jj+1; p++)
							{
								if (table[p][3] == 1)
									continue;
								if (table[p][0] == node2)
								{
									table[p][0] = node1;
									for (q = 0; q < tc; q++)
									{
										if (table[q][3] == 1)
											continue;
										if (table[q][0] > node2)
										{
											ind = table[q][0];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][0] -= (input_n - 1);
													break;
												}
											}
										}
										if (table[q][1] > node2)
										{
											ind = table[q][1];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][1] -= (input_n - 1);
													break;
												}
											}
										}
									}
									break;
								}
								if (table[p][1] == node2)
								{
									table[p][1] = node1;
									for (q = 0; q < tc; q++)
									{
										if (table[q][3] == 1)
											continue;
										if (table[q][0] > node2)
										{
											ind = table[q][0];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][0] -= (input_n - 1);
													break;
												}
											}
										}
										if (table[q][1] > node2)
										{
											ind = table[q][1];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][1] -= (input_n - 1);
													break;
												}
											}
										}
									}
									break;
								}


							}
						}
					}
				}
			}
			if (fk != 1)
			{
				note = 0;
			}
			fk = 0;
		}
		//==================================Column Reduce================
		if (table[i][0] == table[i][1])
		{
			k = table[i][0];
			nc = table[i][2];
			order_n = 0;
			for (j = 0; j < i+1; j++)
			{
				if (table[j][3] == 1)
					continue;
				if (table[j][2] == nc)
				{
					order_n++;
				}
			}
			n = (input_n - 1)*(order_n - 1) + nc;
			table[i][3] = 1;
			level_count[nc-1]--;
		
			for (ii = 0; ii < tc; ii++)
			{
				hnote[ii][0] = 0;
				hnote[ii][1] = 0;
			}
		for (ii = 0; ii < tc; ii++)
		{
			if (table[ii][3] == 1)
				continue;
			if (table[ii][0] == n && hnote[ii][0] == 0)
			{
				table[ii][0] = k;
				for (jj = 0; jj < 64; jj++)
				{
					if (table[jj][2] == 0)
						break;
					if (table[jj][3] == 1)
						continue;
					if (table[jj][0] > n)
					{
						ind = table[jj][0];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][0] -= (input_n - 1);
								hnote[jj][0] = 1;
								break;
							}
						}
					}
					if (table[jj][1] > n)
					{
						ind = table[jj][1];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][1] -= (input_n - 1);
								hnote[jj][1] = 1;
								break;
							}
						}
					}
				}
				continue;
			}
			if (table[ii][1] == n && hnote[ii][1] == 0)
			{
				table[ii][1] = k;
				for (jj = 0; jj < 64; jj++)
				{
					if (table[jj][2] == 0)
						break;
					if (table[jj][3] == 1)
						continue;
					if (table[jj][0] > n)
					{
						ind = table[jj][0];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][0] -= (input_n - 1);
								hnote[jj][0] = 1;
								break;
							}
						}
					}
					if (table[jj][1] > n)
					{
						ind = table[jj][1];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][1] -= (input_n - 1);
								hnote[jj][1] = 1;
								break;
							}
						}
					}
				}
				continue;
			}
		}
		note = 0;
		i = -1;
		}
	}
	
	//==============================================Print BDD in Table Form===================================================//
	
	printf("Represent ROBDD in a Table\n\n");
	printf("List of Nodes		Path0			Path1\n");
	for (i = 0; i < 64; i++)
	{
		if (table[i][2] == 0)
			break;
		if (table[i][3] == 1)
			continue;
		ord = 0;
		for (j = 0; j < i+1; j++)
		{
			if (table[j][3] == 1)
				continue;
			if (table[j][2] == table[i][2])
				ord++;
		}
		printf("Node #%d ", ord);
		printf("of level %d	", table[i][2]);
		if (table[i][0] == 0)
			printf("Terminal 0		");
		else if (table[i][0] == 1)
			printf("Terminal 1		");
		else
		{
			k = 0;
			ind = table[i][0];
			while (ind >= 2)
			{
				ind -= (input_n -1);
				k++;
			}
			ind = table[i][0] - (k - 1) * (input_n - 1);
			printf("Node #%d ", k);
			printf("of level %d	", ind);
		}
		if (table[i][1] == 0)
			printf("Terminal 0		\n");
		else if (table[i][1] == 1)
			printf("Terminal 1		\n");
		else
		{
			k = 0;
			ind = table[i][1];
			while (ind >= 2)
			{
				ind -= (input_n -1);
				k++;
			}
			ind = table[i][1] - (k - 1) * (input_n - 1);
			printf("Node #%d ", k);
			printf("of level %d	\n", ind);
		}
	}
	printf("\n\n\n");

	

/*	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 4; j++)
		{
			printf("%d",table[i][j]);
			printf("	");
		}
		printf("\n");
		if (table[i][2] == 0)
			break;
	}


	for (i = 0; i < 6; i++)
	{
		printf("%d", level_count[i]);
		printf("	");
		if (level_count[i] == 0)
			break;
	}
	printf("\n");
*/
}


/**********************************************************************/
/*** GET NUM OF NODES *************************************************/
/**********************************************************************/

int GetNumberofNodes(t_blif_cubical_function *f)
{
	int table[64][4] = {{0}};
	int level_count[6] = {0};
	int hnote[64][2] = {{0}};

	int i, j, k, n, ii, jj, ord, nc, ind, note, p, q;
	int input_n, order_n, tc, fk;
	int node1, node2, level1, level2;
	int testi, testj;
	
	input_n = f->input_count;

	//========================================Build Initial BDD graph=========================================//
	n = 0;
	order_n = 1;
	for (i = 0; i < f->cube_count; i++)
	{
		nc = 0;
		for (j = 0; j < input_n; j++)
		{
			k = read_cube_variable(f->set_of_cubes[i]->signal_status, j) - 1;
			if (nc > 0)
			{
				if (table[nc][k] != 0)
				{
					order_n = (table[nc][k] - (j+2))/(input_n - 1) + 1;
					ord = 0;
					for (jj = 0; jj < 64; jj ++)
					{
						if (table[jj][2] == j+2)
						{
							ord++;
							if (ord == order_n)
							{
								nc = jj;
								break;
							}
						}
						if (table[jj][2] == 0)
						{
							//nc = jj;
							break;
						}
					}
					continue;
				}
			}
			
			if (j == 0 && table[0][2] != 0)
			{
				if (table[0][k] != 0)
				{
					order_n = (table[0][k] - (j+2))/(input_n - 1) + 1;
					ord = 0;
					for (jj = 0; jj < 64; jj ++)
					{
						if (table[jj][2] == j+2)
						{
							ord++;
							if (ord == order_n)
							{
								nc = jj;
								break;
							}
						}
						if (table[jj][2] == 0)
						{
							//nc = jj;
							break;
						}
					}
					continue;
				}
				else
				{
					order_n = 1;
					for (ii = 0; ii < 64; ii++)
					{
						if (table[ii][2] == j+2)	order_n++;
						if (table[ii][2] == 0)	break;
					}
					table[0][k] = (input_n - 1)*(order_n - 1)+(j+2);
					continue;
				}
			}
			

			if (table[nc][2] == 0 || (j > 0 && nc == 0))
			{
				table[n][2] = j+1;
				level_count[j]++;
			}
			if (j == input_n - 1)
			{
				if (nc == 0)	table[n][k] = 1;
				else	table[nc][k] = 1;
			}
			else
			{
				order_n = 1;
				for (ii = 0; ii < 64; ii++)
				{
					if (table[ii][2] == j+2)	order_n++;
					if (table[ii][2] == 0)	break;
				}
				if (nc == 0)	table[n][k] = (input_n - 1)*(order_n - 1)+(j+2);
				else	table[nc][k] = (input_n - 1)*(order_n - 1)+(j+2);
			}
			if (nc == 0)	n++;
			nc = 0;
		}
	}

	//==============================================Get ROBDD===================================================//
	ind = 0;
	note = 1;
	tc = 0;
	fk = 0;
	for (i = 0; i < 6; i++)
	{
		tc += level_count[i];
	}
	for (i = 0; i < 64; i++)
	{
		if (table[i][2] == 0)
			break;
		if (table[i][3] == 1)
			continue;
		//==================================Row Reduce================
		while(note == 1)
		{
			for (ii = 0; ii < tc-1; ii++)
			{
				for (jj = ii+1; jj < tc; jj++)
				{
					if (table[ii][3] == 0 && table[jj][3] == 0)
					{
						k = 0;
						for (j = 0; j < 3; j++)
						{
							if (table[ii][j] == table[jj][j])
								k++;
						}
						if (k == 3)
						{
							fk = 1;
							level1 = table[ii][2];
							ord = 0;
							for (j = 0; j < ii+1; j++)
							{
								if (table[j][3] == 1)	continue;
								if (table[j][2] == level1)
								{
									ord++;
								}
							}
							node1 = (input_n - 1) * (ord - 1) + level1;

							ord = 0;
							for (j = 0; j < jj+1; j++)
							{
								if (table[j][3] == 1)	continue;
								if (table[j][2] == level1)
								{
									ord++;
								}
							}
							node2 = (input_n - 1) * (ord - 1) + level1;

							table[jj][3] = 1;
							level_count[level1-1]--;


							for (p = 0; p < jj+1; p++)
							{
								if (table[p][3] == 1)
									continue;
								if (table[p][0] == node2)
								{
									table[p][0] = node1;
									for (q = 0; q < tc; q++)
									{
										if (table[q][3] == 1)
											continue;
										if (table[q][0] > node2)
										{
											ind = table[q][0];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][0] -= (input_n - 1);
													break;
												}
											}
										}
										if (table[q][1] > node2)
										{
											ind = table[q][1];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][1] -= (input_n - 1);
													break;
												}
											}
										}
									}
									break;
								}
								if (table[p][1] == node2)
								{
									table[p][1] = node1;
									for (q = 0; q < tc; q++)
									{
										if (table[q][3] == 1)
											continue;
										if (table[q][0] > node2)
										{
											ind = table[q][0];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][0] -= (input_n - 1);
													break;
												}
											}
										}
										if (table[q][1] > node2)
										{
											ind = table[q][1];
											while (ind > 0)
											{
												ind = ind - (input_n - 1);
												if (ind == node2)
												{
													table[q][1] -= (input_n - 1);
													break;
												}
											}
										}
									}
									break;
								}


							}
						}
					}
				}
			}
			if (fk != 1)
			{
				note = 0;
			}
			fk = 0;
		}
		//==================================Column Reduce================
		if (table[i][0] == table[i][1])
		{
			k = table[i][0];
			nc = table[i][2];
			order_n = 0;
			for (j = 0; j < i+1; j++)
			{
				if (table[j][3] == 1)
					continue;
				if (table[j][2] == nc)
				{
					order_n++;
				}
			}
			n = (input_n - 1)*(order_n - 1) + nc;
			table[i][3] = 1;
			level_count[nc-1]--;
		
			for (ii = 0; ii < tc; ii++)
			{
				hnote[ii][0] = 0;
				hnote[ii][1] = 0;
			}
		for (ii = 0; ii < tc; ii++)
		{
			if (table[ii][3] == 1)
				continue;
			if (table[ii][0] == n && hnote[ii][0] == 0)
			{
				table[ii][0] = k;
				for (jj = 0; jj < 64; jj++)
				{
					if (table[jj][2] == 0)
						break;
					if (table[jj][3] == 1)
						continue;
					if (table[jj][0] > n)
					{
						ind = table[jj][0];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][0] -= (input_n - 1);
								hnote[jj][0] = 1;
								break;
							}
						}
					}
					if (table[jj][1] > n)
					{
						ind = table[jj][1];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][1] -= (input_n - 1);
								hnote[jj][1] = 1;
								break;
							}
						}
					}
				}
				continue;
			}
			if (table[ii][1] == n && hnote[ii][1] == 0)
			{
				table[ii][1] = k;
				for (jj = 0; jj < 64; jj++)
				{
					if (table[jj][2] == 0)
						break;
					if (table[jj][3] == 1)
						continue;
					if (table[jj][0] > n)
					{
						ind = table[jj][0];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][0] -= (input_n - 1);
								hnote[jj][0] = 1;
								break;
							}
						}
					}
					if (table[jj][1] > n)
					{
						ind = table[jj][1];
						while (ind > 0)
						{
							ind = ind - (input_n - 1);
							if (ind == n)
							{
								table[jj][1] -= (input_n - 1);
								hnote[jj][1] = 1;
								break;
							}
						}
					}
				}
				continue;
			}
		}
		note = 0;
		i = -1;
		}
	}
	tc = 0;
	for (i = 0; i < 6; i++)
	{
		tc += level_count[i];
	}
	return tc;

}




/**********************************************************************/
/*** SHIFTING ALGORITHM ***********************************************/
/**********************************************************************/

void shifting_alg(t_blif_cubical_function *f)
{
	int i, j, ii, jj, k, n, factor, tc, nf, input_n, ind, kk;
	int testi, testj;
	int aid_col = 0;
	int Nofnodes[121] = {0};
	int sfactor[6] = {0};
	int label[6] = {0};

	clock_t start, stop;
	double duration;
	
	t_blif_cube *copy[64];
	t_blif_cube *copy1[64];

	start = clock();

	for (i = 0; i < f->cube_count; i++)
	{
		copy[i] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
		memcpy(copy[i], f->set_of_cubes[i], sizeof(t_blif_cube));
		copy1[i] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
		memcpy(copy1[i], f->set_of_cubes[i], sizeof(t_blif_cube));
	}
	
	factor = 1;
	tc = f->cube_count;
	input_n = f->input_count;
	for (i = 1; i <= f->input_count; i++)	factor *= i;

	nf = 0;
	if (f->input_count > 2)
	{
		for (i = 0; i < f->input_count - 2; i++)
		{
			sfactor[i] = 1;
			for (j = 1; j <= i+1; j++)	sfactor[i] *= j;
			nf++;
		}
	}

	

	kk = 1;
	ind = -1;
	while (kk == 1)
	{
		n = 0;
		Nofnodes[n] = GetNumberofNodes(f);
		if (n == ind)
		{
			printf("One of the best shifting results is:\n\n");
			ROBDD_package(f);
			printf("Number of Nodes is: %d\n\n", k);
			break;
		}
		n++;
		k = Nofnodes[0];
		for (j = nf-1; j >= 0; j--)
		{
			if (label[j] == sfactor[j]-1)
			{
				//swap columns;
				for (ii = 0; ii < tc; ii++)
				{
					aid_col = read_cube_variable(copy[ii]->signal_status, input_n - 1 - j);
					if (read_cube_variable(copy[ii]->signal_status, input_n - 1 - j - 1) == 1)
						write_cube_variable(copy[ii]->signal_status, input_n - 1 - j, LITERAL_0);
					if (read_cube_variable(copy[ii]->signal_status, input_n - 1 - j - 1) == 2)
						write_cube_variable(copy[ii]->signal_status, input_n - 1 - j, LITERAL_1);
					if (aid_col == 1)
						write_cube_variable(copy[ii]->signal_status, input_n - 1 - j - 1, LITERAL_0);
					if (aid_col == 2)
						write_cube_variable(copy[ii]->signal_status, input_n - 1 - j - 1, LITERAL_1);
				}

				for (jj = 0; jj < tc; jj++)	f->set_of_cubes[jj] = copy[jj];

				Nofnodes[n] = GetNumberofNodes(f);
				if (ind == n)
				{
					printf("One of the best shifting results is:\n\n");
					ROBDD_package(f);
					printf("Number of Nodes is: %d\n\n", Nofnodes[n]);
					kk = 0;
					break;
				}
				n++;

				for (jj = j+1; jj < nf; jj++)	label[jj]++;
				for (ii = 0; ii <= j; ii++)	label[ii] = 0;
				if (j == 0 && n < factor)
				{
					j = nf;
				}
			}
//			for (jj = 0; jj < nf; jj++)	printf("%d  ", label[jj]);
//			printf("\n\n");
		}
//		for (i = 0; i < factor; i++) printf("%d  ", Nofnodes[i]);
//		printf("\n");
		
		k = Nofnodes[0];
		ind = 0;
		for (j = 1; j < factor; j++)
		{
			if (k > Nofnodes[j])
			{
				k = Nofnodes[j];
				ind = j;
//				printf("nodes %d\n",k);
//				printf("node ind %d\n",ind);
			}
		}

		//back to initial state.
		for (i = 0; i < 6; i++)	label[i] = 0;
		for (i = 0; i < tc; i++)	copy[i] = copy1[i];
		for (jj = 0; jj < tc; jj++)	f->set_of_cubes[jj] = copy[jj];
	}

	stop = clock();
	duration = (double)(stop - start);
	printf("Run time of shifting algorithm is: %f ms\n\n\n", duration);
	/*
	printf("The final simplified result is:\n\n");

	printf("Final Prime Implicants \t\t Outputs\n");

	for (testi = 0; testi < f->cube_count; testi++)
	{
		for (testj = 0; testj < f->input_count; testj++)
		{
			if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 2)	printf("%d", 1);
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 3)	printf("%c", '-');
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 1)	printf("%d", 0);
			printf("  ");
		}
		printf("\t\t\t ");
		if (!f->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n");
	*/
}






/**********************************************************************/
/*** APPLY ALGORITHM **************************************************/
/**********************************************************************/
/*
void apply_alg(t_blif_cubical_function *f1, t_blif_cubical_function *f2)
{
	int testi, testj;
	int i, j, ii, jj, k, n, ind;
	int n_and, n_or, n_xor;
	clock_t start1, start2, start3, stop1, stop2, stop3, start11, start22, start33, stop11, stop22, stop33;
	double duration1, duration2, duration3;

	t_blif_cube *f_and[64];
	t_blif_cube *f_or[128];
	t_blif_cube *f_xor[64];
	
	//=============================make sure f1 and f2 have equivalent number of inputs===============//

	if ((f1->input_count) > (f2->input_count))
	{
		for (i = 0; i < f2->cube_count; i++)
		{
			f2->set_of_cubes[i]->data_size = f1->input_count;
			for (j = f2->input_count; j < f1->input_count; j++)
			{
				write_cube_variable(f2->set_of_cubes[i]->signal_status, j, LITERAL_DC);
			}
		}
		f2->input_count = f1->input_count;
		simplify_function(f2);
	}

	if ((f2->input_count) > (f1->input_count))
	{
		for (i = 0; i < f1->cube_count; i++)
		{
			f1->set_of_cubes[i]->data_size = f2->input_count;
			for (j = f1->input_count; j < f2->input_count; j++)
			{
				write_cube_variable(f1->set_of_cubes[i]->signal_status, j, LITERAL_DC);
			}
		}
		f1->input_count = f2->input_count;
		simplify_function(f1);
	}

	//=============================apply algorithm -- AND operation========================================//
	start1 = clock();
	n = 0;
	for (i = 0; i < f1->cube_count; i++)
	{
		for (j = 0; j < f2->cube_count; j++)
		{
			k = 0;
			for (ii = 0; ii < f1->input_count; ii++)
			{
				if (read_cube_variable(f1->set_of_cubes[i]->signal_status, ii) == read_cube_variable(f2->set_of_cubes[j]->signal_status, ii))
					k++;
			}
			if (k == f1->input_count)
			{
				f_and[n] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
				memcpy(f_and[n], f1->set_of_cubes[i], sizeof(t_blif_cube));
				n++;
			}
		}
	}
	n_and = n;
	stop1 = clock();
	duration1 = (double)(stop1 - start1);

	//=============================apply algorithm -- XOR operation========================================//
	start2 = clock();
	n = 0;
	for (i = 0; i < f1->cube_count; i++)
	{
		ind = 0;
		for (j = 0; j < f2->cube_count; j++)
		{
			k = 0;
			for (ii = 0; ii < f1->input_count; ii++)
			{
				if (read_cube_variable(f1->set_of_cubes[i]->signal_status, ii) == read_cube_variable(f2->set_of_cubes[j]->signal_status, ii))
					k++;
			}
			if (k != f1->input_count)
			{
				ind++;
			}
		}
		if (ind == f2->cube_count)
		{
			f_xor[n] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
			memcpy(f_xor[n], f1->set_of_cubes[i], sizeof(t_blif_cube));
			n++;
		}
	}
	for (i = 0; i < f2->cube_count; i++)
	{
		ind = 0;
		for (j = 0; j < f1->cube_count; j++)
		{
			k = 0;
			for (ii = 0; ii < f2->input_count; ii++)
			{
				if (read_cube_variable(f2->set_of_cubes[i]->signal_status, ii) == read_cube_variable(f1->set_of_cubes[j]->signal_status, ii))
					k++;
			}
			if (k != f2->input_count)
			{
				ind++;
			}
		}
		if (ind == f1->cube_count)
		{
			f_xor[n] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
			memcpy(f_xor[n], f2->set_of_cubes[i], sizeof(t_blif_cube));
			n++;
		}
	}
	n_xor = n;
	stop2 = clock();
	duration2 = (double)(stop2 - start2);

	//=============================apply algorithm -- OR operation========================================//
	start3 = clock();
	n = 0;
	for (i = 0; i < f1->cube_count; i++)
	{
		f_or[n] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
		memcpy(f_or[n], f1->set_of_cubes[i], sizeof(t_blif_cube));
		n++;
	}
	for (i = 0; i < f2->cube_count; i++)
	{
		f_or[n] = (t_blif_cube *) malloc (sizeof(t_blif_cube));
		memcpy(f_or[n], f2->set_of_cubes[i], sizeof(t_blif_cube));
		n++;
	}
	n_or = n;
	stop3 = clock();
	duration3 = (double)(stop3 - start3);

	//============Display apply algorithm results with AND OR XOR operation=========================//
	start11 = clock();
	for (j = 0; j < n_and; j++) f1->set_of_cubes[j] = f_and[j];
	f1->cube_count = n_and;
	printf("Apply Algorithm with AND operation\n\n");
	ROBDD_package (f1);
	stop11 = clock();
	duration1 = duration1 + (double)(stop11 - start11);
	printf("Run time of apply algorithm with AND operation is %f ms.\n\n", duration1);

	start33 = clock();
	for (j = 0; j < n_or; j++) f1->set_of_cubes[j] = f_or[j];
	f1->cube_count = n_or;
	simplify_function(f1);
	printf("Apply Algorithm with OR operation\n\n");
	ROBDD_package (f1);
	stop33 = clock();
	duration3 = duration3 + (double)(stop33 - start33);
	printf("Run time of apply algorithm with OR operation is %f ms.\n\n", duration3);

	start22 = clock();
	for (j = 0; j < n_xor; j++) f1->set_of_cubes[j] = f_xor[j];
	f1->cube_count = n_xor;
	printf("Apply Algorithm with XOR operation\n\n");
	ROBDD_package (f1);
	stop22 = clock();
	duration2 = duration2 + (double)(stop22 - start22);
	printf("Run time of apply algorithm with XOR operation is %f ms.\n\n", duration2);

	//============print f1=========================//
/*	for (testi = 0; testi < f1->cube_count; testi++)
	{
		for (testj = 0; testj < f1->input_count; testj++)
		{
			if (read_cube_variable(f1->set_of_cubes[testi]->signal_status, testj) == 2)	printf("%d", 1);
			else if (read_cube_variable(f1->set_of_cubes[testi]->signal_status, testj) == 3)	printf("%c", '-');
			else if (read_cube_variable(f1->set_of_cubes[testi]->signal_status, testj) == 1)	printf("%d", 0);
			printf("  ");
		}
		printf("\t\t\t ");
		if (!f1->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n");
	*/
	//============print f2=========================//
/*	for (testi = 0; testi < f2->cube_count; testi++)
	{
		for (testj = 0; testj < f2->input_count; testj++)
		{
			if (read_cube_variable(f2->set_of_cubes[testi]->signal_status, testj) == 2)	printf("%d", 1);
			else if (read_cube_variable(f2->set_of_cubes[testi]->signal_status, testj) == 3)	printf("%c", '-');
			else if (read_cube_variable(f2->set_of_cubes[testi]->signal_status, testj) == 1)	printf("%d", 0);
			printf("  ");
		}
		printf("\t\t\t ");
		if (!f2->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n");
*/

	

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
	printf("Quine-McCluskey 2-level logic minimization program.\r\n");

	/* Read BLIF circuit. */
	printf("Reading file %s...\n",argv[1]);
	circuit = ReadBLIFCircuit(argv[1]);

	if (circuit != NULL)
	{
		int index;

		/* Minimize each function, one at a time. */
		printf("Minimizing logic functions\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			simplify_function(function);

			printf("Shifting Algorithm of Function\n\n");
			shifting_alg(function);
		}

		/* Print out synthesis report. */
		printf("Report:\r\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			/* Print function information. */
			printf("Function %i: #inputs = %i; #cubes = %i; cost = %i\n", index+1, function->input_count, function->cube_count, function_cost(function));
		
		}

		/* Finish. */ 
		printf("Done.\r\n");
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
//	system("pause");
	return 0;
}

