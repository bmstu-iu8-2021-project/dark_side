// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_ENCRYPTION_H
#define DARK_SIDE_ENCRYPTION_H

#include "botan/botan.h"
#include "botan/hash.h"
#include "botan/hex.h"

std::string hash(std::string pw, const std::string& salt) {
  pw += salt;

  std::unique_ptr<Botan::HashFunction> hash_f(
      Botan::HashFunction::create("SHA-256"));
  hash_f->update(pw);

  return Botan::hex_encode(hash_f->final());
}

#endif  // DARK_SIDE_ENCRYPTION_H
