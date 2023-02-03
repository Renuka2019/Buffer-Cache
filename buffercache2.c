#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#define TRUE 1
#define FALSE 0
#define SUCCESS 1
#define FAILURE 0
#define LIST_DATA_NOT_FOUND 2
#define LIST_EMPTY 3
#define NO_OF_HASH_QUEUES 4
#define SIZE_OF_WAITING_QUEUE 10

typedef int status_t;  
typedef int bool;  

typedef struct Data data_t;
typedef  struct  buffer_header buffer_header_node_t;
typedef  buffer_header_node_t  buffer_header_list_t;


struct buffer_header
{
        unsigned int logical_divice_no;
        unsigned int logical_block_no;
        int status;
        struct buffer_header *hash_next;
        struct buffer_header *hash_prev;
        struct  buffer_header *freelist_next;
        struct buffer_header  *freelist_prev;
        char *pointer_data_area;
  
};

struct Data
{
       unsigned int logical_divice_no;
        unsigned int logical_block_no;
        int status;
        char *pointer_data_area;                             
};
                                                         
typedef enum STATE {                                     

  // Is buffer currently in use by a process
  BUFFER_BUSY = 1,

  // Is buffer data valid
  BUFFER_DATA_VALID = 2,

  // Is buffer marked delayed write
  BUFFER_MARKED_DELAYED_WRITE = 4,

  // Is kernel currently reading/writing this buffer to memory
  KERNEL_READING_WRITING = 8, 

  // Is another process waiting for this buffer
  BUFFER_AWAITED = 16,  

  // Is this Buffer's data OLD
  BUFFER_DATA_OLD = 32

} STATE;

typedef enum WAITING_TYPE
{
  WAITING_FOR_THIS_BUFFER = -1,   // negative for THIS buffer
  WAITING_FOR_ANY_BUFFER = 1      // positive for ANY buffer
} WAITING_TYPE;




//global variable definition
buffer_header_list_t *free_list = NULL;
buffer_header_list_t **hash_list_arr = NULL;
int waiting_queue[SIZE_OF_WAITING_QUEUE];

//global function declration
// ************************hash_list declaration **********************************
buffer_header_list_t* create_hash_list(void);
buffer_header_node_t* get_hash_list_node(data_t data);
void hash_list_generic_insert(buffer_header_list_t* p_beg, buffer_header_node_t*p_mid, buffer_header_node_t*p_end); 
status_t hash_list_insert_beg(buffer_header_list_t *p_list, data_t new_data);
status_t hash_list_insert_end(buffer_header_list_t *p_list, data_t new_data);
buffer_header_node_t *hash_list_search_node(buffer_header_list_t *p_list,unsigned logical_block_no);
data_t get_data(unsigned int logical_block_no,unsigned int logical_device_no,int status);
void hash_list_show_list(buffer_header_list_t *p_list, const char *msg);
status_t hash_list_push_beg(buffer_header_list_t *p_list,buffer_header_node_t *p_node);
status_t hash_list_push_end(buffer_header_list_t *p_list,buffer_header_node_t *p_node);
status_t hash_list_pop_beg(buffer_header_list_t *p_list,buffer_header_node_t **p_node); 
status_t hash_list_pop_end(buffer_header_list_t *p_list,buffer_header_node_t **p_node );
void hash_list_generic_delete(buffer_header_node_t *p_delete_node);
bool is_hash_list_empty(buffer_header_list_t *p_list);
status_t remove_buffer_from_hash_queue(buffer_header_list_t *plist,buffer_header_node_t *p_delete_node);

