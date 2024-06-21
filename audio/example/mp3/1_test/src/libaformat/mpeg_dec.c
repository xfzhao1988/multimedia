#include <stdio.h>

#include "dbg.h"
#include "mpeg_dec.h"
#include "list.h"
#include "bs.h"

#define ID3V2_TAG_HEADER_SIZE 10
#define ID3V2_TAG_ID "ID3"
#define MP3_BUF_SIZE (16*1024) /* buffer which can hold minimum 10 consecutive mp3 frames (~16KB) worst case */
#define MPEG_AUDIO_FRAME_HEADER_LEN 4

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
    uint32_t size;
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
    uint8_t buf[MP3_BUF_SIZE];
    uint64_t remain_data_size;
    uint32_t total_frames;
    uint64_t total_file_size;
    uint64_t total_parsed;
    uint64_t duration;
    bitrate_type_e bit_rate_type;
} audio_info_t;

typedef struct
{
    audio_info_t info;
    mpeg_dec_state_e state;
    uint32_t flags;
} mpeg_decoder_internal_t;

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


static int32_t _mpeg_dec_parse_header(mpeg_decoder_internal_t* decoder,
                                      uint64_t* consumed)
{
    bs_t bs = {0};
    //int32_t ret = MPEG_DEC_EOK;

    //_remain_data_size指示decoder->info.buf中有多少bytes的数据待parse
    uint64_t _remain_data_size = decoder->info.remain_data_size;

    //check id3v2 tag exist or not
    if(!decoder->info.id3.id3v2.id3v2_checked)
    {
        if(ID3V2_TAG_HEADER_SIZE >= _remain_data_size)
        {
            DBG_ERROR("\n");
            return MPEG_DEC_ENEED_MORE_DATA;
        }

        decoder->info.id3.id3v2.id3v2_checked = true;

        bs_init(&bs, decoder->info.buf, _remain_data_size);

        if(!memcmp("ID3", bs.buf, 3))
        {
            skip_bits(&bs, 3*8); //skip ID3, 3bytes
            decoder->info.id3.id3v2.version = get_bits_u8(&bs); //version, 1 byte
            decoder->info.id3.id3v2.revision = get_bits_u8(&bs); //revision, 1byte
            decoder->info.id3.id3v2.flag = get_bits_u8(&bs); //flag, 1byte
            decoder->info.id3.id3v2.size = (uint32_t)(get_bits_u8(&bs) & 0x7F) * 0x200000 +
                                        (uint32_t)(get_bits_u8(&bs) & 0x7F) * 0x4000 +
                                        (uint32_t)(get_bits_u8(&bs) & 0x7F) * 0x80 +
                                        (uint32_t)(get_bits_u8(&bs) & 0x7F); //tag size, 4bytes, 28bits

            DBG_INFO("%#x %#x %#x %#x\n", decoder->info.id3.id3v2.version, decoder->info.id3.id3v2.revision, decoder->info.id3.id3v2.flag, decoder->info.id3.id3v2.size);

            decoder->info.total_parsed += ID3V2_TAG_HEADER_SIZE; //update total_parsed
            *consumed += ID3V2_TAG_HEADER_SIZE; //update *consumed
            _remain_data_size -= ID3V2_TAG_HEADER_SIZE;

            memmove(decoder->info.buf, decoder->info.buf + ID3V2_TAG_HEADER_SIZE, _remain_data_size);
            bs.pos = 0;
            bs.limit -= ID3V2_TAG_HEADER_SIZE * 8;
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
        bs_init(&bs, decoder->info.buf, decoder->info.remain_data_size);
    }

    //loop parse id3v2 tags
    while(1)
    {
        if(decoder->info.total_parsed >= decoder->info.id3.id3v2.size)
        {
            decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;

            return MPEG_DEC_EOK;
        }
    }


    *consumed = decoder->info.remain_data_size;
    decoder->state = MPEG_DEC_STATE_PARSE_AUDIO_FRAME;

    DBG_INFO("%ld\n", *consumed);

    return MPEG_DEC_EOK;
}

static int32_t _mpeg_dec_parse_audio_frame(mpeg_decoder_internal_t* decoder,
                                           uint64_t* consumed)
{
    *consumed = decoder->info.remain_data_size;
    decoder->state = MPEG_DEC_STATE_PARSE_TAIL;

    DBG_INFO("%ld\n", *consumed);

    return MPEG_DEC_EOK;
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
    int ret = MPEG_DEC_EOK;
    long int file_size = 0; // 文件size
    long int readed = 0; // 已读size
    size_t need_read = 0; // 每次读取循环将要读取的size

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
                                 readed >= file_size);
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
            DBG_INFO("file end arrival, parse file tail\n");
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
                    decoder->info.total_parsed += consumed_size;
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
                                              &consumed_size);
            //not break, continue parse tail
            if(MPEG_DEC_EOK == ret &&
               MPEG_DEC_STATE_PARSE_TAIL == decoder->state)
            {
                //将剩余的数据挪到buf开头
                if(0 != consumed_size)
                {
                    decoder->info.total_parsed += consumed_size;
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
            decoder->info.total_parsed += consumed_size;
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
