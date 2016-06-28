#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <net/sock.h>
#include <linux/vmalloc.h>
#include "netConst.h"

#define PCelula struct TCelula*
#define MAX_VETORES 16
#define MAX_MEM 1024576
//MODULE_LICENSE("GPL");

struct sock *nl_sk = NULL;

int MEM[MEM_SIZE];
int FILE[FILE_SIZE];
int GPR[REG_NUM];
int PC;
int SP;
int FP;
char PSW[2];
int     MSW;

//------------------------------------------------------------------------------------------------------------------------------------
typedef struct{
    int* endReal;
    int tamMaximo;
}Tabela;

Tabela TABELA[MAX_VETORES];

unsigned char REG_VET[MAX_VETORES];//banco de registradores para guardar endereços virtuais de vetores

//------------------------------------------------------------------------------------------------------------------------------------
//valloc

typedef struct TCelula{
    int inicio;
    int comprimento;
    PCelula proximo;
}TCelula;

TCelula Cabeca;

unsigned char Mem[MAX_MEM];

void inicializa_gerencia(){
    Cabeca.comprimento = 0;
    Cabeca.inicio = 0;
    Cabeca.proximo = NULL;
}

void finaliza_gerencia(){
    while(Cabeca.proximo!=NULL)
        vafree(&Mem[Cabeca.proximo->inicio]);

}

void Inserir(PCelula Anterior, int comprimento, int inicio){
     PCelula Novo = (PCelula)vmalloc(sizeof(TCelula));
     Novo->comprimento = comprimento;
     Novo->inicio = inicio;
     Novo->proximo = Anterior->proximo;
     Anterior->proximo = Novo;
}

void Eliminar(PCelula Anterior){
    PCelula Atual = Anterior->proximo;
    Anterior->proximo = Atual->proximo;
    vfree(Atual);
}


int Conversor(void* endereco){return((long int)endereco - (long int) &Mem[0]);}


void imprime_status_memoria(){
     PCelula A = Cabeca.proximo, *B = &Cabeca;
     int i;

     printk("Status agora:\n");

     while(A!=NULL){
        i = (A->inicio)-(B->inicio+B->comprimento);
        if(i!=0)printk(KERN_ERR "Pos: %d, Size: %d, Status: FREE\n",(B->inicio+B->comprimento),i);
        printk(KERN_ERR "Pos: %d, Size: %d, Status: USED\n", A->inicio, A->comprimento);
        B = A;
        A = A->proximo;
    }

     i = MAX_MEM - (B->inicio+B->comprimento);
     if(i>0)printk(KERN_ERR "Pos: %d, Size: %d, Status: FREE\n",(B->inicio+B->comprimento),i);

}


void* valloc(size_t size){
     if(size<=0)return NULL;
     PCelula A = Cabeca.proximo, *B = &Cabeca;
     while(A!=NULL){
        if(((A->inicio)-(B->inicio+B->comprimento))>=size){
            Inserir(B, size, (B->inicio+B->comprimento));
            return(&Mem[(B->inicio+B->comprimento)]);
        }
        B = A;
        A = A->proximo;
        }

     if(MAX_MEM - (B->inicio+B->comprimento)>=size){
        Inserir(B, size, (B->inicio+B->comprimento));
        return(&Mem[(B->inicio+B->comprimento)]);
    }
     return NULL;

}

void* vcalloc(size_t nitems, size_t size){
     char* c = (char*)valloc(nitems*size);

     if(c==NULL)return NULL;

     int i, d = Conversor(c);

     if((d<0)||(d>MAX_MEM-1))return NULL;

     for(i=d; i<d+nitems*size;i++)Mem[i] = 0;

     return c;

}

