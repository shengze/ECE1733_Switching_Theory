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

void simplify_function(t_blif_cubical_function *f);


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
		for (index = 0; index < f->cube_count; index++)
		{
			cost += cube_cost(f->set_of_cubes[index], f->input_count);
		}
		if (f->cube_count > 1)
		{
			cost += (f->cube_count + 1);
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
		result += num_cubes + 1;
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
	/*
	t_blif_cube **replace = f->set_of_cubes;
	replace = (t_blif_cube **) malloc (sizeof(t_blif_cube *));
	replace[0] = f->set_of_cubes[0];
	for(testi = 0; testi < f->input_count; testi++)
	{
	write_cube_variable(replace[0]->signal_status, testi, LITERAL_1);
	printf("%d   ",read_cube_variable(replace[0]->signal_status, testi));
	}
	f->set_of_cubes = replace;
	f->cube_count = 1;
	printf("replace finished!\n\n");
	*/
	/*
	t_blif_cube **exp;

	t_blif_cube *test;
	test = (t_blif_cube *) malloc (sizeof(t_blif_cube));
	memcpy (test, f->set_of_cubes[0], sizeof(t_blif_cube));
	write_cube_variable (test->signal_status, 2, LITERAL_0);
	exp = (t_blif_cube **) malloc (f->cube_count * sizeof(t_blif_cube *));
	exp[0] = test;
	*/
	printf("The content in the BLIF file is:\n\n");

	printf("Minterms \t\t\t Outputs\n");

	for (testi = 0; testi < f->cube_count; testi++)
	{
		for (testj = 0; testj < f->input_count; testj++)
		{
			if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 2)	printf("%d", 1);
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 3)	printf("%c", '-');
			else if (read_cube_variable(f->set_of_cubes[testi]->signal_status, testj) == 1)	printf("%d", 0);
			printf("  ");
		}
		printf("\t\t");
		if (!f->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n\n");


	int i, j, n, note, k;
	//========================================Expand initial circuit to 0s and 1s=========================================//
	int epn, epnforever, newmem, PIcount;

	int coverTc[50][50] = { { 0 } };
	int coverN[50] = { 0 };
	int coverNote[50] = { 0 };
	int covBr[50] = { 0 };
	int covBc[50] = { 0 };

	int i1, i2, nrc, PIn, finalCount;

	int custate[50] = { 0 };

	t_blif_cube **expansion;
	t_blif_cube *expaid[100];
	t_blif_cube *norepeat[100];
	t_blif_cube *PI[100];
	t_blif_cube *PIno[100];
	t_blif_cube *finalPI[50];


	expansion = (t_blif_cube **)malloc(f->cube_count * f->cube_count * sizeof(t_blif_cube *));
	n = 1;
	epnforever = 0;
	while (n)
	{
		n = 0;
		epn = 0;
		for (i = 0; i < f->cube_count; i++)
		{
			if (f->set_of_cubes[i]->is_DC) continue;
			expaid[epnforever] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(expaid[epnforever], f->set_of_cubes[i], sizeof(t_blif_cube));
			expansion[epn] = expaid[epnforever];
			epnforever++;
			for (j = 0; j < f->input_count; j++)
			{
				if (read_cube_variable(f->set_of_cubes[i]->signal_status, j) == 3)
				{
					n++;
					write_cube_variable(expansion[epn]->signal_status, j, LITERAL_0);
					epn++;

					expaid[epnforever] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
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
	norepeat[nrc] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
	memcpy(norepeat[nrc], f->set_of_cubes[0], sizeof(t_blif_cube));
	nrc++;
	for (i = 1; i < f->cube_count; i++)
	{
		n = 0;
		for (j = 0; j < i; j++)
		{
			for (i1 = 0; i1 < f->input_count; i1++)
			{
				if (read_cube_variable(f->set_of_cubes[i]->signal_status, i1) != read_cube_variable(f->set_of_cubes[j]->signal_status, i1))
				{
					n++;
					break;
				}
			}
		}
		if (n == i)
		{
			norepeat[nrc] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(norepeat[nrc], f->set_of_cubes[i], sizeof(t_blif_cube));
			nrc++;
		}
	}
	for (j = 0; j < nrc; j++) f->set_of_cubes[j] = norepeat[j];
	f->cube_count = nrc;
	for (j = epnforever - epn; j < epnforever; j++) free(expaid[j]);

	//========================================Find Out Prime Implicants by Combining=========================================//
	note = -1;
	PIcount = 0;
	k = 1;
	while (k)
	{
		k = 0;
		newmem = 0;
		for (i = 0; i < f->cube_count - 1; i++)
		{
			for (j = i + 1; j < f->cube_count; j++)
			{
				n = 0;
				for (i1 = 0; i1 < f->input_count; i1++)
				{
					if (read_cube_variable(f->set_of_cubes[i]->signal_status, i1) != read_cube_variable(f->set_of_cubes[j]->signal_status, i1))
					{
						if (n == 0)	note = i1;
						n++;
					}
				}
				if (n == 1)
				{
					PI[PIcount] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
					memcpy(PI[PIcount], f->set_of_cubes[i], sizeof(t_blif_cube));
					write_cube_variable(PI[PIcount]->signal_status, note, LITERAL_DC);
					//					for(testi = 0; testi < f->input_count; testi++) printf("%d	", read_cube_variable(PI[PIcount]->signal_status, testi));
					//					printf("\n");
					PIcount++;
					newmem++;
					k++;
					custate[i] = 1;
					custate[j] = 1;
				}
			}
		}
		for (i = 0; i < f->cube_count; i++)
		{
			if (!custate[i])
			{
				PI[PIcount] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
				memcpy(PI[PIcount], f->set_of_cubes[i], sizeof(t_blif_cube));
				//				for(testi = 0; testi < f->input_count; testi++) printf("kk%d	", read_cube_variable(PI[PIcount]->signal_status, testi));
				//				printf("\n");
				PIcount++;
				newmem++;
				custate[i] = 1;
			}
		}
		for (i = 0; i < 20; i++) custate[i] = 0;
		for (j = 0; j < newmem; j++) f->set_of_cubes[j] = PI[PIcount - newmem + j];
		f->cube_count = newmem;
	}

	//========================================Remove All the Repeated Items from PI List=========================================//
	PIn = 0;
	PIno[PIn] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
	memcpy(PIno[PIn], f->set_of_cubes[0], sizeof(t_blif_cube));
	PIn++;
	for (i = 1; i < f->cube_count; i++)
	{
		n = 0;
		for (j = 0; j < i; j++)
		{
			for (i1 = 0; i1 < f->input_count; i1++)
			{
				if (read_cube_variable(f->set_of_cubes[i]->signal_status, i1) != read_cube_variable(f->set_of_cubes[j]->signal_status, i1))
				{
					n++;
					break;
				}
			}
		}
		if (n == i)
		{
			PIno[PIn] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(PIno[PIn], f->set_of_cubes[i], sizeof(t_blif_cube));
			PIn++;
		}
	}
	for (j = 0; j < PIn; j++) f->set_of_cubes[j] = PIno[j];
	f->cube_count = PIn;

	//========================================Build Cover Table & Get a Reduced Cover Table=========================================//
	finalCount = 0;
	memset(covBr, 0, sizeof(int) * 50);
	memset(covBc, 0, sizeof(int) * 50);
	//=====Build A Cover Table=====//
	for (i = 0; i < nrc; i++)
	{
		for (j = 0; j < PIn; j++)
		{
			k = 0;
			for (i1 = 0; i1 < f->input_count; i1++)
				if ((read_cube_variable(norepeat[i]->signal_status, i1) != read_cube_variable(PIno[j]->signal_status, i1)) && (read_cube_variable(PIno[j]->signal_status, i1) != 3))
					k++;
			if (k == 0)
			{
				coverN[i]++;
				coverTc[j][i] = 1;
				if (coverN[i] == 1)	coverNote[i] = j;
			}
		}
	}
	for (i = 0; i < nrc; i++)
	{
		if (covBr[i] == 1)	continue;
		if (coverN[i] == 1)
		{
			covBr[i] = 1;
			covBc[coverNote[i]] = 1;
			for (j = 0; j < nrc; j++)
				if (coverTc[coverNote[i]][j] == 1)
					covBr[j] = 1;
		}
	}
	/*	for (i = 0; i < PIn; i++)
	{
	for (j = 0; j < nrc; j++)
	{
	printf("%d  ", coverTc[i][j]);
	}
	printf("\n");
	}
	for (i = 0; i < nrc; i++)	printf("%d  ", coverN[i]);
	printf("\n");
	for (i = 0; i < nrc; i++)	printf("%d  ", covBr[i]);
	printf("\n");
	for (i = 0; i < PIn; i++)	printf("%d  ", covBc[i]);
	printf("\n");
	*/
	//=====Get A Reduced Cover Table=====//
	epn = 0;
	newmem = 0;
	for (i = 0; i < nrc - epn; i++)
	{
		if (covBr[i] == 0)
		{
			norepeat[nrc] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(norepeat[nrc], norepeat[i], sizeof(t_blif_cube));
			nrc++;
			epn++;
		}
	}
	for (i = 0; i < PIn - newmem; i++)
	{
		if (covBc[i] == 0)
		{
			PIno[PIn] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(PIno[PIn], PIno[i], sizeof(t_blif_cube));
			PIn++;
			newmem++;
		}
		else if (covBc[i] == 1)
		{
			finalPI[finalCount] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
			memcpy(finalPI[finalCount], PIno[i], sizeof(t_blif_cube));
			finalCount++;
		}
	}
	for (j = 0; j < finalCount; j++) f->set_of_cubes[j] = finalPI[j];
	f->cube_count = finalCount;

	//===============================Put the Reduced Table into Iteration and Starting Row/Col Domain Minimization=================================//

	while (epn)
	{


		memset(coverTc, 0, sizeof(int) * 50 * 50);
		memset(coverN, 0, sizeof(int) * 50);
		memset(coverNote, 0, sizeof(int) * 50);
		memset(covBr, 0, sizeof(int) * 50);
		memset(covBc, 0, sizeof(int) * 50);
		for (i = 0; i < epn; i++)
		{
			for (j = 0; j < newmem; j++)
			{
				k = 0;
				for (i1 = 0; i1 < f->input_count; i1++)
					if ((read_cube_variable(norepeat[nrc - epn + i]->signal_status, i1) != read_cube_variable(PIno[PIn - newmem + j]->signal_status, i1)) && (read_cube_variable(PIno[PIn - newmem + j]->signal_status, i1) != 3))
						k++;
				if (k == 0)
				{
					coverN[i]++;
					coverTc[j][i] = 1;
					if (coverN[i] == 1)	coverNote[i] = j;
				}
			}
		}
		/*
		for (i = 0; i < newmem; i++)
		{
		for (j = 0; j < epn; j++)
		{
		printf("%d  ", coverTc[i][j]);
		}
		printf("\n");
		}
		for (i = 0; i < epn; i++)	printf("%d  ", coverN[i]);
		printf("\n");
		for (i = 0; i < epn; i++)	printf("%d  ", covBr[i]);
		printf("\n");
		for (i = 0; i < newmem; i++)	printf("%d  ", covBc[i]);
		printf("\n");
		*/

		for (i = 0; i < newmem - 1; i++)
		{
			for (j = i + 1; j < newmem; j++)
			{
				n = 0;
				k = 0;
				for (i1 = 0; i1 < epn; i1++)
				{
					if (coverTc[i][i1] - (coverTc[i][i1] * coverTc[j][i1]) == 0)
						k++;
					if (coverTc[j][i1] - (coverTc[i][i1] * coverTc[j][i1]) == 0)
						n++;
				}
				if (k == epn && n != epn)	covBc[i] = 1;
				if (n == epn && k != epn)	covBc[j] = 1;
				if (k == epn && n == epn)
				{
					for (i2 = 0; i2 < f->input_count; i2++)
					{
						if (read_cube_variable(PIno[PIn - newmem + i]->signal_status, i2) > read_cube_variable(PIno[PIn - newmem + j]->signal_status, i2))
						{
							covBc[i] = 1;
							break;
						}
						if (read_cube_variable(PIno[PIn - newmem + i]->signal_status, i2) < read_cube_variable(PIno[PIn - newmem + j]->signal_status, i2))
						{
							covBc[j] = 1;
							break;
						}
					}
				}
			}
		}
		testi = 0;
		for (i = 0; i < newmem; i++)
		{
			if (covBc[i] == 0)
			{
				PIno[PIn + testi] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
				memcpy(PIno[PIn + testi], PIno[PIn - newmem + i], sizeof(t_blif_cube));
				testi++;
			}
		}
		PIn += testi;
		newmem = testi;
		//	printf("%d  %d  %d  %d\n", testi, PIn, epn, nrc);
		//====================================
		//=====Build A Cover Table=====//
		memset(coverTc, 0, sizeof(int) * 50 * 50);
		memset(coverN, 0, sizeof(int) * 50);
		memset(coverNote, 0, sizeof(int) * 50);
		memset(covBr, 0, sizeof(int) * 50);
		memset(covBc, 0, sizeof(int) * 50);
		for (i = 0; i < epn; i++)
		{
			for (j = 0; j < newmem; j++)
			{
				k = 0;
				for (i1 = 0; i1 < f->input_count; i1++)
					if ((read_cube_variable(norepeat[nrc - epn + i]->signal_status, i1) != read_cube_variable(PIno[PIn - newmem + j]->signal_status, i1)) && (read_cube_variable(PIno[PIn - newmem + j]->signal_status, i1) != 3))
						k++;
				if (k == 0)
				{
					coverN[i]++;
					coverTc[j][i] = 1;
					if (coverN[i] == 1)	coverNote[i] = j;
				}
			}
		}
		for (i = 0; i < epn; i++)
		{
			if (covBr[i] == 1)	continue;
			if (coverN[i] == 1)
			{
				covBr[i] = 1;
				covBc[coverNote[i]] = 1;
				for (j = 0; j < epn; j++)
					if (coverTc[coverNote[i]][j] == 1)
						covBr[j] = 1;
			}
		}
		/*
		for (i = 0; i < newmem; i++)
		{
		for (j = 0; j < epn; j++)
		{
		printf("%d  ", coverTc[i][j]);
		}
		printf("\n");
		}
		for (i = 0; i < epn; i++)	printf("%d  ", coverN[i]);
		printf("\n");
		for (i = 0; i < epn; i++)	printf("%d  ", covBr[i]);
		printf("\n");
		for (i = 0; i < newmem; i++)	printf("%d  ", covBc[i]);
		printf("\n");
		*/

		//=====Get A Reduced Cover Table=====//
		testi = 0;
		testj = 0;
		for (i = 0; i < epn; i++)
		{
			if (covBr[i] == 0)
			{
				norepeat[nrc] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
				memcpy(norepeat[nrc], norepeat[nrc - testi - epn + i], sizeof(t_blif_cube));
				nrc++;
				testi++;
			}
		}
		for (i = 0; i < newmem; i++)
		{
			if (covBc[i] == 0)
			{
				PIno[PIn] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
				memcpy(PIno[PIn], PIno[PIn - testj - newmem + i], sizeof(t_blif_cube));
				PIn++;
				testj++;
			}
			else if (covBc[i] == 1)
			{
				//			printf("%d\n", read_cube_variable(PIno[PIn - testj - newmem + i]->signal_status,1));
				finalPI[finalCount] = (t_blif_cube *)malloc(sizeof(t_blif_cube));
				memcpy(finalPI[finalCount], PIno[PIn - testj - newmem + i], sizeof(t_blif_cube));
				finalCount++;
			}
		}
		epn = testi;
		newmem = testj;
		//	printf("%d  %d  %d  %d\n\n\n\n\n",testi,testj,PIn,nrc);

	}

	for (j = 0; j < finalCount; j++) f->set_of_cubes[j] = finalPI[j];
	f->cube_count = finalCount;
	//====================================

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
		printf("\t\t");
		if (!f->set_of_cubes[testi]->is_DC) {
			printf("%d", 1);
		}
		else {
			printf("%s", "-");
		}
		printf("\n");
	}
	printf("\n\n");
}


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
	printf("Reading file %s...\n", argv[1]);
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
		}

		/* Print out synthesis report. */
		printf("Report:\r\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			/* Print function information. */
			printf("Function %i: #inputs = %i; #cubes = %i; cost = %i\n", index + 1, function->input_count, function->cube_count, function_cost(function));

		}

		/* Finish. */
		printf("Done.\r\n");
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
    system("pause");
	return 0;
}

