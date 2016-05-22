#include <stdio.h>
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/string.h>

#ifndef VMACHINE_H
#define VMACHINE_H

#define MEM_SIZE 1000

//Variaveis globais
int MEM[MEM_SIZE];
int GPR[16];
int PC;
int SP;
char PSW[2];

// Funcao responsavel por ler instrucoes de um arquivo e salva-los na memoria. 
void loadInstructions(char *buffer, int start);

// Imprime o status do sistema.
void printStatus(int opcode);

// Carrega registrador.
void LOAD(void);

// Armazena registrador.
void STORE(void);

// Le valor para registrador.
void READ(void);

// Escreve conteudo do registrador.
void WRITE(void);

// Copia registrador.
void COPY(void);

// Inverte o sinal do registrador.
void NEG(void);

// Subtrai dois registradores.
void SUB(void);

// Soma dois registradores.
void ADD(void);

// AND(bit a bit) de dois registradores.
void AND(void);

// OR(bit a bit) de dois registradores.
void OR(void);

// XOR(bit a bit) de dois registradores.
void XOR(void);

// NOT(bit a bit) de dois registradores.
void NOT(void);

// Desvio incondicional.
void JMP(void);

// Desvia se zero.
void JZ(void);

// Desvia se nao zero.
void JNZ(void);

// Desvia se negativo.
void JN(void);

// Desvia se nao negativo.
void JNN(void);

// Empilha valor do registrador.
void PUSH(void);

// Desempilha valor do registrador.
void POP(void);

// Chamada de subrotina.
void CALL(void);

// Retorno de subrotina.
void RET(void);
#endif