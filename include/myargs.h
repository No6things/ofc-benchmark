#ifndef MYARGS_H
#define MYARGS_H

#include <getopt.h>

enum argsType {
    NONE,
    INTEGER,
    FLAG,
    STRING,
    DECIMAL
};



struct args {
    char *  name;
    char    id;
    char * description;
    enum argsType type;
    union argValue
    {
        unsigned short   none;
        int     integer;
        short   flag;
        char *  string;
        double  decimal;
    } default_val;
};

/*-------------------------Functions------------------------------*/

/**
 *  Prints manual for Arguments

 * @param options A list of args where the last arg is all zeros for null purposes
 * @param title    A string to print in the usage, i.e., "program name [options]"
 * @param  exit_val    The value to pass to exit()
 */
void
myargsManual(struct args options[], char * title, int exit_val);


/**
 * Return a list of long options suitable for getopt_long() from getopt.h
 *  @param options   A list of args where the last arg is all zeros
 *  @return A list of long options defined by getopt.h

      struct option {
      #if defined (__STDC__) && __STDC__
        const char *name;
      #else
        char *name;
      #endif
        has_arg can't be an enum because some compilers complain about
           type mismatches in all the code that assumes it is an int.
        int has_arg;
        int *flag;
        int val;
      };
 */
const struct option *
myargsToLong(struct args options[]);

/**
 * Return a string of short options suitable for getopt() from getopt.h
 *  @param options   A list of args where the last arg is all zeros
 *  @return A string with colons for all of the short names of the options, i.e., "e:fg:"
 */
char *
myargsToShort(struct args options[]);


/**
 * Return the default value for the option
 *  abort() if does not exist or is not a string
 *  @param options   A list of args where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A string
 */
char *
myargsGetDefaultStr(struct args options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not an int
 *  @param options   A list of args where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return An int
 */
int
myargsGetDefaultInt(struct args options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not a flag
 *  @param options   A list of args where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A zero for off, a one for on
 */
short
myargsGetDefaultFlag(struct args options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not a decimal
 *  @param options   A list of args where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A double
 */
double
myargsGetDefaultDec(struct args options[], char * argname); //not implemented


#endif
