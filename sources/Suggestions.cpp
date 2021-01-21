// Copyright 2020 Your Name <your_email>

#include "Suggestions.hpp"

#include <algorithm>

Suggestions::Suggestions() { suggestions_ = {}; }
void Suggestions::Update(json Json) {
  std::sort(Json.begin(), Json.end(), [](const json& a, const json& b) -> bool {
    return a.at("cost") < b.at("cost");
  });
  suggestions_ = Json;
}
json Suggestions::DoSuggest(const std::string& str) {
  json JsonFile;
  for (size_t i = 0, m = 0; i < suggestions_.size(); ++i) {
    if (str == suggestions_[i].at("id")) {
      json JSON;
      JSON["text"] = suggestions_[i].at("name");
      JSON["position"] = m++;
      JsonFile["suggestions"].push_back(JSON);
    }
  }
  return JsonFile;
}
