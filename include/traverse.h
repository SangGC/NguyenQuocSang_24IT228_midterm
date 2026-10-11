#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "ls.h"

/*
 * Processes file and directory operands according to the given options.
 *
 * Parameters:
 *   argc: Arguments count
 *   argv: Arguments vector
 *   operand_start_idx: Index in argv where operands start
 *   opts: Pointer to parsed Options
 *
 * Returns:
 *   0 on success, non-zero on error.
 *
 * TODO: Fully implement traversal and lstat/opendir logic in Phase 3.
 */
int process_operands(int argc, char *argv[], int operand_start_idx, const Options *opts);

#endif /* TRAVERSE_H */