// ************************freelist declaration **********************************
buffer_header_list_t* create_free_list(void);
buffer_header_list_t* get_free_list_node(data_t data);
void freelist_generic_insert(buffer_header_list_t* p_beg, buffer_header_node_t*p_mid, buffer_header_node_t*p_end); 
status_t insert_end_freelist(buffer_header_list_t *p_list, buffer_header_node_t* hash_node); 
status_t freelist_insert_beg(buffer_header_list_t *p_list, buffer_header_node_t* hash_node); 
void freelist_show_list(buffer_header_list_t *p_list, const char *msg);
buffer_header_node_t *freelist_search_node(buffer_header_list_t *p_list,unsigned logical_block_no);
status_t freelist_pop_beg(buffer_header_list_t *p_list, buffer_header_node_t **p_node); 
status_t freelist_pop_end(buffer_header_list_t *p_list, buffer_header_node_t **p_node);
void freelist_generic_delete(buffer_header_node_t *p_delete_node);
bool is_Freelist_empty(buffer_header_list_t *p_list);
status_t remove_buffer_from_freelist(buffer_header_list_t *plist,buffer_header_node_t *p_delete_node);
buffer_header_node_t*  get_node_from_free_list();

// *************************************** status Manegment *****************************

void add_state(buffer_header_node_t *p_buffer, STATE state);
void remove_state(buffer_header_node_t *p_buffer, STATE state);
void set_state(buffer_header_node_t *p_buffer, STATE state);
int is_in_state(buffer_header_node_t *p_buffer, STATE state);


//  ******************************************* getblk************************************
   buffer_header_node_t *getblk(unsigned int blk_num);    // getblock
   void brelse(buffer_header_node_t *p_buffer);    // release   

// **************************************wating Queue*************************************
  void add_to_waiting_queue(unsigned int blk_num, WAITING_TYPE WAITING_FOR_THIS_BUFFER);
  int find_in_waiting_queue(int blockNumber);
  int get_process_from_waiting_queue(int blockNumber);
  int is_waiting_queue_empty();
// ****************************************************************************************



int main(void)
{
  status_t s ;
  //data_t data;
  hash_list_arr = (buffer_header_list_t**)malloc(NO_OF_HASH_QUEUES * sizeof(buffer_header_list_t *));
  free_list = create_free_list();
  for(int i = 0;i<NO_OF_HASH_QUEUES;i++)
  {
      hash_list_arr[i] = create_hash_list();    
  }


// insert data as per unix figure
  //hash array 0

  s = hash_list_insert_beg(hash_list_arr[0],get_data(64,1,BUFFER_DATA_VALID|BUFFER_BUSY));  
   assert(s == SUCCESS); 
  s = hash_list_insert_beg(hash_list_arr[0],get_data(4,1,BUFFER_DATA_VALID));
   assert(s == SUCCESS); 
   s = hash_list_insert_beg(hash_list_arr[0],get_data(28,1,BUFFER_DATA_VALID));
   assert(s == SUCCESS); 

   hash_list_show_list(hash_list_arr[0],"After insert beg\n");

//hash array 1
   s = hash_list_insert_beg(hash_list_arr[1],get_data(97,1,BUFFER_DATA_VALID));  
   assert(s == SUCCESS); 
  s = hash_list_insert_beg(hash_list_arr[1],get_data(5,1,BUFFER_DATA_VALID));
   assert(s == SUCCESS); 
   s = hash_list_insert_beg(hash_list_arr[1],get_data(17,1,BUFFER_DATA_VALID|BUFFER_BUSY));
   assert(s == SUCCESS); 
 hash_list_show_list(hash_list_arr[1],"After insert beg\n");
 //hash array 2
   s = hash_list_insert_beg(hash_list_arr[2],get_data(10,1,BUFFER_DATA_VALID));  
   assert(s == SUCCESS); 
  s = hash_list_insert_beg(hash_list_arr[2],get_data(50,1,BUFFER_DATA_VALID|BUFFER_BUSY));
   assert(s == SUCCESS); 
   s = hash_list_insert_beg(hash_list_arr[2],get_data(98,1,BUFFER_DATA_VALID|BUFFER_BUSY));
   assert(s == SUCCESS); 
 hash_list_show_list(hash_list_arr[2],"After insert beg\n");
//hash array 3
   s = hash_list_insert_beg(hash_list_arr[3],get_data(99,1,BUFFER_DATA_VALID|BUFFER_BUSY));  
   assert(s == SUCCESS); 
  s = hash_list_insert_beg(hash_list_arr[3],get_data(35,1,BUFFER_DATA_VALID|BUFFER_BUSY));
   assert(s == SUCCESS); 
   s = hash_list_insert_beg(hash_list_arr[3],get_data(3,1,BUFFER_DATA_VALID));
   assert(s == SUCCESS);

  
 hash_list_show_list(hash_list_arr[3],"After insert beg\n");
   

 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[3%4]),3));
 assert(s == SUCCESS);
 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[5%4]),5));
 assert(s == SUCCESS); 
 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[4%4]),4));
 assert(s == SUCCESS); 
 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[28%4]),28));
 assert(s == SUCCESS);
 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[97%4]),97));
 assert(s == SUCCESS); 
 s =insert_end_freelist(free_list,hash_list_search_node((hash_list_arr[10%4]),10));
 assert(s == SUCCESS);  