void* vrealloc(void *p, size_t size){
      PCelula A = Cabeca.proximo, *B = &Cabeca;
      int i = Conversor(p),j,k,l;

      if(((i<0)||(i>MAX_MEM-1))&&(p!=NULL))return NULL;//Elimina ponteiros fora do MEM

      char *c;

      if(size==0){

                  vafree(p);

                  return NULL;

                  }

      if(p==NULL)return(vcalloc(size,1));

      while((A!=NULL)&&(A->inicio!=i)){

                                       B = A;

                                       A = A->proximo;

                                       }

      if(A==NULL)return NULL;//Desnecessario, mas so por garantia

      l = A->comprimento;

      //Verifica se há espaço à frente do vetor original ou se o vetor está sendo diminuido

      k = (A->proximo==NULL)?MAX_MEM:A->proximo->inicio;

      if((size<=l)||((k-(A->inicio))>= size)){

                 A->comprimento = size;

                 return p;

                 }

      B->proximo = A->proximo;

      c = (char*)vcalloc(size,1);

      if(c==NULL){

                  B->proximo = A;

                  return NULL;

                  }else{

                        j = Conversor(c);

                        for(k=0;k<l;k++) Mem[j+k] = Mem[i+k];

                        vfree(A);

                        return c;

                        }

}

void vafree(void* endereco){

    PCelula A = Cabeca.proximo, *B = &Cabeca;

    int end = Conversor(endereco);

    if((end<0)||(end>MAX_MEM-1))return;

    while((A!=NULL)&&(A->inicio!=end)){

                                       B = A;

                                       A = A->proximo;

                                       }

    if(A!=NULL){

                B->proximo = A->proximo;

                vfree(A);

                }

}

//------------------------------------------------------------------------------------------------------------------------------------
//vet - vetor dinamico
void inicializaTabela(){

    int i;

    //Deixa todas as entradas da tabela inicialmente inválidas

    for(i=0;i<MAX_VETORES;i++){

        TABELA[i].endReal = NULL;

    }

}



void inicializaRegVet(){

    int i;

    //Deixa todas as entradas da tabela inicialmente inválidas

    for(i=0;i<MAX_VETORES;i++){

        REG_VET[i]=-1;

    }

}



int insereNaTabela(int endereco, int* endReal, int tamanho){

    int i;

    if((endereco<0)||(endereco>=MAX_VETORES))return -1;

    if(TABELA[endereco].endReal==NULL){

            TABELA[endereco].endReal  = endReal;

            TABELA[endereco].tamMaximo = tamanho;

            return i;

    }

    return -1;

}



int retireDaTabela(int endereco){

    int i;

    if((endereco<0)||(endereco>=MAX_VETORES))return -1;

    if(TABELA[endereco].endReal!=NULL){

            TABELA[endereco].endReal = NULL;

            return endereco;

    }

    return -1;

}



//Rotorna o valor do elemento de determinado index em um vetor

int get(int endereco, int offset){

    if((endereco<0)||(endereco>=MAX_VETORES)){

        MSW = MSW|(1<<2);//terceiro bit assinalado pra indicar endereço inválido

        return 0;

    }

    if((offset<0)||(offset>=TABELA[endereco].tamMaximo)){

        MSW = MSW|(1<<3);//quarto bit assinalado pra indicar offset errado

        return 0;

    }

    if(TABELA[endereco].endReal==NULL){

        MSW = MSW|(1<<4);//quinto bit assinalado pra indicar vetor não declarado

        return 0;

    }

    MSW = MSW & ~((1<<2)|(1<<3)|(1<<4));//terceiro, quarto e quinto bits não assinalados, ocorreu tudo bem

    return *((TABELA[endereco].endReal)+offset);

}



//Define o valor do elemento de determinado index em um vetor

