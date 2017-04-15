#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../include/myreport.h"

report *reports;

void displayMessages(report *myreport)
{
  struct message *temp;
  int index = 0;
  temp = myreport->list;
  printf("--- Entire report --- \n");
  while (temp != NULL)
  {
    printf("message %d : %s\n", index, temp->buffer);
    temp = temp->next;
    index++;
  }
}

void enqueueMessage(char* item,  report *myreport)
{
  struct message *temp = (message *)malloc(sizeof(struct message));
  temp->buffer = item;
  if (myreport->list != NULL) {
    temp->next = myreport->list;
    //printf("last item: %s | ", (myreport->list)->buffer); //it may be needed to use & instead
  } else {
    temp->next = NULL;
  }
  myreport->list = temp;
  //printf("new last item:  %s\n", (myreport->list)->buffer);

}

void dequeueMessage(report *myreport)
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