freelist_show_list(free_list,"After insert end in freelist\n");
/*
 buffer_header_node_t *pop_data = NULL;                 
  s = freelist_pop_end(free_list,&pop_data);
  assert(s == SUCCESS);
  printf("pop_data->logical_block = %d\n",pop_data->logical_block_no);
 freelist_show_list(free_list,"After insert end in freelist\n");
*/
//senario 5
 buffer_header_node_t *p_node = getblk(18);
 
 //printf("p_node->logical_block_no= %d",p_node->logical_block_no);

/*buffer_header_node_t *s_node = freelist_search_node(free_list,3);
printf("s_node->logical_block_no =  %d\n",s_node->logical_block_no);
 */
//freelist_show_list(free_list,"After senrio1");
hash_list_show_list(hash_list_arr[18%4],"After senario4");
//add_state(freelist_search_node(free_list,3),BUFFER_MARKED_DELAYED_WRITE);
//add_state(freelist_search_node(free_list,5),BUFFER_MARKED_DELAYED_WRITE);

}

// **********************Hash queue ***************************
buffer_header_list_t*  create_hash_list(void)
{
    data_t data;
    data.logical_divice_no =0;
    data.logical_block_no = 0;
    data.status = 0;
    //data.pointer_data_area = NULL;
  
    buffer_header_node_t  *p_new_node = get_hash_list_node(data);
    p_new_node->hash_next = p_new_node; 
    p_new_node->hash_prev= p_new_node; 
    p_new_node->freelist_next= NULL;
    p_new_node->freelist_prev = NULL;       
   return(p_new_node);

}


buffer_header_node_t* get_hash_list_node(data_t data)
{
    buffer_header_node_t *p_new_node  =(buffer_header_node_t*) malloc(sizeof(buffer_header_node_t));
    p_new_node->logical_divice_no = data.logical_divice_no;
    p_new_node->logical_block_no = data.logical_block_no;
    p_new_node->status = data.status;
    //p_new_node->pointer_data_area = data.pointer_data_area;
    return p_new_node;
    
}

status_t hash_list_insert_beg(buffer_header_list_t *p_list, data_t new_data)
{
   hash_list_generic_insert(p_list, get_hash_list_node(new_data), p_list->hash_next);
     return (SUCCESS);
}

void hash_list_generic_insert(buffer_header_list_t* p_beg, buffer_header_node_t*p_mid, buffer_header_node_t*p_end)
{
     p_mid->hash_next = p_end; 
    p_mid->hash_prev = p_beg; 
    p_beg->hash_next = p_mid; 
    p_end->hash_prev = p_mid; 
}

bool is_hash_list_empty(buffer_header_list_t *p_list)
{
  return(p_list->hash_next ==p_list && p_list->hash_prev == p_list);

}
status_t hash_list_pop_beg(buffer_header_list_t *p_list, buffer_header_node_t **pp_data)
{
  
    if(is_hash_list_empty(p_list) == TRUE)
    {
        return (LIST_EMPTY); 
    }
  
    *pp_data = (buffer_header_node_t*)p_list->hash_next; 
    hash_list_generic_delete(p_list->hash_next); 
    return (SUCCESS); 

}
status_t hash_list_pop_end(buffer_header_list_t *p_list, buffer_header_node_t **pp_data)
{
  
    if(is_hash_list_empty(p_list) == TRUE)
    {
        return (LIST_EMPTY); 
    }
    
    *pp_data = (buffer_header_node_t*)p_list->hash_prev; 
    hash_list_generic_delete(p_list->hash_prev); 
    return (SUCCESS); 
}



