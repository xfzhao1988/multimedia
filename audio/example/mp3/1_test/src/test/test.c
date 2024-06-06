#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "dbg.h"
#include "test.h"

#define ID3V2_TAG_HEADER_SIZE 10
#define ID3V2_TAG_ID "ID3"
#define MPEG_AUDIO_FRAME_HEADER_SIZE 4

// http://www.mp3-tech.org/programmer/frame_header.html
typedef struct
  {
//byte 1
    unsigned char frame_sync_a : 8; /* all bits should be set */
//byte 2
    unsigned char protection_bit : 1;
    unsigned char layer : 2;
    unsigned char id : 2;
    unsigned char frame_sync_b : 3; /* all bits should be set */
//byte 3
    unsigned char private_bit : 1;
    unsigned char padding_bit : 1;
    unsigned char frequency : 2;
    unsigned char bit_rate_index : 4;
//byte 4
    unsigned char emphasis : 2;
    unsigned char original : 1;
    unsigned char copyright : 1;
    unsigned char mode_ext : 2; /* only used in joint stereo */
    unsigned char mode : 2;
  } _mpeg_header_internal_t;

typedef struct
{
    const uint8_t *buffer;
    uint32_t size;
} _mpeg_dec_map_t;

static const mpeg_bitrates_e _mpeg_bitrates[2][3][16] =
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

