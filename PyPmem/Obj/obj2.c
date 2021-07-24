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
static int append_tensor(PMEMobjpool *pop, void* ptr, void* arg){
     struct mem_tensor* tens_ptr = (struct mem_tensor*) arg;
     printf("Copying Tensor to Address %p\n",ptr);
     printf("Writing String %s to Persistent Memory\n",tens_ptr->tensor_buf);
     pmemobj_memcpy_persist(pop, (char*) ptr, tens_ptr->tensor_buf,
                                 tens_ptr->tensor_len);
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
     printf("Root ID is %d and Root Offset is %d\n",root.pool_uuid_lo,root.off);
     struct root_metadata* root_meta = pmemobj_direct(root);
     size_t file_len = root_meta->ckpt_len;
     root = pmemobj_root(pool,sizeof(struct root_metadata) + file_len + tens_len);
     printf("File Length is %d\n",file_len);
     /*
     char* testr = pmemobj_direct(root);
     PMEMoid* new_tensor = (PMEMoid*)(testr + sizeof(struct root_metadata) + file_len);
     printf("Root ID is %d and Root Offset is %d\n",root.pool_uuid_lo,root.off);
     
     printf("Pointer is %p\n",new_tensor);
     printf("Root pointer %p\n",pmemobj_direct(root));
     */
     //PMEMoid* new_tensor = pmemobj_direct(root) + 1000;//sizeof(struct root_metadata) + file_len;
     PMEMoid new_tensor;
     printf("Pointer is %p\n",new_tensor);
     //PMEMoid* new_tensor = pmemobj_direct(root) + sizeof(struct root_metadata) + file_len;
     
     struct mem_tensor* tensor_obj = (struct mem_tensor*) malloc(sizeof(struct mem_tensor));
     tensor_obj->tensor_len = tens_len;
     tensor_obj->tensor_buf = event_str;
     //if(OID_IS_NULL(*new_tensor))
//	     printf("NULL ALERT\n");
     if(pmemobj_alloc(pool,&new_tensor,tens_len,0,append_tensor,tensor_obj))
          perror("Failed to allocate: %m\n");

     //Copy new ckpt file length to Persistent Memory
     root = pmemobj_root(pool,sizeof(struct root_metadata));
     root_meta = pmemobj_direct(root);
     root_meta->ckpt_len = file_len + tens_len;
     pmemobj_persist(pool,&root_meta->ckpt_len,sizeof(root_meta->ckpt_len));
     pmemobj_close(pool);
}
int main(char** args, char* argv)
{
     //char buf[] = "Hello World\n";
     char buf[] = "Hikerllo World\n";
     /*
     char* buf_ptr = (char*) malloc(sizeof(char)*400);
     for(int i = 0; i < 400; i++){
         buf_ptr[i] = 'b';
     }
     buf_ptr[399] = '\0';
     */
     char buf2[] = "Hawaii\n";
     size_t str_len = strlen(buf);
     size_t str_len2 = strlen(buf2);
     persist_tensor(buf,str_len);
     persist_tensor(buf2,str_len2);
     printf("Completed Writing to Persistent Memory\n");
     return 0;
}
