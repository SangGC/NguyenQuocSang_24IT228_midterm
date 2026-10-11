#ifndef LS_H
#define LS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

/* Compatibility macros for POSIX/NetBSD symlink handling on Windows build env */
#ifndef S_ISLNK
#ifdef S_IFLNK
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(m) (0)
#endif
#endif

#if defined(_WIN32) && !defined(lstat)
#define lstat stat
#endif

/* Enum for time type selection (-c vs -u) */
typedef enum {
    TIME_MTIME = 0, /* Default: file modification time */
    TIME_CTIME,     /* -c: file status last changed time */
    TIME_ATIME      /* -u: file last access time */
} TimeType;

/* Enum for directory handling (-d vs -R) */
typedef enum {
    DIR_DEFAULT = 0, /* List directory contents */
    DIR_PLAIN,       /* -d: list directories as plain files (no recursion/contents) */
    DIR_RECURSIVE    /* -R: recursively list subdirectories */
} DirHandling;

/* Enum for size format display (-h vs -k) */
typedef enum {
    SIZE_BYTES = 0,  /* Default: bytes */
    SIZE_HUMAN,      /* -h: human-readable format (B, KB, MB, etc.) */
    SIZE_KB          /* -k: kilobytes */
} SizeFormat;

/* Enum for owner/group display format (-l vs -n) */
typedef enum {
    FMT_SHORT = 0,   /* Default: short format (one entry per line) */
    FMT_LONG_NAME,   /* -l: long format with owner & group names */
    FMT_LONG_NUMERIC /* -n: long format with numeric UID & GID */
} OutputFormat;

/* Enum for non-printable character handling (-q vs -w) */
typedef enum {
    CHAR_DEFAULT = 0, /* Default: -q if output is terminal, -w if not */
    CHAR_QUESTION,    /* -q: force printing non-printable chars as '?' */
    CHAR_RAW          /* -w: force raw printing of non-printable chars */
} NonPrintableChar;

/* Structure storing state of all command line options */
typedef struct {
    /* Flags with override pairs */
    TimeType time_type;            /* -c vs -u */
    DirHandling dir_handling;      /* -d vs -R */
    SizeFormat size_format;        /* -h vs -k */
    OutputFormat output_format;    /* -l vs -n */
    NonPrintableChar char_handling;/* -q vs -w */

    /* Individual boolean flags */
    bool show_almost_all; /* -A: list all except . and .. */
    bool show_all;        /* -a: include directory entries starting with . */
    bool classify;        /* -F: display indicator symbols (/, *, @, %, =, |) */
    bool unsorted;        /* -f: unsorted output */
    bool print_inode;     /* -i: print file inode number */
    bool reverse_sort;    /* -r: reverse sort order */
    bool sort_by_size;    /* -S: sort by size, largest first */
    bool print_blocks;    /* -s: print file system blocks count */
    bool sort_by_time;    /* -t: sort by time modified/accessed/changed */
} Options;

/* Structure representing a single file entry */
typedef struct FileEntry {
    char *name;           /* File basename */
    char *path;           /* Full path to file */
    struct stat st;       /* File stat metadata */
    bool stat_valid;      /* True if stat/lstat succeeded */
    char *link_target;    /* Path target if symlink, otherwise NULL */
} FileEntry;

/* Structure representing a dynamic list of file entries */
typedef struct EntryList {
    FileEntry *entries;
    size_t count;
    size_t capacity;
} EntryList;

#endif /* LS_H */
