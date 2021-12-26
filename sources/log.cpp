// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "log.h"

void log_init() {
  size_t rotation = 1;
  boost::log::add_file_log(
      boost::log::keywords::file_name = boost::filesystem::path(
          "/home/demon/CLionProjects/dark_side/data/info.log"),

      boost::log::keywords::rotation_size = rotation * 1024 * 1024,
      boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%");

  boost::log::add_common_attributes();
}
