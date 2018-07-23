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
 * \file ia_emd_types.h
 * \brief Enumerations, structures and definitions used in the Embedded Data decoder.
*/

#ifndef _EMD_TYPES_H_
#define _EMD_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  Revision of Sensor Data Decoder library, format 0xYYMMDDVV, where:
 * - YY: year,
 * - MM: month,
 * - DD: day,
 * - VV: version ('01','02' etc.)
 */
#define IA_SENSOR_DATA_DECODER_REVISION          0x14061603

#define MAX_NUMBER_OF_BLOCK_IDS                  0xF00

/* CMC Comment */
#define AIQB_EMD_RECORD_1_ID                     1
#define AIQB_EMD_RECORD_1_VERSION                100

/* CMC Embedded Data */
#define AIQB_EMD_RECORD_21_ID                    21
#define AIQB_EMD_RECORD_21_VERSION               100
#define AIQB_EMD_RECORD_21_VERSION_101           101


/*!
 * \brief Elementary embedded data block IDs.
 */
typedef enum
{
    /* Parameters in sensor units */
    fine_int_time_sensor_units = 0,                 /*!<  0: Fine integration time in sensor units. */
    coarse_int_time_sensor_units,                   /*!<  1: Coarse integration time in sensor units. */
    a_gain_global_sensor_units,                     /*!<  2: Analog gain global in sensor units. */
    d_gain_global_sensor_units,                     /*!<  3: Digital gain global in sensor units. */
    d_gain_R_sensor_units,                          /*!<  4: Digital gain R in sensor units. */
    d_gain_GR_sensor_units,                         /*!<  5: Digital gain GR in sensor units. */
    d_gain_GB_sensor_units,                         /*!<  6: Digital gain GB in sensor units. */
    d_gain_B_sensor_units,                          /*!<  7: Digital gain B in sensor units. */

    /* Parameters in generic units */
    fine_int_time_generic_units,                    /*!<  8: Fine integration time in generic_units. */
    coarse_int_time_generic_units,                  /*!<  9: Coarse integration time in generic_units. */
    a_gain_global_generic_units,                    /*!< 10: Analog gain global in generic_units. */
    d_gain_global_generic_units,                    /*!< 11: Digital gain global in generic_units. */
    d_gain_R_generic_units,                         /*!< 12: Digital gain R in generic_units. */
    d_gain_GR_generic_units,                        /*!< 13: Digital gain GR in generic_units. */
    d_gain_GB_generic_units,                        /*!< 14: Digital gain GB in generic_units. */
    d_gain_B_generic_units,                         /*!< 15: Digital gain GB in generic_units. */

    /* Additional parameters */
    analog_gain_apex_format,                        /*!< 16: Analog gain in APEX format. */
    digital_gain_apex_format,                       /*!< 17: Digital gain in APEX format. */
    total_gain_apex_format,                         /*!< 18: Total gain in APEX format. */
    exposure_time_us,                               /*!< 19: Exposure time in us. */
    iso_speed,                                      /*!< 20: ISO speed value. */
    f_number_apex,                                  /*!< 21: F-number in APEX format. */
    face_count,                                     /*!< 22: Number of faces recognized in the image. */
    face_largest_index,                             /*!< 23: Index of the largest face in a face array. */
    face_top_left_corner_x,                         /*!< 24: Top-left corner of face region, X coordinates of faces recognized in the image. */
    face_top_left_corner_y,                         /*!< 25: Top-left corner of face region, Y coordinates of faces recognized in the image. */
    face_bottom_right_x,                            /*!< 26: Bottom-right corner of face region, X coordinates of faces recognized in the image. */
    face_bottom_right_y,                            /*!< 27: Bottom-right corner of face region, Y coordinates of faces recognized in the image. */

    /* Misc. parameters */
    frame_length,                                   /*!< 28: Total line number per one frame. */
    line_length,                                    /*!< 29: Total pixel number per one line. */
    frame_counter,                                  /*!< 30: Frame counter. */
    x_output_size,                                  /*!< 31: Horizontal output width. */
    y_output_size,                                  /*!< 32: Vertical output height. */

    /* Keep it as last one */
    num_of_embedded_data_block_ids                  /*!< Number of Embedded Data block IDs. */
} ia_emd_block_id_t;


/*!
 * \brief Elementary Embedded Data block.
 * Each block describes the content of a single elementary data unit, which could be provided from different
 * sources, e.g.:
 * - RAW data embedded lines;
 * - External ISP embedded data block;
 * - Any other type of sources.
 */
