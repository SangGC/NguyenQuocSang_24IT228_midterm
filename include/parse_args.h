#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include "ls.h"

/*
 * Parses command line options and locates operand arguments.
 *
 * Parameters:
 *   argc: Arguments count
 *   argv: Arguments vector
 *   opts: Pointer to Options struct to store parsed flags
 *   operand_start_idx: Pointer to int to store the starting index of file/directory operands in argv
 *
 * Returns:
 *   0 on success, or -1 if an invalid option is encountered.
 */
int parse_arguments(int argc, char *argv[], Options *opts, int *operand_start_idx);

/* Prints utility usage syntax to stderr */
void print_usage(const char *prog_name);

#endif /* PARSE_ARGS_H */
