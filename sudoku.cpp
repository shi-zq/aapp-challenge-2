/*                    
 *  This file is part of Christian's OpenMP parallel Sudoku Solver
 *  
 *  Copyright (C) 2013 by Christian Terboven <christian@terboven.com>
 *                                                                       
 *  This program is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include "SudokuBoard.h"


bool check(CSudokuBoard* tmpSudoku1) {	
	int fieldSize = tmpSudoku1->getFieldSize();
	int blockSize = tmpSudoku1->getBlockSize();
	int* counter = new int[fieldSize];
	for(int i = 0;  i < fieldSize; i++) {
		counter[i] = 0;
	}
	//check row						
	for(int i = 0; i < fieldSize; i++) {
		for(int j = 0; j < fieldSize; j++) {
			counter[tmpSudoku1->get(i, j)-1] = counter[tmpSudoku1->get(i, j)-1]+1;
		}
		for(int k = 0;  k < fieldSize; k++) {
			if(counter[k] != 1) {
				return false;
			}
			counter[k] = 0;
		}
	}							
	//check column
	for(int i = 0; i < fieldSize; i++) {
		for(int j = 0; j < fieldSize; j++) {
			counter[tmpSudoku1->get(j, i)-1] = counter[tmpSudoku1->get(j, i)-1]+1;
		}
		for(int k = 0;  k < fieldSize; k++) {
			if(counter[k] != 1) {
				return false;
			}
			counter[k] = 0;								
		}
	}			
	//check block
	for (int row = 0; row < blockSize; row++) {
		for (int col = 0; col < blockSize; col++) {
			for (int i = 0; i < blockSize; i++) {
				for (int j = 0; j < blockSize; j++) {
					counter[tmpSudoku1->get(row*blockSize + i, col*blockSize + j)-1] = counter[tmpSudoku1->get(row*blockSize + i, col*blockSize + j)-1]+1;
				}
			}
			for(int k = 0;  k < fieldSize; k++) {
				if(counter[k] != 1) {
					return false;
				}
				counter[k] = 0;								
			}
		}
	}
	return true;
}

bool valid(CSudokuBoard* tmpSudoku1, int row, int col, int num) {
	int fieldSize = tmpSudoku1->getFieldSize();
	int blockSize = tmpSudoku1->getBlockSize();
	for(int i = 0; i < fieldSize; i++) {
		if(tmpSudoku1->get(row, i) == num) {
			return false;
		}
	}
	for(int i = 0; i < fieldSize; i++) {
		if(tmpSudoku1->get(i, col) == num) {
			return false;
		}
	}
	int startRow = row - (row % blockSize);
	int startCol = col - (col % blockSize);
	for(int i = 0; i < blockSize; i++) {
		for (int j = 0; j < blockSize; j++ ) {
			if(tmpSudoku1->get(i + startRow, j + startCol) == num) {
				return false;
			}
		}
	}
	return true;
}

void solver(CSudokuBoard* tmpSudoku1, int row, int col, int lastRow, int lastCol) {
	// #pragma omp parallel
	// {
	// 	#pragma omp single
	// 	{
			for(int i = 0; i < tmpSudoku1->getFieldSize(); i++) {
				for(int j = 0; j < tmpSudoku1->getFieldSize(); j++) {
					if(tmpSudoku1->get(i, j) == 0) {
						for(int num = 1; num < (tmpSudoku1->getFieldSize()+1); num++) {
							//std::cout << valid(&copySudoku, i, j, num) << i << " "<< j << " set " << num << std::endl;
							//std::cout << num << i << j <<std::endl;
							//std::cout << "found "<< omp_get_thread_num() << num << i << j <<std::endl;
							if(valid(tmpSudoku1, i, j, num)) {
								// #pragma omp task firstprivate(i, j, tmpSudoku1, num)
								// {
									CSudokuBoard copySudoku = CSudokuBoard(*tmpSudoku1);
									copySudoku.set(i, j, num);
									//copySudoku.printBoard();
									//std::cout << "found "<< omp_get_thread_num() << num << i << j <<std::endl;
									// #pragma critical
									// {
										if(i == lastRow && j == lastCol) {
											//found = true;
											std::cout << "found "<< omp_get_thread_num() << num << i << j <<std::endl;
											copySudoku.printBoard();
										}
										else {
											solver(&copySudoku, 0, 0, lastRow, lastCol);
										}
									//}
								//}
							}
						}
					}
				}
			}
	// 	}
	// 	#pragma omp taskwait
	// }
}


void printSol(int* sol, int length) {
	for(int i = 0; i < length; i++) {
		std::cout << sol[i] << " ";
	}
	std::cout << std::endl;
}

void printMatrx(int* matrix, int fieldSize, int zeros)
{
	for(int i = 0; i < zeros; i++) {
		for(int j = 0; j < fieldSize; j++) {
			std::cout << std::setw(1) << 
				matrix[i*fieldSize+j] 
				<< " ";
		}
		std::cout << std::endl;
	}
}


int found_sudokus = 0;

int main(int argc, char* argv[]) {
	// measure the time
	double t3, t4;

	// expect three command line arguments: field size, block size, and input file
	if (argc != 4) {
		std::cout << "Usage: sudoku1->exe <field size> <block size> <input filename>" << std::endl;
		std::cout << std::endl;
		return -1;
	}
	else {
		CSudokuBoard *sudoku1 = new CSudokuBoard(atoi(argv[1]), atoi(argv[2]));
		if (!sudoku1->loadFromFile(argv[3])) {
			std::cout << "There was an error reading a Sudoku template from " << argv[3] << std::endl;
			std::cout << std::endl;
			return -1;
		}


		// print the Sudoku board template
		std::cout << "Given Sudoku template" << std::endl;
		sudoku1->printBoard();

		//initializng some var
		// solve the Sudoku by finding (and printing) all solutions
		// int rowZero;
		// int colZero;
		// for(int i = 0; i < sudoku1->getFieldSize(); i++) {
		// 	for(int j = 0; j < sudoku1->getFieldSize(); j++) {
		// 		if(sudoku1->get(i, j) == 0) {
		// 			rowZero = i;
		// 			colZero = j;
		// 		}
		// 	}
		// }
		t3 = omp_get_wtime();
		//std::cout << rowZero << " " << colZero;
		//solver(sudoku1, 0, 0, rowZero, colZero);
		// int fieldSize = sudoku1->getFieldSize();
		// int blockSize = sudoku1->getBlockSize();
		// int zeros = 0;
		// for(int i = 0; i < fieldSize; i++) {
		// 	for(int j = 0; j < fieldSize; j++) {
		// 		if(sudoku1->get(i, j) == 0) {
		// 			zeros = zeros + 1;
		// 		}
		// 	}
		// }
		// int* combinations = new int[zeros*(fieldSize)];

		// #pragma omp parallel
		// {
		// 	#pragma omp single
		// 	{
		// 		int index = 0;
		// 		for(int i = 0; i < fieldSize; i++) {
		// 			for(int j = 0; j < fieldSize; j++) {
		// 				if(sudoku1->get(i, j) == 0) {
		// 					#pragma omp task firstprivate(fieldSize, index, i, j, combinations, sudoku1)
		// 					{
		// 						int x = 0;
		// 						for(int num = 1; num < fieldSize+1; num++) {
		// 							if(valid(sudoku1, i, j, num)) {
		// 								combinations[fieldSize*index+x] = num;
		// 								x++;
		// 							}
		// 						}
		// 						while(x != fieldSize) {
		// 							combinations[fieldSize*index+x] = 0;
		// 							x++;
		// 						}
		// 					}
		// 					index = index + 1;
		// 				}
		// 			}
		// 		}
		// 		#pragma omp taskwait
		// 		printMatrx(combinations, fieldSize, zeros);
		// 	}
		// }
		// #pragma omp parallel
		// {
		// 	#pragma omp single 
		// 	{
		// 		int* comb = new int[zeros+1];
		// 		for(int i = 1;  i < zeros; i++) {
		// 			comb[i] = 0;
		// 		}
		// 		comb[0] = -1;
		// 		comb[zeros] = 0;
		// 		bool run = true;
		// 		while(run) {
		// 			int* tmpSolution = new int[zeros];
		// 			comb[0] = comb[0]+1;
		// 			for(int i = 0; i < zeros; i++) {
		// 				if(combinations[comb[i]] == 0 || comb[i] == fieldSize) {
		// 					comb[i] = 0;
		// 					comb[i+1] = comb[i+1]+1;
		// 				}
		// 				tmpSolution[i] = comb[i];
		// 			}
		// 			if(comb[zeros] == 1) {
		// 				run = false;
		// 			}
		// 			#pragma omp task
		// 			{
		// 				printSol(tmpSolution, zeros);
		// 				int index = 0;
		// 				bool validb = true;
		// 				CSudokuBoard *tmpSudoku1 = new CSudokuBoard(*sudoku1);
		// 				for(int i = 0; i < fieldSize && validb; i++) {
		// 					for(int j = 0; j < fieldSize && validb; j++) {
		// 						if(tmpSudoku1->get(i, j) == 0) {
		// 							if(valid(tmpSudoku1, i, j, tmpSolution[index])) {
		// 								tmpSudoku1->set(i, j, tmpSolution[index]);
		// 								index++;
		// 							}
		// 							else {
		// 								validb = false;
		// 							}
		// 						}
		// 					}
		// 				}
		// 				if(validb) {
		// 					tmpSudoku1->printBoard();
		// 				}
		// 				delete tmpSudoku1;
		// 				delete tmpSolution;
		// 			}
		// 		}
		// 		#pragma omp taskwait
		// 	}

		// }



		/* 9^50 sembra troppe combinazione da riuscire a completare tutto
		int fieldSize = sudoku1->getFieldSize();
		int blockSize = sudoku1->getBlockSize();
		int zeros = 0;
		for(int i = 0; i < fieldSize; i++) {
			for(int j = 0; j < fieldSize; j++) {
				if(sudoku1->get(i, j) == 0) {
					zeros = zeros + 1;
				}
			}
		}
		int* solution = new int[zeros+1];
		solution[0] = 0;
		solution[zeros] = 0;
		for(int i = 1;  i < zeros; i++) {
			solution[i] = 1;
		}
		bool run = true;
		#pragma omp parallel sections
		{
			while(run) {
				//generate the new solution and copy to tmpSolution for thread
				int* tmpSolution = new int[zeros];
				solution[0] = solution[0]+1;
				for(int i = 0; i < zeros; i++) {
					if(solution[i] > fieldSize) {
						solution[i] = 1;
						solution[i+1] = solution[i+1] + 1;
					}
					tmpSolution[i] = solution[i];
				}
				if(solution[zeros] == 1) {
					run = false;
				}
				#pragma omp task shared(*sudoku1)
				{
					if(run) {
						//std::cout << "from " << omp_get_thread_num() << " ";
						//printSol(tmpSolution, zeros);
						//create a copy of sudoku1 and fill with solution
						CSudokuBoard *tmpSudoku1 = new CSudokuBoard(*sudoku1);
						int index = 0;
						bool found = true;
						for(int i = 0; i < fieldSize; i++) {
							for(int j = 0; j < fieldSize; j++) {
								if(tmpSudoku1->get(i, j) == 0) {
									tmpSudoku1->set(i, j, tmpSolution[index]);
									index = index+1;
								}
							}
						}
						//std::cout << "from " << omp_get_thread_num() << std::endl;
						//tmpSudoku1->printBoard();
						int* counter = new int[fieldSize];
						for(int i = 0;  i < fieldSize; i++) {
							counter[i] = 0;
						}		
						if(found) {
							std::cout << "found from " << omp_get_thread_num() << std::endl; 
							tmpSudoku1->printBoard();
						}

						//exit here
						delete tmpSolution;
						delete tmpSudoku1;
						delete counter;
					}
				}
			}
			#pragma omp taskwait
		}
		*/
		t4 = omp_get_wtime();
		
		delete sudoku1;
	}
	
	// print the time
	std::cout << "Parallel computation took " << t4 - t3 << " seconds (" 
			<< omp_get_max_threads() << " threads)." << std::endl << std::endl;

	return 0;
}