typedef struct
{
    uint32_t offset;                                /*!< Offset from beginning of data source blob in bytes. */
    uint16_t total_blocks;                          /*!< Total number of Elementary Data blocks in a value. */
    uint16_t block_number;                          /*!< Number of the current block in a series. */
    uint8_t  data_src;                              /*!< Value from emd_source_t enumeration. */
    uint8_t  block_id;                              /*!< Value from emd_block_id_t enumeration. */
    uint8_t  bit_mask;                              /*!< Bit mask, applyed to the byte (before applying the shift). */
    int8_t   byte_shift_value;                      /*!< Shift value (bits) applyed to byte. E.g. -2 is (N >> 2), 3 is (N << 3). */
    uint8_t  data_format;                           /*!< Format of the data (block describes), value from ia_mkn_dfid enumeration. */
    uint8_t  msb_num;                               /*!< Most significant bit of the block (e.g 11 if [11:8]), if applicable. */
    uint8_t  lsb_num;                               /*!< Least significant bit of the block (e.g 8 if [11:8]), if applicable. */
    uint8_t  embedded_line;                         /*!< Embedded line #, where data block is located. */
} ia_emd_config_block_t;


/*!
 * \brief Embedded Data Decoder Configuration structure (AIQB/CPF data).
 * A complete configuration of data decoder.
 */
typedef struct
{
    uint16_t total_num_of_cfg_blocks;               /*!< Total number of config blocks (config_data_block_t blocks[]). */
    uint16_t total_num_of_data_blocks;              /*!< Total number of decoded data blocks (decoded_emd_block_t blocks[]). */
    uint8_t num_of_exposure_sets;                   /*!< Number of exposure sets in sensor embedded data. */
    uint8_t num_of_color_channels;                  /*!< Number of color channels in sensor embedded data. */
    uint8_t max_num_of_faces;                       /*!< Maximum number of faces in external ISP embedded data. */
    uint8_t reserved;                               /*!< Reserved. */
    /* config_data_block_t blocks[];*/              /*!< Config.blocks array. */
} ia_emd_decoder_config_t;
#define SIZEOF_CONFIG_DATA_BLOCK_T      16          /*!< Size of sensor_embedded_data_block_t structure in bytes. */
#define SIZEOF_EMD_DECODER_CONFIG_T     8           /*!< Size of emd_decoder_config_t data in bytes. */


/*!
 * \brief Describes a run-time configuration of sensor embedded data.
 */
typedef struct
{
    uint32_t exp_id;                                /*!< A unique exposure ID of the frame. */
    int32_t stride;                                 /*!< Stride of embedded data lines. */
    int32_t height;                                 /*!< Num of lines in sensor emb.data, e.g for imx227 it should '2'. */
    int32_t *effective_width;                       /*!< Effective data for each line, e.g for imx227 it should be {160, 62}. */
} ia_emd_mode_t;


/*!
 * \brief Decoded Embedded Data block.
 * Each block contains one decoded embedded data parameter (e.g.: fine integration time, analog gain, etc.), copyied to int64 container.
 */
typedef struct
{
    int64_t data;                                   /*!< Data of the block. */
    uint8_t data_src;                               /*!< Value from emd_source_t enumeration. */
    uint8_t block_id;                               /*!< Value from emd_block_id_t enumeration. */
    uint8_t data_format;                            /*!< Format of the data (block describes), value from ia_mkn_dfid enumeration. */
    uint8_t reserved;                               /*!< Reserved. */
} ia_emd_decoded_block_t;

/*!
 * \brief Decoded Embedded Data block.
 * Each block contains one decoded embedded data parameter (e.g.: fine integration time, analog gain, etc.), copyied to int64 container.
 */
typedef struct
{
    uint32_t frame_counter;                         /*!< Frame counter. */
    uint16_t x_output_size;                         /*!< Horizontal output width. */
    uint16_t y_output_size;                         /*!< Vertical output height. */
} ia_emd_misc_parameters_t;

/*!
 * \brief Decoded Exposure parameters.
 */
typedef struct
{
    ia_aiq_exposure_sensor_parameters *sensor_units_p; /*!< Exposure parameters in terms of sensor units. */
    ia_aiq_exposure_parameters *generic_units_p;    /*!< Exposure parameters in terms of generic units. */
    ia_emd_misc_parameters_t *misc_parameters_p;    /*!< Misc decoded parameters. */
} ia_emd_result_t;


/*!
 * \brief Embedded Data Decoder handle.
 */
typedef struct
{
    uint32_t decoder_revision;                      /*!< Version of decoder: 0xMAjor.MInor. */
    ia_emd_decoder_config_t *decoder_cfg_p;         /*!< Pointer to configuration structure. */
    uint16_t max_block_ID;                          /*!< Maximum block_id value from decoder_cfg_p. */
    uint16_t config_flags;                          /*!< Value from embedded_data_decoder_flags_t enumeration. */
    uint16_t *block_id_mapping;                     /*!< Array[max_block_ID + 1] for mapping of block_id to data blocks array index. */
    ia_emd_result_t decoded_data;                   /*!< Result of EMD decoding. */
    /* ia_emd_decoded_block_t blocks[];*/           /*!< Decoded data blocks array. */
} ia_emd_decoder_t;

#ifdef __cplusplus
}
#endif

#endif /* _EMD_TYPES_H_ */
