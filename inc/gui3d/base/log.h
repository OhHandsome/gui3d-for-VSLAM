#pragma once

#ifdef LOGD
#undef LOGD
#endif

#ifdef LOGI
#undef LOGI
#endif

#ifdef LOGW
#undef LOGW
#endif

#ifdef LOGE
#undef LOGE
#endif

#ifndef CLOSE_LOG
#define LOGD(...)  do { printf(__VA_ARGS__);printf("\n");} while(0)
#define LOGI(...)  do { printf("\033[32m"); printf(__VA_ARGS__);printf("\033[0m\n");} while(0)
#define LOGW(...)  do { printf("\033[33m"); printf(__VA_ARGS__);printf("\033[0m\n");} while(0)
#define LOGE(...)  do { printf("\033[31m"); printf("%s:%d", __FILE__, __LINE__); \
                                            printf(__VA_ARGS__);printf("\033[0m\n");} while(0)
#define print(...) do { printf(__VA_ARGS__);} while(0)
#define IO         if(1) std::cout
#else
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define print(...)
#define IO         if(0) std::cout
#endif
