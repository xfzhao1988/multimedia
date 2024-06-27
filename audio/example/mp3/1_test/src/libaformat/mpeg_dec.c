#include <stdio.h>

#include "dbg.h"
#include "mpeg_dec.h"
#include "list.h"
#include "u_bit_stream.h"
#include "u_byte_stream.h"

#define ID3V2_TAG_HEADER_SIZE 10
#define ID3V2_TAG_ID "ID3"

#define MP3_BUF_SIZE (16*1024) /* buffer which can hold minimum 10 consecutive mp3 frames (~16KB) worst case */

#define MPEG_AUDIO_FRAME_HEADER_LEN         4

#define MAX_FREE_FORMAT_FRAME_SIZE          2304 /* more than ISO spec's */
#ifndef MAX_FRAME_SYNC_MATCHES
#define MAX_FRAME_SYNC_MATCHES              10
#endif /* MAX_FRAME_SYNC_MATCHES */

#define CHECK_ID3V2_HEADER_VERSION(x)       (x == 2 || x == 3 || x == 4)
#define HDR_GET_LAYER(h)                    (((h[1]) >> 1) & 3)
#define HDR_GET_BITRATE(h)                  ((h[2]) >> 4)
#define HDR_GET_SAMPLE_RATE(h)              (((h[2]) >> 2) & 3)
#define HDR_IS_LAYER_1(h)                   ((h[1] & 6) == 6)
#define HDR_IS_FRAME_576(h)                 ((h[1] & 14) == 2)
#define HDR_IS_FREE_FORMAT(h)               (((h[2]) & 0xF0) == 0)
#define HDR_TEST_MPEG1(h)                   ((h[1]) & 0x8)
#define HDR_TEST_NOT_MPEG25(h)              ((h[1]) & 0x10)
#define HDR_TEST_PADDING(h)                 ((h[2]) & 0x2)

// Xing/Info and VBRI tag defines
#define VBRI_TAG_OFFSET                     32
#define XING_FLAG_FRAMES                    0x01
#define XING_FLAG_SIZE                      0x02
#define XING_FLAG_TOC                       0x04
#define XING_FLAC_QSCALE                    0x08

/**
 * Audio version ID
 *
 * 00 - MPEG Version 2.5 (unofficial extension of MPEG 2)
 * 01 - reserved
 * 10 - MPEG Version 2 (ISO/IEC 13818-3)
 * 11 - MPEG Version 1 (ISO/IEC 11172-3)
*/
typedef enum
{
    MPEG_VERSION_FALSE = -1,
    MPEG_VERSION_2_5,
    MPEG_VERSION_Reserved,
    MPEG_VERSION_2,
    MPEG_VERSION_1
} mpeg_version_e;

/**
 * Layer index:
 *
 * 00 - reserved
 * 01 - Layer III
 * 10 - Layer II
 * 11 - Layer I
*/
typedef enum
{
    MPEG_LAYER_FALSE = -1,
    MPEG_LAYER_UNDEFINED,
    MPEG_LAYER_III,
    MPEG_LAYER_II,
    MPEG_LAYER_I
} mpeg_layers_e;

/**
 * The number of samples per MPEG audio frame as follows:
 *
 * 	              MPEG 1      MPEG 2 (LSF)      MPEG 2.5 (LSF)
 *   Layer I        384	          384                384
 *   Layer II	    1152	      1152	             1152
 *   Layer III	    1152	      576	             576
*/
typedef enum
{
    MPEG_SAMPLES_PER_FRAME_NONE = 0,
    MPEG_SAMPLES_PER_FRAME_384 = 384,
    MPEG_SAMPLES_PER_FRAME_576 = 576,
    MPEG_SAMPLES_PER_FRAME_1152 = 1152
} mpeg_samples_per_frame_e;

/**
 * mpeg biterate define.
 *
 * The bitrates are always displayed in kilobits per second.
 * Note that the prefix kilo (abbreviated with the small 'k')
 * doesn't mean 1024 but 1000 bits per second! The bitrate index 1111
 * is reserved and should never be used. In the MPEG audio standard
 * there is a free format described. This free format means that the
 * file is encoded with a constant bitrate, which is not one of the
 * predefined bitrates.
 *
*/
typedef enum
{
MPEG_BITRATE_FALSE = -1,
MPEG_BITRATE_NONE = 0,
MPEG_BITRATE_8K   = 8000,
MPEG_BITRATE_16K  = 16000,
MPEG_BITRATE_24K  = 24000,
MPEG_BITRATE_32K  = 32000,
MPEG_BITRATE_40K  = 40000,
MPEG_BITRATE_48K  = 48000,
MPEG_BITRATE_56K  = 56000,
MPEG_BITRATE_64K  = 64000,
MPEG_BITRATE_80K  = 80000,
MPEG_BITRATE_96K  = 96000,
MPEG_BITRATE_112K = 112000,
MPEG_BITRATE_128K = 128000,
MPEG_BITRATE_144K = 144000,
MPEG_BITRATE_160K = 160000,
MPEG_BITRATE_176K = 176000,
MPEG_BITRATE_192K = 192000,
MPEG_BITRATE_224K = 224000,
MPEG_BITRATE_256K = 256000,
MPEG_BITRATE_288K = 288000,
MPEG_BITRATE_320K = 320000,
MPEG_BITRATE_352K = 352000,
MPEG_BITRATE_384K = 384000,
MPEG_BITRATE_416K = 416000,
MPEG_BITRATE_448K = 448000
} mpeg_bitrates_e;

/**
 * mpeg sampling rate define.
 *
 * The sampling rate specifies how many samples per second are recorded.
 * Each MPEG version can handle different sampling rates.
*/
typedef enum
{
    MPEG_FREQUENCIES_FALSE = -1,
    MPEG_FREQUENCIES_Reserved = 0,
    MPEG_FREQUENCIES_8000HZ = 8000,
    MPEG_FREQUENCIES_11025HZ = 11025,
    MPEG_FREQUENCIES_12000HZ = 12000,
    MPEG_FREQUENCIES_16000HZ = 16000,
    MPEG_FREQUENCIES_22050HZ = 22050,
    MPEG_FREQUENCIES_24000HZ = 24000,
    MPEG_FREQUENCIES_32000HZ = 32000,
    MPEG_FREQUENCIES_48000HZ = 48000,
    MPEG_FREQUENCIES_44100HZ = 44100,
} mpeg_frequencies_e;

/**
 * Channel mode
 * 00 - Stereo
 * 01 - Joint Stereo (Stereo)
 * 10 - Dual channel (Two mono channels)
 * 11 - Single channel (Mono)
 *
 * Note: Dual channel files are made of two independent mono channels.
 * Each one uses exactly half the bitrate of the file. Most decoders output them as stereo,
 * but it might not always be the case.
*/
typedef enum
{
  MPEG_CHANNEL_MODE_FALSE = -1,
  MPEG_CHANNEL_MODE_STEREO,
  MPEG_CHANNEL_MODE_JOINT_STEREO,
  MPEG_CHANNEL_MODE_DUAL_CHANNEL,
  MPEG_CHANNEL_MODE_SINGLE_CHANNEL
} mpeg_channel_mode_e;

/**
 * Mode extension (Only used in Joint Stereo)
*/
typedef enum
{
  MPEG_MODE_EXT_FALSE = -1,
  MPEG_MODE_EXT_0,
  MPEG_MODE_EXT_1,
  MPEG_MODE_EXT_2,
  MPEG_MODE_EXT_3
} mpeg_mode_extension_e;

