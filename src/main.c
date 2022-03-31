#include "median_filter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <libgen.h>

#define OPTSTR "i:o:w:b:h"
#define USAGE_FMT "%s [-i input-file] [-o output-file] [-w window-size] [-b batch-size] [-h]\n"
#define ERR_FOPEN_INPUT "fopen input err"
#define ERR_FOPEN_OUTPUT "fopen output err"
#define ERR_WINDOW_SIZE "Wrong window size. w must be in [3, 255]"
#define ERR_BATCH_SIZE "Wrong batch size. b must be not less than 1"

typedef struct
{
    size_t window_size;
    size_t batch_size;
    FILE* input;
    FILE* output;
} options_t;

extern char* optarg;
extern int opterr, optind;

#define DEFAULT_OPTIONS { DEFAULT_WINDOW_SIZE, DEFAULT_BATCH_SIZE, stdin, stdout }

void print_usage_and_exit(char* progname, int opt);
int process_input(options_t* options);


int main(int argc, char *argv[])
{
    int ret;
    int opt;
    options_t options = DEFAULT_OPTIONS;

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
    {
        switch(opt)
        {
            case 'i':
                if (!(options.input = fopen(optarg, "r+b")))
                {
                    perror(ERR_FOPEN_INPUT);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'o':
                if (!(options.output = fopen(optarg, "w+b")))
                {
                    perror(ERR_FOPEN_OUTPUT);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'w':
                options.window_size = (size_t)strtoull(optarg, NULL, 0);
                if (options.window_size < MIN_WINDOW_SIZE
                    || options.window_size > MAX_WINDOW_SIZE)
                {
                    fprintf(stderr, ERR_WINDOW_SIZE"\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'b':
                options.batch_size = (size_t)strtoull(optarg, NULL, 0);
                if (options.batch_size < MIN_BATCH_SIZE)
                {
                    fprintf(stderr, ERR_BATCH_SIZE"\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
            default:
                print_usage_and_exit(basename(argv[0]), opt);
                break;
        }
    }

    if ((ret = process_input(&options)) != EXIT_SUCCESS)
    {
        fprintf(stderr, "error in process_input()\n");
    }

    if (options.input != NULL) fclose(options.input);
    if (options.output != NULL) fclose(options.output);

    return ret;
}

void print_usage_and_exit(char* progname, int opt)
{
    fprintf(stderr, USAGE_FMT, progname);
    exit(EXIT_FAILURE);
}

int process_input(options_t* options)
{
    rc_t rc = median_filter_streamed(options->window_size, options->batch_size,
                                     options->input, options->output);

    return (rc == RC_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}
