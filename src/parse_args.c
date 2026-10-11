#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse_args.h"

void print_usage(const char *prog_name)
{
    fprintf(stderr, "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n", prog_name);
}

int parse_arguments(int argc, char *argv[], Options *opts, int *operand_start_idx)
{
    if (opts == NULL || operand_start_idx == NULL) {
        return -1;
    }

    /* Set default option values */
    opts->time_type = TIME_MTIME;
    opts->dir_handling = DIR_DEFAULT;
    opts->size_format = SIZE_BYTES;
    opts->output_format = FMT_SHORT;
    opts->char_handling = CHAR_DEFAULT;

    opts->show_almost_all = false;
#ifndef _WIN32
    if (getuid() == 0) {
        opts->show_almost_all = true;
    }
#endif
    opts->show_all = false;
    opts->classify = false;
    opts->unsorted = false;
    opts->print_inode = false;
    opts->reverse_sort = false;
    opts->sort_by_size = false;
    opts->print_blocks = false;
    opts->sort_by_time = false;

    /* Reset getopt state */
    optind = 1;

    int ch;
    const char *optstring = "AacdFfhiklnqRrSstuw";

    while ((ch = getopt(argc, argv, optstring)) != -1) {
        switch (ch) {
        case 'A':
            /* -A and -a override each other (last specified determines behavior) */
            opts->show_almost_all = true;
            opts->show_all = false;
            break;
        case 'a':
            opts->show_all = true;
            opts->show_almost_all = false;
            break;
        case 'c':
            /* -c and -u override each other */
            opts->time_type = TIME_CTIME;
            break;
        case 'd':
            /* -d and -R override each other */
            opts->dir_handling = DIR_PLAIN;
            break;
        case 'F':
            opts->classify = true;
            break;
        case 'f':
            opts->unsorted = true;
            break;
        case 'h':
            /* -h and -k override each other */
            opts->size_format = SIZE_HUMAN;
            break;
        case 'i':
            opts->print_inode = true;
            break;
        case 'k':
            /* -k and -h override each other */
            opts->size_format = SIZE_KB;
            break;
        case 'l':
            /* -l and -n override each other */
            opts->output_format = FMT_LONG_NAME;
            break;
        case 'n':
            /* -n and -l override each other */
            opts->output_format = FMT_LONG_NUMERIC;
            break;
        case 'q':
            /* -q and -w override each other */
            opts->char_handling = CHAR_QUESTION;
            break;
        case 'R':
            /* -R and -d override each other */
            opts->dir_handling = DIR_RECURSIVE;
            break;
        case 'r':
            opts->reverse_sort = true;
            break;
        case 'S':
            /* -S and -t override each other */
            opts->sort_by_size = true;
            opts->sort_by_time = false;
            break;
        case 's':
            opts->print_blocks = true;
            break;
        case 't':
            /* -t and -S override each other */
            opts->sort_by_time = true;
            opts->sort_by_size = false;
            break;
        case 'u':
            /* -u and -c override each other */
            opts->time_type = TIME_ATIME;
            break;
        case 'w':
            /* -w and -q override each other */
            opts->char_handling = CHAR_RAW;
            break;
        case '?':
        default:
            print_usage(argv[0]);
            return -1;
        }
    }

    *operand_start_idx = optind;
    return 0;
}
