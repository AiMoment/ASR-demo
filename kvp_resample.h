#ifndef KVP_RESAMPLE_H
#define KVP_RESAMPLE_H

extern "C" {
#include <libavutil/samplefmt.h>
}

// mic 输入的不用重采样

struct SwrContext;

class KVP_Resample
{
public:
    KVP_Resample();
    ~KVP_Resample();

    int init(int _src_sample_rate,
             int _src_nb_samples,
             int64_t _src_ch_layout,
             AVSampleFormat _src_sample_fmt,
             int _dst_sample_rate,
             int64_t _dst_ch_layout,
             AVSampleFormat _dst_sample_fmt);

    void uninit();
    int resample(uint8_t **_src_data);

private:
    SwrContext *swrCtx;

    uint8_t **src_data,
            **dst_data;

    bool is_initialized;

    int /// 声道数量（输入）
        src_nb_channels,
        /// 声道数量（输出）
        dst_nb_channels,
        /// 采样率（输入）
        src_sample_rate,
        /// 采样率（输出）
        dst_sample_rate,
        src_bufsize,
        dst_bufsize,
        src_linesize,
        dst_linesize,
        /// 样本数量（输入）
        /**
          如果时最后的话可能样本数量达不到数量
        */
        src_nb_samples,
        dst_nb_samples,
        max_dst_nb_samples;

    int64_t src_ch_layout,
            dst_ch_layout;

    AVSampleFormat  src_sample_fmt,
                    dst_sample_fmt;
};

#endif // KVP_RESAMPLE_H
