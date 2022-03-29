#include "median_filter.h"

#include <stdlib.h>
#include <stdio.h>

uint8_t data[] = {0, 2, 3, 80, 6, 2, 3, 0};


int main(int argc, char *argv[])
{
    rc_t ret = RC_OK;
    size_t data_size = sizeof(data) / sizeof(*data);
    uint8_t* filtered = (uint8_t*)calloc(data_size, sizeof(*data));

    ret = median_filter(DEFAULT_WINDOW_SIZE, data, data_size, filtered, data_size);

    for (size_t i = 0; i < data_size; ++i)
    {
        printf("%u", data[i]);
        printf(" ");
    }

    printf("\n");

    for (size_t i = 0; i < data_size; ++i)
    {
        printf("%u", filtered[i]);
        printf(" ");
    }

    return EXIT_SUCCESS;
}
