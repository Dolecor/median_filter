#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Return status codes */
typedef int rc_t;
#define RC(code) ((rc_t)(code))

/* Success*/
#define RC_OK RC(0)
/* NULL pointer */
#define RC_NULL_PTR RC(1)
/* Window size not odd */
#define RC_WS_NOT_ODD RC(2)
/* Incorrect buffer size */
#define RC_INCORRECT_BUF_SIZE RC(3)

#define DEFAULT_WINDOW_SIZE 3
#define MIN_WINDOW_SIZE 3
#define MAX_WINDOW_SIZE 255

#define DEFAULT_BATCH_SIZE 256
#define MIN_BATCH_SIZE 1

/**
 * @brief   Performs median filtering of input data. 
 *          Data should be provided with padding (e.g. zeroes),
 *          or [ws - 1] number of elements will be skipped
 * 
 * @param[in] ws             size of window
 * @param[in] data           pointer to input data
 * @param[in] data_size      size of data
 * @param[out] filtered_data pointer to median-filtered data
 * @param[in] filtered_size  size of filtered_data
 * @return rt_t
 *      RC_OK if success, 
 *      RC_NULL_PTR if data or filtered_data is NULL, 
 *      RC_WS_NOT_ODD if ws is not odd, 
 *      or RC_INCORRECT_BUF_SIZE if (filtered_size + (ws - 1) < data_size).
 */
rc_t median_filter(const size_t ws,
                   const uint8_t* data, const size_t data_size,
                   uint8_t* filtered_data, const size_t filtered_size);

/**
 * @brief   Performs median filtering of input data. 
 *          Function reads from file_in and writes to file_out.
 * 
 * @param[in] ws          size of window
 * @param[in] bs          size of batch
 * @param[in] stream_in   input data stream
 * @param[in] stream_out  output data stream
 * @return rc_t 
 *      RC_OK if success, 
 *      RC_NULL_PTR if stream_in or stream_out is NULL, 
 *      RC_WS_NOT_ODD if ws is not odd.
 */
rc_t median_filter_streamed(const size_t ws, const size_t bs,
                            FILE* stream_in, FILE* stream_out);

#endif /* MEDIAN_FILTER_H */