void hash_list_generic_delete(buffer_header_node_t *p_delete_node)
{
   p_delete_node->hash_next->hash_prev = p_delete_node->hash_prev; 
    p_delete_node->hash_prev->hash_next = p_delete_node->hash_next; 
    free(p_delete_node); 
}

buffer_header_node_t *hash_list_search_node(buffer_header_list_t *p_list,unsigned logical_block_no)
{
    buffer_header_node_t *p_new_node = NULL;
buffer_header_node_t *p_run =NULL;
     for(p_run = p_list->hash_next; p_run != p_list; p_run = p_run->hash_next)
    {
        if(p_run->logical_block_no == logical_block_no)
        {
             p_new_node = p_run;
          break;
        } 
    }
   
   
     
    return(p_new_node);

}



void hash_list_show_list(buffer_header_list_t *p_list, const char *msg)
{
   buffer_header_node_t *p_run = NULL;
    if(msg){
        puts(msg); 
    }
    printf("[BEG]<->"); 
    for(p_run = p_list->hash_next; p_run != p_list; p_run = p_run->hash_next)
    {
        printf("[%d]<->", p_run->logical_block_no); 
    }
    puts("[END]"); 

}



status_t hash_list_insert_end(buffer_header_list_t *p_list, data_t new_data)
{
 hash_list_generic_insert(p_list->hash_prev,get_hash_list_node(new_data), p_list); 
  return SUCCESS;
}

status_t hash_list_push_beg(buffer_header_list_t *p_list,buffer_header_node_t *p_node)
{
  hash_list_generic_insert(p_list, p_node, p_list->hash_next);
  return SUCCESS;
}

status_t hash_list_push_end(buffer_header_list_t *p_list,buffer_header_node_t*p_node)
{
   hash_list_generic_insert(p_list->hash_prev,p_node,p_list);
   return SUCCESS;
}

status_t remove_buffer_from_hash_queue(buffer_header_list_t *plist,buffer_header_node_t *p_delete_node)
{
 //  buffer_header_list_t *p_serch_node = hash_list_search_node(plist,p_delete_node->logical_block_no);
      hash_list_generic_delete(p_delete_node);
  
    return SUCCESS;
}


// ****************************** Free List***************************

buffer_header_list_t*  create_free_list(void)
{
    data_t data;
    data.logical_divice_no =0;
    data.logical_block_no = 0;
    data.status = 0;
    data.pointer_data_area = NULL;
  
    buffer_header_node_t  *p_new_node = get_free_list_node(data);
    p_new_node->hash_next = NULL; 
    p_new_node->hash_prev= NULL; 
    p_new_node->freelist_next= p_new_node;
    p_new_node->freelist_prev = p_new_node;       
   return(p_new_node);

}


buffer_header_node_t* get_free_list_node(data_t data)
{
    buffer_header_node_t *p_new_node  =(buffer_header_node_t*) malloc(sizeof(buffer_header_node_t));
    p_new_node->logical_divice_no = data.logical_divice_no;
    p_new_node->logical_block_no = data.logical_block_no;
    p_new_node->status = data.status;
 //   p_new_node->pointer_data_area = data.pointer_data_area;
    return p_new_node;
    
}



status_t insert_end_freelist(buffer_header_list_t *p_list, buffer_header_node_t* hash_node)
{
    freelist_generic_insert(p_list->freelist_prev,hash_node, p_list); 
     return (SUCCESS); 
} 

status_t freelist_insert_beg(buffer_header_list_t *p_list, buffer_header_node_t* hash_node)
{
    freelist_generic_insert(p_list,hash_node, p_list->freelist_next);
    return SUCCESS;
}

