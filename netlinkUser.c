
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netConst.h"

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
struct msghdr msg;
int sock_fd;

void error(char *msg);

int main(int argc, char *argv[]){
	FILE *fpCode = NULL;
	FILE *fpData = NULL;
	int i, lim, data, cap;
	char *fCode, *fData, *buffer, *aux;
	char c;

	// Reading the main code file.
	if(argc < 2)
		error("Missing the code file.");
	

	fCode = argv[1];
	fpCode = fopen(fCode, "r");
	if(fpCode == NULL){
		sprintf(aux, "File %s not found.", fCode);
		error(aux);
	}

	// Creating the content of the message.
	cap = MAX_BUFFER;
	buffer = (char *)malloc(cap * sizeof(char));
	if(buffer == NULL)
		error("Memory allocation error.");


	i = 0;
	lim = 0;
	while(1){
		if(EOF == fscanf(fpCode, "%d\n", &data))
			break;

		// A interger generates 4 characters.
		lim = i + 4;
		for(i; i < lim; i++){
			buffer[i] = 65 + (data & 0x000F);
			data = data >> 4;
		}

		// Double the size of the buffer if it is filled.
		if(lim + 4 >= MAX_BUFFER){
			cap *= 2;
			buffer = (char *)realloc(buffer, cap);
			if(buffer == NULL)
				error("Memory allocation error.");
					
		}
	}

	buffer[i] = CODEF_MARKER;
	i++;

	// If there is a data file, then it will be added in the end of te message.
	if(argc == 3){
		// Marking the end of the code.
		fData = argv[2];

		fpData = fopen(fData, "r");
		if(fpData == NULL){
			sprintf(aux, "File %s not found.", fData);
			error(aux);
		}

		lim = 0;
		while(1){
			if(EOF == fscanf(fpData, "%d\n", &data))
				break;

			// A interger generates 4 characters.
			lim = i + 4;
			for(i; i < lim; i++){
				buffer[i] = 65 + (data & 0x000F);
				data = data >> 4;
			}

			// Double the size of the buffer if it is filled.
			if(lim + 4 >= MAX_BUFFER){
				cap *= 2;
				buffer = (char *)realloc(buffer, cap);
				if(buffer == NULL)
					error("Memory allocation error.");

			}
		}

		buffer[i] = DATAF_MARKER;
		i++;
	}

	buffer[i] = '\0';

	/*int j;
	for(j = 0; j < i; j++)
		printf("%d\n", buffer[j]);
	printf(">%d\n", i);*/
	//----------------Sending the message------------------------------
	sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if(sock_fd<0)
		return -1;

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); // self pid 

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel 
	dest_addr.nl_groups = 0; // unicast 

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), buffer);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("Sending message to kernel\n");
	sendmsg(sock_fd,&msg,0);
	printf("Waiting for message from kernel\n");

	// Read message from kernel 
	recvmsg(sock_fd, &msg, 0);
	printf("RESULT RECEIVED: %s\n", (char *)NLMSG_DATA(nlh));
	close(sock_fd);

	if(fpData != NULL) 
		fclose(fpData);
	fclose(fpCode);
	return 0;
}

void error(char *msg){
	printf("Error: ");
	printf("%s\n", msg);
	exit(1);
}