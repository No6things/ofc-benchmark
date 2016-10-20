#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/myargs.h"

static int
countOptions(struct args options[]);
static int
findArg(struct args options[], char * argname);

void
argsManual(struct args options[], char * title, int exit_val)
{
    #define nameFMT "-20"       //Title Format
    #define descriptionFMT "6"  //Option Format
    #define exampleFMT "-70"    //Comment Format
    struct args * optptr;
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
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(%s)\n", "<flag>", optptr->description, optptr->default_val.flag? "on" : "off");
                break;
            case STRING:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(\"%s\")\n", "<str>", optptr->description, optptr->default_val.string);
                break;
            case DECIMAL:
                fprintf(stderr, " %"descriptionFMT"s %"exampleFMT"s | Default value:(%lf)\n", "<double>", optptr->description, optptr->default_val.decimal);
                break;
            default:
                fprintf(stderr, "Unhandled argument type %d", optptr->type);
                abort();
        };
    }
    fprintf(stderr, "\n");
    exit(exit_val);
}


const struct option *
argsToLong(struct args options[])
{
    struct option * longOpts;
    int n = countOptions(options);
    int i;
    longOpts = malloc(sizeof(struct option) * (n+1));
    for(i=0;i<=n;i++)
    {
        if(options[i].name)
            longOpts[i].name = strdup(options[i].name);
        else
            longOpts[i].name = NULL;
        if( options[i].type == NONE )
            longOpts[i].has_arg = no_argument;
        else if ( options[i].type == FLAG)
            longOpts[i].has_arg = optional_argument;
        else
            longOpts[i].has_arg = required_argument;
        longOpts[i].flag =  NULL;
        longOpts[i].val  = options[i].id;
    }
    return longOpts;
}


char *
argsToShort(struct args options[])
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

int
countOptions(struct args options[])
{
    int count = 0;
    struct args *opt;
    for ( opt = &options[0]; opt->name != NULL; opt++)
        count++;
    return count;
}

int
findArg(struct args options[], char * arg)
{
    int i = 0;
    struct args *opt;
    for ( opt = &options[0]; opt->name != NULL; opt++)
    {
        if(!strcmp(opt->name, arg))
            return i;
        i++;
    }
    return -1;
}

char *
argsGetDefaultStr(struct args options[], char * argname)
{
    int NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != NOTFOUND);
    assert(options[i].type == STRING);
    return options[i].default_val.string;
}

int
argsGetDefaultInt(struct args options[], char * argname)
{
    int NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != NOTFOUND);
    assert(options[i].type == INTEGER);
    return options[i].default_val.integer;
}

short
argsGetDefaultFlag(struct args options[], char * argname)
{
    int NOTFOUND = -1;
    int i = findArg(options,argname);
    assert(i != NOTFOUND);
    assert(options[i].type == FLAG);
    return options[i].default_val.flag;
}