static const mpeg_frequencies_e _mpeg_frequencies[4][4] =
{
    { MPEG_FREQUENCIES_11025HZ, MPEG_FREQUENCIES_12000HZ, MPEG_FREQUENCIES_8000HZ,MPEG_FREQUENCIES_Reserved },  //MPEG_VERSION_2_5
    { MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved, MPEG_FREQUENCIES_Reserved}, //MPEG_VERSION_Reserved
    { MPEG_FREQUENCIES_22050HZ, MPEG_FREQUENCIES_24000HZ, MPEG_FREQUENCIES_16000HZ, MPEG_FREQUENCIES_Reserved }, //MPEG_VERSION_2
    { MPEG_FREQUENCIES_44100HZ, MPEG_FREQUENCIES_48000HZ, MPEG_FREQUENCIES_32000HZ, MPEG_FREQUENCIES_Reserved }  //MPEG_VERSION_1
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
static const mpeg_samples_per_frame_e _mpeg_samples_per_frame[2][3] =
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

static uint32_t _get_id3v2_tagsize(const uint8_t *buf, uint32_t buf_size)
{
    if ( buf_size >= ID3V2_TAG_HEADER_SIZE &&
        !memcmp(buf, ID3V2_TAG_ID, 3) &&
        !((buf[5] & 15) || (buf[6] & 0x80) || (buf[7] & 0x80) || (buf[8] & 0x80) || (buf[9] & 0x80)) )
    {
        uint32_t size = (((buf[6] & 0x7f) << 21) | ((buf[7] & 0x7f) << 14) | ((buf[8] & 0x7f) << 7) | (buf[9] & 0x7f)) + 10;
        if ((buf[5] & 16))
            size += 10; /* footer */
        //DBG_INFO("size: %08x\n", size);
        return size;
    }

    return 0;
};

static uint32_t _get_id3v1_tagsize(const uint8_t *buf, uint32_t buf_size)
{
    uint32_t size = buf_size;

    if (size >= 128 && !memcmp(buf + size - 128, "TAG", 3))
    {
        size -= 128;
        if (size >= 227 && !memcmp(buf + size - 227, "TAG+", 4))
            size -= 227;
    }

    if (size > 32 && !memcmp(buf + size - 32, "APETAGEX", 8))
    {
        size -= 32;
        const uint8_t *tag = buf + size + 8 + 4;
        uint32_t tag_size = (uint32_t)(tag[3] << 24) | (tag[2] << 16) | (tag[1] << 8) | tag[0];
        if (size >= tag_size)
            size -= tag_size;
    }

    return (buf_size - size);
}

static void _skip_id3_tag(const uint8_t **pbuf, uint32_t *pbuf_size, mpeg_audio_info_t* info)
{
    uint8_t *buf = (uint8_t *)(*pbuf);
    uint32_t buf_size = *pbuf_size;

    uint32_t id3v2size = _get_id3v2_tagsize(buf, buf_size);
    if (id3v2size)
    {
        if (id3v2size >= buf_size)
            id3v2size = buf_size;
        buf      += id3v2size;
        buf_size -= id3v2size;
        info->id3.id3v2.size = id3v2size;
    }

    uint32_t id3v1size = _get_id3v1_tagsize(buf, buf_size);
    if (id3v1size)
    {
        buf      += id3v1size;
        buf_size -= id3v1size;
        info->id3.id3v1.size = id3v1size;
    }

    DBG_INFO("id3v1: %d, id3v2: %08x\n", info->id3.id3v1.size, info->id3.id3v2.size);

    *pbuf = (const uint8_t *)buf;
    *pbuf_size = buf_size;
}

static void _mpeg_dec_reset_audio_header(mpeg_audio_frame_header_info_t* info)
{
    info->version = MPEG_VERSION_FALSE;
    info->layer = MPEG_LAYER_FALSE;
    info->bitrate = MPEG_BITRATE_FALSE;
    info->channelmode = MPEG_CHANNEL_MODE_FALSE;
    info->modeext = MPEG_MODE_EXT_FALSE;
    info->emphasis = MPEG_EMPHASIS_FALSE;
    info->crc = MPEG_CRC_MISMATCH;
    info->frequency = 0;
    info->frame_length = 0;
    info->samples_per_frame = 0;
    info->frame_count = 0;
    info->next = NULL;
    //info->frames = 0;
    //info->time = 0;
    //info->vbr_bitrate = 0;
}

static void _mpeg_audio_frame_header_print(mpeg_audio_frame_header_info_t* info)
{
    printf("\n");
    printf("Version: Mpeg %s\n", info->version == MPEG_VERSION_2_5 ?
                                 "2.5": ((info->version == MPEG_VERSION_2)) ?
                                 "2" : "1");
    printf("Layer: %s\n", info->layer == MPEG_LAYER_I ?
                          "1" : (info->layer == MPEG_LAYER_II) ?
                          "2" : "3");
    printf("Bitrate: %d\n", info->bitrate);
    printf("Channelmode: %s\n", info->channelmode == MPEG_CHANNEL_MODE_STEREO ?
                                "Stereo" : (info->channelmode == MPEG_CHANNEL_MODE_JOINT_STEREO ?
                                "Joint Stereo" : (info->channelmode == MPEG_CHANNEL_MODE_DUAL_CHANNEL ?
                                "Dual Channel" : "Mono")));
    //printf("Modeext: %d\n", info->modeext);
    //printf("Emphasis: %d\n", info->emphasis);
    //printf("Crc: %d\n", info->crc);
    printf("Frequency: %dHz\n", info->frequency);
    //printf("Privatebit: %d\n", info->privatebit);
    //printf("Copyrighted: %d\n", info->copyrighted);
    //printf("Original: %d\n", info->original);
    printf("Frame_length: %d bytes\n", info->frame_length);
    printf("Samples_per_frame: %d\n", info->samples_per_frame);
    printf("\n");
}

static bool _mpeg_audio_header_parse(_mpeg_header_internal_t* header,
                                     mpeg_audio_frame_header_info_t* out_info)
{
    int bit_rate_index;

    // parse version
    switch (header->id)
    {
        case 3:
            out_info->version = MPEG_VERSION_1; //11 - MPEG Version 1 (ISO/IEC 11172-3)
            bit_rate_index = 0;
            break;
        case 2:
            out_info->version = MPEG_VERSION_2; //10 - MPEG Version 2 (ISO/IEC 13818-3)
            bit_rate_index = 1;
            break;
        case 1:
            DBG_ERROR("id: %d\n", header->id);
            return false; //wouldn't know how to handle it
        case 0:
            out_info->version = MPEG_VERSION_2_5; //00 - MPEG Version 2.5 (unofficial extension of MPEG 2)
            bit_rate_index = 1;
            break;
        default:
            DBG_ERROR("id: %d\n", header->id);
            return false;
    };

    // parse layer
    switch (header->layer)
    {
        case 3:
            out_info->layer = MPEG_LAYER_I;
            break;
        case 2:
            out_info->layer = MPEG_LAYER_II;
            break;
        case 1:
            out_info->layer = MPEG_LAYER_III;
            break;
        case 0:
            DBG_ERROR("layer: %d\n", header->layer);
            return false; //wouldn't know how to handle it
        default:
            DBG_ERROR("layer: %d\n", header->layer);
            return false; //how can two unsigned bits be something else??
    };

    // parse bitrate
    //DBG_INFO("%d, %d, %d\n", bit_rate_index, 3-header->layer, header->bit_rate_index);
    out_info->bitrate = _mpeg_bitrates[bit_rate_index][3-header->layer][header->bit_rate_index];
    if (out_info->bitrate == MPEG_BITRATE_FALSE)
    {
        DBG_ERROR("bitrate: %d\n", out_info->bitrate);
        return false;
    }

    // parse frequency
    out_info->frequency = _mpeg_frequencies[header->id][header->frequency];
    if (out_info->frequency == MPEG_FREQUENCIES_Reserved)
    {
        DBG_ERROR("frequency: %d\n", out_info->frequency);
        return false;
    }

    out_info->privatebit = (bool)header->private_bit;
    out_info->copyrighted = (bool)header->copyright;
    out_info->original = (bool)header->original;
    out_info->crc = (mpeg_crc_e)!(bool)header->protection_bit;

    //parse channel mode
    switch (header->mode)
    {
        case 3:
            out_info->channelmode = MPEG_CHANNEL_MODE_SINGLE_CHANNEL;
            break;
        case 2:
            out_info->channelmode = MPEG_CHANNEL_MODE_DUAL_CHANNEL;
            break;
        case 1:
            out_info->channelmode = MPEG_CHANNEL_MODE_JOINT_STEREO;
            break;
        case 0:
            out_info->channelmode = MPEG_CHANNEL_MODE_STEREO;
            break;
        default:
            DBG_ERROR("mode: %d\n", header->mode);
            return false; //wouldn't know how to handle it
    }

    //parse external mode for joint stereo
    if (out_info->channelmode == MPEG_CHANNEL_MODE_JOINT_STEREO)
    {
        // these have a different meaning for different layers, better give them a generic name in the enum
        switch (header->mode_ext)
        {
            case 3:
                out_info->modeext = MPEG_MODE_EXT_3;
                break;
            case 2:
                out_info->modeext = MPEG_MODE_EXT_2;
                break;
            case 1:
                out_info->modeext = MPEG_MODE_EXT_1;
                break;
            case 0:
                out_info->modeext = MPEG_MODE_EXT_0;
                break;
            default:
                DBG_ERROR("mode_ext: %d\n", header->mode_ext);
                return false; //wouldn't know how to handle it
        }
    }
    else //it's valid to have a valid false one in this case, since it's only used with joint stereo
    {
        out_info->modeext = MPEG_MODE_EXT_FALSE;
    }

    //parse emphasis
    switch (header->emphasis)
    {
        case 3:
            out_info->emphasis = MPEG_EMPHASIS_CCIT_J17;
            break;
        case 2:
            out_info->emphasis = MPEG_EMPHASIS_Reserved;
            break;
        case 1:
            out_info->emphasis = MPEG_EMPHASIS_50_15MS;
            break;
        case 0:
            out_info->emphasis = MPEG_EMPHASIS_NONE;
            break;
        default:
            DBG_ERROR("emphasis: %d\n", header->emphasis);
            return false; //wouldn't know how to handle it
    }

    //parse frame length with bytes
    //http://www.mp3-tech.org/programmer/frame_header.html
    if (out_info->bitrate != MPEG_BITRATE_NONE && out_info->frequency > 0)
    {
        switch(out_info->layer)
        {
            case MPEG_LAYER_I: // Layer 1
                out_info->frame_length = 4 * (12 * out_info->bitrate / out_info->frequency + (header->padding_bit ? 1 : 0));
                break;
            case MPEG_LAYER_II: // Layer 2
                out_info->frame_length = 144 * out_info->bitrate / out_info->frequency + (header->padding_bit ? 1 : 0);
                break;
            case MPEG_LAYER_III: // Layer 3
                if(out_info->version == MPEG_VERSION_2_5)
                    out_info->frame_length = 144 * out_info->bitrate / out_info->frequency + (header->padding_bit ? 1 : 0); //Mpeg1
                else
                    out_info->frame_length =  72 * out_info->bitrate / out_info->frequency + (header->padding_bit ? 1 : 0); //Mpeg2 + Mpeg2.5
                break;
            default:
                DBG_ERROR("layer: %d\n", header->layer);
                return false;
        }
    }
    else
    {
        out_info->frame_length = 0; //unable to determine
    }

    // calculate the number of samples per MPEG audio frame
    bool b_lsf; // true means lower sampling frequencies (= MPEG2/MPEG2.5)

    if (out_info->version == MPEG_VERSION_1)
		b_lsf = false;
	else
		b_lsf = true;

    out_info->samples_per_frame = _mpeg_samples_per_frame[b_lsf][out_info->layer];

    DBG_INFO("_mpeg_audio_header_parse ok\n");

    return true;
}

static int _mpeg_dec_open_file(const char *file_name, _mpeg_dec_map_t *map_info)
{
    if (!file_name)
        return -1;
    int file;
    struct stat st;
    memset(map_info, 0, sizeof(*map_info));
retry_open:
    file = open(file_name, O_RDONLY);
    if (file < 0 && (errno == EAGAIN || errno == EINTR))
        goto retry_open;
    if (file < 0 || fstat(file, &st) < 0)
    {
        close(file);
        return -1;
    }

    map_info->size = st.st_size;
retry_mmap:
    map_info->buffer = (const uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, file, 0);
    if (MAP_FAILED == map_info->buffer && (errno == EAGAIN || errno == EINTR))
        goto retry_mmap;
    close(file);
    if (MAP_FAILED == map_info->buffer)
        return -1;
    return 0;
}

static void _mpeg_dec_close_file(_mpeg_dec_map_t *map_info)
{
    if (map_info->buffer && MAP_FAILED != map_info->buffer)
        munmap((void *)map_info->buffer, map_info->size);
    map_info->buffer = 0;
    map_info->size   = 0;
}

static int32_t _mpeg_dec_parse_file(mpeg_audio_info_t* info, _mpeg_dec_map_t* map)
{
    if(!info || !map)
    {
        DBG_ERROR("invalid parameters\n");
        return -1;
    }

    const uint8_t* pbuff = map->buffer;
    uint32_t size = map->size;

    // skip id3 tag
    _skip_id3_tag(&pbuff, &size, info);

    if(MPEG_AUDIO_FRAME_HEADER_SIZE > size)
    {
        DBG_ERROR("current buffer not contain mpeg audio frame header\n");
        return -1;
    }

    bool founded = false;
    bool success_parsed = false;
    uint32_t success_count = 0;
    uint32_t fail_count = 0;
    _mpeg_header_internal_t* _tmp_header = NULL;
    mpeg_audio_frame_header_info_t tmp_header = {0};

    for(int i = 0; i + MPEG_AUDIO_FRAME_HEADER_SIZE <= size; ++i)
    {
        if (((pbuff[i] & 0xFF) != 0xFF) || ((pbuff[i + 1] & 0xE0) != 0xE0)) //first 11 bits should be 1
        {
            continue;
        }

        founded = true;
        _tmp_header = (_mpeg_header_internal_t*)(void*)(pbuff + i);
        _mpeg_dec_reset_audio_header(&tmp_header);

        DBG_INFO("%x %x %x %x\n", pbuff[i], pbuff[i+1], pbuff[i+2], pbuff[i+3]);

        success_parsed = _mpeg_audio_header_parse(_tmp_header, &tmp_header);
        if(success_parsed)
        {
            success_count++;
            success_parsed = true;
            DBG_INFO("_mpeg_audio_header_parse success, %d\n", success_count);
            _mpeg_audio_frame_header_print(&tmp_header);
            i += tmp_header.frame_length;
        }
        else
        {
            fail_count++;
            DBG_ERROR("_mpeg_audio_header_parse fail, %d\n", fail_count);
        }

        founded = false;
        success_parsed = false;
    }

    DBG_INFO("founded: %d, success_parsed: %d\n", founded, success_parsed);

    if(founded && success_parsed)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void _mpeg_dec_free_audio_info(mpeg_audio_info_t* info)
{
    if(info)
    {
        if(info->name)
        {
            free(info->name);
            info->name = NULL;
        }

    }
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        DBG_ERROR("\n");
        return -1;
    }

    int ret = 0;
    _mpeg_dec_map_t map_info = {0};
    mpeg_audio_info_t info = {0};
    int len = strlen(argv[1]);

    do
    {
        info.name = (char*)malloc(len + 1);
        if(NULL == info.name)
        {
            DBG_ERROR("\n");
            ret = -1;
            break;
        }

        memset(info.name, 0, len + 1);
        strncpy(info.name, argv[1], len);

        ret = _mpeg_dec_open_file(info.name, &map_info);
        if (0 != ret)
        {
            DBG_ERROR("ret: %d\n", ret);
            break;
        }

        ret = _mpeg_dec_parse_file(&info, &map_info);
        if (0 != ret)
        {
            DBG_ERROR("ret: %d\n", ret);
            break;
        }

    } while (0);

    _mpeg_dec_free_audio_info(&info);
    _mpeg_dec_close_file(&map_info);

    return ret;
}
