#include "vmachine.h"

// Funcao responsavel por ler instrucoes de um arquivo e salva-los na memoria. 
void loadInstructions(char *buffer, int start){
	int i;
	
	i = start;
	// Salva as instrucoes a serem realizadas na memoria.
	while(1){
		if(EOF == sscanf(fp, "%d\n", &memory[i]))
			break;
		
		if(memory[i] == 1 || memory[i] == 2 || memory[i] == 5 ||
		   memory[i] == 7 || memory[i] == 8 || memory[i] == 9 ||
		   memory[i] == 10 || memory[i] == 11){
		    // Estas instrucoes possuem dois operandos.
		    i++;
			fscanf(fp, "%d\n", &memory[i]);
			i++;
			fscanf(fp, "%d\n", &memory[i]);
		} else if(memory[i] == 21 || memory[i] == 22){
			// Estas intrucoes nao possuem operando.
		} else{
			// As demais possuem somente um operando.
			i++;
			fscanf(fp, "%d\n", &memory[i]);
		}
		   
		i++;	
	}
	
	return;
}

// Imprime o status do sistema.
void printStatus(int opcode){
	int i;

	printk(KERN_INFO "PC(%d) SP(%d) PSW(%d,%d) instrucao(%d)\nRegistradores: ", 
		    PC, SP, PSW[0], PSW[1], opcode);
	// Imprimindo valores no banco de registradores.
	for(i = 0; i < 15; i++){
		printk(KERN_INFO "R%d(%d) ", i, GPR[i]);
	}

	printk(KERN_INFO "R%d(%d)\n", i, GPR[i]);

	return;
}

// Carrega registrador.
void LOAD(void){
	int R, M;
	PC++;
	R = MEM[PC];
	PC++;
	M = MEM[PC];
	PC++;
	GPR[R] = MEM[PC + M];
	return;
}

// Armazena registrador.
void STORE(void){
	int R, M;
	PC++;
	R = MEM[PC];
	PC++;
	M = MEM[PC];
	PC++;
	MEM[PC + M] = GPR[R];
	return;
}

// Le valor para registrador.
void READ(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	scanf("%d", &GPR[R]);
	return;
}

// Escreve conteudo do registrador.
void WRITE(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	printk(KERN_INFO "%d\n", GPR[R]);
	return;
}

// Copia registrador.
void COPY(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] = GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}
		
	return;
}

// Inverte o sinal do registrador.
void NEG(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	GPR[R] = -GPR[R];
	// Atualização do PSW.
	if(GPR[R] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// Subtrai dois registradores.
void SUB(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] -= GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// Soma dois registradores.
void ADD(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] += GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// AND(bit a bit) de dois registradores.
void AND(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] &= GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// OR(bit a bit) de dois registradores.
void OR(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] |= GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// XOR(bit a bit) de dois registradores.
void XOR(void){
	int R1, R2;
	PC++;
	R1 = MEM[PC];
	PC++;
	R2 = MEM[PC];
	PC++;
	GPR[R1] ^= GPR[R2];
	// Atualização do PSW.
	if(GPR[R1] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R1] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// NOT(bit a bit) de dois registradores.
void NOT(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	GPR[R] = ~GPR[R];
	// Atualização do PSW.
	if(GPR[R] == 0){
		PSW[0] = 1;
		PSW[1] = 0; 
	} else if(GPR[R] < 0){
		PSW[0] = 0;
		PSW[1] = 1;
	} else{
		PSW[0] = 0;
		PSW[1] = 0;
	}

	return;
}

// Desvio incondicional.
void JMP(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	PC += M;
	return;
}

// Desvia se zero.
void JZ(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(PSW[0])
		PC += M;
	return;
}

// Desvia se nao zero.
void JNZ(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(!PSW[0])
		PC += M;
	return;
}

// Desvia se negativo.
void JN(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(PSW[1])
		PC += M;
	return;
}

// Desvia se nao negativo.
void JNN(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(!PSW[1])
		PC += M;
	return;
}

// Empilha valor do registrador.
void PUSH(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	SP--;
	MEM[SP] = GPR[R];
	return;
}

// Desempilha valor do registrador.
void POP(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	GPR[R] = MEM[SP];
	SP++;
	return;
}

// Chamada de subrotina.
void CALL(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	SP--;
	MEM[SP] = PC;
	PC += M;
	return;
}

// Retorno de subrotina.
void RET(void){
	PC = MEM[SP];
	SP += 1;
	return;
}