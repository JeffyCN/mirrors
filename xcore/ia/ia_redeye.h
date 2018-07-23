/*
 * Copyright 2012 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IA_REDEYE_H_
#define _IA_REDEYE_H_

/** @file ia_redeye.h
 * Red-eye removal corrects a red eye in an input image and returns the
 * corrected eye and its location.
*/
#ifdef __cplusplus
extern "C" {
#endif
/*
 * include ia_types for ia_frame and ia_coordinate
*/
#include "ia_types.h"

/** @brief Eye data structure
*/
typedef struct {
  ia_coordinate top_left;           /**< Top-left position of the corrected eye */
  ia_frame      frame;             /**< Eye pixel values */
} ia_redeye_eye;

/** @brief Correct a red eye in an image
 *
 * @param[in]   image           Input image.
 * @param[in]   coord           Position of the approximate center of the eye.
 * @return                      Output Pointer to eye struct.
 *
 * This function runs red eye removal on an input image for one eye whose
 * position is specified by the user. The user-specified position typically
 * indicates the center of the input eye but the algorithm searches in the
 * vicinity of the input position if necessary. The output of this function is
 * a pointer to a struct that contains the corrected or the NULL pointer in
 * case no red eye was found at the specified position.
 */
ia_redeye_eye *
ia_redeye_correct(const ia_frame *image, const ia_coordinate *coord);

/** @brief Deallocate memory used to store the corrected eye data
 *
 * @param[in]   eye             Pointer to an eye struct.
  * Helper function to correctly free eye struct.
*/
void
ia_redeye_free(ia_redeye_eye *eye);

#ifdef __cplusplus
}
#endif

#endif /* _IA_REDEYE_H_ */
