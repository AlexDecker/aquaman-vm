#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
//#include "vmachine.h"

#define NETLINK_USER 31

struct sock *nl_sk = NULL;

#define MEM_SIZE 1000

//Variaveis globais
int MEM[MEM_SIZE];
int GPR[16];
int PC;
int SP;
char PSW[2];

// Funcao responsavel por ler instrucoes de um arquivo e salva-los na memoria. 
void loadInstructions(char *buffer){
	int i = 0;
	signed char final = -1;
	
	// Salva as instrucoes a serem realizadas na memoria.
	while(buffer[i] != final){
		MEM[i] = (int)(buffer[i]);		   
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
/*
void READ(void){
	int R;
	PC++;
	R = MEM[PC];
	PC++;
	//scanf("%d", &GPR[R]);
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
*/

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

static int exec_machine(char *program){
	
	int simples, MP, end;
	
	// Rotina de inicializacao:
	end = 0;
	PC = 0;				//atoi(argv[1]);
	SP = MEM_SIZE - 1;	//atoi(argv[2]);
	MP = 0;				//atoi(argv[3]);

	// Carregando o programa para a memoria.
	loadInstructions(program);

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
			case 3:	//READ();
					break;
			case 4:	//WRITE();
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

static void hello_nl_recv_msg(struct sk_buff *skb) {

	struct nlmsghdr *nlh;
	int pid;
	struct sk_buff *skb_out;
	int msg_size;
	char *msg="Hello from kernel";
	int res;

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	msg_size=strlen(msg);

	nlh=(struct nlmsghdr*)skb->data;
	printk(KERN_INFO "Netlink received msg payload:%s\n",(char*)nlmsg_data(nlh));
	pid = nlh->nlmsg_pid; /*pid of sending process */

	exec_machine((char*)nlmsg_data(nlh));

	skb_out = nlmsg_new(msg_size,0);

	if(!skb_out)
	{

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
	    .input = hello_nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	//nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg,NULL,THIS_MODULE);
	if(!nl_sk)
	{

	    printk(KERN_ALERT "Error creating socket.\n");
	    return -10;

	}

	return 0;
}

static void __exit hello_exit(void) {
	printk(KERN_INFO "exiting hello module\n");
	netlink_kernel_release(nl_sk);
}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");