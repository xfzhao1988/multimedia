
- Readme - MPEG Audio Info Tool V2.2 - 2007-04-09

Description:
This tool can display information about MPEG audio files. It supports
MPEG1, MPEG2, MPEG2.5 in all three layers. You can get all the fields
from the MPEG audio frame header in each frame of the file. Additionally you
can check the whole file for inconsistencies.
描述：
此工具可以显示有关 MPEG 音频文件的信息。它支持所有三个层的 MPEG1、MPEG2、MPEG2.5。
您可以从文件的每个帧中的 MPEG 音频帧头中获取所有字段。此外，您还可以检查整个文件是
否存在不一致之处。

This tool was written as an example on how to use the classes:
CMPAFile, CMPAFrame, CMPAHeader, CVBRHeader, CMPAStream, CTag and
CMPAException.
此工具是作为如何使用以下类的示例而编写的：
CMPAFile、CMPAFrame、CMPAHeader、CVBRHeader、CMPAStream、CTag 和
CMPAException。

The article MPEG Audio Frame Header on Sourceproject
[http://www.codeproject.com/audio/MPEGAudioInfo.asp]
provides additional information about these classes and the MPEG audio
frame header in general.
Sourceproject 上的文章 MPEG Audio Frame Header
[http://www.codeproject.com/audio/MPEGAudioInfo.asp]
提供了有关这些类以及 MPEG 音频帧头的其他信息。

This tool was written with MS Visual C++ 8.0. The MFC library is
statically linked.
此工具使用 MS Visual C++ 8.0 编写。MFC 库是
静态链接的。


Changes from version 2.1:

- added Musicmatch-Tag detection
- better exception handling
- improved design of CMPAStream
- added Drag&Drop functionality to dialog
- fixed memory leaks
- updated solution to Visual Studio 2005
- better performance for finding frames
与 2.1 版相比的变化：

- 添加了 Musicmatch-Tag 检测
- 更好的异常处理
- 改进了 CMPAStream 的设计
- 为对话框添加了拖放功能
- 修复了内存泄漏
- 更新了 Visual Studio 2005 的解决方案
- 查找帧的性能更好