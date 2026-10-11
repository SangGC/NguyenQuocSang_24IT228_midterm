#include <stdio.h>
#include <stdlib.h>
#include "ls.h"
#include "parse_args.h"
#include "traverse.h"
#include "sort.h"
#include "display.h"
#include "utils.h"

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32)
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    Options opts;
    int operand_start_idx = 1;

    if (parse_arguments(argc, argv, &opts, &operand_start_idx) != 0) {
        exit(EXIT_FAILURE);
    }

    utils_init();

    int status = process_operands(argc, argv, operand_start_idx, &opts);
    if (status != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
