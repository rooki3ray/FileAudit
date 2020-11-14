#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <fcntl.h>
#include <asm/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pwd.h>
#include "db.h"

#define TM_FMT "%Y-%m-%d %H:%M:%S"

#define NETLINK_TEST 29
#define TASK_COMM_LEN 16
#define MAX_LENGTH 256
#define MAX_PAYLOAD 1024  /* maximum payload size*/
int sock_fd;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct sockaddr_nl src_addr, dest_addr;
struct iovec iov;

// distinguish log from netlink socket
char *syscall_name[] = {"open", "read", "write", "close"}; 

FILE *logfile;

void LogOpen(char *commandname, int uid, int pid, char *file_path, int flags, int ret) {
	char logtime[64];
	char username[32];
	struct passwd *pwinfo;
	char result[10];

	if (ret > 0) strcpy(result,"success");
	else strcpy(result,"failed");

	time_t t=time(0);
	if (logfile == NULL)	return;
	pwinfo = getpwuid(uid);
	strcpy(username,pwinfo->pw_name);
	strftime(logtime, sizeof(logtime), TM_FMT, localtime(&t) );

	char opentype[16];
	if (flags & O_RDONLY ) strcpy(opentype, "Read");
	else if (flags & O_WRONLY ) strcpy(opentype, "Write");
	else if (flags & O_RDWR ) strcpy(opentype, "Read/Write");
	else strcpy(opentype,"other");

	// fprintf(logfile,"%s %s(%d) %s(%d) %s \"%s\" %s %s\n",syscall, username,uid,commandname,pid,logtime,file_path,opentype, result);
	printf("OPEN %s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,opentype, result);
    insert_record("OPEN", username, uid, commandname, pid, logtime, file_path, opentype, result);
}

void LogRead(char *commandname, int uid, int pid, char *file_path, int flags, int ret, char *fd_name) {
	char logtime[64];
	char username[32];
	struct passwd *pwinfo;
	char result[10];

	if (ret > 0) strcpy(result,"success");
	else strcpy(result,"failed");

	time_t t=time(0);
	if (logfile == NULL)	return;
	pwinfo = getpwuid(uid);
	strcpy(username, pwinfo->pw_name);
	strftime(logtime, sizeof(logtime), TM_FMT, localtime(&t) );

	printf("READ %s(%d) %s(%d) %s \"%s\" %d %s %s\n",username,uid,commandname,pid,logtime,file_path,flags,result,fd_name);
}

void LogClose(char *commandname, int uid, int pid, char *file_path, int flags, int ret) {
	char logtime[64];
	char username[32];
	struct passwd *pwinfo;
	char result[10];

	if (ret > 0) strcpy(result,"success");
	else strcpy(result,"failed");

	time_t t=time(0);
	if (logfile == NULL)	return;
	pwinfo = getpwuid(uid);
	strcpy(username,pwinfo->pw_name);
	strftime(logtime, sizeof(logtime), TM_FMT, localtime(&t) );

	char closetype[16];
	if (flags & O_RDONLY ) strcpy(closetype, "Read");
	else if (flags & O_WRONLY ) strcpy(closetype, "Write");
	else if (flags & O_RDWR ) strcpy(closetype, "Read/Write");
	else strcpy(closetype,"other");

	// fprintf(logfile,"%s %s(%d) %s(%d) %s \"%s\" %s %s\n",syscall, username,uid,commandname,pid,logtime,file_path,closetype, result);
	printf("CLOSE %s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,closetype, result);
    // insert_record("OPEN", username, uid, commandname, pid, logtime, file_path, closetype, result);
}


void sendpid(unsigned int pid)
{
	//Send message to initialize
	memset(&msg, 0, sizeof(msg));
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = pid;  //self pid
	src_addr.nl_groups = 0;  //not in mcast groups
	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;   //For Linux Kernel
	dest_addr.nl_groups = 0; //unicast

	/* Fill the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = pid;  /* self pid */
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	//printf(" Sending message. ...\n");
	sendmsg(sock_fd, &msg, 0);
}

void killdeal_func()
{
	printf("The process is killed! \n");
	close(sock_fd);
	if (logfile != NULL)
		fclose(logfile);
	if (nlh != NULL)
	 	free(nlh);
	exit(0);
}

int main(int argc, char *argv[]){
	char buff[110];
	//void killdeal_func();
	char logpath[32];
	if (argc == 1) strcpy(logpath,"./log");
	else if (argc == 2) strncpy(logpath, argv[1],32);
	else {
		printf("commandline parameters error! please check and try it! \n");
		exit(1);
	}

	signal(SIGTERM,killdeal_func);
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    sendpid(getpid());

	/* open the log file at the begining of daemon, in case of this operation causes deadlock */
	logfile=fopen(logpath, "w+");
	if (logfile == NULL) {
		printf("Waring: can not create log file\n");
		exit(1);
	}

    char *filename = "test.db";
    create_table(filename);

    int count = 0;
	//Loop to get message
	while(1) {	//Read message from kernel
		if (count > 20) break;
        count++;
        unsigned int uid, pid,flags,ret;
		char * flag;
		char * file_path;
		char * commandname;
		recvmsg(sock_fd, &msg, 0);
		flag = (char *)((unsigned int *)NLMSG_DATA(nlh));		
		// printf("%s\n", syscall_name[atoi(flag)]);

		if (strcmp(syscall_name[atoi(flag)], "open") == 0) {
			uid = *( 2 + (unsigned int *)NLMSG_DATA(nlh) );
			pid = *( 3 + (int *)NLMSG_DATA(nlh)  );
			flags = *( 4 + (int *)NLMSG_DATA(nlh)  );
			ret = *( 5 + (int *)NLMSG_DATA(nlh)  );
			commandname = (char *)( 6 + (int *)NLMSG_DATA(nlh));
			file_path = (char *)( 6 + TASK_COMM_LEN/4 + (int *)NLMSG_DATA(nlh));
			LogOpen(commandname, uid, pid, file_path, flags, ret);
		} 
		else if (strcmp(syscall_name[atoi(flag)], "read") == 0) {
			char *fd_name;
			uid = *( 2 + (unsigned int *)NLMSG_DATA(nlh) );
			pid = *( 3 + (int *)NLMSG_DATA(nlh)  );
			flags = *( 4 + (int *)NLMSG_DATA(nlh)  );   // read buf size
			ret = *( 5 + (int *)NLMSG_DATA(nlh)  );
			commandname = (char *)( 6 + (int *)NLMSG_DATA(nlh));
			file_path = (char *)( 6 + TASK_COMM_LEN/4 + (int *)NLMSG_DATA(nlh));
			fd_name = (char *)(6 + TASK_COMM_LEN/4 + MAX_LENGTH/4+ (int*)NLMSG_DATA(nlh) );
			// LogRead(commandname, uid, pid, file_path, flags, ret, fd_name);
			printf("flag:%s pid:%d flags:%d commandname:%s file_path:%s\n", flag, pid, flags, commandname, file_path);
		} 
		else if (strcmp(syscall_name[atoi(flag)], "close") == 0) {
			uid = *( 2 + (unsigned int *)NLMSG_DATA(nlh) );
			pid = *( 3 + (unsigned int *)NLMSG_DATA(nlh)  );
			flags = *( 4 + (unsigned int *)NLMSG_DATA(nlh)  );   // 文件描述字
			ret = *( 5 + (unsigned int *)NLMSG_DATA(nlh)  );
			commandname = (char *)( 6 + (unsigned int *)NLMSG_DATA(nlh));
			file_path = (char *)( 6 + TASK_COMM_LEN/4 + (unsigned int *)NLMSG_DATA(nlh));
			LogClose(commandname, uid, pid, file_path, flags, ret);
			// printf("flag:%s pid:%d flags:%d commandname:%s file_path:%s\n", flag, pid, flags, commandname, file_path);
		}
		
	}
	close(sock_fd);
	free(nlh);
	fclose(logfile);
    close_table();
	return 0;
}

