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
static int append_tensor(PMEMobjpool *pop, void* ptr, char* event_str,size_t tens_len){
     pmemobj_memcpy_persist(pop, (char*) ptr,event_str,tens_len); 
     return 0;
}
void persist_tensor(char* pool_name, char* event_str, size_t tens_len){
     printf("BEFORE OPENING POOL\n");
     PMEMobjpool* pool = pmemobj_open(pool_name, LAYOUT);
     printf("AFTER OPENING POOL\n");
     if(!pool){
          pool = pmemobj_create(pool_name,LAYOUT,PMEMOBJ_MIN_POOL,0666);
          if(!pool)
               perror("Could not open path\n");
          PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata));
          struct root_metadata* root_meta = pmemobj_direct(root);
          root_meta->ckpt_len = 0;
          pmemobj_persist(pool,&root_meta->ckpt_len,sizeof(root_meta->ckpt_len));
     }
     printf("BEFORE READ CKPT LENGTH\n");
     PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata));
     struct root_metadata* root_meta = pmemobj_direct(root);
     size_t file_len = root_meta->ckpt_len;
     printf("AFTER READ CKPT LENGTH\n");
     printf("BEFORE ADJUSTING ROOT POINTER TO WRITE TENSOR\n");
     printf("Root Pointer Extension is %zu\n",sizeof(struct root_metadata) + file_len + tens_len);
     printf("Pointer Offset is %zu\n",sizeof(struct root_metadata) + file_len);
     root = pmemobj_root(pool,sizeof(struct root_metadata) + file_len + tens_len);
     root_meta = pmemobj_direct(root);
     root_meta = ((char*) root_meta) + sizeof(struct root_metadata) + file_len; 
     printf("AFTER ADJUSTING ROOT POINTER TO WRITE TENSOR\n");
     append_tensor(pool,root_meta,event_str,tens_len);
     printf("AFTER WRITING TENSOR\n");

     printf("BEFORE WRITING CKPT LEN\n");
     root = pmemobj_root(pool,sizeof(struct root_metadata));
     root_meta = pmemobj_direct(root);
     root_meta->ckpt_len = file_len + tens_len;
     pmemobj_persist(pool,&root_meta->ckpt_len,sizeof(root_meta->ckpt_len));
     printf("AFTER WRITING CKPT LEN\n");
     pmemobj_close(pool);
}
int main(char** args, char* argv)
{
     char buf[] = "Hawaii\n";
     char buf2[] = "Howdy World\n";
     size_t str_len = strlen(buf);
     size_t str_len2 = strlen(buf2);
     persist_tensor(POOL,buf,str_len);
     persist_tensor(POOL,buf2,str_len2);
     printf("Completed Writing to Persistent Memory\n");
     return 0;
}
