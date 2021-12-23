// Copyright 2021 Demon1747 Specdok117@mail.ru

#ifndef DARK_SIDE_DATABASE_REDACTOR_H
#define DARK_SIDE_DATABASE_REDACTOR_H

#include <string>

#include "database.h"

void database_redactor(const std::string& db_path);
void add_user(Database& db);
void delete_user(Database& db);

#endif  // DARK_SIDE_DATABASE_REDACTOR_H
