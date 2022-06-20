#include "kvp_packet.h"
#include <ukui-log4qt.h>

KVP_Packet *KVP_Packet::alloc_packet()
{
    KVP_Packet *pkt = new KVP_Packet;
    return pkt;

    pkt->max_dst_nb_samples = pkt->dst_nb_samples =
        av_rescale_rnd(1024, (int64_t)pkt->dst_sample_rate, (int64_t)pkt->src_sample_rate, AV_ROUND_UP);
}

int KVP_Packet::init_packet_src(KVP_Packet **_pkt, int _src_nb_channels, AVSampleFormat _src_sample_fmt, int _src_nb_samples)
{
    KVP_Packet *pkt = *_pkt;
    if (!pkt)
        return -1;

    pkt->is_resample = false;
    pkt->src_sample_fmt = _src_sample_fmt;
    pkt->src_nb_samples = _src_nb_samples;
    pkt->src_bufsize = av_samples_alloc_array_and_samples(&pkt->src_data, &pkt->src_linesize, _src_nb_channels,
                                                          _src_nb_samples, _src_sample_fmt, 0);
    if (pkt->src_bufsize < 0) {
        KyWarning() << "Could not allocate source samples";
        free_pakcet(&pkt);
        return -1;
    }
    pkt->is_init_src = true;
    return pkt->src_bufsize;
}

void KVP_Packet::free_pakcet(KVP_Packet **_pkt)
{
    KVP_Packet *pkt = *_pkt;
    if (!pkt)
        return;
    pkt->is_resample = false;
    pkt->src_sample_fmt = AV_SAMPLE_FMT_NONE;
    pkt->src_nb_samples = 0;
    pkt->src_bufsize = 0;

    if (pkt->src_data)
        av_freep(&pkt->src_data[0]);
    av_freep(&pkt->src_data);

    if (pkt->dst_data)
        av_freep(&pkt->dst_data[0]);
    av_freep(&pkt->dst_data);
}

KVP_Packet::KVP_Packet()
{
    is_resample = false;
}

KVP_Packet::~KVP_Packet()
{
    
}
