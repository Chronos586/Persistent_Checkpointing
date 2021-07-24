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

PMEMlogpool* create_event_pool(const char* path){
	PMEMlogpool* plp;
	printf("BEFORE EVENT POOL CREATE");
	plp = pmemlog_create(path, POOL_SIZE, 0666);
	if(plp == NULL)
	   plp = pmemlog_open(path);

	if(plp == NULL){
	   perror(path);
	   exit(1);
	}
	printf("LEAVE EVENT POOL");
	return plp;
}
void append_str(const PMEMlogpool* plp, const char* str){
     printf("ENTER APPEND STR");
     if(pmemlog_append(plp,str,strlen(str)) < 0){
	    perror("pmemlog_append");
	    exit(1);
     }
     printf("LEAVE APPEND STR");
}
void close_pool(const PMEMlogpool* plp){
     pmemlog_close(plp);
}
void print_log(const PMEMlogpool* plp){
     pmemlog_walk(plp,0,printit,NULL);
}
int
main(int argc, char *argv[])
{
	const char path[] = "./myfile";
	PMEMlogpool *plp;
	char *str;

	/* create the pmemlog pool or open it if it already exists */
	plp = create_event_pool(path);
	str = "Testing\n";
        append_str(plp,str);
	print_log(plp);
	close_pool(plp);

	//pmemlog_close(plp);
}
