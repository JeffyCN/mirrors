/*
* INTEL CONFIDENTIAL
* Copyright (c) 2014 Intel Corporation
* All Rights Reserved.
*
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation or its
* suppliers or licensors. Title to the Material remains with Intel
* Corporation or its suppliers and licensors. The Material may contain trade
* secrets and proprietary and confidential information of Intel Corporation
* and its suppliers and licensors, and is protected by worldwide copyright
* and trade secret laws and treaty provisions. No part of the Material may be
* used, copied, reproduced, modified, published, uploaded, posted,
* transmitted, distributed, or disclosed in any way without Intel's prior
* express written permission.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*
* Unless otherwise agreed by Intel in writing, you may not remove or alter
* this notice or any other notice embedded in Materials by Intel or Intels
* suppliers or licensors in any way.
*/

/*!
 * \file ia_emd_decoder.h
 * \brief Definitions of functions in Embedded Data decoder.
*/

#ifndef _IA_EMD_DECODER_H_
#define _IA_EMD_DECODER_H_

#include "ia_abstraction.h"
#include "ia_aiq_types.h"
#include "ia_emd_types.h"
#include "ia_types.h"
#include "ia_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_EMD
#define IA_EMD_LOG(fmt, ...) IA_LOG(ia_log_debug, "IAEMD: " fmt, ## __VA_ARGS__)
#define IA_EMD_LOG_ERROR(fmt, ...) IA_LOG(ia_log_error, "IAEMD: " fmt, ## __VA_ARGS__)
#define IA_EMD_LOG_INFO(fmt, ...) IA_LOG(ia_log_info, "IAEMD: " fmt, ## __VA_ARGS__)
#else
#define IA_EMD_LOG(...) ((void)0)
#define IA_EMD_LOG_ERROR(...) ((void)0)
#define IA_EMD_LOG_INFO(...) ((void)0)
#endif


/*!
 * \brief Creates Embedded Data Decoder.
 *
 * \param[in] bin_data_decoder_config       Mandatory.\n
 *                                          Pointer to Embedded Data Decoder Configuration data (AIQB/CPF file).
 * \return                                  Pointer to Embedded Data Decoder handle.
 */
LIBEXPORT ia_emd_decoder_t *
ia_emd_decoder_init(
    const ia_binary_data *bin_emd_decoder_config);


/*!
 * \brief Deletes Sensor Data Decoder.
 *
 * \param[in] emd_decoder                   Mandatory. \n
 *                                          Pointer to decoder handle.
 * \return                                  None.
 */
LIBEXPORT void
ia_emd_decoder_deinit(ia_emd_decoder_t *emd_decoder);


/*!
 * \brief Runs Sensor Data Decoder.
 *
 * \param[in] emd_bin                       Mandatory. \n
 *                                          Pointer to sensor embedded data binary blob.
 * \param[in] emd_mode                      Mandatory. \n
 *                                          Pointer to sensor embedded data run-time configuration.
 * \param[in/out] emd_decoder               Mandatory. \n
                                            Pointer to decoder handle. Contains decoded exposure data as well.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_emd_decoder_run(
    const ia_binary_data *emd_bin,
    const ia_emd_mode_t *emd_mode,
    ia_emd_decoder_t *emd_decoder);

#ifdef __cplusplus
}
#endif

#endif /* _IA_EMD_DECODER_H_ */
