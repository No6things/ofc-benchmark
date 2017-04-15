#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../include/myreport.h"

void displayMessages(struct report *myreport)
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

void enqueueMessage(char* item, struct report *myreport)
{
  printf("about to enque %s\n", item);
  struct message *temp = (struct message *)malloc(sizeof(struct message));
  temp->buffer = item;
  temp->next = myreport->list;
  printf("last item - %s |", (myreport->list)->buffer); //it may be needed to use & instead
  myreport->list = temp;
  printf("new last item - %s\n", (myreport->list)->buffer);
  printf("end of queue\n");
}

void dequeueMessage(struct report *myreport)
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
