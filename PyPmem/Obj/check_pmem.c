#include <libpmemobj.h>
#include <libpmem.h>
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
void persist_tensor(char* event_str, size_t tens_len){
     PMEMobjpool* pool = pmemobj_open(POOL, LAYOUT);
     if(!pool){
          pool = pmemobj_create(POOL,LAYOUT,PMEMOBJ_MIN_POOL,0666);
          if(!pool)
               perror("Could not open path\n");
          PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata));
          struct root_metadata* root_meta = pmemobj_direct(root);
          root_meta->ckpt_len = 0;
          pmemobj_persist(pool,&root_meta->ckpt_len,sizeof(root_meta->ckpt_len));
     }
     PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata));
     struct root_metadata* root_meta = pmemobj_direct(root);
     size_t file_len = root_meta->ckpt_len;
     root = pmemobj_root(pool,sizeof(struct root_metadata) + file_len + tens_len);
     root_meta = pmemobj_direct(root);
     root_meta = ((char*) root_meta) + sizeof(struct root_metadata) + file_len; 
    
     append_tensor(pool,root_meta,event_str,tens_len);

     root = pmemobj_root(pool,sizeof(struct root_metadata));
     root_meta = pmemobj_direct(root);
     root_meta->ckpt_len = file_len + tens_len;
     pmemobj_persist(pool,&root_meta->ckpt_len,sizeof(root_meta->ckpt_len));
     pmemobj_close(pool);
}
int main(char** args, char* argv)
{
     PMEMobjpool* pool = pmemobj_open(POOL, NULL);
     PMEMoid root = pmemobj_root(pool,sizeof(struct root_metadata));
     int is_pmem = pmem_is_pmem(pmemobj_direct(root),sizeof(struct root_metadata));
     printf("IS PMEM IS %d\n",is_pmem);
     /*
     char buf[] = "Hawaii\n";
     char buf2[] = "Howdy World\n";
     size_t str_len = strlen(buf);
     size_t str_len2 = strlen(buf2);
     persist_tensor(buf,str_len);
     persist_tensor(buf2,str_len2);
     printf("Completed Writing to Persistent Memory\n");
     */
     return 0;
}
