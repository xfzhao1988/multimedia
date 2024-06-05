#ifndef _TEST_H_
#define _TEST_H_

#include <stdint.h>
#include <stdbool.h>

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
} mpeg_version;

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
} mpeg_layers;

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
} mpeg_samples_per_frame;

/**
 * mp3 biterate define.
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
MP3_BITRATE_FALSE = -1,
MP3_BITRATE_NONE = 0,
MP3_BITRATE_8K   = 8000,
MP3_BITRATE_16K  = 16000,
MP3_BITRATE_24K  = 24000,
MP3_BITRATE_32K  = 32000,
MP3_BITRATE_40K  = 40000,
MP3_BITRATE_48K  = 48000,
MP3_BITRATE_56K  = 56000,
MP3_BITRATE_64K  = 64000,
MP3_BITRATE_80K  = 80000,
MP3_BITRATE_96K  = 96000,
MP3_BITRATE_112K = 112000,
MP3_BITRATE_128K = 128000,
MP3_BITRATE_144K = 144000,
MP3_BITRATE_160K = 160000,
MP3_BITRATE_176K = 176000,
MP3_BITRATE_192K = 192000,
MP3_BITRATE_224K = 224000,
MP3_BITRATE_256K = 256000,
MP3_BITRATE_288K = 288000,
MP3_BITRATE_320K = 320000,
MP3_BITRATE_352K = 352000,
MP3_BITRATE_384K = 384000,
MP3_BITRATE_416K = 416000,
MP3_BITRATE_448K = 448000
} mp3_bitrates;

/**
 * mp3 sampling rate define.
 *
 * The sampling rate specifies how many samples per second are recorded.
 * Each MPEG version can handle different sampling rates.
*/
typedef enum
{
    MP3_FREQUENCIES_FALSE = -1,
    MP3_FREQUENCIES_Reserved = 0,
    MP3_FREQUENCIES_8000HZ = 8000,
    MP3_FREQUENCIES_11025HZ = 11025,
    MP3_FREQUENCIES_12000HZ = 12000,
    MP3_FREQUENCIES_16000HZ = 16000,
    MP3_FREQUENCIES_22050HZ = 22050,
    MP3_FREQUENCIES_24000HZ = 24000,
    MP3_FREQUENCIES_32000HZ = 32000,
    MP3_FREQUENCIES_48000HZ = 48000,
    MP3_FREQUENCIES_44100HZ = 44100,
} mp3_frequencies;

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
  MP3_CHANNEL_MODE_FALSE = -1,
  MP3_CHANNEL_MODE_STEREO,
  MP3_CHANNEL_MODE_JOINT_STEREO,
  MP3_CHANNEL_MODE_DUAL_CHANNEL,
  MP3_CHANNEL_MODE_SINGLE_CHANNEL
} mp3_channel_mode;

/**
 * Mode extension (Only used in Joint Stereo)
*/
typedef enum
{
  MP3_MODE_EXT_FALSE = -1,
  MP3_MODE_EXT_0,
  MP3_MODE_EXT_1,
  MP3_MODE_EXT_2,
  MP3_MODE_EXT_3
} mp3_mode_extension;

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
  MP3_EMPHASIS_FALSE = -1,
  MP3_EMPHASIS_NONE,
  MP3_EMPHASIS_50_15MS,
  MP3_EMPHASIS_Reserved,
  MP3_EMPHASIS_CCIT_J17
} mp3_emphasis;

/**
 * Protection bit
 * 0 - protected by 16 bit CRC following header
 * 1 - no CRC
*/
typedef enum
{
  MP3_CRC_ERROR_SIZE = -2,
  MP3_CRC_MISMATCH = -1,
  MP3_CRC_NONE = 0,
  MP3_CRC_OK = 1
} mp3_crc;


/**
 * mp3 audio frame header structure define.
*/
typedef struct
{
    /**
     * 如下字段来自mpeg音频帧头的parse
    */
    mpeg_version version; // 版本号
    mpeg_layers layer; // 层
    mp3_bitrates bitrate; // 比特率
    mp3_channel_mode channelmode; // 通道模式
    mp3_mode_extension modeext; // Mode extension (Only used in Joint Stereo)
    mp3_emphasis emphasis; // Emphasis
    mp3_crc crc; // Protection bit, MP3_CRC_OK：需要crc校验，MP3_CRC_NONE：无需crc校验
    uint32_t frequency; // 采样频率
    bool privatebit; // Private bit (only informative)
    bool copyrighted; // Copyright bit (only informative)
    bool original; // Original bit (only informative)

    /**
     * 如下字段可通过mpeg音频帧头parse的结果计算得到
    */
    uint32_t frame_length; //该帧包含的字节数
    uint32_t samples_per_frame; //该帧包含的采样点数

    uint32_t frame_pos; // start of first frame in this "chain" of headers
    uint32_t next_frame_pos; // here we expect the next header with same parameters

} mp3_audio_frame_header_info;

/*
//! Used to describe a potential (chain of) MP3 headers we found
typedef struct mp3_hdr {
  off_t frame_pos; // start of first frame in this "chain" of headers
  off_t next_frame_pos; // here we expect the next header with same parameters
  int mp3_chans;
  int mp3_freq;
  int mpa_spf;
  int mpa_layer;
  int mpa_br;
  int cons_hdrs; // if this reaches MIN_MP3_HDRS we accept as MP3 file
  struct mp3_hdr *next;
} mp3_hdr_t;
*/

/**
 * parse mp3 audio frame header function.
*/
bool mp3_audio_frame_header_parse(mp3_audio_frame_header_info* out_info,
                                  const uint8_t* buffer,
                                  uint32_t size);

#endif /* _TEST_H_ */
