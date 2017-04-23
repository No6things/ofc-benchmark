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

void enqueueMessage(char* item,  report *myreport, int delimit)
{
  struct message *temp = (struct message *)malloc(sizeof(*temp));
  temp->buffer = (char *)malloc(151 + 1);
  if (delimit) {
    snprintf(temp->buffer, 151, "%s%c", item, LIMITER);
  } else {
    snprintf(temp->buffer, 151, "%s", item);
  }

  printf("%s\n", temp->buffer);
  if (myreport->queue != NULL) {
    myreport->queue->back = temp;
    temp->next = myreport->queue;
    myreport->length = myreport->length + 1;
  } else {
    myreport->length = 1;
    myreport->first = temp;
    temp->next = NULL;
  }
  temp->back = NULL;
  myreport->queue = temp;
  free(item);
}

void dequeueMessage(report *myreport)
{
  if (myreport->queue == NULL) {
    printf("Queue is empty \n");
  } else {
    struct message *temp;
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
