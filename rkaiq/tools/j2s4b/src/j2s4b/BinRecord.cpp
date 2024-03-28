#include "BinRecord.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// #define DEBUG

int BinMapLoader::collectBinMap() {
  uint8_t *data = struct_buffer;
  size_t len = buffer_size;
  size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
  size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
  size_t map_index = 0;
  map_index_t *map_addr = NULL;
  block_count = map_len;

  map_addr = (map_index_t *)(data + map_offset);

  block_vec.insert(block_vec.end(), data,
                   data + (uint64_t)map_addr[0].ptr_offset);

  for (map_index = 0; map_index < map_len; map_index++) {
    map_index_t tmap = (map_addr[map_index]);
    map_vec.insert(map_vec.end(), (uint8_t *)&tmap,
                   (uint8_t *)&tmap + sizeof(map_index_t));

    if (buffer_map[(uint64_t)tmap.ptr_offset]) {
#ifdef DEBUG
      printf("skip:[%zu][%lu]!\n", map_index, (uint64_t)tmap.ptr_offset);
#endif
      continue;
    } else {
      uint8_t *block_data = (uint64_t)tmap.ptr_offset + data;
      buffer_map[(uint64_t)tmap.ptr_offset] = block_data;
      uint64_t block_size = (uint64_t)tmap.len;
      if (!block_data || block_size <= 0) {
        printf("reduce error\n");
        exit(0);
      }
      block_vec.insert(block_vec.end(), block_data, block_data + block_size);
    }
  }

  return 0;
}

int BinMapLoader::genBinary(void *buffer, size_t buffer_size) {
  if (buffer_size < block_vec.size() + map_vec.size()) {
    printf("[BIN] buffer too small!\n");
    return -1;
  }

  size_t map_offset = block_vec.size();
  size_t map_len = map_vec.size() / sizeof(map_index_t);
  size_t file_size = (block_vec.size() + map_vec.size() + sizeof(size_t) * 2);

  memcpy(buffer, &block_vec[0], block_vec.size());
  memcpy((uint8_t*)buffer + block_vec.size(), &map_vec[0], map_vec.size());
  memcpy((uint8_t*)buffer + block_vec.size() + map_vec.size(), &map_offset,
         sizeof(size_t));
  memcpy((uint8_t*)buffer + block_vec.size() + map_vec.size() + sizeof(size_t),
         &map_len, sizeof(map_len));

#ifdef DEBUG
  printf("[BIN] file size:%lu, %lu, %lu\n", file_size, block_vec.size(),
         map_vec.size());
#endif

  return file_size;
}

int BinMapLoader::dumpMap() {
  size_t curr_index = 0;

  for (curr_index = 0; curr_index < block_count; curr_index++) {
    map_index_t *temp_item =
        (map_index_t *)&map_vec[curr_index * sizeof(map_index_t)];
#ifdef DEBUG
    printf("[%lu]---dst:%lu-", curr_index, (uint64_t)temp_item->dst_offset);
    printf("ptr:%lu-", (uint64_t)temp_item->ptr_offset);
    printf("len:%lu\n", temp_item->len);
#endif

    dst_map[(uint64_t)temp_item->dst_offset] = (void *)0xfffff;
  }

  return 0;
}

int BinMapLoader::parseBinStructMap(uint8_t *data, size_t len) {
  size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
  size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
  size_t map_index = 0;
  map_index_t *map_addr = NULL;

  map_addr = (map_index_t *)(data + map_offset);

  for (map_index = 0; map_index < map_len; map_index++) {
    map_index_t tmap = (map_addr[map_index]);
    void **dst_obj_addr = (void **)(data + (size_t)tmap.dst_offset);
    *dst_obj_addr = data + (uintptr_t)tmap.ptr_offset;
#ifdef DEBUG
    printf("ori[%lu]---dst:%lu-", map_index, (uint64_t)tmap.dst_offset);
    printf("ptr:%lu-", (uint64_t)tmap.ptr_offset);
    printf("len:%lu\n", tmap.len);
#endif
  }

  return 0;
}

