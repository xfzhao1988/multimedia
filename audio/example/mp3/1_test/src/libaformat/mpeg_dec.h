#ifndef _MPEG_DEC_H_
#define _MPEG_DEC_H_

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#include <stdint.h>
#include <stdbool.h>

/* 当拿到duration信息后停止parse */
#define MPEG_DEC_FLAG_DURATION_PARSED_STOP 0x0001
#define MPEG_DEC_FLAG_EOF 0x0002

typedef void* mpeg_decoder;

typedef enum
{
    MPEG_DEC_EOK = 0, // There is no error
    MPEG_DEC_ERROR = -1, // A generic error happens
    MPEG_DEC_EINVAL = -2, // Invalid argument
    MPEG_DEC_ENOMEM = -3, // No memory
    MPEG_DEC_EINVAL_FILE = -4, // Invalid audio file
    MPEG_DEC_ESTATE_NOT_ALLOW = -5, // Decorder state not allow parse
    MPEG_DEC_ENEED_MORE_DATA = -6, // Parse continue need more data
    MPEG_DEC_EPARSE_FAIL = -7, // Parse fail
} mpeg_dec_error_e;

typedef enum
{
    MPEG_DEC_META_KEY_ID3V1, //id3v1相关字段
    MPEG_DEC_META_KEY_ID3V2, //id3v2相关字段
    MPEG_DEC_META_KEY_DURATION, //duration
} mpeg_dec_meta_key_e;

typedef struct
{
    uint32_t size;
} id3v1_taginfo_t;

typedef struct
{
    uint32_t size;
} id3v2_taginfo_t;

typedef struct
{
    mpeg_dec_meta_key_e key;

    union
    {
        id3v1_taginfo_t id3v1;
        id3v2_taginfo_t id3v2;
        uint64_t duration;
    } meta_data;
} mpeg_dec_meta_data_t;

typedef struct
{
    void (*on_meta_data) (mpeg_dec_meta_key_e key, const mpeg_dec_meta_data_t* meta_data);
    void (*on_complete) (void);
} mpeg_dec_parse_callbacks_t;

int32_t mpeg_dec_init(mpeg_decoder* dec);
int32_t mpeg_dec_parse_file(mpeg_decoder dec,
                            const char* file_name,
                            const mpeg_dec_parse_callbacks_t* cbs,
                            uint32_t flags);
int32_t mpeg_dec_parse_buf(mpeg_decoder dec,
                           const uint8_t* buf,
                           uint64_t size,
                           const mpeg_dec_parse_callbacks_t* cbs,
                           uint32_t flags,
                           bool eof);
//int32_t mpeg_dec_parse_cb(mpeg_decoder* dec);
int32_t mpeg_dec_parse_reset(mpeg_decoder dec);
int32_t mpeg_dec_deinit(mpeg_decoder* dec);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_MPEG_DEC_H_