void freelist_generic_insert(buffer_header_list_t* p_beg, buffer_header_node_t*p_mid, buffer_header_node_t*p_end)
{
     p_mid->freelist_next = p_end; 
    p_mid->freelist_prev = p_beg; 
    p_beg->freelist_next = p_mid; 
    p_end->freelist_prev = p_mid; 
}

status_t freelist_pop_beg(buffer_header_list_t *p_list, buffer_header_node_t **pp_data)
{
  /*
    if(is_list_empty(p_list) == TRUE){
        return (LIST_EMPTY); 
    }
    */
    *pp_data = (buffer_header_node_t*)p_list->freelist_next; 
    freelist_generic_delete(p_list->freelist_next);  
    return (SUCCESS); 

}
status_t freelist_pop_end(buffer_header_list_t *p_list, buffer_header_node_t **pp_data)
{
  /*
    if(is_list_empty(p_list) == TRUE){
        return (LIST_EMPTY); 
    }
    */
    *pp_data = (buffer_header_node_t*)p_list->freelist_prev;
  freelist_generic_delete(p_list->freelist_prev);
   
    
    return (SUCCESS); 
}

void freelist_generic_delete(buffer_header_node_t *p_delete_node)
{
   p_delete_node->freelist_next->freelist_prev = p_delete_node->freelist_prev; 
    p_delete_node->freelist_prev->freelist_next = p_delete_node->freelist_next; 
    free(p_delete_node); 
  
}
buffer_header_node_t *freelist_search_node(buffer_header_list_t *p_list,unsigned logical_block_no)
{
    buffer_header_node_t *p_new_node = NULL;
   buffer_header_node_t *p_run =NULL;
     for(p_run = p_list->freelist_next; p_run != p_list; p_run = p_run->freelist_next)
    {
        if(p_run->logical_block_no == logical_block_no)
        {
          break;
        } 
    }
   
    p_new_node = p_run;
     
    return(p_new_node);

}

void freelist_show_list(buffer_header_list_t *p_list, const char *msg)
{
   buffer_header_node_t *p_run = NULL;
    if(msg){
        puts(msg); 
    }
    printf("[BEG]<->"); 
    for(p_run = p_list->freelist_next; p_run != p_list; p_run = p_run->freelist_next)
    {
        printf("[%d]<->", p_run->logical_block_no); 
    }
    puts("[END]"); 

}
bool is_Freelist_empty(buffer_header_list_t *p_list)
{
   return(p_list->freelist_next== p_list && p_list->freelist_prev == p_list);
}
status_t remove_buffer_from_freelist(buffer_header_list_t *plist,buffer_header_node_t *p_delete_node)
{
 //  buffer_header_list_t *p_serch_node = hash_list_search_node(plist,p_delete_node->logical_block_no);
      freelist_generic_delete(p_delete_node);
    return SUCCESS;
}

buffer_header_node_t *get_node_from_free_list(void)
{
    buffer_header_node_t *ret = free_list->freelist_next;
     if (is_Freelist_empty(free_list)) 
     {
        ret = NULL;
        return ret;
    }

  
  while (is_in_state(ret, BUFFER_MARKED_DELAYED_WRITE)) 
  {
    ret = ret->freelist_next;
  }
   remove_buffer_from_freelist(free_list,free_list->freelist_next);

    remove_buffer_from_hash_queue(hash_list_arr[(ret->logical_block_no)%4],ret);
    
    
   return ret;


}


// ************************************ common routines *****************************
data_t get_data(unsigned int logical_block_no,unsigned int logical_divice_no,int status)
{
    data_t data;
    data.logical_block_no = logical_block_no;
    data.logical_divice_no = logical_divice_no;
    data.status = status;
    return data;
}

