#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <net/sock.h>
#include "netConst.h"

struct sock *nl_sk = NULL;

//Variaveis globais
int MEM[MEM_SIZE];
int FILE[FILE_SIZE];
int GPR[REG_NUM];
int PC;
int SP;
int FP;
char PSW[2];

static void encode(char *buffer, int n){
	int i;

	for(i = 0; i < 4; i++){
		buffer[i] = BASE + (n & 0x000F);
		n = n >> 4;
	}
}

static int decode(char *buffer){
	int valor, i;

	valor = 0;
	for(i = 0; i < 4; i++)
		valor += (buffer[i] - BASE) << (i*4);

	return valor;
}

// Funcao responsavel por ler instrucoes de um arquivo e salva-los na memoria. 
static void loadInstructions(char *buffer){
	int i, j;
	int final;
	
	j = 0;
	i = 0;
	do{
		// Recuperando informacoes juntando 4 caracteres para virar um inteiro.
		MEM[i] = decode(&buffer[i*4]);
		i++;
		final = buffer[i * 4];

	} while(final != CODEF_MARKER && final != 0);
	/*
	printk(KERN_INFO "Memory content:");
	for(j = 0; j < i; j++)
		printk(KERN_INFO "%d|", MEM[j]);
	*/
}

// Carrega as informacoes do arquivo para serem utilizadas.
static int loadFileData(char *buffer){
	int i, j, k;
	
	j = 0;
	i = 0;
	while(buffer[i] != CODEF_MARKER && buffer[i] != 0)
		i++;

	if(buffer[i] == '\0' || buffer[i+1] == '\0')
		return 0;

	i++;
	do{
		// Recuperando informacoes juntando 4 caracteres para virar um inteiro.
		FILE[j] = decode(&buffer[i]);
		i += 4;
		j++;
	} while(buffer[i] != DATAF_MARKER);
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
	PC++;
	R = MEM[PC];
	PC++;
	M = MEM[PC];
	PC++;
	GPR[R] = MEM[PC + M];
	return;
}

// Armazena registrador.
static void STORE(void){
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
static void vm_READ(void){
	int R;
	PC++;
	R = MEM[PC];
	GPR[R] = FILE[FP];
	FP++;
	PC++;
	//scanf("%d", &GPR[R]);
	return;
}

// Escreve conteudo do registrador.
static char* vm_WRITE(void){
	int R;
	char buffer[MAX_SIZE];
	int dig[MAX_SIZE];
	int i, j, value;

	PC++;
	R = MEM[PC];
	PC++;
	value = GPR[R];

	encode(buffer, value);
	buffer[4] = '\0';

	printk(KERN_INFO "REG %d: %d\n", R, GPR[R]);

	return buffer;
}

// Copia registrador.
static void COPY(void){
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
static void NEG(void){
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
static void SUB(void){
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
static void ADD(void){
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
static void AND(void){
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
static void OR(void){
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
static void XOR(void){
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
static void NOT(void){
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
static void JMP(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	PC += M;
	return;
}

// Desvia se zero.
static void JZ(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(PSW[0])
		PC += M;
	return;
}

// Desvia se nao zero.
static void JNZ(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(!PSW[0])
		PC += M;
	return;
}

// Desvia se negativo.
static void JN(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(PSW[1])
		PC += M;
	return;
}

// Desvia se nao negativo.
static void JNN(void){
	int M;
	PC++;
	M = MEM[PC];
	PC++;
	if(!PSW[1])
		PC += M;
	return;
}

// Empilha valor do registrador.
static void PUSH(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	SP--;
	MEM[SP] = GPR[R];
	return;
}

// Desempilha valor do registrador.
static void POP(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	GPR[R] = MEM[SP];
	SP++;
	return;
}

// Chamada de subrotina.
static void CALL(void){
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
static void RET(void){
	PC = MEM[SP];
	SP += 1;
	return;
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
	PSW[0] = 0; PSW[1] = 0;
}

static char* exec_machine(int hasFile){
	int simples, MP, end;
	char mesg[MSG_SIZE];
	
	// Rotina de inicializacao:
	end = 0;
	MP = 0;				//atoi(argv[3]);
	simples = 1;

	// Executa o programa até receber uma inst. de HALT.
	while(!end){
		if(PC >= MEM_SIZE){
			printk(KERN_ERR "ERROR: INVALID MEMORY POSITION.\n");
			return NULL;
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
						return NULL;
					}
					break;
			case 4:	strcat(mesg, vm_WRITE());
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
				printk(KERN_ERR "ERROR: (%d) IS AN INVALID OPCODE.\n", MEM[PC]);
				return NULL;
				//end = 1;
				break;				
		}
	}
	
	printk(KERN_INFO "<%s>\n", mesg);
	return mesg;
}

// --------------------------- Main code ------------------------------
static void aquaman_vm_recv_msg(struct sk_buff *skb) {
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int pid, msg_size, res, hasFile;
	char *received, *msgToSend;
	char *msg = " ";

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	nlh=(struct nlmsghdr*)skb->data;
	printk(KERN_INFO "Code received.");
	pid = nlh->nlmsg_pid; /*pid of sending process */
	received = (char*)nlmsg_data(nlh);

	// ------------------Executing the code----------------------------
	init_machine();
	loadInstructions(received);
	hasFile = loadFileData(received);
	msgToSend = exec_machine(hasFile);
	if(msgToSend != NULL)
		msg = msgToSend;

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

MODULE_LICENSE("GPL");
