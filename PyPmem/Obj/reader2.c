#include <libpmemobj.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define POOL "./test_pool_file"
#define LAYOUT "layout_test"

struct root_metadata{
     size_t ckpt_len;
};
struct mem_tensor{
     size_t tensor_len;
     char* tensor_buf;
};
int main(char** args, char* argv)
{
     char buf[] = "Hello World\n";
     size_t str_len = strlen(buf);

     printf("Length of the String is %d\n",str_len);

     PMEMobjpool* pool = pmemobj_open(POOL, LAYOUT);
     if(pool == NULL){
	     perror("pmemobj_open");
             return 1;
     }
     printf("Struct Size %d\n",sizeof(PMEMoid));
     PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata) + 100);
     struct root_metadata* root_meta = pmemobj_direct(root);
     printf("Length of the Ckpt on Disk is %d\n",root_meta->ckpt_len);
     if(root_meta->ckpt_len == str_len || 1){
         
	 for(int i = 0; i < 100; i++){
             printf("Index %d contains %s\n", i,((char*)root_meta) + i);// + sizeof(struct root_metadata));
	 }
         
         printf("File Data %s\n",root_meta + sizeof(struct root_metadata)*4);
	 printf("Buffer Pointer %p\n", root_meta + sizeof(struct root_metadata)*4);
	 printf("Root Pointer %p\n", root);
     }
     return 0;
}
