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
    rc_t mf_ret = RC_OK;
    int ret = EXIT_SUCCESS;
    size_t num_read;
    bool first_iter = true;
    const size_t WS = options->window_size;
    /* maximum size of elements to add for madian_filter() */
    const size_t MAX_PAD_SIZE = 3 * (WS / 2);
    /* size of skipped elements when calling median_filter() */
    const size_t SKIP_SIZE = options->window_size - 1;
    /* size of batch that contains data batch with maximum possible padding */
    const size_t MAX_PREPROC_SIZE = DATA_BATCH_SIZE + MAX_PAD_SIZE;
    /* size of filtered data without trailing (skipped) elements */
    const size_t MAX_FILTERED_SIZE = DATA_BATCH_SIZE;
    uint8_t* data_batch =
        (uint8_t*)calloc(DATA_BATCH_SIZE, sizeof(uint8_t));
    uint8_t* preprocessed_data =
        (uint8_t*)calloc(MAX_PREPROC_SIZE, sizeof(uint8_t));
    uint8_t* filtered_batch =
        (uint8_t*)calloc(MAX_FILTERED_SIZE, sizeof(uint8_t));
    /* actual sizes */
    size_t head_pad_size;
    size_t tail_pad_size;
    size_t data_size;
    size_t preproc_size;
    size_t filter_size;

    while (!feof(options->input))
    {
        num_read = fread(data_batch, sizeof(uint8_t),
                         DATA_BATCH_SIZE, options->input);
        data_size = num_read;

        /* Preprocess data */
        if (first_iter) /* first read from data stream */
        {
            /* Stream is empty */
            if (feof(options->input) && num_read == 0)
            {
                fprintf(stderr, "stream/file was empty");
                ret = EXIT_FAILURE;
                goto cleanup;
            }

            /* Special case when EOF occured on first read but num_read > 0,
               i.e. size of data is less than size of data batch */
            if (feof(options->input) && num_read < DATA_BATCH_SIZE)
            {
                head_pad_size = WS / 2;
                tail_pad_size = WS / 2;
                preproc_size = data_size + head_pad_size + tail_pad_size;
                filter_size = data_size;
                memcpy(preprocessed_data + head_pad_size,
                       data_batch, sizeof(uint8_t) * data_size);
                /* Add two blocks of padding in head and tail that repeats values of the data batch */
                memset(preprocessed_data,
                       data_batch[0],
                       sizeof(uint8_t) * head_pad_size);
                memset(preprocessed_data + data_size + head_pad_size,
                       data_batch[data_size],
                       sizeof(uint8_t) * tail_pad_size);
            }
            else /* if num_read == DATA_BATCH_SIZE */
            {
                head_pad_size = WS / 2;
                tail_pad_size = 0;
                preproc_size = data_size + head_pad_size;
                filter_size = data_size - WS / 2;
                memcpy(preprocessed_data + head_pad_size,
                       data_batch, sizeof(uint8_t) * data_size);
                /* Add block of padding in head that repeats values of the data batch */
                memset(preprocessed_data,
                       data_batch[0],
                       sizeof(uint8_t) * head_pad_size);
            }

            first_iter = false;
        }
        else if (num_read == DATA_BATCH_SIZE) /* data stream continues */
        {
            head_pad_size = 2 * (WS / 2);
            tail_pad_size = 0;

            /* Copy elements from tail of previous data batch.
               Values from previous iteration are used. */
            memcpy(preprocessed_data,
                   preprocessed_data + preproc_size - head_pad_size,
                   sizeof(uint8_t) * head_pad_size);

            preproc_size = data_size + head_pad_size + tail_pad_size;
            filter_size = data_size;
            memcpy(preprocessed_data + head_pad_size,
                   data_batch, sizeof(uint8_t) * data_size);
        }
        else if (feof(options->input)) /* end of data stream */
        {
            head_pad_size = 2 * (WS / 2);
            tail_pad_size = WS / 2;

            /* Copy elements from tail of previous data batch.
               Values from previous iteration are used. */
            memcpy(preprocessed_data,
                   preprocessed_data + preproc_size - head_pad_size,
                   sizeof(uint8_t) * head_pad_size);

            preproc_size = data_size + head_pad_size + tail_pad_size;
            filter_size = data_size + WS / 2;
            memcpy(preprocessed_data + head_pad_size,
                   data_batch, sizeof(uint8_t) * data_size);
        }

        mf_ret = median_filter(WS, preprocessed_data, preproc_size,
                               filtered_batch, filter_size);

        if (mf_ret != RC_OK)
        {
            fprintf(stderr, "median_filter() failed: rc=%d\n", mf_ret);
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        fwrite(filtered_batch, sizeof(uint8_t),
               filter_size, options->output);
    }

cleanup:
    free(data_batch);
    free(preprocessed_data);
    free(filtered_batch);

    return ret;
}