// **************************************getblk ***************************************
 buffer_header_node_t*  getblk(unsigned int blk_num)
   {

       while(1)
       {
        buffer_header_node_t *p_buffer = hash_list_search_node(hash_list_arr[blk_num%4],blk_num);
        if(p_buffer == NULL)
        {
            printf("NULL");
        }
        if (p_buffer != NULL)
        {    
          // if buffer found in hash Queue
            if (is_in_state(p_buffer, BUFFER_BUSY))
            {    // buffer is busy : SCENARIO 5
             printf(" ********* Scenario 5 *********\n");
              printf("The kernel finds the block on the hash queue, but its buffer is currently busy.\n");
               printf("Process goes to sleep.\n");
                printf("Process will wake up when this buffer becomes free\n");
               add_state(p_buffer, BUFFER_AWAITED);
               //printf("p_buffer->status %d",p_buffer->status);
               add_to_waiting_queue(blk_num, WAITING_FOR_THIS_BUFFER);      
             return p_buffer; 
             //continue;
            }
         else
         {
          printf("********* Scenario 1 *********\n");
          printf("The kernel finds the block on its hash queue, and its buffer is free.\n");
          set_state(p_buffer, (BUFFER_BUSY | BUFFER_DATA_VALID));
          printf("Buffer Marked Busy.\n");
          remove_buffer_from_freelist(free_list,p_buffer);
          printf("Removed Buffer from Free List. \n");
        return p_buffer;  
         }
         
     }
     

     else  if (p_buffer==NULL)
     {

          if (is_Freelist_empty(free_list))
    {   
        // if there are no buffers in free list : SCENARIO 4
        
        printf("********* Scenario 4 *********\n");
        printf("The kernel cannot find the block on the hash queue, and the free list of buffers is empty.\n");
        printf("Process goes to sleep.\n");
        printf("Process will wake up when any buffer becomes free.\n");
        add_to_waiting_queue(blk_num, WAITING_FOR_ANY_BUFFER);
        // sleep();
        // continue;
        return NULL;  // fo

     }
     else
     {   
          printf("********* Scenario 3 *********\n");
        printf("The kernel cannot find the block on the hash queue. Found free block is marked delayed write.\n");
          buffer_header_node_t *temp = free_list->freelist_next;
        
            if( is_in_state(temp,BUFFER_MARKED_DELAYED_WRITE))
             {
                 remove_state(temp,BUFFER_MARKED_DELAYED_WRITE);
                 add_state(temp,BUFFER_DATA_OLD);
                 remove_buffer_from_freelist(free_list,temp); 
              
/*
             else 
             {
                
                  remove_buffer_from_freelist(free_list,temp);
                 hash_list_insert_end(hash_list_arr[blk_num%4],get_data(blk_num,1,BUFFER_AWAITED|BUFFER_BUSY));
                 //freelist_insert_beg(free_list,temp);
                 return(temp);
  */           
           
           continue;
            
          }

         else{

          // SCENARIO 2  :Found a free Buffer
      printf("********* Scenario 2 *********\n");
      printf("The kernel cannot find the block on the hash queue, so it allocates a buffer from the free list.\n");
      buffer_header_node_t *freeBuffer = get_node_from_free_list();
      remove_state(freeBuffer, BUFFER_DATA_VALID);
      printf("REMOVING FROM OLD HASH QUEUE.\nPUTTING IN NEW HASH QUEUE\n");
      freeBuffer->logical_block_no = blk_num;
      //addBufferToHashQueue(freeBuffer);
       data_t data;
       data.logical_divice_no = freeBuffer->logical_divice_no;
       data.logical_block_no = freeBuffer->logical_block_no;
       data.status = freeBuffer->status;
      hash_list_insert_end(hash_list_arr[blk_num],data);
      printf("Kernel DISK access occuring. Status of Block Updated.\n");
      freeBuffer = hash_list_search_node(hash_list_arr[blk_num%4],blk_num);
      add_state(freeBuffer, KERNEL_READING_WRITING);
      printf("Kernel DISK access finished. Status of Block Updated.\n");
      remove_state(freeBuffer, KERNEL_READING_WRITING);
      printf("BUFFER now contains new and Valid Data. Updating status.\n");
      add_state(freeBuffer, BUFFER_DATA_VALID);
      return freeBuffer;

    }
     }
  }
  
  }
   }
   
//**************************************** status routines ****************************
// Add this state to buffer status
void add_state(buffer_header_node_t *p_buffer, STATE state) 
{
   p_buffer->status = (p_buffer->status) | state;

}

