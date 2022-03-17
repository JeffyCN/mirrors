#include "include/iqconverter.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("usage:\n\t%s input.xml output.json\n\n", argv[0]);
    return 0;
  }

  RkCam::IQConverter converter(argv[1], argv[2]);

  return converter.convert();

}
