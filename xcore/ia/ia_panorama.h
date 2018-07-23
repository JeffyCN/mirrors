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

#ifndef _IA_PANORAMA_H_
#define _IA_PANORAMA_H_

/** @file ia_panorama.h
 * This file declares structures and APIs of panorama engines.
 */

#include "ia_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Panorama state data.
 * This structure is used to hold output from function calls and also works as a handle of the engine instance.
 */
typedef struct {
        ia_frame *output_image;                  /**<  Holds result image of panorama stitching. Memory is allocated by the callee. */
        int32_t   direction;                     /**<  Direction of the camera movement. (Up/Down/Left/Right) Enums will be in ia_types.h */
        int32_t   horizontal_displacement;       /**<  Horizontal displacement in pixels of the camera between current and previous frames. */
        int32_t   vertical_displacement;         /**<  Vertical displacement in pixels of the camera between current and previous frames. */
} ia_panorama_state;

/** @brief Initialize panorama engine.
 *
 *  @param env [IN] platform environment parameters.
 *  @return panorama state data which will be used to hold output of each function call.
 *
 * This function instantiates and initializes the panorama engine and returns internally allocated panorama state data.
 */
ia_panorama_state *
ia_panorama_init(const ia_env *env);

/** @brief Destroy panorama engine.
 *
 *  @param state [IN] panorama state data which was created by calling ia_panorama_init().
 *
 * This function destroys engine instance and frees all internal memory.
 */
void
ia_panorama_uninit(ia_panorama_state *state);

/** @brief Initialize panorama engine.
 *
 *  @param state [IN] panorama state data.
 *
 * Initialize panorama engine to the initial state. All memory blocks internally allocated (including output_image) are freed.
 */
void
ia_panorama_reinit(ia_panorama_state *state);

/** @brief Get version information.
 */
ia_version
ia_panorama_get_version(void);

/** @brief Detect overlap (movement of the camera) between the base frame and the current frame.
 *
 *  @param state [IN] panorama state data.
 *  @param frame [IN] new(current) frame.
 *  @return capture trigger flag. If 1 is returned, it indicates that it is time to capture and call ia_panorama_stitch() with the captured frame. Otherwise returns 0.
 *
 * Detect the amount of camera movement in pixels by measuring overlap region between the base image and the current input frame,
 * where the base image is the latest image passed to ia_panorama_stitch().
 * The x and y component of movement vector are stored into state->horizontal_displacement and state->vertical_displacement respectively.
 */
int32_t
ia_panorama_detect_overlap(ia_panorama_state *state, const ia_frame *frame);

/** @brief Prepare stitching.
 *
 *  @param state [IN] panorama state data.
 *  @return stitch ID to synchronize with ia_panorama_stitch() function.
 *
 * This function will clear the capture trigger flag and the motion vector which are returned by ia_panorama_detect_overlap() function.
 * It is strongly recommended to call this function in the same thread where ia_panorama_detect_overlap() is called.
 */
int32_t
ia_panorama_prepare_stitch(ia_panorama_state *state);

/** @brief Stitch an input frame to the end of current panorama image which is currently being processed.
 *
 *  @param state [IN] panorama state data.
 *  @param frame [IN] new frame to stitch.
 *  @param stitch_id [IN] stitch ID to synchronize with ia_panorama_prepare_stitch().
 *  @return on success, returns number of photos which have been stitched so far. Returns negative value on failure.
 *
 * This function incrementally stitches one additional frame to the end of current panorama image.
 * If this function is called for the first time after the engine is initialized (by calling init or reinit), it effectively starts a panorama capture.
 * This function returns after the input frame is processed, so the caller is free to discard the frame after the call.
 */
int32_t
ia_panorama_stitch(ia_panorama_state *state, const ia_frame *frame, int32_t stitch_id);

/** @brief Cancel current panorama capture.
 *
 *  @param state [IN] panorama state data.
 *
 * Cancel current panorama capture which was not finalized. It is recommended to call ia_panorama_reinit() before starting a new panorama capture
 * if the previous panorama capture is canceled.
 */
void
ia_panorama_cancel_stitching(ia_panorama_state *state);

/** @brief Check current stitch status.
 *
 *  @param state [IN] panorama state data.
 *  @return number of pending stitches. If positive value is returned, it means that some of previously called ia_panorama_stitch() haven't finished yet.
 *          If 0 is returned, we are safe to call ia_panorama_finalize(). Nagative value can be returned on an unexpected error.
 *
 * It is highly recommended to call this function and check if the return value is 0 before calling ia_panorama_finalize().
 * If there is any pending ia_panorama_stitch() calls, ia_panorama_finalize() may be blocked or return error.
 * Thus in case of pending stitches, user should wait for the stitch thread to finish.
 */
int32_t
ia_panorama_check_stitch_status(ia_panorama_state *state);

/** @brief Finalize current panorama capture to generate the output image.
 *
 *  @param state [IN] panorama state data.
 *  @return ia_frame result frame. Same as state->output_image.
 *
 * This function finalizes stitching and generates the final panorama image.
 * After this function returns, state->output_image will be available.
 * Users are not allowed to directly free the result image. Please use ia_panorama_uninit() or ia_panorama_reinit()
 * to clean up the memory.
 */
ia_frame *
ia_panorama_finalize(ia_panorama_state *state);

#ifdef __cplusplus
}
#endif

#endif /* _IA_PANORAMA_H_ */
