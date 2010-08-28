/*  Copyright (C) 2003 Aleksey Krivoshey <voodoo@foss.kharkov.ua>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef DOTCONFPP_H
#define DOTCONFPP_H

#include <list>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#define PATH_MAX _MAX_PATH
#ifndef snprintf
#define snprintf sprintf_s
#endif
#define strcasecmp _stricmp
#define realpath(path,resolved_path) _fullpath(resolved_path, path, _MAX_PATH)
#include <io.h>
#else
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <strings.h>
#endif

#include "mempool.h"

class DOTCONFDocument;

class DOTCONFDocumentNode
{
friend class DOTCONFDocument;
private:
    DOTCONFDocumentNode * previousNode;
    DOTCONFDocumentNode * nextNode;
    DOTCONFDocumentNode * parentNode;
    DOTCONFDocumentNode * childNode;
    char ** values;
    int valuesCount;
    char * name;
    const DOTCONFDocument * document;
    int lineNum;
    char * fileName;
    bool closed;

    void pushValue(char * _value);

public:
    DOTCONFDocumentNode();
    ~DOTCONFDocumentNode();

    const char * getConfigurationFileName()const { return fileName; }
    int getConfigurationLineNumber() const { return lineNum; }

    const DOTCONFDocumentNode * getNextNode() const { return nextNode; }
    const DOTCONFDocumentNode * getPreviuosNode() const { return previousNode; }
    const DOTCONFDocumentNode * getParentNode() const { return parentNode; }
    const DOTCONFDocumentNode * getChildNode() const { return childNode; }
    const char* getValue(int index = 0) const;
    const char * getName() const { return name; }
    const DOTCONFDocument * getDocument() const { return document; }
};

class DOTCONFDocument
{
public:
    enum CaseSensitive { CASESENSETIVE, CASEINSENSETIVE };
protected:
    AsyncDNSMemPool * mempool;
private:
    DOTCONFDocumentNode * curParent;
    DOTCONFDocumentNode * curPrev;
    int curLine;
    bool quoted;
    std::list<DOTCONFDocumentNode*> nodeTree;
    std::list<char*> requiredOptions;
    std::list<char*> processedFiles;
    FILE * file;
    char * fileName;
    std::list<char*> words;
    int (*cmp_func)(const char *, const char *);

    int checkRequiredOptions();
    int parseLine();
    int parseFile(DOTCONFDocumentNode * _parent = NULL);
    int checkConfig(const std::list<DOTCONFDocumentNode*>::iterator & from);
    int cleanupLine(char * line);
    char * getSubstitution(char * macro, int lineNum);
    int macroSubstitute(DOTCONFDocumentNode * tagNode, int valueIndex);

protected:
    virtual void error(int lineNum, const char * fileName, const char * fmt, ...);

public:
    DOTCONFDocument(CaseSensitive caseSensitivity = CASESENSETIVE);
    virtual ~DOTCONFDocument();

    int setContent(const char * _fileName);

    void setRequiredOptionNames(const char ** requiredOptionNames); // !TERMINATE ARRAY WITH NULL
    const DOTCONFDocumentNode * getFirstNode() const;
    const DOTCONFDocumentNode * findNode(const char * nodeName, const DOTCONFDocumentNode * parentNode = NULL, const DOTCONFDocumentNode * startNode = NULL) const;
};

#endif