void set(int endereco, int offset, int valor){

    int* nAddr;



    if((endereco<0)||(endereco>=MAX_VETORES)){

        MSW = MSW|(1<<2);//terceiro bit assinalado pra indicar endereço inválido

        return 0;

    }



    if(TABELA[endereco].endReal==NULL){//Se o vetor não tiver sido declarado,

        nAddr = (int*) vcalloc((offset+1)*2,sizeof(int));

        insereNaTabela(endereco,nAddr, (offset+1)*2);

    }



    if(offset<0){

        MSW = MSW|(1<<3);//quarto bit assinalado pra indicar offset errado

        return;

    }

    if(offset>=TABELA[endereco].tamMaximo){//Se tentar acessar um valor além do limite de tamanho,

        TABELA[endereco].endReal=vrealloc(TABELA[endereco].endReal, sizeof(int)*(offset+1)*2);

        TABELA[endereco].tamMaximo = (offset+1)*2;

    }



    //Posição existente no momento, realiza a atribuição

    *((TABELA[endereco].endReal)+offset) = valor;

}
//------------------------------------------------------------------------------------------------------------------------------------
//coletor de lixo
void coletarLixo(){

    unsigned char Apagar[MAX_VETORES];

    int i;

    for(i=0;i<MAX_VETORES;i++){

        Apagar[i]=1;

    }



    //Verifica quais vetores estão sendo referenciados ainda

    for(i=0;i<MAX_VETORES;i++){

        if((REG_VET[i]>=0)||(REG_VET[i]<MAX_VETORES)){//se index válido

            Apagar[REG_VET[i]] = 0;//Não apague este.

        }

    }



    //Apaga o que estiver dangling

    for(i=0;i<MAX_VETORES;i++){

        if(Apagar[i]){

            if(TABELA[i].endReal!=NULL){//Se já tiver sido alocado

                vafree(TABELA[i].endReal);//Desaloca

                retireDaTabela(i);

            }

        }

    }

}
//------------------------------------------------------------------------------------------------------------------------------------

static void exit(int num){
	panic("ERRO");
}

static void error(char *msg){
	printk(KERN_ERR "Error: %s\n", msg);
	exit(1);
}

// Codifica um inteiro em 4 chars para serem enviados.
static void encode(char *buffer, int n){
	int i;

	for(i = 0; i < 4; i++){
		buffer[i] = BASE + (n & 0x000F);
		n = n >> 4;
	}
}

// Decodifica 4 chars em 1 inteiro.
static int decode(char *buffer){
	int valor, i;

	valor = 0;
	for(i = 0; i < 4; i++)
		valor += (buffer[i] - BASE) << (i*4);

	return valor;
}

// Verifica se o PC estourou os limites da memoria.
static void check_PC(){
	if(PC >= MEM_SIZE)
		error("MEMORY VIOLATION. (PC)");
}

// Incrimenta o PC, verificando se nao estourou o limite.
static void inc_and_check_PC(){
	PC++;
	check_PC();
}

// Funcao responsavel por ler instrucoes de um arquivo e salva-los na memoria. 
static void loadInstructions(char *buffer, int buffer_size){
	int i, j, isNeg;
	int final;
	
	j = 0;
	i = 0;
	do{
		// Recuperando informacoes juntando 4 caracteres para virar um inteiro.
		isNeg = buffer[i*5] - BASE;
		MEM[i] = decode(&buffer[i*5+1]);
		if(isNeg)
			MEM[i] = -MEM[i];

		i++;

		if(i*4+1 >= buffer_size)
			error("OUT OF MESSAGE. (LOAD_INST)");

		final = buffer[i * 5];

	} while(final != CODEF_MARKER && final != 0 && i < MEM_SIZE);

	if(i >= MEM_SIZE)
		error("MEMORY VIOLATION. (LOAD_INST)");

	/**/
	printk(KERN_INFO "Memory content:");
	for(j = 0; j < i; j++)
		printk(KERN_INFO "%d|", MEM[j]);
	
}

// Carrega as informacoes do arquivo para serem utilizadas.
static int loadFileData(char *buffer, int buffer_size){
	int i, j, k;
	
	j = 0;
	i = 0;
	while(i < buffer_size &&buffer[i] != CODEF_MARKER && buffer[i] != 0)
		i++;

	if(buffer[i] == '\0' || buffer[i+1] == '\0' || i ==buffer_size)
		return 0;

	i++;
	do{
		// Recuperando informacoes juntando 4 caracteres para virar um inteiro.
		if(i + 4 >= buffer_size)
			error("OUT OF MESSAGE. (LOAD_FILE)");

		FILE[j] = decode(&buffer[i]);
		i += 4;
		j++;
	} while(j < FILE_SIZE && buffer[i] != DATAF_MARKER);
	
	if(j >= FILE_SIZE)
		error("MEMORY VIOLATION. (LOAD_FILE)");
	
	/*
	printk(KERN_INFO "File content:");
	for(i = 0; i < j; i++)
		printk(KERN_INFO "%d|", FILE[i]);
	*/
	return 1;
}

