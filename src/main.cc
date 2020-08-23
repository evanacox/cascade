/*---------------------------------------------------------------------------*
 *
 * Copyright 2020 Evan Cox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *---------------------------------------------------------------------------*
 *
 * main.cc:
 *   Defines the `int main(int, char **)` entry point for the program
 *
 *---------------------------------------------------------------------------*/

#include "cascade/driver.hh"
#include "cascade/util/logging.hh"

int main(int argc, const char **argv) {
  cascade::driver driver(argc, argv);

#ifdef NDEBUG
  try {
#endif
    return driver.run();
#ifdef NDEBUG
  } catch (std::exception &e) {
    using namespace std::literals::string_literals;

    cascade::util::error(
        "internal compiler error: "s + e.what()
        + ". If you see this, please make a bug report immediately with the input that caused it.");
  }
#endif
}