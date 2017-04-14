#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../include/myreport.h"

void display(report *myreport)
{
  struct message *temp;
  temp = myreport->list;
  printf(" \n");
  while (temp != NULL)
  {
    printf("%s\t", temp->buffer);
    temp = temp->next;
  }
}


void enqueue(char* item, report *myreport)
{
  struct message *temp = (struct message *)malloc(sizeof(struct message));
  temp->buffer = item;
  temp->next = myreport->list;
  myreport->list = temp;
}


void dequeue(report *myreport)
{
  if (myreport->list == NULL) {
    printf("\n\nqueue is empty \n");
  } else {
    struct message *temp;
    temp = myreport->list;
    myreport->list = temp->next;
    printf("\n\n%s deleted", temp->buffer);
    free(temp);
  }
}
