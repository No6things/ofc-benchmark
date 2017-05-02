#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../include/myreport.h"

report *myreport;
report *reports;

void displayMessages(report *myreport, int  id)
{
  struct message *temp;
  int index = 0;
  temp = myreport->queues[id].first;
  printf("DISPLAY\n");
  while (temp != NULL)
  {
    printf("%s\n", temp->buffer);
    temp = temp->back;
    index++;
  }
  printf("TOTAL: %d messages\n", index);
}

void enqueueMessage(char* item,  report *myreport, int id, int delimit, int size)
{
  struct message *temp = (struct message *)malloc(sizeof(struct message));
  temp->back = (struct message *)malloc(sizeof(struct message));
  temp->next = (struct message *)malloc(sizeof(struct message));
  temp->buffer = (char *)malloc(size + 1);

  temp->back = NULL;

  if (delimit) {
    snprintf(temp->buffer, size + 1, "%s%c", item, LIMITER);
  } else {
    snprintf(temp->buffer, size + 1, "%s", item);
  }

  if (myreport->queues[id].last != NULL) {
    (myreport->queues[id].last)->back = temp;
    temp->next = myreport->queues[id].last;
    myreport->queues[id].length = myreport->queues[id].length + 1;
  } else {
    temp->next = NULL;
    myreport->queues[id].length = 1;
    myreport->queues[id].first = temp;
  }
  myreport->queues[id].last = temp;
  free(item);

  //printf("\n[ENQUEUED]%s[ENQUEUED]\n", myreport->queues[id].last->buffer);
}