/**
 * Emphasis
 * 00 - none
 * 01 - 50/15 ms
 * 10 - reserved
 * 11 - CCIT J.17
 *
 * The emphasis indication is here to tell the decoder that the file must
 * be de-emphasized, that means the decoder must 're-equalize' the sound
 * after a Dolby-like noise suppression. It is rarely used.
*/
typedef enum
{
  MPEG_EMPHASIS_FALSE = -1,
  MPEG_EMPHASIS_NONE,
  MPEG_EMPHASIS_50_15MS,
  MPEG_EMPHASIS_Reserved,
  MPEG_EMPHASIS_CCIT_J17
} mpeg_emphasis_e;

/**
 * Protection bit
 * 0 - protected by 16 bit CRC following header
 * 1 - no CRC
*/
typedef enum
{
  MPEG_CRC_ERROR_SIZE = -2,
  MPEG_CRC_MISMATCH = -1,
  MPEG_CRC_NONE = 0,
  MPEG_CRC_OK = 1
} mpeg_crc_e;

typedef enum
{
    BITRATE_TYPE_FALSE = -1,
    BITRATE_TYPE_CBR = 0,
    BITRATE_TYPE_VBR = 1,
    BITRATE_TYPE_ABR = 2,
    BITRATE_TYPE_FREE = 3
} bitrate_type_e;

typedef enum
{
    MPEG_DEC_STATE_PARSE_HEADER = 0x0000,
    MPEG_DEC_STATE_PARSE_AUDIO_FRAME = 0x0001,
    MPEG_DEC_STATE_PARSE_TAIL = 0x0002,
    MPEG_DEC_STATE_PARSE_COMPLETED = 0x0004,
} mpeg_dec_state_e;

typedef struct
{
    uint32_t size;
} id3v1_tag_info_t;

typedef struct
{
    uint8_t version;
    uint8_t revision;
    uint8_t flag;
    uint32_t size; //The tag size does not include the 10bytes tag header.
    bool id3v2_checked;
} id3v2_tag_info_t;

typedef struct
{
    id3v1_tag_info_t id3v1;
    id3v2_tag_info_t id3v2;
} id3_tag_info_t;

/**
 * mpeg audio frame header structure define.
*/
typedef struct
{
    /**
     * 如下字段来自mpeg音频帧头的parse
    */
    mpeg_version_e version; // 版本号
    mpeg_layers_e layer; // 层
    mpeg_crc_e crc; // Protection bit, MPEG_CRC_OK：需要crc校验，MPEG_CRC_NONE：无需crc校验
    mpeg_bitrates_e bitrate; // 比特率
    uint32_t frequency; // 采样频率
    bool paddingbit; // 填充位
    bool privatebit; // Private bit (only informative)
    mpeg_channel_mode_e channelmode; // 通道模式
    mpeg_mode_extension_e modeext; // Mode extension (Only used in Joint Stereo)
    bool copyrighted; // Copyright bit (only informative)
    bool original; // Original bit (only informative)
    mpeg_emphasis_e emphasis; // Emphasis

    /**
     * 如下字段可通过mpeg音频帧头parse的结果计算得到
    */
    uint32_t frame_length; //该帧包含的字节数
    uint32_t samples_per_frame; //该帧包含的采样点数

    uint32_t frame_pos; // start of first frame in this "chain" of headers
    uint8_t bytes[MPEG_AUDIO_FRAME_HEADER_LEN];

    struct list_head list;

} mpeg_audio_frame_header_info_t;

typedef struct
{
    //char* name;
    bool is_valid_mpeg_audio;
    id3_tag_info_t id3;
    struct list_head header_list;
    bool is_first_header_parsed;
    bool has_header_tag;
    uint8_t buf[MP3_BUF_SIZE];
    uint64_t remain_data_size;
    uint32_t total_frames;
    uint64_t total_file_size;
    uint64_t total_parsed;
    uint64_t duration;
    bitrate_type_e bit_rate_type;
    uint32_t free_format_bytes;
} audio_info_t;

typedef struct
{
    audio_info_t info;
    mpeg_dec_state_e state;
    uint32_t flags;
} mpeg_decoder_internal_t;

static const mpeg_bitrates_e _mpeg_bitrate_table[2][3][16] =
{
    {
        { //MPEG 1, LAYER I
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_192K,
            MPEG_BITRATE_224K,
            MPEG_BITRATE_256K,
            MPEG_BITRATE_288K,
            MPEG_BITRATE_320K,
            MPEG_BITRATE_352K,
            MPEG_BITRATE_384K,
            MPEG_BITRATE_416K,
            MPEG_BITRATE_448K,
            MPEG_BITRATE_FALSE
        },
        { //MPEG 1, LAYER II
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_48K,
            MPEG_BITRATE_56K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_80K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_112K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_192K,
            MPEG_BITRATE_224K,
            MPEG_BITRATE_256K,
            MPEG_BITRATE_320K,
            MPEG_BITRATE_384K,
            MPEG_BITRATE_FALSE
        },
        { //MPEG 1, LAYER III
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_40K,
            MPEG_BITRATE_48K,
            MPEG_BITRATE_56K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_80K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_112K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_192K,
            MPEG_BITRATE_224K,
            MPEG_BITRATE_256K,
            MPEG_BITRATE_320K,
            MPEG_BITRATE_FALSE
        }
    },
    {
        { //MPEG 2 or 2.5, LAYER I
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_48K,
            MPEG_BITRATE_56K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_80K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_112K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_144K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_176K,
            MPEG_BITRATE_192K,
            MPEG_BITRATE_224K,
            MPEG_BITRATE_256K,
            MPEG_BITRATE_FALSE
        },
        { //MPEG 2 or 2.5, LAYER II
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_8K,
            MPEG_BITRATE_16K,
            MPEG_BITRATE_24K,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_40K,
            MPEG_BITRATE_48K,
            MPEG_BITRATE_56K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_80K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_112K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_144K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_FALSE
        },
        { //MPEG 2 or 2.5, LAYER III
            MPEG_BITRATE_NONE,
            MPEG_BITRATE_8K,
            MPEG_BITRATE_16K,
            MPEG_BITRATE_24K,
            MPEG_BITRATE_32K,
            MPEG_BITRATE_40K,
            MPEG_BITRATE_48K,
            MPEG_BITRATE_56K,
            MPEG_BITRATE_64K,
            MPEG_BITRATE_80K,
            MPEG_BITRATE_96K,
            MPEG_BITRATE_112K,
            MPEG_BITRATE_128K,
            MPEG_BITRATE_144K,
            MPEG_BITRATE_160K,
            MPEG_BITRATE_FALSE
        }
    }
};

