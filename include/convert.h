// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_CONVERT_H
#define DARK_SIDE_CONVERT_H

#include <string>
#include <vector>

std::vector<uint8_t> num_to_bytes(size_t counter);

size_t bytes_to_num(const std::vector<uint8_t>& bytes);

std::vector<uint8_t> str_to_bytes(const std::string& input);

std::string bytes_to_string(const std::vector<uint8_t>& input);

std::vector<uint8_t> operator+(const std::vector<uint8_t>& a,
                               const std::vector<uint8_t> b);

#endif  // DARK_SIDE_CONVERT_H
