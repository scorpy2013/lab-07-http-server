// Copyright 2020 Your Name <your_email>

#include "Suggestions.hpp"

#include <algorithm>

Suggestions::Suggestions() { suggestions_ = {}; }
void Suggestions::Update(json JSON) {
  std::sort(JSON.begin(), JSON.end(), [](const json& a, const json& b) -> bool {
    return a.at("cost") < b.at("cost");
  });
  suggestions_ = JSON;
}
json Suggestions::do_suggest(const std::string& str) {
  json JSON_FILE;
  for (size_t i = 0, m = 0; i < suggestions_.size(); ++i) {
    if (str == suggestions_[i].at("id")) {
      json JSON;
      JSON["text"] = suggestions_[i].at("name");
      JSON["position"] = m++;
      JSON_FILE["suggestions"].push_back(JSON);
    }
  }
  return JSON_FILE;
}