static const mpeg_frequencies_e _mpeg_frequency_table[4][4] =
{
    { MPEG_FREQUENCIES_11025HZ, MPEG_FREQUENCIES_12000HZ, MPEG_FREQUENCIES_8000HZ,MPEG_FREQUENCIES_Reserved },  //MPEG_VERSION_2_5
    { MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved}, //MPEG_VERSION_Reserved
    { MPEG_FREQUENCIES_22050HZ, MPEG_FREQUENCIES_24000HZ, MPEG_FREQUENCIES_16000HZ, MPEG_FREQUENCIES_Reserved }, //MPEG_VERSION_2
    { MPEG_FREQUENCIES_44100HZ, MPEG_FREQUENCIES_48000HZ, MPEG_FREQUENCIES_32000HZ, MPEG_FREQUENCIES_Reserved }  //MPEG_VERSION_1
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
static const mpeg_samples_per_frame_e _mpeg_samples_per_frame_table[2][3] =
{
	{	// MPEG 1
		384,	// Layer1
		1152,	// Layer2
		1152	// Layer3
	},
	{	// MPEG 2, 2.5
		384,	// Layer1
		1152,	// Layer2
		576		// Layer3
	}
};

static void _mpeg_dec_reset(mpeg_decoder_internal_t* decoder)
{
    if(!decoder)
    {
        return;
    }

    memset(decoder, 0, sizeof(mpeg_decoder_internal_t));
    decoder->info.bit_rate_type = BITRATE_TYPE_FALSE;

    INIT_LIST_HEAD(&decoder->info.header_list);
}

static void _mpeg_dec_id3v2_tags_free(id3v2_tag_info_t* tags)
{
    if(!tags)
    {
        return;
    }
    //todo
}

static void _mpeg_dec_id3v1_tags_free(id3v1_tag_info_t* tags)
{
    if(!tags)
    {
        return;
    }
    //todo
}

static void _mpeg_dec_id3_tags_free(id3_tag_info_t* tags)
{
    if(!tags)
    {
        return;
    }
    _mpeg_dec_id3v1_tags_free(&tags->id3v1);
    _mpeg_dec_id3v2_tags_free(&tags->id3v2);
}

static void _mpeg_dec_header_list_free(struct list_head* list)
{
    if(!list || list_empty(list))
    {
        return;
    }

    mpeg_audio_frame_header_info_t* pos = NULL;
    mpeg_audio_frame_header_info_t* n = NULL;

    list_for_each_entry_safe(pos, n, list, list)
    {
        list_del(&pos->list);
        free(pos);
        pos = NULL;
    }

    INIT_LIST_HEAD(list);
}

static int32_t _mpeg_dec_parse_id3v2_4_0_tag(mpeg_decoder_internal_t* decoder,
                                             get_byte_context_t* gctx)
{
    //parse todo, now just skip tagsize
    int32_t ret = MPEG_DEC_EOK;
    uint64_t need_parse_size = decoder->info.id3.id3v2.size - decoder->info.total_parsed;

    if(need_parse_size > u_byte_stream_get_bytes_left(gctx))
    {
        decoder->info.total_parsed += u_byte_stream_get_bytes_left(gctx);
        u_byte_stream_skip(gctx, u_byte_stream_get_bytes_left(gctx));
        ret = MPEG_DEC_ENEED_MORE_DATA;
    }
    else
    {
        decoder->info.total_parsed += need_parse_size;
        u_byte_stream_skip(gctx, need_parse_size);
        decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
        ret = MPEG_DEC_EOK;
    }

    return ret;
}

static int32_t _mpeg_dec_parse_id3v2_3_0_tag(mpeg_decoder_internal_t* decoder,
                                             get_byte_context_t* gctx)
{
    //parse todo, now just skip tagsize
    int32_t ret = MPEG_DEC_EOK;
    uint64_t need_parse_size = decoder->info.id3.id3v2.size - decoder->info.total_parsed;

    if(need_parse_size > u_byte_stream_get_bytes_left(gctx))
    {
        decoder->info.total_parsed += u_byte_stream_get_bytes_left(gctx);
        u_byte_stream_skip(gctx, u_byte_stream_get_bytes_left(gctx));
        ret = MPEG_DEC_ENEED_MORE_DATA;
    }
    else
    {
        decoder->info.total_parsed += need_parse_size;
        u_byte_stream_skip(gctx, need_parse_size);
        decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
        ret = MPEG_DEC_EOK;
    }

    return ret;
}

static int32_t _mpeg_dec_parse_id3v2_2_0_tag(mpeg_decoder_internal_t* decoder,
                                             get_byte_context_t* gctx)
{
    //parse todo, now just skip tagsize
    int32_t ret = MPEG_DEC_EOK;
    uint64_t need_parse_size = decoder->info.id3.id3v2.size - decoder->info.total_parsed;

    if(need_parse_size > u_byte_stream_get_bytes_left(gctx))
    {
        decoder->info.total_parsed += u_byte_stream_get_bytes_left(gctx);
        u_byte_stream_skip(gctx, u_byte_stream_get_bytes_left(gctx));
        ret = MPEG_DEC_ENEED_MORE_DATA;
    }
    else
    {
        decoder->info.total_parsed += need_parse_size;
        u_byte_stream_skip(gctx, need_parse_size);
        decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
        ret = MPEG_DEC_EOK;
    }

    return ret;
}

static int32_t _mpeg_dec_parse_header(mpeg_decoder_internal_t* decoder,
                                      uint64_t* consumed)
{
    if(MPEG_DEC_STATE_PARSE_HEADER != decoder->state)
    {
        DBG_ERROR("invalid state: %d\n", decoder->state);
        *consumed = 0;
        return MPEG_DEC_EOK;
    }

    int32_t ret = MPEG_DEC_EOK;

    get_byte_context_t gctx = {0};

    u_byte_stream_init(&gctx, decoder->info.buf, decoder->info.remain_data_size);

    //check id3v2 tag exist or not
    if(!decoder->info.id3.id3v2.id3v2_checked)
    {
        if(ID3V2_TAG_HEADER_SIZE > decoder->info.remain_data_size)
        {
            DBG_ERROR("\n");
            return MPEG_DEC_ENEED_MORE_DATA;
        }

        decoder->info.id3.id3v2.id3v2_checked = true;

        if(!memcmp("ID3", gctx.buffer, 3))
        {
            u_byte_stream_skip(&gctx, 3);//skip ID3, 3bytes
            decoder->info.id3.id3v2.version = u_byte_stream_get_byte(&gctx); //version, 1 byte
            decoder->info.id3.id3v2.revision = u_byte_stream_get_byte(&gctx); //revision, 1byte
            decoder->info.id3.id3v2.flag = u_byte_stream_get_byte(&gctx); //flag, 1byte
            decoder->info.id3.id3v2.size = (uint32_t)(u_byte_stream_get_byte(&gctx) & 0x7F) * 0x200000 +
                                        (uint32_t)(u_byte_stream_get_byte(&gctx) & 0x7F) * 0x4000 +
                                        (uint32_t)(u_byte_stream_get_byte(&gctx) & 0x7F) * 0x80 +
                                        (uint32_t)(u_byte_stream_get_byte(&gctx) & 0x7F); //tag size, 4bytes, 28bits

            DBG_INFO("%#x %#x %#x %#x\n",
                     decoder->info.id3.id3v2.version,
                     decoder->info.id3.id3v2.revision,
                     decoder->info.id3.id3v2.flag,
                     decoder->info.id3.id3v2.size);

            // Check the validity of the ID3v2 header
            if(CHECK_ID3V2_HEADER_VERSION(decoder->info.id3.id3v2.version) &&
               decoder->info.id3.id3v2.size > 0)
            {
                decoder->info.total_parsed += ID3V2_TAG_HEADER_SIZE;
            }
            else
            {
                decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
                *consumed = 0;

                return MPEG_DEC_EOK;
            }
        }
        else
        {
            DBG_INFO("no id3v2 tag need parse\n");
            decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
            *consumed = 0;

            return MPEG_DEC_EOK;
        }
    }

    switch (decoder->info.id3.id3v2.version)
    {
    case 2: // id3v2.2.0 hits
        ret = _mpeg_dec_parse_id3v2_2_0_tag(decoder, &gctx);
        break;
    case 3: // id3v2.3.0 hits
        ret = _mpeg_dec_parse_id3v2_3_0_tag(decoder, &gctx);
        break;
    case 4: // id3v2.3.0 hits
        ret = _mpeg_dec_parse_id3v2_4_0_tag(decoder, &gctx);
        break;
    default:
        decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;
        *consumed = 0;
        return MPEG_DEC_EOK;
    }

    *consumed = u_byte_stream_tell(&gctx);

    DBG_INFO("consumed: %ld, total_parsed: %ld, header_tag_size: %d, ret: %d\n",
             *consumed,
             decoder->info.total_parsed,
             decoder->info.id3.id3v2.size + ID3V2_TAG_HEADER_SIZE,
             ret);

    return ret;
}

static bool _mpeg_dec_hdr_valid(const uint8_t *h)
{
    return ((h[0] & 0xFF) == 0xFF) &&
           ((h[1] & 0xF0) == 0xf0 || (h[1] & 0xF8) == 0xe0) &&
           (HDR_GET_LAYER(h) != 0) &&
           (HDR_GET_BITRATE(h) != 15) &&
           (HDR_GET_SAMPLE_RATE(h) != 3);
}

static uint8_t _mpeg_dec_hdr_padding(const uint8_t *h)
{
    return HDR_TEST_PADDING(h) ? (HDR_IS_LAYER_1(h) ? 4 : 1) : 0;
}

static uint32_t _mpeg_dec_hdr_sample_rate_hz(const uint8_t *h)
{
    static const uint32_t g_hz[3] = { 44100, 48000, 32000 };
    return g_hz[HDR_GET_SAMPLE_RATE(h)] >> (uint8_t)!HDR_TEST_MPEG1(h) >> (uint8_t)!HDR_TEST_NOT_MPEG25(h);
}

static uint32_t _mpeg_dec_hdr_bitrate_kbps(const uint8_t *h)
{
    static const uint8_t halfrate[2][3][15] = {
        { { 0,4,8,12,16,20,24,28,32,40,48,56,64,72,80 }, { 0,4,8,12,16,20,24,28,32,40,48,56,64,72,80 }, { 0,16,24,28,32,40,48,56,64,72,80,88,96,112,128 } },
        { { 0,16,20,24,28,32,40,48,56,64,80,96,112,128,160 }, { 0,16,24,28,32,40,48,56,64,80,96,112,128,160,192 }, { 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224 } },
    };
    return 2*halfrate[!!HDR_TEST_MPEG1(h)][HDR_GET_LAYER(h) - 1][HDR_GET_BITRATE(h)];
}

static uint32_t _mpeg_dec_hdr_frame_samples(const uint8_t *h)
{
    return HDR_IS_LAYER_1(h) ? 384 : (1152 >> (uint32_t)HDR_IS_FRAME_576(h));
}

static uint8_t _mpeg_dec_hdr_compare(const uint8_t *h1, const uint8_t *h2)
{
    /**
     * (h1[1] ^ h2[1]) & 0xFE) == 0:表示h1和h2均为mpeg帧的帧头
     * (h1[2] ^ h2[2]) & 0x0C) == 0:表示h1和h2有相同的采样率
     * !(HDR_IS_FREE_FORMAT(h1) ^ HDR_IS_FREE_FORMAT(h2)：表示h1和h2均为自由格式或均为非自由格式
    */
    return _mpeg_dec_hdr_valid(h2) &&
           ((h1[1] ^ h2[1]) & 0xFE) == 0 &&
           ((h1[2] ^ h2[2]) & 0x0C) == 0 &&
           !(HDR_IS_FREE_FORMAT(h1) ^ HDR_IS_FREE_FORMAT(h2));
}

static uint32_t _mpeg_dec_hdr_frame_bytes(const uint8_t *h, uint32_t free_format_size)
{
    uint32_t frame_bytes = _mpeg_dec_hdr_frame_samples(h) * _mpeg_dec_hdr_bitrate_kbps(h) * 125/_mpeg_dec_hdr_sample_rate_hz(h);

    if (HDR_IS_LAYER_1(h))
    {
        frame_bytes &= ~3; /* slot align */
    }

    return frame_bytes ? frame_bytes : free_format_size;
}

static uint32_t _mpeg_dec_match_frame(const uint8_t *hdr, uint32_t mp3_bytes, uint32_t frame_bytes)
{
    uint32_t i, nmatch;

    for (i = 0, nmatch = 0; nmatch < MAX_FRAME_SYNC_MATCHES; nmatch++)
    {
        i += _mpeg_dec_hdr_frame_bytes(hdr + i, frame_bytes) + _mpeg_dec_hdr_padding(hdr + i);

        if (i + MPEG_AUDIO_FRAME_HEADER_LEN > mp3_bytes)
        {
            return nmatch > 0;
        }

        if (!_mpeg_dec_hdr_compare(hdr, hdr + i))
        {
            return 0;
        }
    }
    return 1;
}

static int32_t _mpeg_dec_find_audio_frame_header(const uint8_t *mpeg_pdata,
                                                 uint32_t mpeg_bytes,
                                                 uint32_t* free_format_bytes,
                                                 uint32_t* ptr_frame_bytes,
                                                 uint32_t* skip_size)
{
    if(mpeg_bytes <= MPEG_AUDIO_FRAME_HEADER_LEN)
    {
        *skip_size = 0;
        goto NEED_MORE_DATA;
    }

    uint32_t i, k;
    int32_t ret = MPEG_DEC_ENEED_MORE_DATA;
    uint32_t frame_bytes = 0;
    uint32_t frame_and_padding = 0;

    for (i = 0; i + MPEG_AUDIO_FRAME_HEADER_LEN < mpeg_bytes; i++, mpeg_pdata++)
    {
        if (_mpeg_dec_hdr_valid(mpeg_pdata))
        {
            frame_bytes = _mpeg_dec_hdr_frame_bytes(mpeg_pdata, *free_format_bytes);
            frame_and_padding = frame_bytes + _mpeg_dec_hdr_padding(mpeg_pdata);

            if(0 == frame_bytes) // check free format and calculate free format frame size
            {
                bool found_free_format = false;

                for (k = MPEG_AUDIO_FRAME_HEADER_LEN; k < MAX_FREE_FORMAT_FRAME_SIZE && i + 2*k + MPEG_AUDIO_FRAME_HEADER_LEN < mpeg_bytes; k++)
                {
                    if (_mpeg_dec_hdr_compare(mpeg_pdata, mpeg_pdata + k))
                    {
                        uint32_t fb = k - _mpeg_dec_hdr_padding(mpeg_pdata);
                        uint32_t nextfb = fb + _mpeg_dec_hdr_padding(mpeg_pdata + k);

                        if(i + k + nextfb + MPEG_AUDIO_FRAME_HEADER_LEN > mpeg_bytes)
                        {
                            *skip_size = i;
                            goto NEED_MORE_DATA;
                        }

                        if(!_mpeg_dec_hdr_compare(mpeg_pdata, mpeg_pdata + k + nextfb)) // the third frame not match
                        {
                            continue;
                        }

                        frame_and_padding = k;
                        frame_bytes = fb;
                        *free_format_bytes = fb;
                        found_free_format = true;
                        break; //break自由帧match循环
                    }
                }

                if(!found_free_format)
                {
                    // 由于数据不足未能找到三帧相同格式的自由帧，需要补充数据再来match
                    if((k < MAX_FREE_FORMAT_FRAME_SIZE && i + 2*k + MPEG_AUDIO_FRAME_HEADER_LEN >= mpeg_bytes))
                    {
                        ret = MPEG_DEC_ENEED_MORE_DATA;
                        break;
                    }
                    else
                    {
                        continue; //继续for (i = 0; i + MPEG_AUDIO_FRAME_HEADER_LEN < mpeg_bytes; i++, mpeg_pdata++)查找有效帧
                    }
                }
            }

            if(frame_bytes && i + frame_and_padding > mpeg_bytes)
            {
                ret = MPEG_DEC_ENEED_MORE_DATA;
                break;
            }

            if ((frame_bytes && i + frame_and_padding <= mpeg_bytes &&
                _mpeg_dec_match_frame(mpeg_pdata, mpeg_bytes - i, frame_bytes)) ||
                (!i && frame_and_padding == mpeg_bytes))
            {
                *ptr_frame_bytes = frame_and_padding;
                ret = MPEG_DEC_EOK;
                break;
            }

        }
    }

    *skip_size = i;

    return ret;

NEED_MORE_DATA:

    return MPEG_DEC_ENEED_MORE_DATA;
}

static void _mpeg_dec_audio_info_print(audio_info_t* info)
{
    mpeg_audio_frame_header_info_t* pos = NULL;
    uint32_t count = 0;
    printf("%d frames as follows: \n", info->total_frames);
    uint32_t last_frame_pos = 0;
    uint32_t last_frame_length = 0;

    list_for_each_entry(pos, &info->header_list, list)
    {
        printf("frame %d: \n", ++count);
        printf("Version: Mpeg %s\n", pos->version == MPEG_VERSION_2_5 ?
                                    "2.5": ((pos->version == MPEG_VERSION_2)) ?
                                    "2" : "1");
        printf("Layer: %s\n", pos->layer == MPEG_LAYER_I ?
                            "1" : (pos->layer == MPEG_LAYER_II) ?
                            "2" : "3");
        printf("Bitrate: %d\n", pos->bitrate);
        printf("Channelmode: %s\n", pos->channelmode == MPEG_CHANNEL_MODE_STEREO ?
                                    "Stereo" : (pos->channelmode == MPEG_CHANNEL_MODE_JOINT_STEREO ?
                                    "Joint Stereo" : (pos->channelmode == MPEG_CHANNEL_MODE_DUAL_CHANNEL ?
                                    "Dual Channel" : "Mono")));
        printf("Modeext: %d\n", pos->modeext);
        printf("Emphasis: %d\n", pos->emphasis);
        printf("Crc: %d\n", pos->crc);
        printf("Frequency: %dHz\n", pos->frequency);
        printf("Paddingbit: %d\n", pos->paddingbit);
        printf("Privatebit: %d\n", pos->privatebit);
        printf("Copyrighted: %d\n", pos->copyrighted);
        printf("Original: %d\n", pos->original);
        printf("Frame_length: %d bytes\n", pos->frame_length);
        printf("Samples_per_frame: %d\n", pos->samples_per_frame);
        printf("frame_pos: %08x\n", pos->frame_pos);
        printf("bytes: %x %x %x %x\n", pos->bytes[0], pos->bytes[1], pos->bytes[2], pos->bytes[3]);
        if(pos == list_first_entry(&info->header_list, mpeg_audio_frame_header_info_t, list))
        {
            last_frame_pos = pos->frame_pos;
            last_frame_length = pos->frame_length;
            printf("the first element hit\n");
        }
        else
        {
            if(pos->frame_pos - last_frame_pos > last_frame_length)
            {
                printf("there is frame lost\n");
            }
            last_frame_pos = pos->frame_pos;
            last_frame_length = pos->frame_length;
        }
        printf("\n");
    }
}

static void _mpeg_dec_audio_frame_header_dup(mpeg_audio_frame_header_info_t* dest,
                                             mpeg_audio_frame_header_info_t* src)
{
    dest->version = src->version;
    dest->layer = src->layer;
    dest->crc = src->crc;
    dest->bitrate = src->bitrate;
    dest->frequency = src->frequency;
    dest->paddingbit = src->paddingbit;
    dest->privatebit = src->privatebit;
    dest->channelmode = src->channelmode;
    dest->modeext = src->modeext;
    dest->copyrighted = src->copyrighted;
    dest->original = src->original;
    dest->emphasis = src->emphasis;
    dest->frame_length = src->frame_length;
    dest->samples_per_frame = src->samples_per_frame;
    dest->frame_pos = src->frame_pos;

    memcpy(&dest->bytes, &src->bytes, 4);
}

static void _mpeg_dec_reset_audio_frame_header(mpeg_audio_frame_header_info_t* info)
{
    info->version = MPEG_VERSION_FALSE;
    info->layer = MPEG_LAYER_FALSE;
    info->crc = MPEG_CRC_MISMATCH;
    info->bitrate = MPEG_BITRATE_FALSE;
    info->frequency = 0;
    info->paddingbit = 0;
    info->privatebit = 0;
    info->channelmode = MPEG_CHANNEL_MODE_FALSE;
    info->modeext = MPEG_MODE_EXT_FALSE;
    info->copyrighted = 0;
    info->original = 0;
    info->emphasis = MPEG_EMPHASIS_FALSE;
    info->frame_length = 0;
    info->samples_per_frame = 0;
    info->frame_pos = 0;
    memset(&info->bytes, 0, 4);
    INIT_LIST_HEAD(&info->list);
}

static bool _mpeg_dec_parse_VBRI_tag(mpeg_decoder_internal_t* decoder,
                                          get_byte_context_t* gctx,
                                          mpeg_audio_frame_header_info_t* header)
{
    get_byte_context_t tmp_gcxt = {0};
    uint32_t val;

    u_byte_stream_init(&tmp_gcxt, gctx->buffer, u_byte_stream_get_bytes_left(gctx));

    /* Check for VBRI tag (always 32 bytes after end of mpegaudio header) */
    u_byte_stream_seek(&tmp_gcxt, MPEG_AUDIO_FRAME_HEADER_LEN + VBRI_TAG_OFFSET, SEEK_CUR);

    DBG_INFO("%#lx %c %c %c %c\n",
             decoder->info.total_parsed + u_byte_stream_tell(&tmp_gcxt),
             *tmp_gcxt.buffer,
             *(tmp_gcxt.buffer + 1),
             *(tmp_gcxt.buffer + 2),
             *(tmp_gcxt.buffer + 3));

    val = u_byte_stream_get_be32(&tmp_gcxt);
    if(val == UM_MKBETAG('V', 'B', 'R', 'I'))
    {
        /* Check tag version */
        if(u_byte_stream_get_be16(&tmp_gcxt) == 1)
        {
            /* skip delay and quality */
            u_byte_stream_skip(&tmp_gcxt, 4);
            decoder->info.total_file_size = u_byte_stream_get_be32(&tmp_gcxt);
            decoder->info.total_frames = u_byte_stream_get_be32(&tmp_gcxt); //不包含首帧
            decoder->info.duration = 1000* decoder->info.total_frames * header->samples_per_frame / header->frequency; //ms
            decoder->info.bit_rate_type = BITRATE_TYPE_VBR;

            DBG_INFO("%ld, %d, %ld\n",
                     decoder->info.total_file_size,
                     decoder->info.total_frames,
                     decoder->info.duration);

            return true;
        }
    }

    return false;
}

static bool _mpeg_dec_parse_Xing_Info_tag(mpeg_decoder_internal_t* decoder,
                                          get_byte_context_t* gctx,
                                          mpeg_audio_frame_header_info_t* header)
{
    /* Side info offsets after header:
    /                Mono  Stereo
    /  MPEG1          17     32
    /  MPEG2 & 2.5     9     17*/
    static const int64_t xing_offtbl[2][2] = {{32, 17}, {17,9}};
    uint8_t lsf = 0;
    uint32_t side_info_offset = 0;
    get_byte_context_t tmp_gcxt = {0};
    uint32_t val;
    u_byte_stream_init(&tmp_gcxt, gctx->buffer, u_byte_stream_get_bytes_left(gctx));

    // skip header id filed
    u_byte_stream_skip(&tmp_gcxt, MPEG_AUDIO_FRAME_HEADER_LEN);

    // skip crc filed
    if(MPEG_CRC_OK == header->crc)
    {
        u_byte_stream_skip(&tmp_gcxt, 2);
    }

    // skip side info field
    if(MPEG_VERSION_1 == header->version)
    {
        lsf = 0;
    }
    else if(MPEG_VERSION_2 == header->version ||
            MPEG_VERSION_2_5 == header->version)
    {
        lsf = 1;
    }
    else
    {
        DBG_ERROR("invalid mpeg version %d\n", header->version);
        return false;
    }

    side_info_offset = xing_offtbl[lsf == 1][header->channelmode == MPEG_CHANNEL_MODE_SINGLE_CHANNEL];
    DBG_INFO("side_info_offset: %d\n", side_info_offset);
    u_byte_stream_skip(&tmp_gcxt, side_info_offset);

    DBG_INFO("%#lx %#x %#x %#x %#x\n",
             decoder->info.total_parsed + u_byte_stream_tell(&tmp_gcxt),
             *tmp_gcxt.buffer,
             *(tmp_gcxt.buffer + 1),
             *(tmp_gcxt.buffer + 2),
             *(tmp_gcxt.buffer + 3));

    // tag id: Xing or Info
    val = u_byte_stream_get_be32(&tmp_gcxt);
    if(val == UM_MKBETAG('I', 'n', 'f', 'o'))
    {
        DBG_INFO("Info tag hit\n");
        decoder->info.bit_rate_type = BITRATE_TYPE_CBR;
    }
    else if(val == UM_MKBETAG('X', 'i', 'n', 'g'))
    {
        DBG_INFO("Xing tag hit\n");
        decoder->info.bit_rate_type = BITRATE_TYPE_VBR;
    }
    else
    {
        DBG_INFO("No tag hit\n");
        return false;
    }

    val = u_byte_stream_get_be32(&tmp_gcxt);
    if (val & XING_FLAG_FRAMES)
    {
        decoder->info.total_frames = u_byte_stream_get_be32(&tmp_gcxt); //不包含首帧
    }

    if(val & XING_FLAG_SIZE)
    {
        decoder->info.total_file_size = u_byte_stream_get_be32(&tmp_gcxt);
    }

    decoder->info.duration = 1000* decoder->info.total_frames * header->samples_per_frame / header->frequency; //ms

    DBG_INFO("%ld, %d, %ld\n",
             decoder->info.total_file_size,
             decoder->info.total_frames,
             decoder->info.duration);

    return true;
}

static void _mpeg_dec_parse_audio_frame_header(mpeg_decoder_internal_t* decoder,
                                               get_byte_context_t* gctx,
                                               uint32_t free_format_bytes)
{
    uint8_t version_index;
    bit_stream_t bs = {0};

    u_bit_stream_init(&bs, gctx->buffer, MPEG_AUDIO_FRAME_HEADER_LEN);

    mpeg_audio_frame_header_info_t tmp_header = {0};
    _mpeg_dec_reset_audio_frame_header(&tmp_header);

    // skip sync id
    u_bit_stream_skip_bits(&bs, 11);

    // Version
    switch (u_bit_stream_get_bits(&bs, 2))
    {
    case 0:
        tmp_header.version = MPEG_VERSION_2_5;
        version_index = 1;
        break;
    case 1:
        DBG_ERROR("invalid mpeg version\n");
        break;
    case 2:
        tmp_header.version = MPEG_VERSION_2;
        version_index = 1;
        break;
    case 3:
        tmp_header.version = MPEG_VERSION_1;
        version_index = 0;
        break;
    }

    // Layer
    switch (u_bit_stream_get_bits(&bs, 2))
    {
    case 0:
        DBG_ERROR("invalid mpeg layer\n");
        break;
    case 1:
        tmp_header.layer = MPEG_LAYER_III;
        break;
    case 2:
        tmp_header.layer = MPEG_LAYER_II;
        break;
    case 3:
        tmp_header.layer = MPEG_LAYER_I;
        break;
    }

    // Protection bit
    tmp_header.crc = (mpeg_crc_e)!(bool)u_bit_stream_get_bits(&bs, 1);

    // Bitrate
    uint8_t bitrate_index = u_bit_stream_get_bits(&bs, 4);
    tmp_header.bitrate = _mpeg_bitrate_table[version_index][3-tmp_header.layer][bitrate_index];
    if (tmp_header.bitrate == MPEG_BITRATE_FALSE)
    {
        DBG_ERROR("invalid mpeg bitrate\n");
    }

    // Sampling rate
    uint8_t sample_rate_index =  u_bit_stream_get_bits(&bs, 2);
    tmp_header.frequency = _mpeg_frequency_table[tmp_header.version][sample_rate_index];
    if (tmp_header.frequency == MPEG_FREQUENCIES_Reserved)
    {
        DBG_ERROR("invalid mpeg sample rate\n");
    }

    // Padding bit
    tmp_header.paddingbit = u_bit_stream_get_bits(&bs, 1);

    // Private bit
    tmp_header.privatebit = u_bit_stream_get_bits(&bs, 1);

    // Channel Mode
    switch (u_bit_stream_get_bits(&bs, 2))
    {
        case 0:
            tmp_header.channelmode = MPEG_CHANNEL_MODE_STEREO;
            break;
        case 1:
            tmp_header.channelmode = MPEG_CHANNEL_MODE_JOINT_STEREO;
            break;
        case 2:
            tmp_header.channelmode = MPEG_CHANNEL_MODE_DUAL_CHANNEL;
            break;
        case 3:
            tmp_header.channelmode = MPEG_CHANNEL_MODE_SINGLE_CHANNEL;
            break;
    }

    // Mode extension (Only if Joint stereo)
    if (tmp_header.channelmode == MPEG_CHANNEL_MODE_JOINT_STEREO)
    {
        // these have a different meaning for different layers, better give them a generic name in the enum
        switch (u_bit_stream_get_bits(&bs, 2))
        {
            case 0:
                tmp_header.modeext = MPEG_MODE_EXT_0;
                break;
            case 1:
                tmp_header.modeext = MPEG_MODE_EXT_1;
                break;
            case 2:
                tmp_header.modeext = MPEG_MODE_EXT_2;
                break;
            case 3:
                tmp_header.modeext = MPEG_MODE_EXT_3;
                break;
        }
    }
    else //it's valid to have a valid false one in this case, since it's only used with joint stereo
    {
        u_bit_stream_skip_bits(&bs, 2);
        tmp_header.modeext = MPEG_MODE_EXT_FALSE;
    }

    // Copyright
    tmp_header.copyrighted = u_bit_stream_get_bits(&bs, 1);

    // Original
    tmp_header.copyrighted = u_bit_stream_get_bits(&bs, 1);

    // Emphasis
    switch (u_bit_stream_get_bits(&bs, 2))
    {
        case 0:
            tmp_header.emphasis = MPEG_EMPHASIS_NONE;
            break;
        case 1:
            tmp_header.emphasis = MPEG_EMPHASIS_50_15MS;
            break;
        case 2:
            tmp_header.emphasis = MPEG_EMPHASIS_Reserved;
            break;
        case 3:
            tmp_header.emphasis = MPEG_EMPHASIS_CCIT_J17;
            break;
    }

    // calculate frame length with bytes
    // reference:https://www.datavoyage.com/mpgscript/mpeghdr.htm
    if (tmp_header.bitrate != MPEG_BITRATE_NONE && tmp_header.frequency > 0)
    {
        switch(tmp_header.layer)
        {
            case MPEG_LAYER_I: // Layer 1
                tmp_header.frame_length = 4 * (12 * tmp_header.bitrate / tmp_header.frequency + (tmp_header.paddingbit ? 1 : 0));
                break;
            case MPEG_LAYER_II: // Layer 2
                tmp_header.frame_length = 144 * tmp_header.bitrate / tmp_header.frequency + (tmp_header.paddingbit ? 1 : 0);
                break;
            case MPEG_LAYER_III: // Layer 3
                if(tmp_header.version == MPEG_VERSION_1)
                    tmp_header.frame_length = 144 * tmp_header.bitrate / tmp_header.frequency + (tmp_header.paddingbit ? 1 : 0); //Mpeg1
                else
                    tmp_header.frame_length =  72 * tmp_header.bitrate / tmp_header.frequency + (tmp_header.paddingbit ? 1 : 0); //Mpeg2 + Mpeg2.5
                break;
            default:
                DBG_ERROR("invalid layer %d\n", tmp_header.layer);
                break;
        }
    }
    else
    {
        DBG_ERROR("free format\n");
        tmp_header.frame_length = free_format_bytes; //free format, bitrate = 0
    }

    // calculate the number of samples per MPEG audio frame
    bool b_lsf; // true means lower sampling frequencies (= MPEG2/MPEG2.5)

    if (tmp_header.version == MPEG_VERSION_1)
        b_lsf = false;
    else
        b_lsf = true;

    tmp_header.samples_per_frame = _mpeg_samples_per_frame_table[b_lsf][3-tmp_header.layer];

    // frame_pos
    tmp_header.frame_pos = decoder->info.total_parsed + u_byte_stream_tell(gctx);

    // bytes
    memcpy(&tmp_header.bytes, gctx->buffer, 4);

    // add list
    mpeg_audio_frame_header_info_t* tmp_item = NULL;
    tmp_item = (mpeg_audio_frame_header_info_t*)malloc(sizeof(mpeg_audio_frame_header_info_t));
    if(NULL == tmp_item)
    {
        DBG_ERROR("malloc tmp_item fail\n");
    }
    else
    {
        _mpeg_dec_reset_audio_frame_header(tmp_item);
        _mpeg_dec_audio_frame_header_dup(tmp_item, &tmp_header);
        list_add_tail(&tmp_item->list, &decoder->info.header_list);

        // Xing/Info tag or VBRI tag after the first audio frame header and before the second audio frame header
        if(!decoder->info.is_first_header_parsed)
        {
            decoder->info.is_first_header_parsed = true;

            // parse vbr tags: Xing/Info tag or VBRI tag
            if(_mpeg_dec_parse_VBRI_tag(decoder, gctx, &tmp_header))
            {
                decoder->info.has_header_tag = true;
                DBG_INFO("VBRI tag hit\n");
            }
            else if(_mpeg_dec_parse_Xing_Info_tag(decoder, gctx, &tmp_header))
            {
                decoder->info.has_header_tag = true;
                DBG_INFO("Xing/Info tag hit\n");
            }
            else
            {
                if (tmp_header.bitrate != MPEG_BITRATE_NONE && tmp_header.frequency > 0)
                {
                    decoder->info.bit_rate_type = BITRATE_TYPE_FREE;
                    decoder->info.free_format_bytes = tmp_header.bitrate;
                }
                DBG_INFO("No tag hit\n");
                decoder->info.has_header_tag = false;
                decoder->info.total_frames++;
            }
        }
        else
        {
            if(!decoder->info.has_header_tag)
            {
                decoder->info.total_frames++;
            }
        }

    }


    // skip本帧帧长bytes继续寻找下一帧
    u_byte_stream_skip(gctx, tmp_header.frame_length);

}

static int32_t _mpeg_dec_parse_audio_frame(mpeg_decoder_internal_t* decoder,
                                           bool eof,
                                           uint64_t* consumed)
{
    if(MPEG_DEC_STATE_PARSE_AUDIO_FRAME != decoder->state)
    {
        DBG_ERROR("invalid state: %d\n", decoder->state);
        *consumed = 0;
        return MPEG_DEC_EOK;
    }

    int32_t ret = MPEG_DEC_EOK;
    static uint32_t debut_frame_count = 0;
    get_byte_context_t gctx = {0};

    u_byte_stream_init(&gctx, decoder->info.buf, decoder->info.remain_data_size);

    do
    {
        uint32_t free_format_bytes = decoder->info.free_format_bytes;
        uint32_t frame_size = 0;
        uint32_t skip_size = 0;
        uint32_t mpeg_bytes = u_byte_stream_get_bytes_left(&gctx);
        const uint8_t* mpeg_pdata = gctx.buffer;

        ret = _mpeg_dec_find_audio_frame_header(mpeg_pdata,
                                                mpeg_bytes,
                                                &free_format_bytes,
                                                &frame_size,
                                                &skip_size);
        //DBG_INFO("skip_size: %d, frame_size: %d, free_format_bytes: %d, %d\n", skip_size, frame_size, free_format_bytes,  u_byte_stream_get_bytes_left(&gctx));
        if(skip_size > 0)
        {
            u_byte_stream_skip(&gctx, skip_size);
        }

        // not found and need more data
        if(MPEG_DEC_ENEED_MORE_DATA == ret)
        {
            break;
        }
        if(!frame_size ||
           (frame_size && u_byte_stream_get_bytes_left(&gctx) < frame_size))
        {
            ret = MPEG_DEC_ENEED_MORE_DATA;
            break;
        }

        debut_frame_count++;
        /*DBG_INFO("%d frame at %#lx: %#x %#x %#x %#x\n",
                 debut_frame_count,
                 decoder->info.total_parsed + u_byte_stream_tell(&gctx),
                 *gctx.buffer,
                 *(gctx.buffer+1),
                 *(gctx.buffer+2),
                 *(gctx.buffer+3));*/

        _mpeg_dec_parse_audio_frame_header(decoder, &gctx, free_format_bytes);

    }while(1);


    *consumed = u_byte_stream_tell(&gctx);
     decoder->info.total_parsed += *consumed;

    // Reach the end of the file, set parse tail state and return ok
    if(MPEG_DEC_ENEED_MORE_DATA == ret &&
       true == eof)
    {
        _mpeg_dec_audio_info_print(&decoder->info);
        decoder->state = MPEG_DEC_STATE_PARSE_TAIL;
        ret = MPEG_DEC_EOK;
    }

    return ret;
}

static int32_t _mpeg_dec_parse_tail(mpeg_decoder_internal_t* decoder,
                                    bool eof,
                                    uint64_t* consumed)
{
    int32_t ret = MPEG_DEC_EOK;

    *consumed = decoder->info.remain_data_size;
    if(eof)
    {
        decoder->state = MPEG_DEC_STATE_PARSE_COMPLETED;
    }
    else
    {
        ret = MPEG_DEC_ENEED_MORE_DATA;
    }

    //DBG_INFO("%ld\n", *consumed);

    return ret;
}


int32_t mpeg_dec_init(mpeg_decoder* dec)
{
    if(!dec)
    {
        DBG_ERROR("\n");
        return MPEG_DEC_EINVAL;
    }

    mpeg_decoder_internal_t* decoder = (mpeg_decoder_internal_t*)malloc(sizeof(mpeg_decoder_internal_t));
    if(!decoder)
    {
        DBG_ERROR("\n");
        return MPEG_DEC_ENOMEM;
    }

    _mpeg_dec_reset(decoder);

    *dec = decoder;

    DBG_ERROR("%p\n", dec);

    return MPEG_DEC_EOK;
}

int32_t mpeg_dec_parse_file(mpeg_decoder dec,
                            const char* file_name,
                            const mpeg_dec_parse_callbacks_t* cbs,
                            uint32_t flags)
{
    if(!dec ||
       (!file_name || strlen(file_name) == 0) ||
       !cbs ||
       flags & MPEG_DEC_FLAG_EOF)
    {
        DBG_ERROR("invalid parameter\n");
        return MPEG_DEC_EINVAL;
    }

    DBG_INFO("file_name: %s, flags: 0x%08x\n", file_name, flags);

    mpeg_decoder_internal_t* decoder = dec;
    int32_t ret = MPEG_DEC_EOK;
    long int file_size = 0; // 文件size
    long int readed = 0; // 已读size
    size_t need_read = 0; // 每次读取循环将要读取的size
    bool eof = false;

    FILE* mpeg_file = fopen(file_name, "rb");
    if(!mpeg_file)
    {
        DBG_ERROR("%s fopen fail\n", file_name);
        ret = MPEG_DEC_ERROR;
        goto FAIL;
    }

    ret = fseek(mpeg_file, 0, SEEK_END);
    if(ret)
    {
        DBG_ERROR("%s fseek fail\n", file_name);
        ret = MPEG_DEC_ERROR;
        goto FAIL;
    }

    file_size = ftell(mpeg_file);
    if(file_size < 0)
    {
        DBG_ERROR("%s ftell fail %ld\n", file_name, file_size);
        ret = MPEG_DEC_ERROR;
        goto FAIL;
    }
    else if(file_size < MPEG_AUDIO_FRAME_HEADER_LEN)
    {
        DBG_ERROR("invalid audio file: %s  file size two small: %ld\n", file_name, file_size);
        ret = MPEG_DEC_EINVAL_FILE;
        goto FAIL;
    }

    ret = fseek(mpeg_file, 0, SEEK_SET);
    if(ret)
    {
        DBG_ERROR("%s fseek fail\n", file_name);
        ret = MPEG_DEC_ERROR;
        goto FAIL;
    }

    uint8_t* buf = (uint8_t*)malloc(MP3_BUF_SIZE);
    if(!buf)
    {
        DBG_ERROR("malloc buf fail\n");
        ret = MPEG_DEC_ENOMEM;
        goto FAIL;
    }

    while(readed < file_size)
    {
        if(file_size - readed < MP3_BUF_SIZE)
        {
            need_read = file_size - readed;
            eof = true;
        }
        else
        {
            need_read = MP3_BUF_SIZE;
        }

        if(need_read == 0)
        {
            break;
        }

        if((ret = fread(buf, 1, need_read, mpeg_file)) != need_read)
        {
            DBG_ERROR("fread fail %d\n", ret);
            goto FAIL;
        }

        ret = mpeg_dec_parse_buf(decoder,
                                 buf,
                                 need_read,
                                 cbs,
                                 flags,
                                 eof);
        if(MPEG_DEC_EOK != ret)
        {
            DBG_ERROR("parse buf fail %d\n", ret);
            goto FAIL;
        }

        readed += need_read;

        if(MPEG_DEC_STATE_PARSE_COMPLETED == decoder->state)
        {
            break;
        }
    }

    DBG_INFO("file_size: %ld, readed: %ld\n", file_size, readed);

    return ret;

FAIL:
    if(mpeg_file)
    {
        fclose(mpeg_file);
    }
    if(buf)
    {
        free(buf);
        buf = NULL;
    }

    mpeg_dec_parse_reset(decoder);

    return ret;
}

int32_t mpeg_dec_parse_buf(mpeg_decoder dec,
                           const uint8_t* buf,
                           uint64_t size,
                           const mpeg_dec_parse_callbacks_t* cbs,
                           uint32_t flags,
                           bool eof)
{
    if(!dec ||
       !buf ||
       0 == size ||
       !cbs)
    {
        DBG_ERROR("invalid parameter %ld\n", size);
        return MPEG_DEC_EINVAL;
    }

    mpeg_decoder_internal_t* decoder = dec;

    if(MPEG_DEC_STATE_PARSE_COMPLETED == decoder->state)
    {
        DBG_ERROR("%d state, not allow parse buf\n", decoder->state);
        return MPEG_DEC_ESTATE_NOT_ALLOW;
    }

    int32_t ret = MPEG_DEC_EOK;
    uint64_t need_parse_size = size;
    const uint8_t* copy_pos = buf;
    uint64_t fill_size = 0;
    uint64_t consumed_size = 0;
    bool is_eof = false;

    while(1)
    {
        fill_size = MP3_BUF_SIZE - decoder->info.remain_data_size;

        if(need_parse_size <= fill_size)
        {
            fill_size = need_parse_size;
        }

        memcpy(decoder->info.buf + decoder->info.remain_data_size, copy_pos, fill_size);

        need_parse_size -= fill_size;
        copy_pos += fill_size;
        decoder->info.remain_data_size += fill_size;

        if(0 == need_parse_size &&
           true == eof)
        {
            DBG_INFO("file end arrival\n");
            is_eof = true;
        }

        consumed_size = 0;
        switch (decoder->state)
        {
        case MPEG_DEC_STATE_PARSE_HEADER:
            ret = _mpeg_dec_parse_header(decoder,
                                         &consumed_size);
            //not break, continue parse audio frame
            if(MPEG_DEC_EOK == ret &&
               MPEG_DEC_STATE_PARSE_AUDIO_FRAME == decoder->state)
            {
                //将剩余的数据挪到buf开头
                if(0 != consumed_size)
                {
                    decoder->info.remain_data_size -= consumed_size;
                    memmove(decoder->info.buf, decoder->info.buf + consumed_size, decoder->info.remain_data_size);

                    consumed_size = 0;
                }
            }
            else
            {
                break;
            }

        case MPEG_DEC_STATE_PARSE_AUDIO_FRAME:
            ret = _mpeg_dec_parse_audio_frame(decoder,
                                              is_eof,
                                              &consumed_size);
            //not break, continue parse tail
            if(MPEG_DEC_EOK == ret &&
               MPEG_DEC_STATE_PARSE_TAIL == decoder->state)
            {
                //将剩余的数据挪到buf开头
                if(0 != consumed_size)
                {
                    decoder->info.remain_data_size -= consumed_size;
                    memmove(decoder->info.buf, decoder->info.buf + consumed_size, decoder->info.remain_data_size);

                    consumed_size = 0;
                }
            }
            else
            {
                break;
            }

        case MPEG_DEC_STATE_PARSE_TAIL:
            ret = _mpeg_dec_parse_tail(decoder,
                                       is_eof,
                                       &consumed_size);
            break;

        default:
            DBG_ERROR("invalid state: %d\n", decoder->state);
            return MPEG_DEC_ESTATE_NOT_ALLOW;
        }

        //将剩余的数据挪到buf开头
        if(0 != consumed_size)
        {
            decoder->info.remain_data_size -= consumed_size;
            memmove(decoder->info.buf, decoder->info.buf + consumed_size, decoder->info.remain_data_size);
        }

        /**
         * check是否跳出循环,如下几种case需要跳出循环:
         * 1. ret为MPEG_DEC_ENEED_MORE_DATA且need_parse_size为0，需要跳出循环获取更多数据
         * 2. ret为MPEG_DEC_EPARSE_FAIL，表明音频文件parse fail
         * 3. MPEG_DEC_STATE_PARSE_COMPLETED == decoder->state完成parse
        */
        if(MPEG_DEC_ENEED_MORE_DATA == ret &&
           0 == need_parse_size)
        {
            ret = MPEG_DEC_EOK;
            break;
        }

        if(MPEG_DEC_EPARSE_FAIL == ret)
        {
            DBG_ERROR("parse buf fail, %08lx, %08lx, %08lx\n", size, size - need_parse_size, decoder->info.total_parsed);
            break;
        }

        if(MPEG_DEC_STATE_PARSE_COMPLETED == decoder->state)
        {
            ret = MPEG_DEC_EOK;
            DBG_INFO("parse completed, %ld\n", decoder->info.total_parsed);
            break;
        }

    } //end of while loop

    // 完成parse后notify告知parse的结果
    if(MPEG_DEC_STATE_PARSE_COMPLETED == decoder->state)
    {
        //todo
    }

    if(MPEG_DEC_EPARSE_FAIL == ret)
    {
        DBG_ERROR("\n");
        mpeg_dec_parse_reset(dec);
    }

    return ret;
}

int32_t mpeg_dec_parse_reset(mpeg_decoder dec)
{
    if(!dec)
    {
        DBG_ERROR("\n");
        return MPEG_DEC_EINVAL;
    }

    mpeg_decoder_internal_t* decoder = dec;

    //free id3 tags

    _mpeg_dec_id3_tags_free(&decoder->info.id3);
    //free header list
    _mpeg_dec_header_list_free(&decoder->info.header_list);

    _mpeg_dec_reset(decoder);

    return MPEG_DEC_EOK;
}

int32_t mpeg_dec_deinit(mpeg_decoder* dec)
{
    if(!dec)
    {
        return MPEG_DEC_EINVAL;
    }

    int32_t ret = MPEG_DEC_EOK;

    ret = mpeg_dec_parse_reset(*dec);
    if(MPEG_DEC_EOK != ret)
    {
        DBG_ERROR("\n");
        return ret;
    }

    free(*dec);

    return MPEG_DEC_EOK;
}
