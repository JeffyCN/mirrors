/*
 * Copyright (C) 2015-2017 Intel Corporation
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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
 */

#define LOG_TAG "CameraWindow"

#include <math.h>
#include "CameraWindow.h"
#include <base/log.h>

/******************************************************************************
 * CAMERA WINDOW OPERATIONS
 *****************************************************************************/

/**
 * Default constructor
 */
CameraWindow::CameraWindow() :
    mXleft(0),
    mXright(0),
    mYtop(0),
    mYbottom(0),
    mWeight(0),
    mWidth(0),
    mHeight(0)
{
    CLEAR(mMeteringRectangle);
    CLEAR(mCenter);
}

void CameraWindow::init(ia_coordinate topLeft,ia_coordinate bottomRight, int aWeight)
{
    mXleft = topLeft.x;
    mYtop = topLeft.y;
    mXright = bottomRight.x;
    mYbottom = bottomRight.y;

    mWeight = aWeight;
    mWidth = mXright - mXleft;
    mHeight = mYbottom - mYtop;

    _calculateCenter();

}

void CameraWindow::init(ia_coordinate topLeft,int width, int height, int aWeight)
{
    mXleft = topLeft.x;
    mYtop = topLeft.y;
    mWidth = width;
    mHeight = height;
    mXright = mXleft + mWidth;
    mYbottom = mYtop + mHeight;
    mWeight = aWeight;
    _calculateCenter();

}

void CameraWindow::init(int aWeight)
{
    mWeight = aWeight;

    mXleft = 0;
    mYtop = 0;
    mXright = 0;
    mYbottom = 0;
    mWidth = 0;
    mHeight = 0;
    _calculateCenter();
}

void CameraWindow::_calculateCenter()
{
    mCenter.x =  mXleft + mWidth/2;
    mCenter.y =  mYtop + mHeight/2;
}

/**
 * isValid
 * Check whethe the structure is initialized correctly or not
 * a valid CameraWindow has coordinates such that xmin<xmax and ymin<ymax
 */
bool CameraWindow::isValid()
{
    return mXleft >= 0 && mYtop >= 0 && // top left corner is >= (0,0)
        mXleft < mXright  &&    // bottom right corner point is bigger than
        mYtop < mYbottom;
}

/**
 * clip
 * This method modifies the CameraWindow to fit inside the clipping region.
 * This is effectively an intersection operation
 * It is assumed that both windows are using the same coordinate system
 *
 * Three scenarios are possible
 * -Scenario A:
 *  clipping region does not overlap with this window. In this case the window
 *  is modified to be empty and therefore invalid.
 * - Scenario B:
 *  clipping region partially intersects with this window. In this case the
 *  window will be modified to be the intersection.
 *  - Scenario C: the clipping region is fully contained in this window.
 *   In this case the window will be changed to be the same as the clipping
 *   region
 *
 *   \param clippingRegion [IN]: regions used to intersect with this window
 */
void CameraWindow::clip(CameraWindow &clippingRegion)
{
    Range x_range, y_range;
    Range x_clipping_range, y_clipping_range;
    Range x_intersect_range, y_intersect_range;
    ia_coordinate newTopLeft, newBottomRight;

    INIT_RANGE(x_range, mXleft, mXright);
    INIT_RANGE(y_range, mYtop, mYbottom);

    INIT_RANGE(x_clipping_range, clippingRegion.mXleft, clippingRegion.mXright);
    INIT_RANGE(y_clipping_range, clippingRegion.mYtop, clippingRegion.mYbottom);

    x_intersect_range = _intersect(x_range, x_clipping_range);
    y_intersect_range = _intersect(y_range, y_clipping_range);

    if (x_intersect_range.isEmpty() || y_intersect_range.isEmpty()) {
        //Scenario A
        INIT_COORDINATE(newTopLeft,0, 0);
        INIT_COORDINATE(newBottomRight,0, 0);

    } else {
        //Scenario B and C
        INIT_COORDINATE(newTopLeft, x_intersect_range.start,
                                    y_intersect_range.start);
        INIT_COORDINATE(newBottomRight, x_intersect_range.end,
                                        y_intersect_range.end);

    }

    init(newTopLeft, newBottomRight, mWeight);
    return;
}
/*
 * _intersect
 *
 * calculates the intersection of 2 ranges.
 *
 *    |-----1------|   |------2------|
 * There are 6 possible scenarios that we can group in 3 types
 * Type 1: do not overlap (scenarios 1 and 2)
 *    |-----1------|   |------2------|
 *    |-----2------|   |------1------|
 * Type 2: partial overlap (scenarios 3 and 4)
 *    |-----1------|
 *          |-----2------|
 *    |-----2------|
 *          |-----1------|
 * Type 3: inclusion (scenarios 5 and 6)
 *    |-----1------|
 *       |---2---|
 *    |-----2------|
 *       |---1---|
 *
 * This method returns the range of the intersection in each case
 *
 */
Range CameraWindow::_intersect(Range range1, Range range2)
{
    Range intersect;
    // detect type 1
    if ((range1.start < range2.start && range1.end <= range2.start)  ||
        (range2.start < range1.start && range2.end <= range1.start)) {
        intersect.start = 0;
        intersect.end = 0;
        return intersect;
    }

    // detect type 2 - scenario 3
    if ((range1.start <= range2.start && range1.end <= range2.end)){
        intersect.start = range2.start;
        intersect.end = range1.end;
        return intersect;
    }
    // detect type 2 - scenario 4
    if ((range2.start <= range1.start && range2.end <= range1.end)){
        intersect.start = range1.start;
        intersect.end = range2.end;
        return intersect;
    }
    // detect type 3
    if ((range1.start <= range2.start && range1.end >= range2.end)){
        return range2;
    } else {
        return range1;
    }

}

CameraWindow CameraWindow::scale(float widthRatio, float heightRatio)
{
    CameraWindow ret;
    ia_coordinate topLeft;
    topLeft.x = round(mXleft * widthRatio);
    topLeft.y = round(mYtop * heightRatio);

    int newWidth = round(mWidth*widthRatio);
    int newHeight = round(mHeight*heightRatio);

    ret.init(topLeft,newWidth, newHeight, mWeight);
    return ret;
}

/**
 * meteringRectangle
 *
 * Helper method to use the CameraWindow as a metering rectangle defined
 * like Android metadata type. This is as an array of 5 int in this order
 *  [xmin, ymin, xmax, ymax, weight]
 */
int* CameraWindow::meteringRectangle()
{
    mMeteringRectangle[0] = mXleft;
    mMeteringRectangle[1] = mYtop;
    mMeteringRectangle[2] = mXright;
    mMeteringRectangle[3] = mYbottom;
    mMeteringRectangle[4] = mWeight;
    return mMeteringRectangle;
}
void CameraWindow::dump()
{
    LOGD("Window: TL(%d,%d) BR(%d,%d), width %d height %d, center (%d,%d), weight %d",
            mXleft, mYtop, mXright, mYbottom, mWidth, mHeight,
            mCenter.x, mCenter.y, mWeight);
}
