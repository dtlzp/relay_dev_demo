
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_dbg.h
 * function: debug dbg define
 */

#ifndef _DT_DBG_H_
#define _DT_DBG_H_

#define _DT_FUNC_   __FUNCTION__
#define _DT_LINE_   __LINE__
#define _DT_FILE_   __FILE__
#define _DT_DATE_   __DATE__
#define _DT_TIME_   __TIME__

#define dt_LOG  1

#if (1 == dt_LOG) /* define dt_LOG */

#define DT_LOG_NONE     0
#define DT_LOG_ERROR    1
#define DT_LOG_WARN     2
#define DT_LOG_INFO     3
#define DT_LOG_DEBUG    4
#define DT_LOG_VERBOSE  5

#define DT_LOG_LEVEL  DT_LOG_INFO

//#define ESP_EARLY_LOGE( tag, format, ... ) ESP_LOG_EARLY_IMPL(tag, format, ESP_LOG_ERROR,   E, ##__VA_ARGS__)

#define DT_LOGN(fmt, ...)
#if(DT_LOG_LEVEL == DT_LOG_ERROR)
  #define DT_XLOGE(fmt, ...) ESP_LOGE(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGW(fmt, ...)
  #define DT_XLOGI(fmt, ...)
  #define DT_XLOGD(fmt, ...)
  #define DT_XLOGV(fmt, ...)

  #define DT_LOGE(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGW(fmt, ...)
  #define DT_LOGI(fmt, ...)
  #define DT_LOGD(fmt, ...)
  #define DT_LOGV(fmt, ...)
  #define DT_LOGI_BUFFER_HEX(buf, buf_len)   ESP_LOG_BUFFER_HEX(TAG, buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#elif(DT_LOG_LEVEL == DT_LOG_WARN)
  #define DT_XLOGE(fmt, ...) ESP_LOGE(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGW(fmt, ...) ESP_LOGW(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGI(fmt, ...)
  #define DT_XLOGD(fmt, ...)
  #define DT_XLOGV(fmt, ...)

  #define DT_LOGE(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGW(fmt, ...)  ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGI(fmt, ...)
  #define DT_LOGD(fmt, ...)
  #define DT_LOGV(fmt, ...)
  #define DT_LOGI_BUFFER_HEX(buf, buf_len)   ESP_LOG_BUFFER_HEX(TAG, buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#elif(DT_LOG_LEVEL == DT_LOG_INFO)
  #define DT_XLOGE(fmt, ...) ESP_LOGE(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGW(fmt, ...) ESP_LOGW(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGI(fmt, ...) ESP_LOGI(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGD(fmt, ...)
  #define DT_XLOGV(fmt, ...)

  #define DT_LOGE(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGW(fmt, ...)  ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGI(fmt, ...)  ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGD(fmt, ...)
  #define DT_LOGV(fmt, ...)
  #define DT_LOGI_BUFFER_HEX(buf, buf_len)   ESP_LOG_BUFFER_HEX(TAG, buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#elif(DT_LOG_LEVEL == DT_LOG_DEBUG)
  #define DT_XLOGE(fmt, ...) ESP_LOGE(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGW(fmt, ...) ESP_LOGW(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGI(fmt, ...) ESP_LOGI(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGD(fmt, ...) ESP_LOGD(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGV(fmt, ...)

  #define DT_LOGE(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGW(fmt, ...)  ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGI(fmt, ...)  ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGD(fmt, ...)  ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGV(fmt, ...)
  #define DT_LOGI_BUFFER_HEX(buf, buf_len)   ESP_LOG_BUFFER_HEX(TAG, buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#elif(DT_LOG_LEVEL == DT_LOG_VERBOSE)
  #define DT_XLOGE(fmt, ...) ESP_LOGE(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGW(fmt, ...) ESP_LOGW(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGI(fmt, ...) ESP_LOGI(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGD(fmt, ...) ESP_LOGD(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)
  #define DT_XLOGV(fmt, ...) ESP_LOGV(TAG, "%s:%d " fmt, _DT_FUNC_, _DT_LINE_, ##__VA_ARGS__)

  #define DT_LOGE(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGW(fmt, ...)  ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGI(fmt, ...)  ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGD(fmt, ...)  ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGV(fmt, ...)  ESP_LOGV(TAG, fmt, ##__VA_ARGS__)
  #define DT_LOGI_BUFFER_HEX(buf, buf_len)   ESP_LOG_BUFFER_HEX(TAG, buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#else
  #define DT_XLOGE(fmt, ...)
  #define DT_XLOGW(fmt, ...)
  #define DT_XLOGI(fmt, ...)
  #define DT_XLOGD(fmt, ...)
  #define DT_XLOGV(fmt, ...)

  #define DT_LOGE(fmt, ...)
  #define DT_LOGW(fmt, ...)
  #define DT_LOGI(fmt, ...)
  #define DT_LOGD(fmt, ...)
  #define DT_LOGV(fmt, ...)

  #define DT_LOGI_BUFFER_HEX(buf, buf_len)
  #define DT_LOGN_BUFFER_HEX(buf, buf_len)
#endif

#else /* else not define dt_LOG */
#define DT_LOGN(fmt, ...)

#define DT_XLOGE(fmt, ...)
#define DT_XLOGW(fmt, ...)
#define DT_XLOGI(fmt, ...)
#define DT_XLOGD(fmt, ...)
#define DT_XLOGV(fmt, ...)

#define DT_LOGE(fmt, ...)
#define DT_LOGW(fmt, ...)
#define DT_LOGI(fmt, ...)
#define DT_LOGD(fmt, ...)
#define DT_LOGV(fmt, ...)

#define DT_LOGI_BUFFER_HEX(buf, buf_len)
#define DT_LOGN_BUFFER_HEX(buf, buf_len)

#endif /* end dt_LOG */

/* LWIP_ASSERT("sys_timeout: timeout != NULL, pool MEMP_SYS_TIMEOUT is empty", timeout != NULL); */

#define dt_PLATFORM_ASSERT(x) do {ets_printf("Assertion \"%s\" failed at line %d at funcation %s in file %s\n", \
                                       x, __LINE__, __FUNCTION__, __FILE__); } while(0)

#ifndef dt_ASSERT
#define dt_ASSERT(message, assertion) do { if (!(assertion)) { \
  dt_PLATFORM_ASSERT(message); }} while(0)
//#else  /* dt_ASSERT */
//#define dt_ASSERT(message, assertion)
#endif /* dt_ASSERT */

#define dt_ASSERTx(x) assert(x)

#endif /* end _DT_DBG_H_ */



