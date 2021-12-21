// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_UI_H
#define DARK_SIDE_UI_H

#include <string>

#include "database.h"

bool database_redactor(const std::string& db_path);
void add_user(Database& db);
void delete_user(Database& db);
bool log_in(Database& db);

#endif  // DARK_SIDE_UI_H
