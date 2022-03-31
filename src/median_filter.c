#include "median_filter.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief   Get median of unsorted array
 */
uint8_t median(uint8_t* buf, size_t size);

void median_filter_unsafe(const size_t ws,
                          const uint8_t* data, const size_t data_size,
                          uint8_t* filtered_data, const size_t filtered_size)
{
    size_t mid_ws = ws / 2; /* defines number of padding elements */
    uint8_t* window_buf =
        (uint8_t*)malloc(sizeof(*data) * ws); /* buffer for median() function */
    uint8_t med = 0;

    for (size_t i = mid_ws; i < data_size - mid_ws; ++i)
    {
        memcpy(window_buf, &data[i-mid_ws], ws);
        med = median(window_buf, ws);

        filtered_data[i - mid_ws] = med;
    }

    free(window_buf);
}

rc_t median_filter(const size_t ws,
                   const uint8_t* data, const size_t data_size,
                   uint8_t* filtered_data, const size_t filtered_size)
{
    if (!data || !filtered_data)
    {
        return RC_NULL_PTR;
    }

    if (ws % 2 == 0)
    {
        return RC_WS_NOT_ODD;
    }

    if (filtered_size + (ws - 1) < data_size)
    {
        return RC_INCORRECT_BUF_SIZE;
    }

    median_filter_unsafe(ws, data, data_size, filtered_data, filtered_size);

    return RC_OK;
}

rc_t median_filter_streamed(const size_t ws, const size_t bs,
                            FILE* stream_in, FILE* stream_out)
{
    if (!stream_in || !stream_out)
    {
        return RC_NULL_PTR;
    }

    if (ws % 2 == 0)
    {
        return RC_WS_NOT_ODD;
    }

    int func_ret = EXIT_SUCCESS;
    size_t num_read;
    bool first_iter = true;
    /* maximum size of elements to add for madian_filter() */
    const size_t MAX_PAD_SIZE = 3 * (ws / 2);
    /* size of batch that contains data batch with maximum possible padding */
    const size_t MAX_PREPROC_SIZE = bs + MAX_PAD_SIZE;
    /* size of filtered data */
    const size_t MAX_FILTERED_SIZE = bs;
    uint8_t* data_batch =
        (uint8_t*)calloc(bs, sizeof(uint8_t));
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

    while (!feof(stream_in))
    {
        num_read = fread(data_batch, sizeof(uint8_t),
                         bs, stream_in);
        data_size = num_read;

        /* Preprocess data */
        if (first_iter) /* first read from data stream */
        {
            /* Stream is empty */
            if (feof(stream_in) && num_read == 0)
            {
                fprintf(stderr, "stream/file is empty");
                func_ret = EXIT_FAILURE;
                goto cleanup;
            }

            /* Special case when EOF occured on first read but num_read > 0,
               i.e. size of data is less than size of data batch */
            if (feof(stream_in) && num_read < bs)
            {
                head_pad_size = ws / 2;
                tail_pad_size = ws / 2;
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
                head_pad_size = ws / 2;
                tail_pad_size = 0;
                preproc_size = data_size + head_pad_size;
                filter_size = data_size - ws / 2;
                memcpy(preprocessed_data + head_pad_size,
                       data_batch, sizeof(uint8_t) * data_size);
                /* Add block of padding in head that repeats values of the data batch */
                memset(preprocessed_data,
                       data_batch[0],
                       sizeof(uint8_t) * head_pad_size);
            }

            first_iter = false;
        }
        else if (num_read == bs) /* data stream continues */
        {
            head_pad_size = 2 * (ws / 2);
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
        else if (feof(stream_in)) /* end of data stream */
        {
            head_pad_size = 2 * (ws / 2);
            tail_pad_size = ws / 2;

            /* Copy elements from tail of previous data batch.
               Values from previous iteration are used. */
            memcpy(preprocessed_data,
                   preprocessed_data + preproc_size - head_pad_size,
                   sizeof(uint8_t) * head_pad_size);

            preproc_size = data_size + head_pad_size + tail_pad_size;
            filter_size = data_size + ws / 2;
            memcpy(preprocessed_data + head_pad_size,
                   data_batch, sizeof(uint8_t) * data_size);
        }

        median_filter_unsafe(ws, preprocessed_data, preproc_size,
                             filtered_batch, filter_size);

        fwrite(filtered_batch, sizeof(uint8_t),
               filter_size, stream_out);
    }

cleanup:
    free(data_batch);
    free(preprocessed_data);
    free(filtered_batch);

    return func_ret;
}

int cmp(const void* a, const void* b)
{
    int lhs = *(const uint8_t*)a;
    int rhs = *(const uint8_t*)b;
 
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

uint8_t median(uint8_t* buf, size_t size)
{
    qsort(buf, size, sizeof(*buf), cmp);

    return buf[size / 2];
}
