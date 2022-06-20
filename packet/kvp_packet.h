#ifndef KVP_PACKET_H
#define KVP_PACKET_H

#include <unistd.h>

extern "C" {
#include <libavutil/samplefmt.h>
}

#define DEFAULT_SRC_NB_SAMPLES 1024

class KVP_Packet
{
public:
    static KVP_Packet *alloc_packet();
    static int init_packet_src(KVP_Packet **_pkt, int _src_nb_channels, AVSampleFormat _src_sample_fmt, int _src_nb_samples = DEFAULT_SRC_NB_SAMPLES);
    static int init_packet_dst(KVP_Packet **_pkt, int _src_nb_channels, AVSampleFormat _src_sample_fmt, int _src_nb_samples = DEFAULT_SRC_NB_SAMPLES);
    static void free_pakcet(KVP_Packet **_pkt);

    KVP_Packet();
    ~KVP_Packet();

    __uint8_t **src_data;
    __uint8_t **dst_data;

    int data_element_size;
    int data_element_number;

    bool is_init_src,
         is_init_dst,
         is_resample;
    int src_nb_channels,
        dst_nb_channels,
        src_sample_rate,
        dst_sample_rate,
        src_bufsize,
        dst_bufsize,
        src_linesize,
        dst_linesize,
        src_nb_samples,
        dst_nb_samples,
        max_dst_nb_samples;

    __int64_t src_ch_layout,
              dst_ch_layout;

    AVSampleFormat src_sample_fmt,
                   dst_sample_fmt;
};

#endif // KVP_PACKET_H
