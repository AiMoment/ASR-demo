#include "kvp_resample.h"

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

KVP_Resample::KVP_Resample()
{
    uninit();
    is_initialized = false;
}

KVP_Resample::~KVP_Resample()
{

}

int KVP_Resample::init(int _src_sample_rate,
                       int _src_nb_samples,
                       int64_t _src_ch_layout,
                       AVSampleFormat _src_sample_fmt,
                       int _dst_sample_rate,
                       int64_t _dst_ch_layout,
                       AVSampleFormat _dst_sample_fmt)
{
    uninit();
    int ret = -1;
    swrCtx = swr_alloc();
    if (!swrCtx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        ret = AVERROR(ENOMEM);
        return ret;
    }

    src_data = NULL;
    dst_data = NULL;

    src_sample_rate = _src_sample_rate;
    src_nb_samples  = _src_nb_samples;
    src_ch_layout   = _src_ch_layout;
    src_sample_fmt  = _src_sample_fmt;

    dst_sample_rate = _dst_sample_rate;
    dst_ch_layout   = _dst_ch_layout;
    dst_sample_fmt  = _dst_sample_fmt;

    av_opt_set_int(swrCtx, "in_channel_layout",    src_ch_layout,   0);
    av_opt_set_int(swrCtx, "in_sample_rate",       src_sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", src_sample_fmt,  0);

    av_opt_set_int(swrCtx, "out_channel_layout",    dst_ch_layout,  0);
    av_opt_set_int(swrCtx, "out_sample_rate",       dst_sample_rate,0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", dst_sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(swrCtx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return ret;
    }

#if 1
    src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
    src_bufsize = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels,
                                                     src_nb_samples, src_sample_fmt, 0);
    if (src_bufsize < 0) {
        fprintf(stderr, "Could not allocate source samples\n");
        return src_bufsize;
    }

    max_dst_nb_samples = dst_nb_samples =
        av_rescale_rnd(src_nb_samples, dst_sample_rate, src_sample_rate, AV_ROUND_UP);

    dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    dst_bufsize = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
                                                     dst_nb_samples, dst_sample_fmt, 0);
    if (dst_bufsize < 0) {
        fprintf(stderr, "Could not allocate destination samples\n");
        return dst_bufsize;
    }
#endif
    is_initialized = true;
}

void KVP_Resample::uninit()
{
    is_initialized = false;

    src_nb_channels = -1;
    src_sample_rate = -1;
    src_linesize    = -1;
    src_ch_layout   = -1;
    src_sample_fmt  = AV_SAMPLE_FMT_NONE;

    dst_nb_channels = -1;
    dst_sample_rate = -1;
    dst_linesize    = -1;
    dst_ch_layout   = -1;
    dst_sample_fmt  = AV_SAMPLE_FMT_NONE;
    max_dst_nb_samples = -1;

    if (swr_is_initialized(swrCtx)) {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    if (src_data)
        av_freep(&src_data[0]);
    av_freep(&src_data);

    if (dst_data)
        av_freep(&dst_data[0]);
    av_freep(&dst_data);
}

int KVP_Resample::resample(uint8_t **_src_data)
{
    src_data = _src_data;
    int ret = 0;
    if (!is_initialized)
        return 0;

    // 计算转换后的样本数
    dst_nb_samples = av_rescale_rnd(swr_get_delay(swrCtx, src_sample_rate) +
                                    src_nb_samples, dst_sample_rate, src_sample_rate, AV_ROUND_UP);

    if (dst_nb_samples > max_dst_nb_samples) {
        av_freep(&dst_data[0]);
        ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
                               dst_nb_samples, dst_sample_fmt, 1);
        if (ret < 0)
            return 0;
        max_dst_nb_samples = dst_nb_samples;
    }

    // 转换
    ret = swr_convert(swrCtx, dst_data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        return 0;
    }

    // 计算转换后的 buffer 大小
    ret = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                     ret, dst_sample_fmt, 1);
    if (ret < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        return ret;
    }
    return ret;
}
