// Copyright 2020 Your Name <your_email>

#include "Json_Massiv.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

JsonArray::JsonArray(const std::string& name_) : name(name_) {}
json JsonArray::GetMemory() const { return memory; }
void JsonArray::ReadJson() {
  try {
    std::ifstream in(name);
    in >> memory;
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
  }
}
