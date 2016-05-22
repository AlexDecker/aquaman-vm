#include <stdlib.h>
#include <stdio.h>
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include "vmachine.h"

int exec_machine(char *program){
	
	int simples, MP, end;
	
	// Rotina de inicializacao:
	end = 0;
	PC = 0;				//atoi(argv[1]);
	SP = MEM_SIZE - 1;	//atoi(argv[2]);
	MP = 0;				//atoi(argv[3]);
	//prog = fopen(argv[4], "r");
	/* MODO VERBOSE
	if(argv[5] == NULL || argv[5][0] == 's')
		simples = 1;
	else 
		simples = 0;*/

	// Carregando o programa para a memoria.
	loadInstructions(program, MP);

	// Executa o programa até receber uma inst. de HALT.
	while(!end){
		// Caso o modo definido seja o 'verbose', imprimir status.
		if(!simples){
			printStatus(MEM[PC]);
		}

		switch(MEM[PC]){
			case 1:	LOAD();
					break;
			case 2:	STORE();
					break;
			case 3:	READ();
					break;
			case 4:	WRITE();
					break;
			case 5:	COPY();				
					break;
			case 6:	NEG();
					break;
			case 7:	SUB();
					break;
			case 8:	ADD();
					break;
			case 9:	AND();
					break;
			case 10:OR();
					break;
			case 11:XOR();
					break;
			case 12:NOT();				
					break;
			case 13:JMP();
					break;
			case 14:JZ();
					break;
			case 15:JNZ();
					break;
			case 16:JN();
					break;
			case 17:JNN();
					break;
			case 18:PUSH();
					break;
			case 19:POP();
					break;
			case 20:CALL();
					break;
			case 21:RET();
					break;
			case 22:end = 1;
					break;
			default:
				printk(KERN_INFO "ERROR: (%d) IS AN INVALID OPCODE.\n", MEM[PC]);
				end = 1;
				break;				
		}
	}
	
	return 0;
}