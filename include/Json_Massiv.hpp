// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_JSON_MASSIV_HPP_
#define INCLUDE_JSON_MASSIV_HPP_

#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

class Json_Massiv {
 public:
  Json_Massiv(const std::string& name_);
  json get_memory() const;
  void read_json();

 private:
  std::string name;
  json memory;
};

#endif  // INCLUDE_JSON_STORAGE_HPP_
