/*
 * x3a_statistics_queue.c - statistics queue
 *
 *  Copyright (c) 2014-2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "x3a_statistics_queue.h"
#include <linux/videodev2.h>
#include <linux/rkisp.h>
#include <math.h>

namespace XCam {

X3aIspStatsData::X3aIspStatsData (struct cifisp_stat_buffer *isp_data, XCam3AStats *data)
    : X3aStatsData (data)
    , _isp_data (isp_data)
{
    XCAM_ASSERT (_isp_data);
}

X3aIspStatsData::~X3aIspStatsData ()
{
    if (_isp_data) {
        xcam_free (_isp_data);
    }
}

bool
X3aIspStatsData::fill_standard_stats ()
{
    XCam3AStats *standard_stats = get_stats ();

    XCAM_ASSERT (_isp_data);
    XCAM_ASSERT (standard_stats);
    XCAM_FAIL_RETURN (
        WARNING,
        _isp_data && standard_stats,
        false,
        "X3aIspStatsData fill standard stats failed with null data allocated");
/*
    _isp_data->params.ae.exp_mean;
    _isp_data->params.ae.bls_val;

    _isp_data->params.awb.awb_mean;
    _isp_data->params.af.window;
*/

    XCamGridStat *standard_data = standard_stats->stats;

    for (uint32_t i = 0; i < CIFISP_AE_MEAN_MAX; ++i)
            standard_data[i].avg_y =_isp_data->params.ae.exp_mean[i];
#if RKISP
    standard_data[0].mean_cr_or_r= _isp_data->params.awb.awb_mean[0].mean_cr_or_r;
    standard_data[0].mean_y_or_g= _isp_data->params.awb.awb_mean[0].mean_y_or_g;
    standard_data[0].mean_cb_or_b = _isp_data->params.awb.awb_mean[0].mean_cb_or_b;
#else
    if(_isp_data->params.awb.awb_mean[0].mean_y != 0) {
        standard_data[0].mean_y_or_g = _isp_data->params.awb.awb_mean[0].mean_y;
        standard_data[0].mean_cb_or_b = _isp_data->params.awb.awb_mean[0].mean_cb;
        standard_data[0].mean_cr_or_r = _isp_data->params.awb.awb_mean[0].mean_cr;
    } else {
        standard_data[0].mean_cr_or_r= _isp_data->params.awb.awb_mean[0].mean_r;
        standard_data[0].mean_y_or_g= _isp_data->params.awb.awb_mean[0].mean_g;
        standard_data[0].mean_cb_or_b = _isp_data->params.awb.awb_mean[0].mean_b;
    }
#endif
    standard_data[0].valid_wb_count = _isp_data->params.awb.awb_mean[0].cnt;

    uint32_t hist_bins = CIFISP_HIST_BIN_N_MAX;
    uint32_t *hist_y = standard_stats->hist_y;

    for (uint32_t i = 0; i < hist_bins; i++) {
        hist_y[i] = _isp_data->params.hist.hist_bins[i];
    }
#if 1
    XCAM_LOG_INFO("> AE Measurement:\n");
    
    for (int i = 0; i < 81; i += 9)
        XCAM_LOG_INFO("> Exposure means %d-%d: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                            i, i+8,
                            _isp_data->params.ae.exp_mean[i],
                            _isp_data->params.ae.exp_mean[i + 1],
                            _isp_data->params.ae.exp_mean[i + 2],
                            _isp_data->params.ae.exp_mean[i + 3],
                            _isp_data->params.ae.exp_mean[i + 4],
                            _isp_data->params.ae.exp_mean[i + 5],
                            _isp_data->params.ae.exp_mean[i + 6],
                            _isp_data->params.ae.exp_mean[i + 7],
                            _isp_data->params.ae.exp_mean[i + 8]);

    XCAM_LOG_INFO("> AWB mean lumin-ycbcr-rgb=[%d-%d-%d]",
        standard_data[0].mean_cr_or_r,
        standard_data[0].mean_cb_or_b,
        standard_data[0].mean_cr_or_r);

    XCAM_LOG_INFO("> AF stats win0[%d-%d], win1[%d-%d], win2[%d-%d]",
        _isp_data->params.af.window[0].lum,
        _isp_data->params.af.window[0].sum,
        _isp_data->params.af.window[1].lum,
        _isp_data->params.af.window[1].sum,
        _isp_data->params.af.window[2].lum,
        _isp_data->params.af.window[2].sum);
