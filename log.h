// Original Author: Albert Zhang<xhzzhang@yeah.net>
// Last Changed: 2019-03-22 14:08:29
#pragma once

#ifdef __cplusplus

#include "glog/logging.h"
#include "glog/raw_logging.h"

// 一些不频繁但有关键的信息，比如说服务器地址和端口（调试信息除外）
#ifndef AINFO
#define AINFO LOG_IF(INFO, FLAGS_v >= google::GLOG_INFO)
#endif // AINFO

// 调试信息
#ifndef ADEBUG
#define ADEBUG AINFO << "[DEBUG] "
#endif // ADEBUG

// 异常信息，但是系统能运行
// 告警日志
#ifndef AWARN
#define AWARN LOG(WARNING)
#endif // AWARN

// 直接影响系统的错误出现
// 故障日志
#ifndef AERROR
#define AERROR LOG(ERROR)
#endif // AERROR

// 程序运行不下去的信息（调用该接口会立即将堆栈信息打印并挂掉）
#ifndef AFATAL
#define AFATAL LOG(FATAL)
#endif // AFATAL

#endif // __cplusplus

#define LOG_RESET   "\033[0m"
#define LOG_BLACK   "\033[30m"      /* Black */
#define LOG_RED     "\033[31m"      /* Red */
#define LOG_GREEN   "\033[32m"      /* Green */
#define LOG_YELLOW  "\033[33m"      /* Yellow */
#define LOG_BLUE    "\033[34m"      /* Blue */
#define LOG_MAGENTA "\033[35m"      /* Magenta */
#define LOG_CYAN    "\033[36m"      /* Cyan */
#define LOG_WHITE   "\033[37m"      /* White */
#define LOG_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define LOG_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define LOG_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define LOG_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define LOG_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define LOG_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define LOG_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define LOG_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


#include <time.h>
#include <sys/time.h>
#define cidi_log(m, x, y, z, a, b) do { \
  long long_tmp_value = y * 1e3; \
  long ms_value = long_tmp_value % (long)1e3; \
  time_t time_secs = (time_t)y; \
  struct tm * p_tm = localtime(&time_secs); \
  printf(LOG_RED "%d-%02d-%02d %02d:%02d:%02d.%03ld %s:%d %d [%s] %s\n" LOG_RESET,\
      p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday, p_tm->tm_hour, \
      p_tm->tm_min, p_tm->tm_sec, ms_value, a, b, x, m, z); \
} while (0)

/// params required: frame_id
/// i.e.: vsion_error << frame->seq << "model not exist" << std::endl;
#define vision_error AERROR << "vision "
#define vision_info AINFO << "vision "

#define lidar_error AERROR << "lidar "
#define lidar_info AINFO << "vision "

#define radar_error AERROR << "radar "
#define radar_info AINFO << "vision "

#define fusion_error AERROR << "fusiion "
#define fusion_info AINFO << "vision "


/*
 * @param
 *    module_name   The tag name of log filename
 *
 */
void init_log(int argc, char *argv[], char *module_name);