// Imprime o status do sistema.
static void printStatus(int opcode){
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
static void LOAD(void){
	int R, M;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	GPR[R] = MEM[PC + M];
	return;
}

// Armazena registrador.
static void STORE(void){
	int R, M;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	MEM[PC + M] = GPR[R];
	return;
}

// Le valor para registrador.
static void vm_READ(void){
	int R;
	inc_and_check_PC();
	R = MEM[PC];
	GPR[R] = FILE[FP];
	FP++;
	inc_and_check_PC();
	return;
}

// Escreve conteudo do registrador.
static void vm_WRITE(char *buffer){
	int R;
	int i, j, value;

	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
	value = GPR[R];

	// Transformando inteiro em 4 caracteres para enviar.
	encode(buffer, value);
	buffer[4] = '\0';

	printk(KERN_INFO "REG %d: %d\n", R, GPR[R]);
}

// Copia registrador.
static void COPY(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void NEG(void){
	int R;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
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
static void SUB(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void ADD(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void AND(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void OR(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void XOR(void){
	int R1, R2;
	inc_and_check_PC();
	R1 = MEM[PC];
	inc_and_check_PC();
	R2 = MEM[PC];
	inc_and_check_PC();
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
static void NOT(void){
	int R;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
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
static void JMP(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	PC += M;
	check_PC();
	return;
}

// Desvia se zero.
static void JZ(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	if(PSW[0]){
		PC += M;
		check_PC();
	}
	return;
}

// Desvia se nao zero.
static void JNZ(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	if(!PSW[0]){
		PC += M;
		check_PC();
	}
	return;
}

// Desvia se negativo.
static void JN(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	if(PSW[1]){
		PC += M;
		check_PC();
	}
	return;
}

// Desvia se nao negativo.
static void JNN(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	if(!PSW[1]){
		PC += M;
		check_PC();
	}
	return;
}

// Empilha valor do registrador.
static void PUSH(void){
	int R;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();
	SP--;
	if(SP <= PC)
		error("INVALID STACK POSITION. (PUSH)");
	
	MEM[SP] = GPR[R];
	return;
}

// Desempilha valor do registrador.
static void POP(void){
	int R;
	inc_and_check_PC();
	R = MEM[PC];
	inc_and_check_PC();

	if(SP <= PC || SP >= MEM_SIZE)
		error(KERN_ERR "INVALID STACK POSITION. (POP)");

	GPR[R] = MEM[SP];
	SP++;

	if(SP >= MEM_SIZE)
		error("INVALID STACK POSITION. (POP)");

	return;
}

// Chamada de subrotina.
static void CALL(void){
	int M;
	inc_and_check_PC();
	M = MEM[PC];
	inc_and_check_PC();
	SP--;
	if(SP <= PC)
		error("INVALID STACK POSITION. (CALL)");
	
	MEM[SP] = PC;
	PC += M;
	check_PC();
	return;
}

// Retorno de subrotina.
static void RET(void){
	PC = MEM[SP];
	SP += 1;
	if(SP >= MEM_SIZE)
		error("INVALID STACK POSITION. (RET)");
	
	return;
}

//
static void GET(void){
	int address, offset, reg;
	inc_and_check_PC();
	address = MEM[PC];
	inc_and_check_PC();
	offset = MEM[PC];
	inc_and_check_PC();
	reg = MEM[PC];
	inc_and_check_PC();

	GPR[reg] = get(REG_VET[address], offset);
}

//
static void SET(void){
	int address, offset, value;
	inc_and_check_PC();
	address = MEM[PC];
	inc_and_check_PC();
	offset = MEM[PC];
	inc_and_check_PC();
	value = MEM[PC];
	inc_and_check_PC();

	set(REG_VET[address], offset, value);
}

//
static void SETP(void){
	int reg, pointer;
	inc_and_check_PC();
	reg = MEM[PC];
	inc_and_check_PC();
	pointer = MEM[PC];
	inc_and_check_PC();

	REG_VET[pointer] = GPR[reg];
}

// Inicia a maquina zerando todas as variaveis.
static void init_machine(){
	int i;

	for(i = 0; i < MEM_SIZE; i++)
		MEM[i] = 0;

	for(i = 0; i < REG_NUM; i++)
		GPR[i] = 0;

    for(i = 0; i < FILE_SIZE; i++)
        FILE[i] = 0;

	PC = 0;
	FP = 0;
	SP = MEM_SIZE - 1;
	PSW[0] = PSW[1] = 0;
}

static void exec_machine(int hasFile, char *mesg){
	int simples, MP, end, i, j, inicio;
    char value[5];
	
	// Rotina de inicializacao:
	end = 0;
	MP = 0;				
	simples = 1;
    mesg[0] = '\0';
    inicio = 0;

	// Executa o programa até receber uma inst. de HALT.
	while(!end){
		if(PC >= MEM_SIZE){
			printk(KERN_ERR "ERROR: INVALID MEMORY POSITION.\n");
			mesg[0] = '\0';
		    return;
		}

		// Caso o modo definido seja o 'verbose', imprimir status.
		if(!simples){
			printStatus(MEM[PC]);
		}

		switch(MEM[PC]){
			case 1:	LOAD();
					break;
			case 2:	STORE();
					break;
			case 3:	if(hasFile)
						vm_READ();
					else{
						printk(KERN_ERR "ERROR: THERE IS NO FILE TO READ.\n");
                        mesg[0] = '\0';
						return;
					}
					break;
			case 4:	vm_WRITE(&value); 
                    for(j = 0; j < 4; j++)
                        mesg[inicio + j] = value[j];
      
                    inicio += 4; 
                    mesg[inicio] = '\0'; 
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
			case 23:SETP();
					break;
            case 24:GET();
            		break;
            case 25:SET();
            		break;
			default:
				printk(KERN_ERR "ERROR: (%d) IS AN INVALID OPCODE.\n", MEM[PC]);
				mesg[0] = '\0';
			    return;
				break;				
		}
	}
	
	printk(KERN_INFO "<%s>\n", mesg);
}

// --------------------------- Main code ------------------------------
static void aquaman_vm_recv_msg(struct sk_buff *skb) {
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int pid, msg_size, res, hasFile;
	char *received, msg[MSG_SIZE];

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	nlh=(struct nlmsghdr*)skb->data;
	printk(KERN_INFO "Code received.");
	pid = nlh->nlmsg_pid; /*pid of sending process */
	received = (char*)nlmsg_data(nlh);

	// ------------------Executing the code----------------------------
	init_machine();
	loadInstructions(received, strlen(received));
	hasFile = loadFileData(received, strlen(received));

	exec_machine(hasFile, &msg[1]);
    // First position is the lenght of the message to send.
    msg[0] = BASE + strlen(&msg[1]);
    msg[strlen(msg)] = '\0';

	//strncpy(msg, msgToSend, strlen(msgToSend));

	printk(KERN_INFO ">%s - %d\n", msg, strlen(msg));
	// --------------------Sending the answer--------------------------
	msg_size=strlen(msg);
	skb_out = nlmsg_new(msg_size,0);

	if(!skb_out){
	    printk(KERN_ERR "Failed to allocate new skb\n");
	    return;
	} 

	nlh=nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);  
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
	strncpy(nlmsg_data(nlh),msg,msg_size);

	res=nlmsg_unicast(nl_sk,skb_out,pid);

	if(res<0)
	    printk(KERN_INFO "Error while sending bak to user\n");
}

static int __init hello_init(void) {

	printk("Entering: %s\n",__FUNCTION__);
	//This is for 3.6 kernels and above.
	struct netlink_kernel_cfg cfg = {
	    .input = aquaman_vm_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	//nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg,NULL,THIS_MODULE);
	if(!nl_sk){
	    printk(KERN_ALERT "Error creating socket.\n");
	    return -10;
	}

	return 0;
}

static void __exit hello_exit(void) {
	printk(KERN_INFO "exiting hello module\n");
	netlink_kernel_release(nl_sk);
}

module_init(hello_init); 
module_exit(hello_exit);
