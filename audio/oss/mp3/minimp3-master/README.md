minimp3
==========

[![Build Status](https://travis-ci.org/lieff/minimp3.svg)](https://travis-ci.org/lieff/minimp3)
<a href="https://scan.coverity.com/projects/lieff-minimp3">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/14844/badge.svg"/>
</a>
[![codecov](https://codecov.io/gh/lieff/minimp3/branch/master/graph/badge.svg)](https://codecov.io/gh/lieff/minimp3)

/**
 *
 “perf”是一个功能强大的性能分析工具，通常用于Linux系统上。它能够提供关于程序运行时各种性能指标的详细信息，帮助开发者分析和优化程序的性能。

下面是使用“perf”工具进行性能测量的一般步骤：

安装perf：首先，确保你的系统上安装了perf工具。在大多数Linux发行版中，perf通常包含在“perf”或“linux-tools”软件包中。你可以使用包管理器来安装它，例如在Ubuntu上可以使用以下命令：

arduino
复制代码
sudo apt-get install linux-tools-common linux-tools-generic
选择性能事件：perf可以监测各种不同的性能事件，例如CPU周期、指令执行数、缓存命中率等。你可以使用perf list命令来列出所有可用的性能事件，并选择你感兴趣的事件。

运行perf：使用perf命令启动你要测试的程序，并将性能事件作为参数传递给perf。例如，要监测一个名为“my_program”的程序的CPU周期，你可以运行以下命令：

bash
复制代码
perf stat -e cycles ./my_program
分析结果：perf会在程序运行结束后输出性能指标的统计信息。你可以分析这些信息来了解程序的性能状况，找出性能瓶颈所在，并进行优化。perf还可以生成一些报告，如调用图、热点函数等，帮助你更好地理解程序的性能特征。

调整参数：根据需要，你可以调整perf的参数来获取更多或更详细的性能信息。perf提供了丰富的命令选项和配置参数，可以根据具体情况进行调整。

总的来说，perf是一个非常强大的性能分析工具，可以帮助开发者深入了解程序的性能特征，并进行针对性的优化。
 */
Minimalistic, single-header library for decoding MP3. minimp3 is designed to be
small, fast (with SSE and NEON support), and accurate (ISO conformant). You can
find a rough benchmark below, measured using ``perf`` on an i7-6700K, IO
included, no CPU heat to address speedstep:
用于解码 MP3 的简约单头库。minimp3 的设计目标是小巧、快速（支持 SSE 和 NEON）和准确（符合 ISO 标准）。您可以在下面找到一个粗略的基准，使用 i7-6700K 上的“perf”测量，包括 IO，没有 CPU 热量来解决 speedstep：


| Vector      | Hz    | Samples| Sec    | Clockticks | Clockticks per second | PSNR | Max diff |
| ----------- | ----- | ------ | ------ | --------- | ------ | ------ | - |
|compl.bit    | 48000 | 248832 | 5.184  | 14306684  | 2.759M | 124.22 | 1 |
|he_32khz.bit | 32000 | 172800 | 5.4    | 8426158   | 1.560M | 139.67 | 1 |
|he_44khz.bit | 44100 | 472320 | 10.710 | 21296300  | 1.988M | 144.04 | 1 |
|he_48khz.bit | 48000 | 172800 | 3.6    | 8453846   | 2.348M | 139.67 | 1 |
|hecommon.bit | 44100 | 69120  | 1.567  | 3169715   | 2.022M | 133.93 | 1 |
|he_free.bit  | 44100 | 156672 | 3.552  | 5798418   | 1.632M | 137.48 | 1 |
|he_mode.bit  | 44100 | 262656 | 5.955  | 9882314   | 1.659M | 118.00 | 1 |
|si.bit       | 44100 | 135936 | 3.082  | 7170520   | 2.326M | 120.30 | 1 |
|si_block.bit | 44100 | 73728  | 1.671  | 4233136   | 2.533M | 125.18 | 1 |
|si_huff.bit  | 44100 | 86400  | 1.959  | 4785322   | 2.442M | 107.98 | 1 |
|sin1k0db.bit | 44100 | 725760 | 16.457 | 24842977  | 1.509M | 111.03 | 1 |

Conformance test passed on all vectors (PSNR > 96db).
所有矢量均通过一致性测试 (PSNR > 96db)。

## Comparison with keyj's [minimp3](https://keyj.emphy.de/minimp3/)
与 keyj 的 [minimp3](https://keyj.emphy.de/minimp3/) 进行比较

Comparison by features:
按功能比较：

| Keyj minimp3 | Current |
| ------------ | ------- |
| Fixed point  | Floating point |
| source: 84kb | 70kb |
| binary: 34kb (20kb compressed) | 30kb (20kb) |
| no vector opts | SSE/NEON intrinsics |
                   SSE/NEON 内部函数
| no free format | free format support |

Below, you can find the benchmark and conformance test for keyj's minimp3:
下面，您可以找到 keyj 的 minimp3 的基准和一致性测试：


| Vector      | Hz    | Samples| Sec    | Clockticks | Clockticks per second | PSNR | Max diff |
| ----------- | ----- | ------ | ------ | --------- | ------  | ----- | - |
|compl.bit    | 48000 | 248832 | 5.184  | 31849373  | 6.143M  | 71.50 | 41 |
|he_32khz.bit | 32000 | 172800 | 5.4    | 26302319  | 4.870M  | 71.63 | 24 |
|he_44khz.bit | 44100 | 472320 | 10.710 | 41628861  | 3.886M  | 71.63 | 24 |
|he_48khz.bit | 48000 | 172800 | 3.6    | 25899527  | 7.194M  | 71.63 | 24 |
|hecommon.bit | 44100 | 69120  | 1.567  | 20437779  | 13.039M | 71.58 | 25 |
|he_free.bit  | 44100 | 0 | 0  | -  | - | -  | - |
|he_mode.bit  | 44100 | 262656 | 5.955  | 30988984  | 5.203M  | 71.78 | 27 |
|si.bit       | 44100 | 135936 | 3.082  | 24096223  | 7.817M  | 72.35 | 36 |
|si_block.bit | 44100 | 73728  | 1.671  | 20722017  | 12.394M | 71.84 | 26 |
|si_huff.bit  | 44100 | 86400  | 1.959  | 21121376  | 10.780M | 27.80 | 65535 |
|sin1k0db.bit | 44100 | 730368 | 16.561 | 55569636  | 3.355M  | 0.15  | 58814 |

Keyj minimp3 conformance test fails on all vectors (PSNR < 96db), and free
format is unsupported. This caused some problems when it was used
[here](https://github.com/lieff/lvg), and was the main motivation for this work.
Keyj minimp3 一致性测试在所有向量上均失败（PSNR < 96db），并且不支持自由格式。这在 [此处](https://github.com/lieff/lvg) 使用它时造成了一些问题，也是本研究的主要动机。

## Usage
使用

First, we need to initialize the decoder structure:
首先，我们需要初始化解码器结构：

```c
//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
...
    static mp3dec_t mp3d;
    mp3dec_init(&mp3d);
```

Note that you must define ``MINIMP3_IMPLEMENTATION`` in exactly one source file.
请注意，您必须在一个源文件中定义“MINIMP3_IMPLEMENTATION”。
You can ``#include`` ``minimp3.h`` in as many files as you like.
您可以在任意数量的文件中“#include”“minimp3.h”。
Also you can use ``MINIMP3_ONLY_MP3`` define to strip MP1/MP2 decoding code.
您还可以使用“MINIMP3_ONLY_MP3”定义来剥离 MP1/MP2 解码代码。
/**
 * 这个描述指的是在使用minimp3库时，MINIMP3_ONLY_SIMD宏定义控制着是否启用了SIMD（Single Instruction, Multiple Data）
 * 指令集的优  化。SIMD指令集允许一次性对多个数据执行相同的操作，从而提高了计算效率，特别是在音频处理等需要大量数据处理的场景下。
 * MINIMP3_ONLY_SIMD宏定义的作用是控制是否仅仅启用了SIMD指令集的优化，而不使用其他任何特定于硬件的优化。当MINIMP3_ONLY_SIMD被定义* * 时，意味着代码将会生成一个通用的、非SSE（Streaming SIMD Extensions）或NEON（ARM的SIMD指令集）的版本，即便在支持这些指令集的环境下* 也是如此。这样可以确保代码在不同的平台上都能够运行，并且对于一些不支持特定SIMD指令集的环境，也可以保证代码的兼容性。
 * 然而，需要注意的是，在x64（64位x86架构）和arm64（64位ARM架构）目标平台上，MINIMP3_ONLY_SIMD宏定义总是被启用的。这是因为这些平台通* 常支持较新的SIMD指令集，而且编译器在这些平台上可能会生成更有效率的SIMD代码，因此即使定义了MINIMP3_ONLY_SIMD宏，也会使用SIMD指令集* 的优化
 */
MINIMP3_ONLY_SIMD define controls generic (non SSE/NEON) code generation (always enabled on x64/arm64 targets).
MINIMP3_ONLY_SIMD 定义控制通用（非 SSE/NEON）代码生成（始终在 x64/arm64 目标上启用）。

In case you do not want any platform-specific SIMD optimizations, you can define ``MINIMP3_NO_SIMD``.
MINIMP3_NONSTANDARD_BUT_LOGICAL define saves some code bytes, and enforces non-standard but logical behaviour of mono-stereo transition (rare case).
如果您不想要任何特定于平台的 SIMD 优化，您可以定义“MINIMP3_NO_SIMD”。
MINIMP3_NONSTANDARD_BUT_LOGICAL 定义节省了一些代码字节，并强制执行非标准但合乎逻辑的单声道立体声转换行为（罕见情况）。

MINIMP3_FLOAT_OUTPUT makes ``mp3dec_decode_frame()`` output to be float instead of short and additional function mp3dec_f32_to_s16 will be available for float->short conversion if needed.
MINIMP3_FLOAT_OUTPUT 使“mp3dec_decode_frame()”输出为浮点数而不是短整型，并且如果需要，附加函数 mp3dec_f32_to_s16 可用于浮点数->短整型的转换。

Then. we decode the input stream frame-by-frame:
然后，我们逐帧解码输入流：

```c
    /*typedef struct
    {
        int frame_bytes;
        int channels;
        int hz;
        int layer;
        int bitrate_kbps;
    } mp3dec_frame_info_t;*/
    mp3dec_frame_info_t info;
    short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    /*unsigned char *input_buf; - input byte stream*/
    samples = mp3dec_decode_frame(&mp3d, input_buf, buf_size, pcm, &info);
```

The ``mp3dec_decode_frame()`` function decodes one full MP3 frame from the
input buffer, which must be large enough to hold one full frame.
“mp3dec_decode_frame()”函数从输入缓冲区解码一个完整的 MP3 帧，该缓冲区必须足够大才能容纳一个完整的帧。

The decoder will analyze the input buffer to properly sync with the MP3 stream,
and will skip ID3 data, as well as any data which is not valid. Short buffers
may cause false sync and can produce 'squealing' artefacts. The bigger the size
of the input buffer, the more reliable the sync procedure. We recommend having
as many as 10 consecutive MP3 frames (~16KB) in the input buffer at a time.
解码器将分析输入缓冲区以正确与 MP3 流同步，并跳过 ID3 数据以及任何无效数据。短缓冲区可能会导致错误同步并
产生“尖叫”伪影。输入缓冲区的大小越大，同步过程越可靠。我们建议一次在输入缓冲区中放置多达 10 个连续的
MP3 帧（~16KB）。

At end of stream just pass rest of the buffer, sync procedure should work even
with just 1 frame in stream (except for free format and garbage at the end can
mess things up, so id3v1 and ape tags must be removed first).
在流的末尾只需传递缓冲区的其余部分，即使流中只有 1 帧，同步过程也应该可以工作（除了自由格式，末尾的垃圾会弄乱事情，
因此必须先删除 id3v1 和 ape 标签）。

/**
 * 在自由格式（Free Format）的MP3文件中，由于缺乏同步信息，需要至少3个帧才能进行正确的同步。

* 在MP3文件中，每个音频帧都以帧同步字（Frame Sync Word）开头，用于标识帧的开始位置。在标准的固定比特率（CBR）MP3
* 文件中，帧同步字的位置是已知的，因此可以很容易地对帧进行同步。

* 然而，在自由格式的MP3文件中，由于帧之间的比特率不固定，因此帧同步字的位置不确定，导致无法直接对帧进行同步。
* 为了在自由格式的MP3文件中进行同步，需要使用一种启发式方法。

 * 一种常见的方法是，先读取两个帧，利用第一个帧来估计帧的长度，然后使用第二个帧来确认估计的帧长度是否正确。
 * 如果估计的帧长度与第二个帧的实际长度一致，那么就可以认为同步成功。如果不一致，可能需要尝试其他方法或进行更复杂的同步处理。

 * 因此，为了在自由格式的MP3文件中进行正确的同步，至少需要读取3个帧：2个用于估计帧长度，1个用于确认同步是否成功。
 */
For free format there minimum 3 frames needed to do proper sync: 2 frames to
detect frame length and 1 next frame to check detect is good.
对于自由格式，至少需要 3 帧才能完成正确的同步：2 帧用于检测帧长度，1 帧用于检查检测是否良好。

The size of the consumed MP3 data is returned in the ``mp3dec_frame_info_t``
field of the ``frame_bytes`` struct; you must remove the data corresponding to
the ``frame_bytes`` field  from the input buffer before the next decoder
invocation.
已消耗的 MP3 数据的大小在“mp3dec_frame_info_t”结构的“frame_bytes”字段中返回；
您必须在下一次解码器调用之前从输入缓冲区中删除与“frame_bytes”字段相对应的数据。

The decoding function returns the number of decoded samples. The following cases
are possible:
解码函数返回解码样本的数量。可能存在以下情况：

- **0:** No MP3 data was found in the input buffer
         输入缓冲区中未找到 MP3 数据
- **384:**  Layer 1
            层1
- **576:**  MPEG 2 Layer 3
            MPEG 2层3
- **1152:** Otherwise
            否则

The following is a description of the possible combinations of the number of
samples and ``frame_bytes`` field values:
以下是样本数量和“frame_bytes”字段值的可能组合的描述：

- More than 0 samples and ``frame_bytes > 0``:  Succesful decode
  样本超过 0 个且“frame_bytes > 0”：解码成功
- 0 samples and ``frame_bytes >  0``: The decoder skipped ID3 or invalid data
  0 个样本和 ``frame_bytes > 0``：解码器跳过了 ID3 或无效数据
- 0 samples and ``frame_bytes == 0``: Insufficient data
  0 个样本且 ``frame_bytes == 0``：数据不足

If ``frame_bytes == 0``, the other fields may be uninitialized or unchanged; if
``frame_bytes != 0``, the other fields are available. The application may call
``mp3dec_init()`` when changing decode position, but this is not necessary.
如果“frame_bytes == 0”，则其他字段可能未初始化或保持不变；如果“frame_bytes != 0”，则其他字段可用。
应用程序可以在改变解码位置时调用“mp3dec_init()”，但这不是必需的。

As a special case, the decoder supports already split MP3 streams (for example,
after doing an MP4 demux). In this case, the input buffer must contain _exactly
one_ non-free-format frame.
作为一种特殊情况，解码器支持已分割的 MP3 流（例如，在执行 MP4 解复用之后）。在这种情况下，
输入缓冲区必须包含一个非自由格式帧。

## Seeking

You can seek to any byte in the stream and call ``mp3dec_decode_frame``; this
will work in almost all cases, but is not completely guaranteed. Probablility of
sync procedure failure lowers when MAX_FRAME_SYNC_MATCHES value grows. Default
MAX_FRAME_SYNC_MATCHES=10 and probablility of sync failure should be very low.
If granule data is accidentally detected as a valid MP3 header, short audio artefacting is
possible.
您可以查找流中的任意字节并调用“mp3dec_decode_frame”；这在几乎所有情况下都有效，但不能完全保证。
当 MAX_FRAME_SYNC_MATCHES 值增加时，同步过程失败的概率会降低。默认 MAX_FRAME_SYNC_MATCHES=10，
同步失败的概率应该非常低。如果意外地将颗粒数据检测为有效的 MP3 标头，则可能出现短暂的音频伪影。

High-level mp3dec_ex_seek function supports precise seek to sample (MP3D_SEEK_TO_SAMPLE)
using index and binary search.
高级 mp3dec_ex_seek 函数支持使用索引和二进制搜索精确查找样本 (MP3D_SEEK_TO_SAMPLE)。

## Track length detect
轨道长度检测

If the file is known to be cbr, then all frames have equal size and
lack ID3 tags, which allows us to decode the first frame and calculate all frame
positions as ``frame_bytes * N``. However, because of padding, frames can differ
in size even in this case.
如果已知文件为 cbr，则所有帧的大小均相等且缺少 ID3 标签，这使我们能够解码第一帧并将所有帧位置计算为“frame_bytes * N”。
但是，由于填充，即使在这种情况下，帧的大小也可能不同。

In general case whole stream scan is needed to calculate it's length. Scan can be
omitted if vbr tag is present (added by encoders like lame and ffmpeg), which contains
length info. High-level functions automatically use the vbr tag if present.
一般情况下，需要扫描整个流来计算其长度。如果存在包含长度信息的 vbr 标签（由 lame 和 ffmpeg 等编码器添加），
则可以省略扫描。如果存在，高级函数会自动使用 vbr 标签。

## High-level API

If you need only decode file/buffer or use precise seek, you can use optional high-level API.
Just ``#include`` ``minimp3_ex.h`` instead and use following additional functions:
如果您只需要解码文件/缓冲区或使用精确seek，则可以使用可选的高级 API。只需“#include”“minimp3_ex.h”并
使用以下附加函数：

```c
#define MP3D_SEEK_TO_BYTE   0
#define MP3D_SEEK_TO_SAMPLE 1

#define MINIMP3_PREDECODE_FRAMES 2 /* frames to pre-decode and skip after seek (to fill internal structures) */
/*#define MINIMP3_SEEK_IDX_LINEAR_SEARCH*/ /* define to use linear index search instead of binary search on seek */
#define MINIMP3_IO_SIZE (128*1024) /* io buffer size for streaming functions, must be greater than MINIMP3_BUF_SIZE */
#define MINIMP3_BUF_SIZE (16*1024) /* buffer which can hold minimum 10 consecutive mp3 frames (~16KB) worst case */
#define MINIMP3_ENABLE_RING 0      /* enable hardware magic ring buffer if available, to make less input buffer memmove(s) in callback IO mode */

#define MP3D_E_MEMORY  -1
#define MP3D_E_IOERROR -2

typedef struct
{
    mp3d_sample_t *buffer;
    size_t samples; /* channels included, byte size = samples*sizeof(mp3d_sample_t) */
    int channels, hz, layer, avg_bitrate_kbps;
} mp3dec_file_info_t;

typedef size_t (*MP3D_READ_CB)(void *buf, size_t size, void *user_data);
typedef int (*MP3D_SEEK_CB)(uint64_t position, void *user_data);

typedef struct
{
    MP3D_READ_CB read;
    void *read_data;
    MP3D_SEEK_CB seek;
    void *seek_data;
} mp3dec_io_t;

typedef struct
{
    uint64_t samples;
    mp3dec_frame_info_t info;
    int last_error;
    ...
} mp3dec_ex_t;

typedef int (*MP3D_ITERATE_CB)(void *user_data, const uint8_t *frame, int frame_size, int free_format_bytes, size_t buf_size, uint64_t offset, mp3dec_frame_info_t *info);
typedef int (*MP3D_PROGRESS_CB)(void *user_data, size_t file_size, uint64_t offset, mp3dec_frame_info_t *info);

/* decode whole buffer block */
int mp3dec_load_buf(mp3dec_t *dec, const uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
int mp3dec_load_cb(mp3dec_t *dec, mp3dec_io_t *io, uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
/* iterate through frames */
int mp3dec_iterate_buf(const uint8_t *buf, size_t buf_size, MP3D_ITERATE_CB callback, void *user_data);
int mp3dec_iterate_cb(mp3dec_io_t *io, uint8_t *buf, size_t buf_size, MP3D_ITERATE_CB callback, void *user_data);
/* streaming decoder with seeking capability */
int mp3dec_ex_open_buf(mp3dec_ex_t *dec, const uint8_t *buf, size_t buf_size, int seek_method);
int mp3dec_ex_open_cb(mp3dec_ex_t *dec, mp3dec_io_t *io, int seek_method);
void mp3dec_ex_close(mp3dec_ex_t *dec);
int mp3dec_ex_seek(mp3dec_ex_t *dec, uint64_t position);
size_t mp3dec_ex_read(mp3dec_ex_t *dec, mp3d_sample_t *buf, size_t samples);
#ifndef MINIMP3_NO_STDIO
/* stdio versions of file load, iterate and stream */
int mp3dec_load(mp3dec_t *dec, const char *file_name, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
int mp3dec_iterate(const char *file_name, MP3D_ITERATE_CB callback, void *user_data);
int mp3dec_ex_open(mp3dec_ex_t *dec, const char *file_name, int seek_method);
#ifdef _WIN32
int mp3dec_load_w(mp3dec_t *dec, const wchar_t *file_name, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
int mp3dec_iterate_w(const wchar_t *file_name, MP3D_ITERATE_CB callback, void *user_data);
int mp3dec_ex_open_w(mp3dec_ex_t *dec, const wchar_t *file_name, int seek_method);
#endif
#endif
```

Use MINIMP3_NO_STDIO define to exclude STDIO functions.
MINIMP3_ALLOW_MONO_STEREO_TRANSITION allows mixing mono and stereo in same file.
In that case ``mp3dec_frame_info_t->channels = 0`` is reported on such files and correct channels number passed to progress_cb callback for each frame in mp3dec_frame_info_t structure.
MP3D_PROGRESS_CB is optional and can be NULL, example of file decoding:
使用 MINIMP3_NO_STDIO 定义排除 STDIO 函数。
MINIMP3_ALLOW_MONO_STEREO_TRANSITION 允许在同一文件中混合单声道和立体声。
在这种情况下，此类文件会报告 ``mp3dec_frame_info_t->channels = 0``，并将正确的声道号传递给
mp3dec_frame_info_t 结构中每帧的 progress_cb 回调。
MP3D​​_PROGRESS_CB 是可选的，可以为 NULL，文件解码示例：

```c
    mp3dec_t mp3d;
    mp3dec_file_info_t info;
    if (mp3dec_load(&mp3d, input_file_name, &info, NULL, NULL))
    {
        /* error */
    }
    /* mp3dec_file_info_t contains decoded samples and info,
       use free(info.buffer) to deallocate samples */
```

Example of file decoding with seek capability:

```c
    mp3dec_ex_t dec;
    if (mp3dec_ex_open(&dec, input_file_name, MP3D_SEEK_TO_SAMPLE))
    {
        /* error */
    }
    /* dec.samples, dec.info.hz, dec.info.layer, dec.info.channels should be filled */
    if (mp3dec_ex_seek(&dec, position))
    {
        /* error */
    }
    mp3d_sample_t *buffer = malloc(dec.samples*sizeof(mp3d_sample_t));
    size_t readed = mp3dec_ex_read(&dec, buffer, dec.samples);
    if (readed != dec.samples) /* normal eof or error condition */
    {
        if (dec.last_error)
        {
            /* error */
        }
    }
```

## Bindings

 * https://github.com/tosone/minimp3 - go bindings
 * https://github.com/notviri/rmp3 - rust `no_std` bindings which don't allocate.
 * https://github.com/germangb/minimp3-rs - rust bindings
 * https://github.com/johangu/node-minimp3 - NodeJS bindings
 * https://github.com/pyminimp3/pyminimp3 - python bindings
 * https://github.com/bashi/minimp3-wasm - wasm bindings
 * https://github.com/DasZiesel/minimp3-delphi - delphi bindings
 * https://github.com/mgeier/minimp3_ex-sys - low-level rust bindings to `minimp3_ex`

## Interesting links

 * https://keyj.emphy.de/minimp3/
 * https://github.com/technosaurus/PDMP3
 * https://github.com/technosaurus/PDMP2
 * https://github.com/packjpg/packMP3
 * https://sites.google.com/a/kmlager.com/www/projects
 * https://sourceforge.net/projects/mp3dec/
 * http://blog.bjrn.se/2008/10/lets-build-mp3-decoder.html
 * http://www.mp3-converter.com/mp3codec/
 * http://www.multiweb.cz/twoinches/mp3inside.htm
 * https://www.mp3-tech.org/
 * https://id3.org/mp3Frame
 * https://www.datavoyage.com/mpgscript/mpeghdr.htm
