#include "dbg.h"
#include "test.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ID3V2_TAG_HEADER_SIZE 10
#define ID3V2_TAG_ID "ID3"
#define AUDIO_FRAME_HEADER_SIZE 4

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
  } _mp3_header_internal;

static const mp3_bitrates _mp3_bitrates[2][3][16] =
{
    {
        { //MPEG 1, LAYER I
            MP3_BITRATE_NONE,
            MP3_BITRATE_32K,
            MP3_BITRATE_64K,
            MP3_BITRATE_96K,
            MP3_BITRATE_128K,
            MP3_BITRATE_160K,
            MP3_BITRATE_192K,
            MP3_BITRATE_224K,
            MP3_BITRATE_256K,
            MP3_BITRATE_288K,
            MP3_BITRATE_320K,
            MP3_BITRATE_352K,
            MP3_BITRATE_384K,
            MP3_BITRATE_416K,
            MP3_BITRATE_448K,
            MP3_BITRATE_FALSE
        },
        { //MPEG 1, LAYER II
            MP3_BITRATE_NONE,
            MP3_BITRATE_32K,
            MP3_BITRATE_48K,
            MP3_BITRATE_56K,
            MP3_BITRATE_64K,
            MP3_BITRATE_80K,
            MP3_BITRATE_96K,
            MP3_BITRATE_112K,
            MP3_BITRATE_128K,
            MP3_BITRATE_160K,
            MP3_BITRATE_192K,
            MP3_BITRATE_224K,
            MP3_BITRATE_256K,
            MP3_BITRATE_320K,
            MP3_BITRATE_384K,
            MP3_BITRATE_FALSE
        },
        { //MPEG 1, LAYER III
            MP3_BITRATE_NONE,
            MP3_BITRATE_32K,
            MP3_BITRATE_40K,
            MP3_BITRATE_48K,
            MP3_BITRATE_56K,
            MP3_BITRATE_64K,
            MP3_BITRATE_80K,
            MP3_BITRATE_96K,
            MP3_BITRATE_112K,
            MP3_BITRATE_128K,
            MP3_BITRATE_160K,
            MP3_BITRATE_192K,
            MP3_BITRATE_224K,
            MP3_BITRATE_256K,
            MP3_BITRATE_320K,
            MP3_BITRATE_FALSE
        }
    },
    {
        { //MPEG 2 or 2.5, LAYER I
            MP3_BITRATE_NONE,
            MP3_BITRATE_32K,
            MP3_BITRATE_48K,
            MP3_BITRATE_56K,
            MP3_BITRATE_64K,
            MP3_BITRATE_80K,
            MP3_BITRATE_96K,
            MP3_BITRATE_112K,
            MP3_BITRATE_128K,
            MP3_BITRATE_144K,
            MP3_BITRATE_160K,
            MP3_BITRATE_176K,
            MP3_BITRATE_192K,
            MP3_BITRATE_224K,
            MP3_BITRATE_256K,
            MP3_BITRATE_FALSE
        },
        { //MPEG 2 or 2.5, LAYER II
            MP3_BITRATE_NONE,
            MP3_BITRATE_8K,
            MP3_BITRATE_16K,
            MP3_BITRATE_24K,
            MP3_BITRATE_32K,
            MP3_BITRATE_40K,
            MP3_BITRATE_48K,
            MP3_BITRATE_56K,
            MP3_BITRATE_64K,
            MP3_BITRATE_80K,
            MP3_BITRATE_96K,
            MP3_BITRATE_112K,
            MP3_BITRATE_128K,
            MP3_BITRATE_144K,
            MP3_BITRATE_160K,
            MP3_BITRATE_FALSE
        },
        { //MPEG 2 or 2.5, LAYER III
            MP3_BITRATE_NONE,
            MP3_BITRATE_8K,
            MP3_BITRATE_16K,
            MP3_BITRATE_24K,
            MP3_BITRATE_32K,
            MP3_BITRATE_40K,
            MP3_BITRATE_48K,
            MP3_BITRATE_56K,
            MP3_BITRATE_64K,
            MP3_BITRATE_80K,
            MP3_BITRATE_96K,
            MP3_BITRATE_112K,
            MP3_BITRATE_128K,
            MP3_BITRATE_144K,
            MP3_BITRATE_160K,
            MP3_BITRATE_FALSE
        }
    }
};

