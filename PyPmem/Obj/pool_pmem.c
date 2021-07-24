#include <libpmemobj.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define POOL "./test_pool_file"
#define LAYOUT "layout_test"
/* size of the pmemlog pool -- 1 GB */
#define POOL_SIZE ((off_t)(1 << 30))

struct tensor_node{
	size_t tens_len;
	PMEMoid tensor;
	PMEMoid next;
};
struct tensor_list{
	PMEMoid head;
	PMEMoid tail;
	size_t ckpt_len;
};
struct tensor_file_list{
	PMEMoid head;
	PMEMoid tail;
};
struct file_node{
	PMEMoid tens_list;
	PMEMoid next;
	PMEMoid filename;
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
	PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_list));
        printf("Root ID: %d\n",root.pool_uuid_lo);
	struct tensor_list* tensor_file = pmemobj_direct(root);
        printf("First Tensor ID: %d\n",tensor_file->head.pool_uuid_lo);
        struct tensor_node* test = pmemobj_direct(tensor_file->head);
	printf("Size is %zu\n",test->tens_len);
        //pmemobj_close(pool);
        char* tensor = pmemobj_direct(test->tensor);
        printf("Tensor is '%s'\n",tensor);
	struct tensor_node* test2= pmemobj_direct(test->next);
        char* tensor2 = pmemobj_direct(test2->tensor);
	printf("Tensor is '%s'\n",tensor2);
        pmemobj_close(pool);
}
int print_info_loop(char* pool_name)
{
	PMEMobjpool* pool = pmemobj_open(pool_name,NULL);//, LAYOUT);
        PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_list));

	struct tensor_list* tensor_file = pmemobj_direct(root);
	PMEMoid cur_node = tensor_file->head;
        while(!(cur_node.pool_uuid_lo == -1 && cur_node.off == -1))
	{
		struct tensor_node* cur_mem = pmemobj_direct(cur_node);
		char* tensor = pmemobj_direct(cur_mem->tensor);
		printf("Size is %zu\n",cur_mem->tens_len);
		printf("Tensor is %s\n",tensor);
		cur_node = cur_mem->next;
	}
        pmemobj_close(pool);
}
/*
char* load_tensors(char* pool_name,char* filename)
{
	PMEMobjpool* pool = pmemobj_open(pool_name,NULL);
	PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));

        struct tensor_file_list* tensor_file = pmemobj_direct(root);
        PMEMoid cur_node = tensor_file->head;
	char* data = (char*) malloc(tensor_file->ckpt_len);
	char* data_cp = data;
	printf("Checkpoint Size is %zu\n",tensor_file->ckpt_len);
	while(!(cur_node.pool_uuid_lo == -1 && cur_node.off == -1))
        {
                struct tensor_node* cur_mem = pmemobj_direct(cur_node);
                char* tensor = pmemobj_direct(cur_mem->tensor);
                //printf("Size is %zu\n",cur_mem->tens_len);
                //printf("Tensor is %s\n",tensor);
		memcpy(data,tensor,cur_mem->tens_len);
                cur_node = cur_mem->next;
		data = data + cur_mem->tens_len;
        }
	pmemobj_close(pool);
	return data_cp;
}
*/
//Assumes pool is open
int file_exists(PMEMobjpool* pool,char* filename)
{
     int exists = 0;
     //Pool Is Open
     if(!pmemobj_open(pool,NULL))
     {
         //Search through the file nodes
          PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));

          struct tensor_file_list* tensor_file = pmemobj_direct(root);
          PMEMoid cur_fnode = tensor_file->head;
          while(!(cur_fnode.pool_uuid_lo == -1 && cur_fnode.off == -1) && !exists)
          {
                  struct file_node* cur_mem = pmemobj_direct(cur_fnode);
                  char* node_fname = pmemobj_direct(cur_mem->filename);
                  if(strcmp(node_fname,filename) == 0)
                      exists = 1;
                  cur_fnode = cur_mem->next;
          }
     }
     else
         printf("Pool Is Not Open\n");
     return exists;
}
/*
 * Retrieves pointer to file node residing in
 * persistent memory
 */
