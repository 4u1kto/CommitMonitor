// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007, 2012 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once
#include <string>


/**
 * class that maps arbitrary files into memory.
 */
class CMappedInFile
{
public:

    CMappedInFile (const std::wstring& fileName);
    virtual ~CMappedInFile();

    const unsigned char* GetBuffer() const;
    size_t GetSize() const;

private:
    HANDLE file;
    HANDLE mapping;
    const unsigned char* buffer;
    size_t size;
    void MapToMemory (const std::wstring& fileName);
    void UnMap();
};

inline const unsigned char* CMappedInFile::GetBuffer() const
{
    return buffer;
}

inline size_t CMappedInFile::GetSize() const
{
    return size;
}
