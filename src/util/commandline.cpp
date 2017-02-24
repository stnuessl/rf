/*
 * Copyright (C) 2016  Steffen Nüssle
 * rf - refactor
 *
 * This file is part of rf.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <util/commandline.hpp>

#ifdef __unix__

/* clang-format off */

#define TERMINAL_COLOR_RED      "\x1B[1;31m"
#define TERMINAL_COLOR_GREEN    "\x1B[1;32m"
#define TERMINAL_COLOR_YELLOW   "\x1B[1;33m"
#define TERMINAL_COLOR_MAGENTA  "\x1B[1;35m"
#define TERMINAL_COLOR_DEFAULT  "\x1B[0m"

/* clang-format on */

#endif

namespace util {
namespace cl {

std::ostream &operator<<(std::ostream &OS, const Error &Err)
{
    (void) Err;

#ifdef __unix__
    OS << TERMINAL_COLOR_RED << "** ERROR: " << TERMINAL_COLOR_DEFAULT;
#else
    OS << "** ERROR: ";
#endif

    return OS;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Error &Err)
{
    (void) Err;

    OS.changeColor(llvm::raw_ostream::RED, true);
    OS << "** ERROR: ";
    OS.resetColor();

    return OS;
}

std::ostream &operator<<(std::ostream &OS, const Info &Info)
{
    (void) Info;

#ifdef __unix__
    OS << TERMINAL_COLOR_GREEN << "** INFO: " << TERMINAL_COLOR_DEFAULT;
#else
    OS << "** INFO: ";
#endif

    return OS;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Info &Info)
{
    (void) Info;

    OS.changeColor(llvm::raw_ostream::GREEN, true);
    OS << "** INFO: ";
    OS.resetColor();

    return OS;
}

std::ostream &operator<<(std::ostream &OS, const Warning &Warning)
{

    (void) Warning;

#ifdef __unix__
    OS << TERMINAL_COLOR_YELLOW << "** WARNING: " << TERMINAL_COLOR_DEFAULT;
#else
    OS << "** WARNING: ";
#endif

    return OS;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Warning &Warning)
{
    (void) Warning;

    OS.changeColor(llvm::raw_ostream::YELLOW, true);
    OS << "** WARNING: ";
    OS.resetColor();

    return OS;
}
}
}
