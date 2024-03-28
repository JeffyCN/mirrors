#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <thread>
#include <vector>

#include "multiframe_process.h"

#define DUMMY_DATA 0
#define BE_TO_LE 0

void sig_exit(int s)
{
    exit(0);
}

void Read(int fd, uint16_t* buffer, int size)
{
#if DUMMY_DATA
    for (int i = 0; i < (size >> 1); i++)
    {
        buffer[i] = 0xFFF0;
    }
#else
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, size);
#endif

#if BE_TO_LE
    ConverToLE(buffer, size >> 1);
#endif
}

void ProcessMultiFrame(int fd_in, int width, int height, int frame_count, int frame_size, int index, uint32_t* pOut0, uint16_t* pOut1, uint16_t* pIn)
{
    fprintf(stderr, "index      %d\n", index);

    Read(fd_in, pIn, frame_size);
    MultiFrameAddition(pOut0, pIn, width, height);
    fprintf(stderr, "MultiFrameAddition\n");
    DumpRawData(pIn, frame_size, 2);
    DumpRawData32(pOut0, frame_size, 2);
    if (index == (frame_count - 1))
    {
        MultiFrameAverage(pOut0, pOut1, width, height, frame_count);
        fprintf(stderr, "MultiFrameAverage\n");
        DumpRawData(pOut1, frame_size, 2);
        DumpRawData32(pOut0, frame_size, 2);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: ./%s input_file frame_count output_file w h\n", argv[0]);
        return 0;
    }

    std::string input_file = argv[1];
    int frame_count = atoi(argv[2]);
    std::string output_file = argv[3];
    int width = atoi(argv[4]);
    int height = atoi(argv[5]);

    std::string output_file_s = output_file;
    output_file_s.append("_s.raw");
    std::string output_file_m = output_file;
    output_file_m.append("_m.raw");

    fprintf(stderr, "input_file    %s\n", input_file.c_str());
    fprintf(stderr, "frame_count   %d\n", frame_count);
    fprintf(stderr, "output_file_s %s\n", output_file_s.c_str());
    fprintf(stderr, "output_file_m %s\n", output_file_m.c_str());
    fprintf(stderr, "width         %d\n", width);
    fprintf(stderr, "height        %d\n", height);

    signal(SIGINT, sig_exit);

    int fd_in = open(input_file.c_str(), O_RDONLY);
    int fd_out_s = open(output_file_s.c_str(), O_RDWR | O_CREAT | O_SYNC, 0664);
    int fd_out_m = open(output_file_m.c_str(), O_RDWR | O_CREAT | O_SYNC, 0664);

    int one_frame_size = width * height * sizeof(uint16_t);
    uint16_t* one_frame = (uint16_t*)malloc(one_frame_size);
    memset(one_frame, 0, one_frame_size);

    one_frame_size = width * height * sizeof(uint32_t);
    uint32_t* averge_frame0 = (uint32_t*)malloc(one_frame_size);
    memset(averge_frame0, 0, one_frame_size);

    one_frame_size = width * height * sizeof(uint16_t);
    uint16_t* averge_frame1 = (uint16_t*)malloc(one_frame_size);
    memset(averge_frame1, 0, one_frame_size);

    one_frame_size = width * height * sizeof(uint16_t);
    for (int i = 0; i < frame_count; i++)
    {
        ProcessMultiFrame(fd_in, width, height, frame_count, one_frame_size, i, averge_frame0, averge_frame1, one_frame);
    }
#if DUMMY_DATA
    DumpFrameU16(one_frame, width, height);
#endif
    write(fd_out_s, one_frame, one_frame_size);
    DumpRawData(one_frame, one_frame_size, 2);

    ConverToLE(averge_frame1, one_frame_size >> 1);
    write(fd_out_m, averge_frame1, one_frame_size);
    DumpRawData(averge_frame1, one_frame_size, 2);

    free(one_frame);
    free(averge_frame0);
    free(averge_frame1);
    close(fd_out_s);
    close(fd_out_m);
    close(fd_in);
    sync();

    return 0;
}