int BinMapLoader::compareBinStruct(map_index_t *binstruct1,
                                   map_index_t *binstruct2) {
  if (!binstruct1 || !binstruct2) {
    return -1;
  }

  if (binstruct1->len != binstruct2->len) {
    return -1;
  }

  if (binstruct1->ptr_offset == binstruct2->ptr_offset) {
    return -1;
  }

  void *first_obj = (uint64_t)binstruct1->ptr_offset + struct_buffer;
  void *second_obj = (uint64_t)binstruct2->ptr_offset + struct_buffer;

  if (0 == memcmp(first_obj, second_obj, binstruct1->len)) {
    same_block += binstruct1->len;
#ifdef DEBUG
    printf("[BIN] found same block:%zu\n", same_block);
#endif
    return 0;
  }

  return -1;
}

void *BinMapLoader::loadWholeFile(const char *fpath, size_t *fsize) {
  struct stat st;
  void *buf;
  int fd;

  if (!fpath || (0 != ::stat(fpath, &st))) {
    printf("load bin file error!\n");
    return NULL;
  }

  fd = open(fpath, O_RDONLY);
  if (fd < 0) {
    printf("failed to open: '%s'\n", fpath);
    return NULL;
  }

  buf = malloc(st.st_size);
  if (!buf) {
    printf("read file oom!\n");
    close(fd);
    return NULL;
  }

  if (read(fd, buf, st.st_size) != st.st_size) {
    printf("failed to read: '%s'\n", fpath);
    free(buf);
    close(fd);
    return NULL;
  }

  *fsize = st.st_size;

  close(fd);

  return buf;
}

int BinMapLoader::saveFile(const char *fpath, void *buf, size_t file_size) {
  FILE *ofp = NULL;

  ofp = fopen(fpath, "wb+");
  if (!ofp) {
    printf("failed to open: '%s'\n", fpath);
    return -1;
  }

  fwrite(buf, 1, file_size, ofp);
  fclose(ofp);

  return 0;
}

int BinMapLoader::loadFile(const char *filename) {
  struct_buffer = (uint8_t *)loadWholeFile(filename, &buffer_size);
  if (!struct_buffer) {
    printf("[BIN] load file faild!\n");
    return -1;
  }

  return 0;
}

int BinMapLoader::suqeezBinMapOne() {
  int curr_index = block_count - 1;

  for (curr_index = block_count - 1; curr_index > 0; curr_index--) {
    map_index_t *map_item =
        (map_index_t *)&map_vec[curr_index * sizeof(map_index_t)];
    map_index_t *pre_item =
        (map_index_t *)&map_vec[(curr_index - 1) * sizeof(map_index_t)];

    // skip the already modfied item
    if (pre_item->ptr_offset >= map_item->ptr_offset) {
      continue;
    }

    if (map_item->len <= sizeof(void*) * 4) {
      continue;
    }

    map_index_t same_item;
    memset(&same_item, 0, sizeof(map_index_t));
    if (0 == findDuplicate(map_item, curr_index, &same_item)) {
#ifdef DEBUG
      printf("[BIN] remove map:[%d]\n", curr_index);
#endif
      removeMap(map_item, curr_index);
      removeBlock(map_item, curr_index, &same_item);
      return 0;
    }
  }

  return -1;
}

int BinMapLoader::deinitBinStructMap(uint8_t *data, size_t len)
{
    size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
    size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
    size_t map_index = 0;
    map_index_t *map_addr = NULL;

    map_addr = (map_index_t *)(data + map_offset);
    for (map_index = 0; map_index < map_len; map_index++) {
        map_index_t tmap = (map_addr[map_index]);
        void** dst_obj_addr = (void**)(data + (size_t)tmap.dst_offset);
        *dst_obj_addr = NULL;
    }

    return 0;
}

int BinMapLoader::suqeezBinMap(uint8_t *buffer, size_t *buffer_len)
{
  int ret = -1;
  uint8_t *inp_buff = NULL;
  uint8_t *out_buff = NULL;
  size_t inp_size = 0;
  size_t final_size = 0;

  if (*buffer_len > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    return -1;
  }

  inp_buff = buffer;
  out_buff = buffer;

  inp_size = *buffer_len;
  do {
    BinMapLoader *loader = new BinMapLoader(inp_buff, inp_size);
    loader->parseBinStructMap(inp_buff, inp_size);
    loader->collectBinMap();
    ret = loader->suqeezBinMapOne();
    memset(out_buff, 0, MAX_IQBIN_SIZE);
    final_size = loader->genBinary(out_buff, MAX_IQBIN_SIZE);
    inp_size = final_size;

    if (ret != 0) {
      loader->deinitBinStructMap(inp_buff, inp_size);
      *buffer_len = final_size;
    }
    delete loader;
    loader = NULL;
  } while (ret == 0);

error:

  return 0;
}

