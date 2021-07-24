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
struct tensor_node{
	size_t tens_len;
	char* tensor;
	struct tensor_node* next;
};
struct tensor_list{
	struct tensor_node* head;
	struct tensor_node* tail;
};
PMEMoid* allocate_tensor(PMEMobjpool* pool, size_t tens_len)
{
     PMEMoid* tensor;
     if(pmemobj_zalloc(pool,tensor,tens_len,0))
          perror("Failed to allocate: %m\n");

     return tensor;
}
static int append_tensor(PMEMobjpool *pop, PMEMoid* tensor_ptr, char* event_str,size_t tens_len){
     pmemobj_memcpy_persist(pop, tensor_ptr,event_str,tens_len);
     return 0;
}
int print_info(char* pool_name)
{
	PMEMobjpool* pool = pmemobj_open(pool_name,NULL);//, LAYOUT);
        printf("Before Root\n");
        if(!pool)
          printf("Not pool\n");
        else
          printf("Pool\n");
	PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_list));
        printf("After Root\n");
	struct tensor_list* tensor_file = pmemobj_direct(root);
        printf("Before Seg Fault\n");
	printf("Pointer %p\n",tensor_file->head);
        struct tensor_node* test = tensor_file->head;
	printf("Size is %zu\n",test->tens_len);
        printf("Tensor is '%s'\n",test->tensor);
}
void persist_tensor(char* pool_name, char* event_str, size_t tens_len){
     printf("Before\n");
     PMEMobjpool* pool = pmemobj_open(pool_name, NULL);
     printf("After\n");
     if(!pool){
          pool = pmemobj_create(pool_name,NULL,PMEMOBJ_MIN_POOL,0666);
          if(!pool)
               perror("Could not open path\n");
          PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_list));
          struct tensor_list* tensor_file = pmemobj_direct(root);

	  PMEMoid tensor_p;
          if(pmemobj_zalloc(pool,&tensor_p,tens_len,0))
              perror("Failed to allocate: %m\n");
          char* tensor = pmemobj_direct(tensor_p);
          pmemobj_memcpy_persist(pool,tensor,event_str,tens_len);

	  PMEMoid node_p;
          //Persist Later
     	  if(pmemobj_zalloc(pool,&node_p,sizeof(struct tensor_node),0))
              perror("Failed to allocate: %m\n");
          struct tensor_node* node = pmemobj_direct(node_p);
          node->tens_len = tens_len;
          node->tensor = tensor;
          node->next = NULL;
          pmemobj_persist(pool,node,sizeof(struct tensor_node));

	  tensor_file->head = node;
          printf("Pointer is %p\n",tensor_file->head);
	  tensor_file->tail = node;
	  //append_tensor(pool,tensor_file->head,event_str,tens_len);
	  //PERSIST THE HEAD AND TAIL LATER
          pmemobj_persist(pool,&tensor_file->head,sizeof(tensor_file->head));
	  pmemobj_persist(pool,&tensor_file->tail,sizeof(tensor_file->tail));
          pmemobj_close(pool);
     }
     else{
	  PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_list));
          struct tensor_list* tensor_file = pmemobj_direct(root);

          PMEMoid tensor_p;
          if(pmemobj_zalloc(pool,&tensor_p,tens_len,0))
              perror("Failed to allocate: %m\n");
          char* tensor = pmemobj_direct(tensor_p);
          pmemobj_memcpy_persist(pool,tensor,event_str,tens_len);

          PMEMoid node_p;
          //Persist Later
          if(pmemobj_zalloc(pool,&node_p,sizeof(struct tensor_node),0))
              perror("Failed to allocate: %m\n");
          struct tensor_node* node = pmemobj_direct(node_p);
          node->tens_len = tens_len;
          node->tensor = tensor;
          node->next = NULL;
          pmemobj_persist(pool,node,sizeof(struct tensor_node));
	  printf("???\n");
	  struct tensor_node* last_tensor = tensor_file->head;
          printf("???\n");
          if(!last_tensor)
             printf("YOYO\n");
          else
	     printf("YOYO2\n");
          printf("Pointer %p\n",last_tensor);
          //printf("Size is %zu\n",last_tensor->tens_len);
          if(!last_tensor->next)
              printf("Howdy\n");
          last_tensor->next = node;
          printf("Before persist\n");
	  pmemobj_persist(pool,&last_tensor->next,sizeof(last_tensor->next));
	  printf("after persist\n");
          tensor_file->tail = node;
          //append_tensor(pool,tensor_file->head,event_str,tens_len);
          //PERSIST THE HEAD AND TAIL LATER
          pmemobj_persist(pool,&tensor_file->tail,sizeof(tensor_file->tail));
          pmemobj_close(pool);
     }
     /*
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
     */
}
int main(char** args, char* argv)
{
     char buf[] = "Hawaii is very nice place";
     //char buf2[] = "Howdy World\n";
     size_t str_len = strlen(buf);
     //size_t str_len2 = strlen(buf2);
     //persist_tensor(POOL,buf,str_len);
     //persist_tensor(POOL,buf2,str_len2);
     print_info(POOL);
     printf("Completed Writing to Persistent Memory\n");
     return 0;
}
