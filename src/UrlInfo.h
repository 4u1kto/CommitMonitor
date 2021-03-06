// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007-2013, 2015 - Stefan Kueng

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
#include <vector>

#include "SCCS.h"
#include "SVN.h"
#include "SerializeUtils.h"
#include "ReaderWriterLock.h"

#define URLINFO_VERSION     15
#define URLINFOS_VERSION    1

#define URLINFO_MAXENTRIES 100000

class CUrlInfo
{
public:
    CUrlInfo(void);
    ~CUrlInfo(void);

    typedef enum SCCS_TYPE_ {
      SCCS_SVN = 0,
      SCCS_ACCUREV,
      SCCS_LEN
    } SCCS_TYPE;

    std::wstring                username;
    std::wstring                password;
    bool                        useDefaultAuth;

    SCCS_TYPE                   sccs;
    std::wstring                accurevRepo;
    std::wstring                url;
    std::wstring                name;
    __time64_t                  lastchecked;
    svn_revnum_t                lastcheckedrev;
    svn_revnum_t                startfromrev;       // not saved to disk, only for runtime
    __time64_t                  lastcheckedrobots;

    int                         minutesinterval;
    int                         minminutesinterval;
    bool                        fetchdiffs;
    bool                        disallowdiffs;
    bool                        monitored;
    std::wstring                ignoreUsers;
    std::wstring                includeUsers;
    std::wstring                ignoreCommitLog;

    std::map<svn_revnum_t,SCCSLogEntry> logentries;
    int                         maxentries;

    bool                        parentpath;
    std::wstring                error;
    apr_status_t                errNr;
    std::wstring                callcommand;
    bool                        noexecuteignored;
    std::wstring                webviewer;

    bool                        Save(FILE * hFile);
    bool                        Load(const unsigned char *& buf);
};

class CUrlInfos
{
public:
    CUrlInfos(void);
    ~CUrlInfos(void);

    void                        Save(bool bForce);
    bool                        Load();
    bool                        Save(LPCWSTR filename);
    bool                        Load(LPCWSTR filename);
    bool                        IsEmpty();
    bool                        Export(LPCWSTR filename, LPCWSTR password);
//unused?    bool                        CheckPassword(LPCWSTR filename, LPCWSTR password);
    bool                        Import(LPCWSTR filename, LPCWSTR password);
    void                        UpdateAuth();

    const std::map<std::wstring,CUrlInfo> *   GetReadOnlyData();
    std::map<std::wstring,CUrlInfo> *     GetWriteData();
    void                        ReleaseReadOnlyData();
    void                        ReleaseWriteData();

protected:
    bool                        Save(FILE * hFile);
    bool                        Load(const unsigned char *& buf);
    std::string                 CalcMD5(LPCWSTR s);

private:
    std::map<std::wstring,CUrlInfo> infos;
    CReaderWriterLock               guard;
};
