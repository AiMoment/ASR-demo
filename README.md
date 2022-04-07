# ASR-demo
> Qt 采集麦克风PCM数据

## 一、pcm 概述
[PCM](./docs/audioArguments.md)(Pulse Code Modulation，脉冲编码调制)音频数据是未经压缩的音频采样数据裸流，它是由模拟信号经过采样、量化、编码转换成的标准数字音频数据。

**描述PCM数据的6个参数:**
- Sample Rate : 采样频率。8kHz(电话)、44.1kHz(CD)、48kHz(DVD)。
- Sample Size : 量化位数。通常该值为16-bit。
- Number of Channels : 通道个数。常见的音频有立体声(stereo)和单声道(mono)两种类型，立体声包含左声道和右声道。另外还有环绕立体声等其它不太常用的类型。
- Sign : 表示样本数据是否是有符号位，比如用一字节表示的样本数据，有符号的话表示范围为-128 ~ 127，无符号是0 ~ 255。
- Byte Ordering : 字节序。字节序是little-endian还是big-endian。通常均为little-endian

**Qt C++ 设置pcm参数**
```c++
QAudioFormat m_audioFormat;

m_audioFormat.setSampleRate(16000); // 设置采样率 每秒钟取得声音样本的次数
m_audioFormat.setChannelCount(1); // 设定声道数目，mono(平声道)的声道数目是1；stero(立体声)的声道数目是2
m_audioFormat.setSampleSize(16); // 设置采样大小，一般为8位或16位
m_audioFormat.setCodec("audio/pcm"); // 编码器
m_audioFormat.setByteOrder(QAudioFormat::LittleEndian); // 设定高低位,LittleEndian（低位优先）/LargeEndian(高位优先)
m_audioFormat.setSampleType(QAudioFormat::SignedInt); // 设置样本数据类型
```
## 二、pcm 播放
**ffplay** 播放 `pcm` 数据，ffplay` 是 `ffmpeg` 中的一部分，所以需要先下载 `ffmpeg。

```bash
$ ffplay -ar 44100 -ac 1 -f s16le -i ./xxx.pcm

`-ar 表示采样率`

`-ac 表示音频通道数`
	`单声道是 1，Android 中为 AudioFormat.CHANNEL_IN_MONO`
	`双声道是 2，Android 中为 AudioFormat.CHANNEL_IN_STEREO`
	
`-f 表示 pcm 格式，sample_fmts + le(小端)或者 be(大端)`
	`sample_fmts可以通过ffplay -sample_fmts来查询`

`-i 表示输入文件，这里就是 pcm 文件`


```


## 三、pcm和wav格式互转

### 2.1 pcm转换成wav

前提是知道pcm的采样率、通道数、数据格式

```bash
$ ffmpeg -f s16le -ar 16000 -ac 1 -i input.pcm output.wav

s16le: 数据格式，有符号、整形、小端
```



### 2.2 wav转成pcm

```bash
$ ffmpeg -i input.wav -f s16le ouput.pcm
```


