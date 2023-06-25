
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cntr.h"

void usage()
{
    printf("Usage: cntr [OPTIONS]\n");
    printf("Count the number of lines in src file(s)\n");
    printf("\n");
    printf("--path      comma separated list of paths where to search for files.\n");
    printf("--exclude   comma separated list of paths to exclude from the search list\n");
    printf("--help      print this message\n");
}

void badarg(const char *opt)
{
    printf("Bad argument for \"%s\".\n", opt);
}

char **comma_separated_to_list(const char *arg, size_t *count_out)
{
    size_t len = strlen(arg);
    size_t pathcnt = 1;

    for (size_t i = 0; i < len; ++i)
        pathcnt += arg[i] == ',';

    char **list = malloc(pathcnt * sizeof(char *));
    if (!list)
        exit(2);

    size_t pathidx = 0;
    size_t start = 0;
    size_t size = 0;
    *count_out = 0;

    for (size_t i = 0; i < len + 1; ++i)
    {
        if (arg[i] == ',' || i == len)
        {
            list[pathidx] = malloc((size + 1) * sizeof(char));
            char *path = list[pathidx];
            if (!path)
                exit(2);

            memcpy(path, arg + start, size);
            path[size] = '\0';

            ++pathidx;
            start += size + 1;
            size = 0;
            ++(*count_out);
        }
        else
        {
            ++size;
        }
    }

    return list;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        return 1;
    }

    char **paths = NULL;
    size_t path_cnt = 0;

    char **exclude = NULL;
    size_t exclude_cnt = 0;

    for (size_t i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            usage();
            return 0;
        }
        else if (strcmp(argv[i], "--path") == 0)
        {
            if (i == argc - 1)
            {
                badarg(argv[i]);
                usage();
                return 1;
            }

            paths = comma_separated_to_list(argv[i + 1], &path_cnt);
            ++i;
        }
        else if (strcmp(argv[i], "--exclude") == 0)
        {
            if (i == argc - 1)
            {
                badarg(argv[i]);
                usage();
                return 1;
            }

            exclude = comma_separated_to_list(argv[i + 1], &exclude_cnt);
            ++i;
        }
        else
        {
            printf("Unknown argument \"%s\".\n", argv[i]);
            usage();
            return 1;
        }
    }

    init();
    run(paths, path_cnt, exclude, exclude_cnt);
    dump();
    destroy();

    for (size_t i = 0; i < path_cnt; ++i)
        free(paths[i]);
    free(paths);

    for (size_t i = 0; i < exclude_cnt; ++i)
        free(exclude[i]);
    free(exclude);

    return 0;
}
