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

int
printit(const void *buf, size_t len, void *arg)
{
	fwrite(buf, len, 1, stdout);
	return 0;
}

PMEMlogpool* create_event_pool(char* path){
	PMEMlogpool* plp;
	plp = pmemlog_create(path, POOL_SIZE, 0666);
	if(plp == NULL)
	   plp = pmemlog_open(path);

	if(plp == NULL){
	   perror(path);
	   exit(1);
	}
	pmemlog_close(plp);
	return plp; //(void*) plp;
}
void validate_ptr(PMEMlogpool* plp){
     printf("PLP Passed IN is %d\n",plp);
}
void num_bytes(char* path){
     PMEMlogpool* plp = pmemlog_open(path);
     int bytes = pmemlog_nbyte(plp);
     printf("BYTES IS %d\n",bytes);
     pmemlog_close(plp);
}
void append_str(char* str,char* path,unsigned long long length){
     PMEMlogpool* plp = pmemlog_open(path);
     printf("STRING LENGTH: %d\n",strlen(str));
     if(pmemlog_append(plp,str,length) < 0){
	    perror("pmemlog_append");
	    exit(1);
     }
     pmemlog_close(plp);
}
void close_pool(PMEMlogpool* plp,const char* path){
     pmemlog_close(plp);
}
void print_log(char* path){
     PMEMlogpool* plp = pmemlog_open(path);
     pmemlog_walk(plp,0,printit,NULL);
     pmemlog_close(plp);
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
