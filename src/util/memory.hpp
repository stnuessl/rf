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

#ifndef RF_MEMORY_HPP_
#define RF_MEMORY_HPP_

#if __cplusplus <= 201103L

#include <memory>
#include <utility>

#include <llvm/Support/MemoryBuffer.h>

#include <util/CommandLine.hpp>

namespace std {
    
template <typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#endif

namespace util {
namespace memory {
    
std::unique_ptr<llvm::MemoryBuffer> 
createMemoryBufferFromFile(const llvm::StringRef Path);

}
}

#endif /* RF_MEMORY_HPP_ */
