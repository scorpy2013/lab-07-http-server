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
      json JsonObject;
      JsonObject["text"] = suggestions_[i].at("name");
      JsonObject["position"] = m++;
      JsonFile["suggestions"].push_back(JsonObject);
    }
  }
  return JsonFile;
}
