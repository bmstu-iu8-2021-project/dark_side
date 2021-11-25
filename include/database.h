// Copyright 2021 Demon1747

#ifndef DARK_SIDE_DATABASE_H
#define DARK_SIDE_DATABASE_H

#include <sqlite3.h>

struct User {
  size_t ID = 0;
  std::string status = "slave";
  std::string username = "username";
  size_t password = 1111;
  std::string address = "127.0.0.1";
  size_t port = 8000;
};

class Database {
 public:
  explicit Database(char* fn_in) : filename(fn_in) {
    run_res = sqlite3_open(filename, &db);
    check_err();
  }

  ~Database() { closeDB(); }

  void create_table() {
    command =
        "CREATE TABLE Users ("
        "ID INT PRIMARY KEY NOT NULL,"
        "status TEXT NOT NULL,"
        "username TEXT NOT NULL,"
        "password INT NOT NULL,"
        "address TEXT NOT NULL,"
        "port INT NOT NULL );";

    run_res = sqlite3_exec(db, command, print_callback, nullptr, &err_msg);
    check_err();
  }

  void insert_user(const User& input) {
    char status[input.status.size() + 1];
    strcpy(status, input.status.c_str());
    char username[input.username.size() + 1];
    strcpy(username, input.username.c_str());
    char address[input.address.size() + 1];
    strcpy(address, input.address.c_str());

    asprintf(&command,
             "INSERT INTO Users VALUES "
             "(%zu, '%s', '%s', %zu, '%s', %zu); ",
             input.ID, status, username, input.password, address, input.port);

    run_res = sqlite3_exec(db, command, print_callback, nullptr, &err_msg);
    check_err();
  }

  User extract_user(size_t input_id) {
    User* ex_data = new User;
    asprintf(&command, "SELECT * FROM Users WHERE ID = %zu", input_id);
    run_res =
        sqlite3_exec(db, command, extract_callback, (void*)ex_data, &err_msg);
    check_err();
    User output = {ex_data->ID,       ex_data->username, ex_data->status,
                   ex_data->password, ex_data->address,  ex_data->port};
    delete ex_data;
    return output;
  }

  void delete_user(const User& del) {
    asprintf(&command, "DELETE FROM Users WHERE ID = %zu; ", del.ID);

    run_res = sqlite3_exec(db, command, print_callback, nullptr, &err_msg);
    check_err();
  }

  void print_table() {
    command = "SELECT * FROM Users";
    run_res = sqlite3_exec(db, command, print_callback, nullptr, &err_msg);
    check_err();
  }

  inline void closeDB() { sqlite3_close(db); }

 private:
  static int print_callback(void* not_used, int argc, char* argv[],
                            char* col_name[]) {
    std::string line(80, '-');
    std::cout << line << std::endl;
    for (size_t i = 0; i < argc; ++i) {
      std::cout << col_name[i] << ": " << (argv[i] ? argv[i] : "NULL")
                << std::endl;
    }
    std::cout << line << std::endl;
    return 0;
  }

  static int extract_callback(void* out_user, int argc, char* argv[],
                              char* col_name[]) {
    User* data = (User*)out_user;
    data->ID = std::stoul(argv[0]);
    data->username = argv[1];
    data->status = argv[2];
    data->password = std::stoul(argv[3]);
    data->address = argv[4];
    data->port = std::stoul(argv[5]);
    return 0;
  }

  inline void check_err() noexcept {
    if (run_res) {
      std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
      closeDB();
    }
  }

 private:
  sqlite3* db = nullptr;
  int run_res = 0;
  char* err_msg = nullptr;
  char* command = nullptr;
  char* filename = "users.db";
};

bool log_in(Database& db) {
  size_t input_id;
  std::string input;
  std::cout << "Enter your ID: ";
  std::cin >> input;
  try {
    input_id = std::stoul(input);
  } catch(const std::invalid_argument& e) {
    std::cerr << "U stoopid" << std::endl;
    return false;
  }

  User incoming_user = db.extract_user(input_id);
  if (incoming_user.ID == 0) {
    std::cerr << "Incorrect ID, see you next time!" << std::endl;
    return false;
  }
  std::string input_password;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter your password: ";
    std::cin >> input_password;
    ///Пароль в структуре User будет храниться в хэшированном виде, функция
    ///будет добавлена позже
    if (std::stoul(input_password) == incoming_user.password) {
      std::cout << "Welcome to the club, buddy!" << std::endl;
      std::string line(80, '-');
      std::cout << line << std::endl;
      return true;
    }
    if (i == 2) {
      std::cerr << "Access denied" << std::endl;
      return false;
    }
    std::cout << "Incorrect password, try again!" << std::endl;
  }
}

void add_user(Database& db, const std::string& status) {
  ///Проверяется статус редактора базы данных
  if (status != "admin") {
    std::cerr << "Access denied" << std::endl;
    return;
  }
  User input;
  std::cout << "Enter user ID: ";
  std::cin >> input.ID;
  std::cout << "Enter user status: ";
  std::cin >> input.status;
  std::cout << "Enter username: ";
  std::cin >> input.username;
  std::cout << "Enter user password: ";
  std::cin >> input.password;
  std::cout << "Enter user address: ";
  std::cin >> input.address;
  std::cout << "Enter user port: ";
  std::cin >> input.port;
  db.insert_user(input);
}

#endif  // DARK_SIDE_DATABASE_H
