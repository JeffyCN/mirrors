## Android libcamera_metadata

This is a copy of the libcamera_medatada from Android goog/nyc-mr1-arc branch:

- Copy include/ and src/*.c from android/system/media/camera/
- Copy include/camera_metadata_hidden.h from
  android/system/media/private/camera/include/
- Modify camera_metadata.c to fix compile errors. The diffs can be found in
  src/diff/
