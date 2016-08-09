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

struct myargs {
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



/**
 *  Prints manual for Arguments

 *      @param options A list of myargs where the last arg is all zeros
 *      @param title    A string to print in the usage, i.e., "program name [options]"
 *      @param  exit_val    The value to pass to exit()
 */


void
myargsManual(struct myargs options[], char * title, int exit_val);


/**
 * Return a list of struct options suitable for getopt_long()
 * @param options   A list of myargs where the last arg is all zeros
 * @return A list of long options


const struct option *
myargsToLong(struct myargs options[]);
*/
/**
 * Return a string of options suitable for getopt()
 * @param options   A list of myargs where the last arg is all zeros
 * @return A string with colons for all of the short names of the options, i.e., "e:fg:"
 */
char *
myargsToShort(struct myargs options[]);


/**
 * Return the default value for the option
 *  abort() if does not exist or is not a string
 *  @param options   A list of myargs where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A string
 */
char *
myargsGetDefaultString(struct myargs options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not an int
 *  @param options   A list of myargs where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return An int
 */
int
myargsGetDefaultInteger(struct myargs options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not a flag
 *  @param options   A list of myargs where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A zero for off, a one for on
 */
short
myargsGetDefaultFlag(struct myargs options[], char * argname);
/**
 * Return the default value for the option
 *  abort() if does not exist or is not a decimal
 *  @param options   A list of myargs where the last arg is all zeros
 *  @param argname   The long name of an argument
 *  @return A double
 */
double
myargsGetDefaultDecimal(struct myargs options[], char * argname);


#endif
