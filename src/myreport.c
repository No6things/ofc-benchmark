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
  temp = myreport->list;
  printf("DISPLAY\n");
  while (temp != NULL)
  {
    printf("%s\n", temp->buffer);
    temp = temp->next;
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
  if (myreport->list != NULL) {
    temp->next = myreport->list;
    myreport->length = myreport->length + 1;
  } else {
    myreport->length = 1;
    temp->next = NULL;
  }
  myreport->list = temp;
  free(item);
}

void dequeueMessage(report *myreport)
{
  if (myreport->list == NULL) {
    printf("Queue is empty \n");
  } else {
    struct message *temp;
    temp = myreport->list;
    myreport->list = temp->next;
    printf("\n%s deleted", temp->buffer);
    free(temp);
  }
}

char * parseReports()
{
  char * result = (char *)malloc(151);
  //
  return result;
}