// Remove this state from buffer status
void remove_state(buffer_header_node_t *p_buffer, STATE state) 
{ 
  p_buffer->status = p_buffer->status ^ state; 
}

// set only this state as the buffer status
void set_state(buffer_header_node_t *p_buffer, STATE state) 
{ 
 p_buffer->status = state;

}

// check if buffer is in the given state
int is_in_state(buffer_header_node_t *p_buffer, STATE state) 
{ 
 
     return (p_buffer->status & state); 

}

// **********************************waiting que routines ******************************

// check if waiting queue is empty
int is_waiting_queue_empty() 
{
    for(int i=0; i<SIZE_OF_WAITING_QUEUE; ++i)
    {
        if(waiting_queue[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

// add process asking `blk_num` to waiting queue
void add_to_waiting_queue(unsigned int blk_num, WAITING_TYPE waitType) 
{
    int len;
    for(len=0; waiting_queue[len]!=0 && len < SIZE_OF_WAITING_QUEUE ;++len);
    if(len == SIZE_OF_WAITING_QUEUE) 
    { 
        printf("TOO MANY WAITING PROCESSES. KILLING LEAST RECENTLY USED ONE.");
        len = 0;
    }
    waiting_queue[len] = blk_num * waitType;
    //printf("wating_queue[len] =  %d\n",waiting_queue[len]);
}

int find_in_waiting_queue(int blk_num) 
{
    blk_num = blk_num*-1;
    for(int i=0; i<SIZE_OF_WAITING_QUEUE; ++i)
    {
        if(waiting_queue[i] == blk_num)
        {
            return 1;
        }
    
    }
    return 0;
}

int get_process_from_waiting_queue(int blockNumber) 
{

    WAITING_TYPE winner ;

    // randomly choose which type of process will win the race condition
    srand(time(NULL));

    if(rand()%2 == 0)
    {  
      winner = WAITING_FOR_ANY_BUFFER;  
    }
    else            
     {  
       winner = WAITING_FOR_THIS_BUFFER;
     }

    // choose a process from the winner category
    int returnValue = 0;
    for(int i=0; i<SIZE_OF_WAITING_QUEUE; ++i)
    {
        if(winner == WAITING_FOR_ANY_BUFFER && waiting_queue[i] > 0)
        {
            returnValue = waiting_queue[i];
            waiting_queue[i] = 0;
            break;
        }
        if(winner == WAITING_FOR_THIS_BUFFER && waiting_queue[i] < 0 && blockNumber == waiting_queue[i]*-1)
        {
            returnValue = waiting_queue[i] * -1;
            waiting_queue[i] = 0;
            break;
        }
    }

    // if no process exists in the winner category
    if(returnValue == 0)
    {
        
        // make the other category winner 
        if(winner == WAITING_FOR_ANY_BUFFER)
        {
            winner = WAITING_FOR_THIS_BUFFER;
        }
        else if(winner == WAITING_FOR_THIS_BUFFER)
        {
            winner = WAITING_FOR_ANY_BUFFER;
        }

        // choose a process from the winner category
        for(int i=0; i<SIZE_OF_WAITING_QUEUE; ++i)
        {
            if(winner == WAITING_FOR_ANY_BUFFER && waiting_queue[i] > 0)
            {
                returnValue = waiting_queue[i];
                waiting_queue[i] = 0;
                break;
            }

            if(winner == WAITING_FOR_THIS_BUFFER && waiting_queue[i] < 0 && blockNumber == waiting_queue[i]*-1)
            {
                returnValue = waiting_queue[i] * -1;
                waiting_queue[i] = 0;
                break;
            }
        }
    }

    if(returnValue == 0)
    {
        return 0;
    }

    // print winner and its required block number 
    if(winner == WAITING_FOR_ANY_BUFFER) {
        printf("Race Winner is a process WAITING_FOR_ANY_BUFFER ");
    }
    else if( winner == WAITING_FOR_THIS_BUFFER)
    {
        printf("Race Winner is a process WAITING_FOR_THIS_BUFFER ");
    }
    printf("and it wants block number %d.\n", returnValue);
    return returnValue;
}
