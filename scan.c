/*
SCAN - Disk Scheduling Algorithm (Elevator)
scans down towards the nearest end and then when it hits the bottom,
it scans up servicing the requests that it didn't get going down.
If a request comes in after it has been scanned it will not be serviced
until the process comes back down or moves back up.
*/

#include <stdio.h>
#include <stdlib.h>

#define LOW 0
#define HIGH 199

int soma(int a, int b){ int c=a+b; return c;}
int main(){
  int queue[20];
  int head, max, q_size, temp, sum;
  int dloc; //location of disk (head) arr

  /*
  printf("%s\t", "Input no of disk locations");
  scanf("%d", &q_size);

  printf("%s\t", "Enter head position");
  scanf("%d", &head);

  printf("%s\n", "Input elements into disk queue");
  for(int i=0; i<q_size; i++){
    scanf("%d", &queue[i]);
  }
  */
  q_size=8;
  head=53;
  queue[0]=98;
  queue[1]=183;
  queue[2]=41;
  queue[3]=122;
  queue[4]=14;
  queue[5]=124;
  queue[6]=65;
  queue[7]=67;
  queue[q_size] = head; //add RW head into queue
  q_size++;

  //sort the array
  for(int i=0; i<q_size;i++){
    for(int j=i; j<q_size; j++){
      if(queue[i]>queue[j]){
        temp = queue[i];
        queue[i] = queue[j];
        queue[j] = temp;
      }
    }
  }

  max = queue[q_size-1];

  //locate head in the queue
  for(int i=0; i<q_size; i++){
    if(head == queue[i]){
      dloc = i;
      break;
    }
  }
  int anterior=head;
  int movimentos=0;
  if(abs(head-LOW) <= abs(head-HIGH)){
     printf("\nSubindo...\n");
     movimentos+=abs(anterior-head);
     printf("(%d-%d) + ",head,anterior);
     for(int j=dloc+1; j<=q_size; j++){
        
        if(j==q_size){
          //printf("%d --> ",HIGH);
          movimentos+=abs(anterior-HIGH);
          printf("(%d-%d) + ",HIGH,anterior);
          anterior=HIGH;
        }
        else{
          //printf("%d --> ",queue[j]);
          movimentos+=abs(anterior-queue[j]);
          printf("(%d-%d) + ",queue[j],anterior);
          anterior=queue[j];
        }
        
      }
      printf("\nMovimentos = %d\n",movimentos);
      printf("\nDescendo...\n");
      for(int j=dloc; j>=0; j--){
          
        if(j==q_size){
          //printf("%d --> ",LOW);
          movimentos+=abs(anterior-LOW);
          printf("(%d-%d) + ",LOW,anterior);
          anterior=LOW;
        }
        else if(j!=dloc){
          //printf("%d --> ",queue[j]);
          movimentos+=abs(anterior-queue[j]);
          printf("(%d-%d) + ",queue[j],anterior);
          anterior=queue[j];
        }
      }
            printf("\nMovimentos = %d\n",movimentos);


  } else {
for(int j=dloc; j>=0; j--){
        printf("%d --> ",queue[j]);
      }
      for(int j=dloc+1; j<q_size; j++){
        printf("%d --> ",queue[j]);
      }
     

  }




  
  printf("\nmovement of total cylinders %d", movimentos);

  return 0;
}
