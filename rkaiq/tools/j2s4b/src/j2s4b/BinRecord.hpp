#ifndef ___BIN_RECORD_H__
#define ___BIN_RECORD_H__

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#define MAX_IQBIN_SIZE 2048000

typedef struct __map_index {
  void *dst_offset;
  void *ptr_offset;
  size_t len;
} map_index_t;

class BinMapLoader {
public:
  explicit BinMapLoader()
      : struct_buffer(NULL), buffer_size(0), same_block(0){};

  explicit BinMapLoader(uint8_t *data, size_t len)
      : struct_buffer(data), buffer_size(len), same_block(0){};

  ~BinMapLoader() = default;

  static int suqeezBinMap(uint8_t *buffer, size_t *buffer_len);

  int saveFile(const char *fpath, void *buf, size_t file_size);
  int genBinary(void *buffer, size_t buffer_size);
  int collectBinMap();
  int suqeezBinMapOne();
  int findDuplicate(map_index_t *map_item, size_t map_index,
                    map_index_t *ori_item);
  int removeBlock(map_index_t *map_item, size_t map_index,
                  map_index_t *same_item);
  int removeMap(map_index_t *map_item, size_t map_index);
  void info();

  int parseBinStructMap(uint8_t *data, size_t len);
  int compareBinStruct(map_index_t *binstruct1, map_index_t *binstruct2);
  int dumpMap();

  void *loadWholeFile(const char *fpath, size_t *fsize);
  int loadFile(const char *filename);
  int deinitBinStructMap(uint8_t *data, size_t len);

private:
  uint8_t *struct_buffer;
  size_t buffer_size;
  size_t same_block;
  size_t block_count{0};
  std::unordered_map<uint64_t, void *> buffer_map;
  std::unordered_map<uint64_t, void *> dst_map;
  std::vector<uint8_t> block_vec;
  std::vector<uint8_t> map_vec;
};

#endif /*___BIN_RECORD_H__*/

