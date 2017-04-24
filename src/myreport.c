#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../include/myreport.h"

report *myreport;
report *snmpReport;
report *reports;

void displayMessages(report *myreport)
{
  struct message *temp;
  int index = 0;
  temp = myreport->first;
  printf("DISPLAY\n");
  while (temp != NULL)
  {
    printf("%s\n", temp->buffer);
    temp = temp->back;
    index++;
  }
  printf("TOTAL: %d messages\n", index);
}

void enqueueMessage(char* item,  report *myreport, int delimit, int size)
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
  if (myreport->queue != NULL) {
    (myreport->queue)->back = temp;
    temp->next = myreport->queue;
    myreport->length = myreport->length + 1;
  } else {
    temp->next = NULL;
    myreport->length = 1;
    myreport->first = temp;
  }
  myreport->queue = temp;
  free(item);

  printf("%s\n", temp->buffer);
}

void dequeueMessage(report *myreport)
{
  struct message *temp;
  if (myreport->queue == NULL) {
    printf("Queue is empty \n");
  } else {
    temp = myreport->queue;
    while(temp != NULL) {
      myreport->queue = temp->next;
      temp->next = NULL;
      temp->back = NULL;
      printf("\n%s deleted", temp->buffer);
      free(temp);
      temp = myreport->queue;
    }
  }
}

char * parseReports()
{
  char * result = (char *)malloc(151);
  //
  return result;
}
