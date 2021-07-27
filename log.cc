#include "log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <stdio.h>

void set_allow_info_log(bool value)
{
  if (value) {
    if (FLAGS_v != google::INFO) {
      FLAGS_v = google::INFO;
      FLAGS_logbuflevel = google::INFO;
      FLAGS_minloglevel = google::INFO;
      AINFO << "glog change to INFO" << std::endl;
    }
  }
  else {
    if (FLAGS_v != google::GLOG_ERROR) {
      AINFO << "glog change to ERROR" << std::endl;
      FLAGS_v = google::GLOG_ERROR;
      FLAGS_logbuflevel = google::ERROR;
      FLAGS_minloglevel = google::ERROR;
    }
  }
}

static void* log_handler(void *p_param)
{
  while (1) {
    if (!access("/tmp/glog_info", F_OK))
      set_allow_info_log(true);
    else
      set_allow_info_log(false);
    sleep(1);
  }
  return NULL;
}

static void start_log_listen(void)
{
  pthread_t id;
  pthread_create(&id, NULL, log_handler, NULL);
  pthread_detach(id);
}

static void signal_handler(const char* data, int size)
{
  FILE *fp = fopen("core_dump.log", "w");
  if (fp) {
    fwrite(data, 1, size, fp);
    fclose(fp);
  }
  AERROR << data << std::endl;
}


void init_log(int argc, char *argv[], char *module_name)
{
  google::InitGoogleLogging(argv[0]);
  //google::ParseCommandLineFlags(&argc, &argv, true);

  std::string home_dir = "./log/";
  if (access(home_dir.c_str(), F_OK) != 0)
    mkdir(home_dir.c_str(), 0755);

  std::string info_log = home_dir + module_name + "_info_";
  google::SetLogDestination(google::INFO, info_log.c_str());

  //std::string warn_log = home_dir + module_nameg + "_warn_";
  //google::SetLogDestination(google::WARNING, warn_log.c_str());

  std::string error_log = home_dir + module_name + "_error_";
  google::SetLogDestination(google::ERROR, error_log.c_str());

  std::string fatal_log = home_dir + module_name + "_fatal_";
  google::SetLogDestination(google::FATAL, fatal_log.c_str());

  //google::InstallFailureSignalHandler();
  //google::InstallFailureWriter(&signal_handler);

  // Real-time output
  FLAGS_logbufsecs = 0;
#ifdef DISABLE_LOG
  // Will output all message when more than WARN
  google::SetStderrLogging(google::ERROR);
  FLAGS_v = google::GLOG_ERROR;
  FLAGS_logbuflevel = google::ERROR;
  FLAGS_minloglevel = google::ERROR;

  start_log_listen();
#else
  google::SetStderrLogging(google::INFO);
#endif
}
