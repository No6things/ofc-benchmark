#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/loci/loci.h"
#include "myargs.h"

static int
countOptions(struct myargs options[]);
static int
findArg(struct myargs options[], char * argname);

/************************************************************************/
void myargsManual(struct myargs options[], char * title, int exit_val)
{
    #define nameFMT "-20"//Title Format
    #define descriptionFMT "6"  //Option Format
    #define exampleFMT "-70" //Comment Format
    struct myargs * optptr;
    if(title) fprintf(stderr, "%s\n", title);
    for(optptr = &options[0]; optptr->name != NULL ; optptr++)
    {
        fprintf(stderr, "   -%c || --%"nameFMT"s  ", optptr->id, optptr->name); // i.e:"  -NS || -NumberofSwitches "
        switch(optptr->type)
        {
            case NONE:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s\n", "",optptr->description);
                break;
            case INTEGER:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(%d)\n", "<int>", optptr->description, optptr->default_val.integer);
                break;
            case FLAG:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(%s)\n", "", optptr->description, optptr->default_val.flag? "on" : "off");
                break;
            case STRING:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(\"%s\")\n", "<str>", optptr->description, optptr->default_val.string);
                break;
            case DECIMAL:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(%lf)\n", "<real>", optptr->description, optptr->default_val.decimal);
                break;
            default:
                fprintf(stderr, "--- unhandled argument type %d", optptr->type);
                abort();
        };
    }
    fprintf(stderr, "\n");
    exit(exit_val);
}
/***********************************************************************x
const struct option *
myargs_to_long(struct myargs options[])
{
    struct option * longopts;
    int n = count_options(options);
    int i;
    longopts = malloc(sizeof(struct option) * (n+1));
    for(i=0;i<=n;i++)
    {
        if(options[i].name)
            longopts[i].name = strdup(options[i].name);
        else
            longopts[i].name = NULL;
        if( options[i].type == NONE )
            longopts[i].has_arg = no_argument;
        else if ( options[i].type == FLAG)
            longopts[i].has_arg = optional_argument;
        else
            longopts[i].has_arg = required_argument;
        longopts[i].flag =  NULL;
        longopts[i].val  = options[i].shortname;
    }
    return longopts;
}*/
/************************************************************************/
char * myargsToShort(struct myargs options[])
{
    char * shortargs;
    int n = countOptions(options);
    int i;
    int len=0;
    int max = n*2 + 1;
    shortargs = malloc(max);
    for(i=0; i< n; i++)
    {
        len+= snprintf(&shortargs[len], max-len, "%c",
                options[i].id);
        if(options[i].type != NONE && options[i].type != FLAG)
            len+= snprintf(&shortargs[len], max-len, ":");
    }
    shortargs[len]=0;
    return shortargs;
}
/************************************************************************/
int countOptions(struct myargs options[])
{
    int count = 0;
    struct myargs *opt;
    for ( opt = &options[0]; opt->name != NULL; opt++)
        count++;
    return count;
}
/************************************************************************/
int findArg(struct myargs options[], char * arg)
{
    int i = 0;
    struct myargs *opt;
    for ( opt = &options[0]; opt->name != NULL; opt++)
    {
        if(!strcmp(opt->name, arg))
            return i;
        i++;
    }
    return -1;
}
/************************************************************************/
char * myargsGetDefaultString(struct myargs options[], char * argname)
{
    int ARGNAME_NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != ARGNAME_NOTFOUND);
    assert(options[i].type == STRING);
    return options[i].default_val.string;
}
/************************************************************************/
int myargsGetDefaultInteger(struct myargs options[], char * argname)
{
    int ARGNAME_NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != ARGNAME_NOTFOUND);
    assert(options[i].type == INTEGER);
    return options[i].default_val.integer;
}
/************************************************************************/
short myargsGetDefaultFlag(struct myargs options[], char * argname)
{
    int ARGNAME_NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != ARGNAME_NOTFOUND);
    assert(options[i].type == FLAG);
    return options[i].default_val.flag;
}