struct file_node* get_file(PMEMobjpool* pool,char* filename)
{
     struct file_node* retfile = NULL;
     int file_found = 0;
     //Pool Is Open
     if(!pmemobj_open(pool,NULL))
     {
         //Search through the file nodes
          PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));

          struct tensor_file_list* tensor_file = pmemobj_direct(root);
          PMEMoid cur_fnode = tensor_file->head;
          while(!(cur_fnode.pool_uuid_lo == -1 && cur_fnode.off == -1) && !file_found)
          {
                  struct file_node* cur_mem = pmemobj_direct(cur_fnode);
                  char* node_fname = pmemobj_direct(cur_mem->filename);
                  //printf("GET FILE FUNC. FILENAME IS %s\n",node_fname);
                  if(strcmp(node_fname,filename) == 0)
                  {
                      retfile = cur_mem;
                      file_found = 1;
                  }
                  cur_fnode = cur_mem->next;
          }
     }
     else
         printf("Pool Is Not Open\n");
     return retfile;
}
/*
 * Given a pool name and filename, the filename is used
 * as the key and the data is copied to DRAM. The pointer
 * to the allocated memory containing the copied data is then
 * returned.
 */
char* load_tensors(char* pool_name,char* filename)
{
        PMEMobjpool* pool = pmemobj_open(pool_name,NULL);
        PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));

        struct tensor_file_list* tensor_file = pmemobj_direct(root);
        struct file_node* fnode = get_file(pool,filename);
        struct tensor_list* tensl = pmemobj_direct(fnode->tens_list);
        char* data = (char*) malloc(tensl->ckpt_len);
        char* data_cp = data;
        printf("Checkpoint Size is %zu\n",tensl->ckpt_len);
        PMEMoid cur_node = tensl->head;
        while(!(cur_node.pool_uuid_lo == -1 && cur_node.off == -1))
        {
                struct tensor_node* cur_mem = pmemobj_direct(cur_node);
                char* tensor = pmemobj_direct(cur_mem->tensor);
                //printf("Size is %zu\n",cur_mem->tens_len);
                //printf("Tensor is %s\n",tensor);
                memcpy(data,tensor,cur_mem->tens_len);
                cur_node = cur_mem->next;
                data = data + cur_mem->tens_len;
        }
        pmemobj_close(pool);
        return data_cp;
}
/*
 * Persist Tensor takes in a given byte string noted as the event_str
 * and then writes it to persistent memory. A character pointer is chosen
 * here in accordance with Amazon Sagemaker Debugger's method of storing
 * tensors in a Google Protocol Buffer found here:
 * https://github.com/awslabs/sagemaker-debugger/tree/7c5c8c4f1379c826e4ec2071ef5ecef91aa85b1d/smdebug/core/tfevent/proto
 * After storing tensor data in the google protocol buffer, it is serialized into a byte string. It
 * is this byte string that is passed to the persist_tensor function to write to persistent memory
 * Within a given persistent memory pool, the filename is used as a key to locate a file within a given
 * persistent memory pool
 */
