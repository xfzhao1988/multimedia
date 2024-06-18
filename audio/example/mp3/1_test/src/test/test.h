#ifndef _TEST_H_
#define _TEST_H_

#include <stdint.h>
#include <stdbool.h>
#include "list.h"

//#define TEST_MPEG_PARSE

#ifdef TEST_MPEG_PARSE
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

typedef struct _id3v1_tag_info
{
    uint32_t size;
} id3v1_tag_info_t;

typedef struct _id3v2_tag_info
{
    uint32_t size;
} id3v2_tag_info_t;

typedef struct _id3_tag_info
{
    id3v1_tag_info_t id3v1;
    id3v2_tag_info_t id3v2;
} id3_tag_info_t;

/**
 * mpeg audio frame header structure define.
*/
typedef struct _mpeg_audio_frame_header_info
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

typedef struct mpeg_audio_info
{
    char* name;
    bool is_valid_mpeg_audio;
    id3_tag_info_t id3;
    struct list_head header_list;
    uint32_t total_frames;
    uint32_t total_file_size;

} mpeg_audio_info_t;

#endif //TEST_MPEG_PARSE

#endif /* _TEST_H_ */
