#include "median_filter.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief get median of unsorted array
 */
uint8_t median(uint8_t* buf, size_t size);

rc_t median_filter(size_t ws,
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

    return RC_OK;
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