void persist_tensor(char* pool_name, char* event_str,char* filename, size_t tens_len){
     PMEMobjpool* pool = pmemobj_open(pool_name, NULL);
     //If pool does not exist
     if(!pool){
          pool = pmemobj_create(pool_name,NULL,POOL_SIZE,0666);
          if(!pool)
               perror("Could not open path\n");
          PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));
          struct tensor_file_list* tensor_file = pmemobj_direct(root);

          PMEMoid tensor_p;

          //Allocate persistent memory to tensor and copy event string
          if(pmemobj_zalloc(pool,&tensor_p,tens_len,0))
              perror("Failed to allocate Tensor: %m\n");
          char* tensor = pmemobj_direct(tensor_p);
          pmemobj_memcpy_persist(pool,tensor,event_str,tens_len);

          PMEMoid node_p;
          //Allocate persistent memory for a tensor node which points to a given tensor within persistent memory
          if(pmemobj_zalloc(pool,&node_p,sizeof(struct tensor_node),0))
              perror("Failed to allocate: %m\n");
          struct tensor_node* node = pmemobj_direct(node_p);
          node->tens_len = tens_len;
          node->tensor = tensor_p;
          //Because PMEMoid is a struct, null pointer I chose to represent as the pool id and pool offset to be -1
          //This is used for traversing the linked list to know where the list ends
          PMEMoid null_p;
          null_p.pool_uuid_lo = -1;
          null_p.off = -1;
          node->next = null_p;

          pmemobj_persist(pool,node,sizeof(struct tensor_node));
          PMEMoid tens_list_p;
          if(pmemobj_zalloc(pool,&tens_list_p,sizeof(struct tensor_list),0))
              perror("Failed to allocate: %m\n");
          struct tensor_list* tens_list = pmemobj_direct(tens_list_p);

          tens_list->head = node_p;
          tens_list->tail = node_p;
          tens_list->ckpt_len = tens_len;

          pmemobj_persist(pool,&tens_list->head,sizeof(tens_list->head));
          pmemobj_persist(pool,&tens_list->tail,sizeof(tens_list->tail));
          pmemobj_persist(pool,&tens_list->ckpt_len,sizeof(tens_list->ckpt_len));

          PMEMoid fnode_p;
          if(pmemobj_zalloc(pool,&fnode_p,sizeof(struct file_node),0))
              perror("Failed to allocate: %m\n");
          struct file_node* fnode = pmemobj_direct(fnode_p);
          PMEMoid filename_p;
          if(pmemobj_zalloc(pool,&filename_p,strlen(filename),0))
              perror("Failed to allocate: %m\n");
          char* filename_meta = pmemobj_direct(filename_p);
          pmemobj_memcpy_persist(pool,filename_meta,filename,strlen(filename));

          fnode->tens_list = tens_list_p;
          fnode->filename = filename_p;
          fnode->next = null_p;
          pmemobj_persist(pool,fnode,sizeof(struct file_node));

          tensor_file->head = fnode_p;
          tensor_file->tail = fnode_p;
          pmemobj_persist(pool,&tensor_file->tail,sizeof(tensor_file->tail));
          pmemobj_persist(pool,&tensor_file->head,sizeof(tensor_file->head));
     }
     else{
	  PMEMoid root = pmemobj_root(pool,sizeof(struct tensor_file_list));
          struct tensor_file_list* tensor_file = pmemobj_direct(root);

	  if(file_exists(pool,filename)){
              struct file_node* fnode = get_file(pool,filename);
              PMEMoid tensor_p;

              //Allocate persistent memory to tensor and copy event string
              if(pmemobj_zalloc(pool,&tensor_p,tens_len,0))
                  perror("Failed to allocate Tensor: %m\n");
              char* tensor = pmemobj_direct(tensor_p);
              pmemobj_memcpy_persist(pool,tensor,event_str,tens_len);

              PMEMoid node_p;

              if(pmemobj_zalloc(pool,&node_p,sizeof(struct tensor_node),0))
                  perror("Failed to allocate: %m\n");
              struct tensor_node* node = pmemobj_direct(node_p);
              node->tens_len = tens_len;
              node->tensor = tensor_p;
              PMEMoid null_p;
              null_p.pool_uuid_lo = -1;
              null_p.off = -1;
              node->next = null_p;

              pmemobj_persist(pool,node,sizeof(struct tensor_node));

              struct tensor_list* tensl = pmemobj_direct(fnode->tens_list);
              struct tensor_node* last_tensor = pmemobj_direct(tensl->tail);
              //printf("Size is %zu\n",last_tensor->tens_len);
              last_tensor->next = node_p;
              pmemobj_persist(pool,&last_tensor->next,sizeof(last_tensor->next));
              tensl->ckpt_len = tensl->ckpt_len + tens_len;
              tensl->tail = node_p;

              pmemobj_persist(pool,&tensl->tail,sizeof(tensl->tail));
              pmemobj_persist(pool,&tensl->ckpt_len,sizeof(tensl->ckpt_len));
          }
          else{
              //Load the File List
              struct file_node* last_fnode = pmemobj_direct(tensor_file->tail);

              PMEMoid tensor_p;

              //Allocate persistent memory to tensor and copy event string
              if(pmemobj_zalloc(pool,&tensor_p,tens_len,0))
                  perror("Failed to allocate Tensor: %m\n");
              char* tensor = pmemobj_direct(tensor_p);
              pmemobj_memcpy_persist(pool,tensor,event_str,tens_len);

              PMEMoid node_p;

              if(pmemobj_zalloc(pool,&node_p,sizeof(struct tensor_node),0))
                  perror("Failed to allocate: %m\n");
              struct tensor_node* node = pmemobj_direct(node_p);
              node->tens_len = tens_len;
              node->tensor = tensor_p;
              PMEMoid null_p;
              null_p.pool_uuid_lo = -1;
              null_p.off = -1;
              node->next = null_p;

              pmemobj_persist(pool,node,sizeof(struct tensor_node));
              PMEMoid tens_list_p;
              if(pmemobj_zalloc(pool,&tens_list_p,sizeof(struct tensor_list),0))
                  perror("Failed to allocate: %m\n");
              struct tensor_list* tens_list = pmemobj_direct(tens_list_p);

              tens_list->head = node_p;
              tens_list->tail = node_p;
              tens_list->ckpt_len = tens_len;

              pmemobj_persist(pool,&tens_list->head,sizeof(tens_list->head));
              pmemobj_persist(pool,&tens_list->tail,sizeof(tens_list->tail));
              pmemobj_persist(pool,&tens_list->ckpt_len,sizeof(tens_list->ckpt_len));

              PMEMoid fnode_p;
              if(pmemobj_zalloc(pool,&fnode_p,sizeof(struct file_node),0))
                  perror("Failed to allocate: %m\n");
              struct file_node* fnode = pmemobj_direct(fnode_p);
              PMEMoid filename_p;
              if(pmemobj_zalloc(pool,&filename_p,strlen(filename),0))
                  perror("Failed to allocate: %m\n");
              char* filename_meta = pmemobj_direct(filename_p);
              pmemobj_memcpy_persist(pool,filename_meta,filename,strlen(filename));

              fnode->tens_list = tens_list_p;
              fnode->filename = filename_p;
              fnode->next = null_p;
              pmemobj_persist(pool,fnode,sizeof(struct file_node));

              last_fnode->next = fnode_p;
              pmemobj_persist(pool,&last_fnode->next,sizeof(last_fnode->next));

              tensor_file->tail = fnode_p;
              pmemobj_persist(pool,&tensor_file->tail,sizeof(tensor_file->tail));
          }
     }
     pmemobj_close(pool);
}
int main(char** args, char* argv)
{
     char buf[] = "Hawaii is very nice place";
     char buf2[] = "Climb the Tower";
     char filename[] = "testfile1";
     char fname2[] = "testfile2";
     //char buf2[] = "Howdy World\n";
     size_t str_len = strlen(buf);
     //size_t str_len2 = strlen(buf2);
     persist_tensor(POOL,buf,filename,str_len);
     persist_tensor(POOL,buf,fname2,str_len);
     persist_tensor(POOL,buf2,fname2,strlen(buf2));
     //persist_tensor(POOL,buf2,str_len2);
     //print_info_loop(POOL);
     char* data;
     printf("Before LOAD TENSORS\n");
     data = load_tensors(POOL,filename);
     printf("AFTER LOAD TENSORS\n");
     printf("Total Tensor Data %s\n",data);
     data = load_tensors(POOL,fname2);
     printf("Total Tensor Data %s\n",data);
     printf("Completed Writing to Persistent Memory\n");
     return 0;
}
