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

#ifndef RF_MACROREFACTORER_HPP_
#define RF_MACROREFACTORER_HPP_

#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/MacroInfo.h>

#include <Refactorers/Base/NameRefactorer.hpp>

class MacroRefactorer : public NameRefactorer {
public:
    
    virtual void MacroExpands(const clang::Token &MacroName, 
                              const clang::MacroDefinition &MD,
                              clang::SourceRange Range,
                              const clang::MacroArgs *Args) override;
                              
    virtual void MacroDefined(const clang::Token &MacroName, 
                              const clang::MacroDirective *MD) override;
    
    virtual void MacroUndefined(const clang::Token &MacroName, 
                                const clang::MacroDefinition &MD) override;
    
    virtual void Defined(const clang::Token &MacroName, 
                         const clang::MacroDefinition &MD, 
                         clang::SourceRange Range) override;
        
    virtual void Ifdef(clang::SourceLocation Loc, 
                       const clang::Token &MacroName, 
                       const clang::MacroDefinition &MD) override;
    
    virtual void Ifndef(clang::SourceLocation Loc, 
                        const clang::Token &MacroName, 
                        const clang::MacroDefinition &MD) override;
                                
private:
    void process(const clang::Token &MacroName, 
                 const clang::MacroDefinition &MD);
    
    bool isVictim(const clang::Token &MacroName, 
                  const clang::MacroDefinition &MD);
    bool isVictim(const clang::Token &MacroName, 
                  const clang::MacroDirective *MD);
};

#endif /* RF_MACROREFACTORER_HPP_ */
