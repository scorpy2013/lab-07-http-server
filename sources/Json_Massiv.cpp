// Copyright 2020 Your Name <your_email>

#include "Json_Massiv.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

Json_Massiv::Json_Massiv(const std::string& name_) : name(name_) {}
json Json_Massiv::get_memory() const { return memory; }
void Json_Massiv::read_json() {
  try {
    std::ifstream in(name);
    in >> memory;
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n';
  }
}