int BinMapLoader::findDuplicate(map_index_t *map_item, size_t map_index,
                                map_index_t *ori_item) {
  size_t curr_index = 0;
  if (map_index == 0) {
    return -1;
  }

  for (curr_index = 0; curr_index < block_count; curr_index++) {
    map_index_t *item =
        (map_index_t *)&map_vec[curr_index * sizeof(map_index_t)];

    if (0 == compareBinStruct(map_item, item)) {
#ifdef DEBUG
      printf("[BIN][%lu]-", curr_index);
      printf("duplicate-dst:%lu-", (uint64_t)item->dst_offset);
      printf("ptr:%lu-", (uint64_t)item->ptr_offset);
      printf("len:%lu\n", item->len);
#endif
      *ori_item = *item;
      return 0;
    }
    curr_index++;
    if (curr_index >= map_index) {
      return -1;
    }
  }

  return -1;
}

int BinMapLoader::removeBlock(map_index_t *map_item, size_t map_index,
                              map_index_t *same_item) {
  int curr_index = 0;
  if (map_index == 0) {
    return -1;
  }

  uint64_t start_addr = (uint64_t)map_item->ptr_offset;
  uint64_t end_addr = (uint64_t)map_item->ptr_offset + map_item->len;

#ifdef DEBUG
  printf("fix ptr after:%lu\n", (uint64_t)map_item->ptr_offset);
#endif

  for (curr_index = 0; curr_index < (int)block_count; curr_index++) {
    map_index_t *temp_item =
        (map_index_t *)&map_vec[curr_index * sizeof(map_index_t)];

    uint64_t curr_dst = (uint64_t)temp_item->dst_offset;
    uint64_t curr_ptr = (uint64_t)temp_item->ptr_offset;
    int64_t d_diff = curr_dst - start_addr;
    int64_t p_diff = curr_ptr - start_addr;

    if (map_item->ptr_offset == temp_item->ptr_offset) {
        if (map_item->dst_offset != temp_item->dst_offset)
            temp_item->ptr_offset = same_item->ptr_offset;
    }

#if 0
    printf("diff is [%ld], [%ld]\n", d_diff, p_diff);
#endif

    if (d_diff > 0) {
      uint64_t new_offset = (uint64_t)temp_item->dst_offset - same_item->len;
      temp_item->dst_offset = (void *)new_offset;
    }

    if (p_diff > 0) {
      uint64_t new_offset = (uint64_t)temp_item->ptr_offset - same_item->len;
      temp_item->ptr_offset = (void *)new_offset;
    }
  }

  map_item->ptr_offset = same_item->ptr_offset;
  map_item->len = 0;
  block_vec.erase(block_vec.begin() + start_addr, block_vec.begin() + end_addr);

  return 0;
}

int BinMapLoader::removeMap(map_index_t *map_item, size_t map_index) {
  size_t curr_index = 0;
  if (map_index == 0) {
    return -1;
  }

  uint64_t start_addr = (uint64_t)map_item->ptr_offset;
  uint64_t end_addr = (uint64_t)map_item->ptr_offset + map_item->len;

  for (curr_index = 0; curr_index < block_count; curr_index++) {
    map_index_t *temp_item =
        (map_index_t *)&map_vec[curr_index * sizeof(map_index_t)];
    if ((uint64_t)temp_item->dst_offset >= start_addr &&
        (uint64_t)temp_item->dst_offset < end_addr) {
#ifdef DEBUG
      printf("[BIN]remove the map->%lu\n", curr_index);
#endif
      map_vec.erase(map_vec.begin() + curr_index * sizeof(map_index_t),
                    map_vec.begin() + (1 + curr_index) * sizeof(map_index_t));
      block_count--;
    }
  }

  return 0;
}

void BinMapLoader::info() {
  printf("block info:[%zu]\n", block_vec.size());
  printf("map info:[%zu]\n", map_vec.size());
}

