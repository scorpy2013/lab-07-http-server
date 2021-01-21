// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_JSONARRAY_HPP_
#define INCLUDE_JSONARRAY_HPP_

#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

class JsonArray {
 public:
  explicit JsonArray(const std::string& name_);
  json GetMemory() const;
  void ReadJson();

 private:
  std::string name;
  json memory;
};

#endif  // INCLUDE_JSONARRAY_HPP_
