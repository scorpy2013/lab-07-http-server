// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_SUGGESTIONS_HPP_
#define INCLUDE_SUGGESTIONS_HPP_
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Suggestions {
 public:
  Suggestions();
  void Update(json Json);
  json DoSuggest(const std::string& str);

 private:
  json suggestions_;
};

#endif  // INCLUDE_SUGGESTIONS_HPP_
