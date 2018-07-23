

#ifndef __EXT_TYPES_H__
#define __EXT_TYPES_H__

typedef void (*GFunc)(void* data, void* user_data);
typedef int32_t (*GCompareFunc)(const void* a, const void* b);
typedef int32_t (*GCompareDataFunc)(const void* a, const void* b, void* user_data);

#endif /* __EXT_TYPES_H__ */
