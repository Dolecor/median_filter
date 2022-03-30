#include "median_filter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <libgen.h>

#define OPTSTR "i:o:w:h"
#define USAGE_FMT "%s [-i input-file] [-o output-file] [-w window-size] [-h]\n"
#define ERR_FOPEN_INPUT "fopen input err"
#define ERR_FOPEN_OUTPUT "fopen output err"
#define DATA_BATCH_SIZE 256

typedef struct
{
    size_t window_size;
    FILE* input;
    FILE* output;
} options_t;

extern char* optarg;
extern int opterr, optind;

#define DEFAULT_OPTIONS { DEFAULT_WINDOW_SIZE, stdin, stdout }

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
                if (options.window_size > MAX_WINDOW_SIZE)
                {
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

    fclose(options.input);
    fclose(options.output);

    return ret;
}

void print_usage_and_exit(char* progname, int opt)
{
    fprintf(stderr, USAGE_FMT, progname);
    exit(EXIT_FAILURE);
}

int process_input(options_t* options)
{
    rc_t mf_ret = RC_OK;
    int ret = EXIT_SUCCESS;
    size_t num_read;
    bool first_iter;
    const size_t WS = options->window_size;
    /* maximum size of elements to add for madian_filter() */
    const size_t MAX_PAD_SIZE = 3 * (WS / 2);
    /* size of skipped elements when calling median_filter() */
    const size_t SKIP_SIZE = options->window_size - 1;
    /* size of batch that contains data batch with maximum possible padding */
    const size_t MAX_BATCH_SIZE = DATA_BATCH_SIZE + MAX_PAD_SIZE;
    /* size of filtered data without trailing (skipped) elements */
    const size_t MAX_FILTERED_SIZE = DATA_BATCH_SIZE;
    uint8_t* data_batch =
        (uint8_t*)calloc(MAX_BATCH_SIZE, sizeof(uint8_t));
    uint8_t* filtered_batch =
        (uint8_t*)calloc(MAX_FILTERED_SIZE, sizeof(uint8_t));
    /* actual values */
    size_t act_head_pad_size;
    size_t act_tail_pad_size;
    size_t act_batch_size;
    size_t act_filter_size;
    first_iter = true;

    while (!feof(options->input))
    {
        if (first_iter) /* first read from data stream */
        {
            bool iseof;
            act_head_pad_size = WS / 2;
            num_read = fread(data_batch + act_head_pad_size, sizeof(uint8_t),
                             DATA_BATCH_SIZE, options->input);
            iseof = feof(options->input);

            /* stream is empty */
            if (iseof && num_read == 0)
            {
                fprintf(stderr, "stream/file was empty");
                ret = EXIT_FAILURE;
                goto cleanup;
            }

            /* special case when EOF occured on first read but num_read > 0,
               i.e. size of data is less than size of data batch */
            if (iseof && num_read < DATA_BATCH_SIZE)
            {
                act_tail_pad_size = WS / 2;
                act_batch_size = num_read + act_head_pad_size + act_tail_pad_size;
                act_filter_size = num_read;
                /* adds two blocks of padding in head and tail that repeats values of the data batch */
                memset(data_batch,
                       data_batch[act_head_pad_size],
                       sizeof(uint8_t) * act_head_pad_size);
                memset(data_batch + num_read + act_head_pad_size,
                       data_batch[num_read + act_head_pad_size],
                       sizeof(uint8_t) * act_tail_pad_size);

                mf_ret = median_filter(WS, data_batch, act_batch_size,
                                       filtered_batch, act_filter_size);

                if (mf_ret != RC_OK)
                {
                    fprintf(stderr, "filter fail: rc=%d\n", mf_ret);
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
            }
            else /* if num_read == DATA_BATCH_SIZE */
            {
                act_tail_pad_size = 0;
                act_batch_size = num_read + act_head_pad_size;
                act_filter_size = num_read;
                /* adds block of padding in head that repeats values of the data batch */
                memset(data_batch,
                       data_batch[act_head_pad_size],
                       sizeof(uint8_t) * act_head_pad_size);

                mf_ret = median_filter(WS, data_batch, act_batch_size,
                                       filtered_batch, act_filter_size);

                if (mf_ret != RC_OK)
                {
                    fprintf(stderr, "filter fail: rc=%d\n", mf_ret);
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
            }

            first_iter = false;
        }
        else if (feof(options->input)) /* end of data stream */
        {
            // WIP
            printf("feof(options->input)\n");
        }
        else if (num_read == DATA_BATCH_SIZE) /* data stream continues  */
        {

            printf("num_read == DATA_BATCH_SIZE\n");
            // WIP program reaches here from first_iter when num_read = DATA_BATCH_SIZE
            // so just exit function by now
            ret = EXIT_SUCCESS;
            goto cleanup;
        }

        fwrite(filtered_batch, sizeof(uint8_t),
               act_filter_size, options->output);
    }

cleanup:
    free(data_batch);
    free(filtered_batch);

    return ret;
}

// uint8_t data[] = {0, 2, 3, 80, 6, 2, 3, 0};


// int main(int argc, char *argv[])
// {
//     rc_t ret = RC_OK;
//     size_t data_size = sizeof(data) / sizeof(*data);
//     uint8_t* filtered = (uint8_t*)calloc(data_size, sizeof(*data));

//     ret = median_filter(DEFAULT_WINDOW_SIZE, data, data_size, filtered, data_size);

//     for (size_t i = 0; i < data_size; ++i)
//     {
//         printf("%u", data[i]);
//         printf(" ");
//     }

//     printf("\n");

//     for (size_t i = 0; i < data_size; ++i)
//     {
//         printf("%u", filtered[i]);
//         printf(" ");
//     }

//     return EXIT_SUCCESS;
// }
