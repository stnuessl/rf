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

#ifndef RF_COMMANDLINE_HPP_
#define RF_COMMANDLINE_HPP_

#include <llvm/Support/raw_ostream.h>
#include <ostream>

namespace util {
namespace cl {

struct Error {
public:
    friend std::ostream &operator<<(std::ostream &OS, const Error &Err);
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                         const Error &Err);
};

struct Info {
public:
    friend std::ostream &operator<<(std::ostream &OS, const Info &Info);
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                         const Info &Info);
};

struct Warning {
public:
    friend std::ostream &operator<<(std::ostream &OS, const Warning &Warning);
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                         const Warning &Warning);
};
}
}

#endif /* RF_COMMANDLINE_HPP_ */