static const mp3_frequencies _mp3_frequencies[4][4] =
{
    { MP3_FREQUENCIES_11025HZ, MP3_FREQUENCIES_12000HZ, MP3_FREQUENCIES_8000HZ,MP3_FREQUENCIES_Reserved },  //MPEG_VERSION_2_5
    { MP3_FREQUENCIES_Reserved, MP3_FREQUENCIES_Reserved, MP3_FREQUENCIES_Reserved, MP3_FREQUENCIES_Reserved}, //MPEG_VERSION_Reserved
    { MP3_FREQUENCIES_22050HZ, MP3_FREQUENCIES_24000HZ, MP3_FREQUENCIES_16000HZ, MP3_FREQUENCIES_Reserved }, //MPEG_VERSION_2
    { MP3_FREQUENCIES_44100HZ, MP3_FREQUENCIES_48000HZ, MP3_FREQUENCIES_32000HZ, MP3_FREQUENCIES_Reserved }  //MPEG_VERSION_1
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
static const mpeg_samples_per_frame _samples_per_frame[2][3] =
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

static uint32_t _skip_id3v2_tag(const uint8_t *buf, uint32_t buf_size)
{
    if ( buf_size >= ID3V2_TAG_HEADER_SIZE &&
        !memcmp(buf, ID3V2_TAG_ID, 3) &&
        !((buf[5] & 15) || (buf[6] & 0x80) || (buf[7] & 0x80) || (buf[8] & 0x80) || (buf[9] & 0x80)) )
    {
        uint32_t size = (((buf[6] & 0x7f) << 21) | ((buf[7] & 0x7f) << 14) | ((buf[8] & 0x7f) << 7) | (buf[9] & 0x7f)) + 10;
        if ((buf[5] & 16))
            size += 10; /* footer */
        return size;
    }

    return 0;
};

static void _reset_mp3_audio_frame_header_info(mp3_audio_frame_header_info* info)
{
    info->version = MPEG_VERSION_FALSE;
    info->layer = MPEG_LAYER_FALSE;
    info->bitrate = MP3_BITRATE_FALSE;
    info->channelmode = MP3_CHANNEL_MODE_FALSE;
    info->modeext = MP3_MODE_EXT_FALSE;
    info->emphasis = MP3_EMPHASIS_FALSE;
    info->crc = MP3_CRC_MISMATCH;
    info->frequency = 0;
    info->frame_length = 0;
    //info->frames = 0;
    //info->time = 0;
    //info->vbr_bitrate = 0;
}

static void _print_mp3_audio_frame_header_info(mp3_audio_frame_header_info* info)
{
    printf("\n\n\n");
    printf("Version: Mpeg %s\n", info->version == MPEG_VERSION_2_5 ?
                                 "2.5": ((info->version == MPEG_VERSION_2)) ?
                                 "2" : "1");
    printf("Layer: %s\n", info->layer == MPEG_LAYER_I ?
                          "1" : (info->layer == MPEG_LAYER_II) ?
                          "2" : "3");
    printf("Bitrate: %d\n", info->bitrate);
    printf("Channelmode: %s\n", info->channelmode == MP3_CHANNEL_MODE_STEREO ?
                                "Stereo" : (info->channelmode == MP3_CHANNEL_MODE_JOINT_STEREO ?
                                "Joint Stereo" : (info->channelmode == MP3_CHANNEL_MODE_DUAL_CHANNEL ?
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
    printf("\n\n\n");
}

static bool _mp3_header_internal_parse(_mp3_header_internal* header,
                                       mp3_audio_frame_header_info* out_info)
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
    out_info->bitrate = _mp3_bitrates[bit_rate_index][3-header->layer][header->bit_rate_index];
    if (out_info->bitrate == MP3_BITRATE_FALSE)
    {
        DBG_ERROR("bitrate: %d\n", out_info->bitrate);
        return false;
    }

    // parse frequency
    out_info->frequency = _mp3_frequencies[header->id][header->frequency];
    if (out_info->frequency == MP3_FREQUENCIES_Reserved)
    {
        DBG_ERROR("frequency: %d\n", out_info->frequency);
        return false;
    }

    out_info->privatebit = (bool)header->private_bit;
    out_info->copyrighted = (bool)header->copyright;
    out_info->original = (bool)header->original;
    out_info->crc = (mp3_crc)!(bool)header->protection_bit;

    //parse channel mode
    switch (header->mode)
    {
        case 3:
            out_info->channelmode = MP3_CHANNEL_MODE_SINGLE_CHANNEL;
            break;
        case 2:
            out_info->channelmode = MP3_CHANNEL_MODE_DUAL_CHANNEL;
            break;
        case 1:
            out_info->channelmode = MP3_CHANNEL_MODE_JOINT_STEREO;
            break;
        case 0:
            out_info->channelmode = MP3_CHANNEL_MODE_STEREO;
            break;
        default:
            DBG_ERROR("mode: %d\n", header->mode);
            return false; //wouldn't know how to handle it
    }

    //parse external mode for joint stereo
    if (out_info->channelmode == MP3_CHANNEL_MODE_JOINT_STEREO)
    {
        // these have a different meaning for different layers, better give them a generic name in the enum
        switch (header->mode_ext)
        {
            case 3:
                out_info->modeext = MP3_MODE_EXT_3;
                break;
            case 2:
                out_info->modeext = MP3_MODE_EXT_2;
                break;
            case 1:
                out_info->modeext = MP3_MODE_EXT_1;
                break;
            case 0:
                out_info->modeext = MP3_MODE_EXT_0;
                break;
            default:
                DBG_ERROR("mode_ext: %d\n", header->mode_ext);
                return false; //wouldn't know how to handle it
        }
    }
    else //it's valid to have a valid false one in this case, since it's only used with joint stereo
    {
        out_info->modeext = MP3_MODE_EXT_FALSE;
    }

    //parse emphasis
    switch (header->emphasis)
    {
        case 3:
            out_info->emphasis = MP3_EMPHASIS_CCIT_J17;
            break;
        case 2:
            out_info->emphasis = MP3_EMPHASIS_Reserved;
            break;
        case 1:
            out_info->emphasis = MP3_EMPHASIS_50_15MS;
            break;
        case 0:
            out_info->emphasis = MP3_EMPHASIS_NONE;
            break;
        default:
            DBG_ERROR("emphasis: %d\n", header->emphasis);
            return false; //wouldn't know how to handle it
    }

    //parse frame length with bytes
    //http://www.mp3-tech.org/programmer/frame_header.html
    if (out_info->bitrate != MP3_BITRATE_NONE && out_info->frequency > 0)
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

    out_info->samples_per_frame = _samples_per_frame[b_lsf][out_info->layer];

    DBG_INFO("_mp3_header_internal_parse ok\n");

    return true;
}

bool mp3_audio_frame_header_parse(mp3_audio_frame_header_info* out_info,
                                  const uint8_t* buffer,
                                  uint32_t size)
{
    if(NULL ==  out_info ||
       buffer == NULL ||
       size == 0)
    {
        DBG_ERROR("invalid parameters\n");
        return false;
    }

    const uint8_t* pbuff = buffer;
    uint32_t id3v2_size = 0;


    // skip id3v2 tag
    id3v2_size = _skip_id3v2_tag(buffer, size);
    DBG_INFO("id3v2size: %d\n", id3v2_size);

    if(id3v2_size + AUDIO_FRAME_HEADER_SIZE > size )
    {
        DBG_ERROR("current buffer not contain mp3 audio frame header\n");
        return false;
    }

    // reset out_info
    _reset_mp3_audio_frame_header_info(out_info);

    uint32_t scan_size = size - id3v2_size;
    pbuff += id3v2_size;
    bool founded = false;
    bool success_parsed = false;
    _mp3_header_internal* tmp_header = NULL;

    for(int i = 0; i + AUDIO_FRAME_HEADER_SIZE <= scan_size; ++i)
    {
        if (((pbuff[i] & 0xFF) != 0xFF) || ((pbuff[i + 1] & 0xE0) != 0xE0)) //first 11 bits should be 1
        {
            continue;
        }

        founded = true;
        tmp_header = (_mp3_header_internal*)(void*)(pbuff + i);

        success_parsed = _mp3_header_internal_parse(tmp_header, out_info);
        if(success_parsed)
        {
            success_parsed = true;
            _print_mp3_audio_frame_header_info(out_info);
            break;
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
        _reset_mp3_audio_frame_header_info(out_info);
        return false;
    }
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        DBG_ERROR("\n");
        goto DONE;
    }

    int ret = 0;

    FILE* mp3_file = fopen(argv[1], "rb");
    if(NULL == mp3_file)
    {
        DBG_ERROR("\n");
        ret = -1;
        goto DONE;
    }

    long int size = 0;

    fseek(mp3_file, 0, SEEK_END);
    size = ftell(mp3_file);

    DBG_INFO("size: %ld\n", size);

    uint8_t* buffer = (uint8_t*)malloc(size);
    if(NULL == buffer)
    {
        DBG_ERROR("\n");
        ret = -1;
        goto DONE;
    }

    fseek(mp3_file, 0, SEEK_SET);
    size_t count = fread(buffer, sizeof(uint8_t), size, mp3_file);
    if(count != size)
    {
        DBG_ERROR("count: %ld\n", count);
        ret = -1;
        goto DONE;
    }

    mp3_audio_frame_header_info info = {0};
    mp3_audio_frame_header_parse(&info, buffer, size);

DONE:
    if(mp3_file)
    {
        fclose(mp3_file);
        mp3_file = NULL;
    }

    if(buffer)
    {
        free(buffer);
        buffer = NULL;
    }

	return ret;
}
