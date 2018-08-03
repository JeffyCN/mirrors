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

#ifndef _CAMERA_WINDOW_H_
#define _CAMERA_WINDOW_H_

/* #include <utils/Errors.h> */
/* #include <stdint.h> */
/* #include "ia_types.h" */
/* #include "FrameInfo.h" */

#define CLEAR(x) memset (&(x), 0, sizeof (x))

/** Coordinate, used in red-eye correction. */
typedef struct {
    int x;
    int y;
} ia_coordinate;

/**
 * \macro INIT_RANGE
 * Initializes the value of a structure of type Range (see CameraWindow.h)
 */
#define INIT_RANGE(x, s, e) (x).start = s; (x).end = e;

/**
 * \macro INIT_COORDINATE
 * Initializes the value of a structure of type ia_coordinate
 *  (see rk_aiq_types.h)
 */
#define INIT_COORDINATE(p,xVal,yVal) (p).x = xVal; (p).y = yVal;



class CameraWindow;

/**
 * \struct Range
 * interval defined by 2 numbers.
 */
typedef struct Range {
    int start;
    int end;
    bool isEmpty() { return (start == end)? true: false; }
} Range;

/**
 * \class CameraWindow
 *  Weighted rectangle in a coordinate system where the top left corner is 0,0
 */
class CameraWindow {
public:
    CameraWindow();

    /**
     * Initializators
     */
    void init(ia_coordinate topLeft,ia_coordinate bottomRight, int aWeight);
    void init(ia_coordinate topLeft,int width, int height, int aWeight);
    void init(int aWeight);
    /**
     * Queries
     */
    bool isValid();
    ia_coordinate center(){ return mCenter; }
    int width() const { return mWidth; }
    int height() const { return mHeight; }
    int left() const { return mXleft; }
    int right() const { return mXright; }
    int top() const { return mYtop; }
    int bottom() const { return mYbottom; }
    int weight() const { return mWeight; }
    int* meteringRectangle();
    /**
     * Operations
     * This methods modify the values of the window
     */
    CameraWindow scale(float widthRatio, float heightRatio);
    // in place modification
    void clip(CameraWindow &clippingRegion);
    /**
     * Debug support
     */
    void dump();

private:
    inline void _calculateCenter();
    Range _intersect(Range range1, Range range2);

private:
    int mXleft;
    int mXright;
    int mYtop;
    int mYbottom;
    int mWeight;

    int mWidth;
    int mHeight;
    ia_coordinate mCenter;
    int mMeteringRectangle[5];
};

#endif // _CAMERA_WINDOW_H_