#endif

    return true;
}

X3aIspStatistics::X3aIspStatistics (const SmartPtr<X3aIspStatsData> &stats_data)
    : X3aStats (SmartPtr<X3aStatsData> (stats_data))
{
}

X3aIspStatistics::~X3aIspStatistics ()
{
}

void *
X3aIspStatistics::get_isp_stats ()
{
    SmartPtr<X3aIspStatsData> stats = get_buffer_data ().dynamic_cast_ptr<X3aIspStatsData> ();

    XCAM_FAIL_RETURN(
        WARNING,
        stats.ptr(),
        NULL,
        "X3aIspStatistics get_stats failed with NULL");

    return stats->get_isp_stats ();
}

bool
X3aIspStatistics::fill_standard_stats ()
{
    SmartPtr<X3aIspStatsData> stats = get_buffer_data ().dynamic_cast_ptr<X3aIspStatsData> ();

    XCAM_FAIL_RETURN(
        WARNING,
        stats.ptr(),
        false,
        "X3aIspStatistics fill standard stats failed with NULL stats data");

    return stats->fill_standard_stats ();
}

X3aStatisticsQueue::X3aStatisticsQueue()
{
    xcam_mem_clear (_grid_info);
}

X3aStatisticsQueue::~X3aStatisticsQueue()
{
}

void
X3aStatisticsQueue::set_grid_info (const struct rkisp_grid_info &info)
{
}

struct cifisp_stat_buffer *
X3aStatisticsQueue::alloc_isp_statsictics ()
{
    struct cifisp_stat_buffer *stats = xcam_malloc0_type (struct cifisp_stat_buffer);
    XCAM_ASSERT (stats);

    return stats;
}

bool
X3aStatisticsQueue::fixate_video_info (VideoBufferInfo &info)
{
    return X3aStatsPool::fixate_video_info (info);
}

SmartPtr<BufferData>
X3aStatisticsQueue::allocate_data (const VideoBufferInfo &buffer_info)
{
    XCAM_UNUSED (buffer_info);

    XCam3AStats *stats = NULL;
    XCam3AStatsInfo stats_info = get_stats_info ();
    struct cifisp_stat_buffer *isp_stats = alloc_isp_statsictics ();
    /*
     * for rk3326, hist bins num is 32, and mean exp grids num is 81,
     * for rk3399, rk3288, the two is 16 and 25, and we allocate the
     * size as the maximum
     */
    int size = sizeof (XCam3AStats) +
                sizeof (XCamHistogram) * CIFISP_HIST_BIN_N_MAX +
                sizeof (uint32_t) * CIFISP_HIST_BIN_N_MAX +
                sizeof (XCamGridStat) * CIFISP_AE_MEAN_MAX;
    
    stats = (XCam3AStats *) xcam_malloc0 (size);

    XCAM_LOG_DEBUG ("---allocate_data, size: %d, stats(%p), info(%p), rgb(%p) , y(%p), stats(%p)",
        size, stats, &stats->info, &stats->hist_rgb, &stats->hist_y, stats->stats);


    //stats = (XCam3AStats *) xcam_malloc0_type (XCam3AStats);
    XCAM_ASSERT (isp_stats && stats);
    /* stats data memory layout:
     * stats ptr:
     *  XCam3AStats
     *  stats
     *  hist_y stats + (XCamGridStat) * CIFISP_AE_MEAN_MAX
     *  hist_rgb  hist_y + (uint32_t) * CIFISP_HIST_BIN_N_MAX
     */
    stats->info = stats_info;
    stats->hist_y = (uint32_t *) ((char*)(stats->stats) + sizeof (XCamGridStat) * CIFISP_AE_MEAN_MAX);
    stats->hist_rgb = (XCamHistogram *) ((char*)(stats->hist_y) + sizeof (uint32_t) * CIFISP_HIST_BIN_N_MAX);

    return new X3aIspStatsData (isp_stats, stats);
}

SmartPtr<BufferProxy>
X3aStatisticsQueue::create_buffer_from_data (SmartPtr<BufferData> &data)
{
    SmartPtr<X3aIspStatsData> stats_data = data.dynamic_cast_ptr<X3aIspStatsData> ();
    XCAM_ASSERT (stats_data.ptr ());

    return new X3aIspStatistics (stats_data);
}

};
