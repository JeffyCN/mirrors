
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "RkAiqCalibDb.h"

using namespace RkCam;
#define CALIBDB_PATH_LEN      256


int main(int argc, char **argv)
{
    char config_file[CALIBDB_PATH_LEN];
    xcam_get_log_level();
    strcpy(config_file, argv[1]);
    RkAiqCalibDb::createCalibDbBinFromXml(config_file);

    return 0;
}
