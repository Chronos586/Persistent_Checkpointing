#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libpmemlog.h>

/* size of the pmemlog pool -- 1 GB */
#define	POOL_SIZE ((off_t)(1 << 30))

/*
 * printit -- log processing callback for use with pmemlog_walk()
 */
PMEMlogpool* static_plp;

int
printit(const void *buf, size_t len, void *arg)
{
	fwrite(buf, len, 1, stdout);
	return 0;
}

PMEMlogpool* create_event_pool(char* path){
	PMEMlogpool* plp;
	printf("PATH IS %s",path);
	printf("\n");
	printf("BEFORE EVENT POOL CREATE\n");
	plp = pmemlog_create(path, POOL_SIZE, 0666);
	if(plp == NULL)
	   plp = pmemlog_open(path);

	if(plp == NULL){
	   perror(path);
	   exit(1);
	}
	static_plp = plp;
	printf("LEAVE EVENT POOL\n");
	printf("PLP IN C IS %d\n",plp);
	pmemlog_close(plp);
	return plp; //(void*) plp;
}
void constant_char(const char* ptr){
	printf("PTR VALUE IS:  %s\n",ptr);
}
void validate_ptr(PMEMlogpool* plp){
     printf("PLP Passed IN is %d\n",plp);
}
void num_bytes(PMEMlogpool* plp, char* path){
     //int bytes = pmemlog_nbyte(plp);
     plp = pmemlog_open(path);
     printf("OPENED PLP %d\n",plp);
     int bytes2 = pmemlog_nbyte(plp);
     //printf("BYTES IS %d",bytes);
     printf("BYTES IS %d\n",bytes2);
     pmemlog_close(plp);
}
void append_str(PMEMlogpool* plp, char* str,char* path){
     plp = pmemlog_open(path);
     printf("ENTER APPEND STR\n");
     //plp = create_event_pool(path);
     if(pmemlog_append(plp,str,strlen(str)) < 0){
	    perror("pmemlog_append");
	    exit(1);
     }
     pmemlog_close(plp);
     printf("LEAVE APPEND STR\n");
}
void close_pool(const PMEMlogpool* plp,const char* path){
     //plp = create_event_pool(path);
     pmemlog_close(plp);
}
void print_log(const PMEMlogpool* plp,const char* path){
     plp = pmemlog_open(path);
     pmemlog_walk(plp,0,printit,NULL);
}
int
main(int argc, char *argv[])
{
	const char path[] = "./pmem-fs/myfile";
	PMEMlogpool *plp;
	size_t nbyte;
	char *str;

	/* create the pmemlog pool or open it if it already exists */
	plp = pmemlog_create(path, POOL_SIZE, 0666);

	if (plp == NULL)
	    plp = pmemlog_open(path);

	if (plp == NULL) {
		perror(path);
		exit(1);
	}

	/* how many bytes does the log hold? */
	nbyte = pmemlog_nbyte(plp);
	printf("log holds %zu bytes\n", nbyte);

	/* append to the log... */
	str = "This is the first string appended\n";
	if (pmemlog_append(plp, str, strlen(str)) < 0) {
		perror("pmemlog_append");
		exit(1);
	}
	str = "This is the second string appended\n";
	if (pmemlog_append(plp, str, strlen(str)) < 0) {
		perror("pmemlog_append");
		exit(1);
	}

	/* print the log contents */
	printf("log contains:\n");
	pmemlog_walk(plp, 0, printit, NULL);

	pmemlog_close(plp);
}
