#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#define totaltickets 10


/* ************************************************** Linked List */
// Linked List altered to run in this problem
// Generic Linked List in C
// Creditos : GeekforGeeks
// Site : https://www.geeksforgeeks.org/generic-linked-list-in-c-2/


struct node {
   int b; // burst time - Time required by a process for CPU execution.
   int p; // priority
   int t; // tickets
   int key; // list order
   int l; //lotery
   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

//display the list
void printList() {
   struct node *ptr = head;
   printf("\n |  Process  |  Priority  |  Brust  |\n\n");

   //start from the beginning
   while(ptr != NULL) {
      printf(" |  %d  |  %d  |  %d  |\n",ptr->key,ptr->p, ptr->b);
      ptr = ptr->next;
   }

}

//insert link at the first location
void insertFirst(int key, int b, int p) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->key = key;
   link->b = b;
   link->p =p;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

//delete first item
struct node* deleteFirst() {

   //save reference to first link
   struct node *tempLink = head;
	
   //mark next to first link as first 
   head = head->next;
	
   //return the deleted link
   return tempLink;
}

//is list empty
bool isEmpty() {
   return head == NULL;
}

int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

//find a link with given key
struct node* find(int key) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}

//delete a link with given key
struct node* delete(int key) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}

void sort() {

   int i, j, k, tempKey, tempP, tempB;
   struct node *current;
   struct node *next;
	
   int size = length();
   k = size ;
	
   for ( i = 0 ; i < size - 1 ; i++, k-- ) {
      current = head;
      next = head->next;
		
      for ( j = 1 ; j < k ; j++ ) {   

         if ( current->p > next->p ) {
            tempP = current->p;
            current->p = next->p;
            next->p = tempP;

            tempKey = current->key;
            current->key = next->key;
            next->key = tempKey;
	
	    tempB = current->b;
            current->b = next->b;
            next->b = tempB;

         }
			
         current = current->next;
         next = next->next;
	 
      }
   }   
}

void reverse(struct node** head_ref) {
   struct node* prev   = NULL;
   struct node* current = *head_ref;
   struct node* next;
	
   while (current != NULL) {
      next  = current->next;
      current->next = prev;   
      prev = current;
      current = next;
   }
	
   *head_ref = prev;
}


/* ************************************************** Thread */
int tbt=0;
int quantum=1;
int taime;
int m_ticket=0;

void pth( int pID ) //Simula um Processamento qualquer
{
	printf("\n\t\t\t\t  Process That Are Running Is: %i",pID);
	int j, k;
	for( j=0; j<16384; j++ )
		for( k=0; k<8192; k++) ;
}


void countTbT() {
   struct node *ptr = head;
   while(ptr != NULL) {
      tbt=tbt+ptr->b;
      ptr = ptr->next;
   }

}

void lotery() {
   	struct node *ptr = head;
	//assign one or more lottery numbers to each process
	int p=1;
	while(ptr != NULL) {
		ptr->l = (ptr->b/quantum) + (taime-ptr->p);
		for (int z=0;z<ptr->l;z++) 
		{
        	    ptr->t = p++;
        	    m_ticket = p;
        	}
		ptr = ptr->next;
	}
}

void winner(void){
	struct node *ptr = head;
	int q;
	struct node *qNode;
	

	while(taime!=tbt)
	{
	
		int winner = (rand()%m_ticket-1)+ 1;
		while(ptr != NULL) {
			for (int z=0;z<ptr->l;z++) 
			{
				if(ptr->t == winner)
					q=ptr->key;
        		}
			ptr = ptr->next;
		}
		        
		printf("\n\n-------------------------------------\n");
		printf("	<<<<   Winner: %d   >>>>	\n",winner);
		printf("-------------------------------------\n\n");
		
		printf("%i",q);
		qNode=find(q);

		if ((qNode->b) >0)  
		{
		   qNode->b -= quantum;
		
			if ((qNode->b) >0) 
			{
		    		taime+=quantum;
			} 
			else 
			{
		    		taime+=(qNode->b)+quantum;
			}
		
			if((qNode->b)<0)
			{
		        	qNode->b=0;
			}		    
			
			pth(ptr->key);
			printf("\n\t   (Total Time << Remaining Brust Time Of This Process << process ):  ( %d  <<  %d  <<  %c )\n",taime,qNode->b, qNode->p);
	       
		}
	    else
	    {
	    	printf("\n\t\t     >>>>>>Related Process With This Ticket Has Been Completed<<<<<<\n");
		}

	}
}

//display the list
void printListTicket() {
   struct node *ptr = head;
   printf("\n\n\nPriority  Process  Brust  Lottery  Tickets");

   //start from the beginning
   while(ptr != NULL) {
      printf("\n  %d\t    %d\t    %d\t    %d \t    %d\t",ptr->p, ptr->key, ptr->b,ptr->l,ptr->t);
      ptr = ptr->next;
   }

}

void creatList(void){
	//List priority x burst
	// priority 0 to 20
	// burst sempre > 0
	insertFirst(1,rand()%100,rand()%20);
	insertFirst(2,rand()%100,rand()%20);
	insertFirst(3,rand()%100,rand()%20);
	insertFirst(4,rand()%100,rand()%20);
	insertFirst(5,rand()%100,rand()%20);
	insertFirst(6,rand()%100,rand()%20);
	insertFirst(7,rand()%100,rand()%20);
	insertFirst(8,rand()%100,rand()%20); 
	insertFirst(9,rand()%100,rand()%20);
	insertFirst(10,rand()%100,rand()%20);

}


/* ************************************************** Main */
int main (void) {
	creatList();
	taime= length()-1;
	sort();
	printList();
	countTbT();
	lotery();
	printListTicket();
	winner();	
	
	return 0;

} 
