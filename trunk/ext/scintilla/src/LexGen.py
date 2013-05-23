#!/usr/bin/env python
# LexGen.py - implemented 2002 by Neil Hodgson neilh@scintilla.org
# Released to the public domain.

# Regenerate the Scintilla and SciTE source files that list
# all the lexers and all the properties files.
# Should be run whenever a new lexer is added or removed.
# Requires Python 2.4 or later
# Most files are regenerated in place with templates stored in comments.
# The VS .NET project file is generated into a different file as the
# VS .NET environment will not retain comments when modifying the file.
# The files are copied to a string apart from sections between a
# ++Autogenerated comment and a --Autogenerated comment which is
# generated by the CopyWithInsertion function. After the whole
# string is instantiated, it is compared with the target file and
# if different the file is rewritten.
# Does not regenerate the Visual C++ 6 project files but does the VS .NET
# project file.

import string
import sys
import os
import glob

# EOL constants
CR = "\r"
LF = "\n"
CRLF = "\r\n"
if sys.platform == "win32":
    NATIVE = CRLF
else:
    # Yes, LF is the native EOL even on Mac OS X. CR is just for
    # Mac OS <=9 (a.k.a. "Mac Classic")
    NATIVE = LF

# Automatically generated sections contain start and end comments,
# a definition line and the results.
# The results are replaced by regenerating based on the definition line.
# The definition line is a comment prefix followed by "**".
# If there is a digit after the ** then this indicates which list to use
# and the digit and next character are not part of the definition
# Backslash is used as an escape within the definition line.
# The part between \( and \) is repeated for each item in the list.
# \* is replaced by each list item. \t, and \n are tab and newline.
def CopyWithInsertion(input, commentPrefix, retainDefs, eolType, *lists):
    copying = 1
    listid = 0
    output = []
    for line in input.splitlines(0):
        isStartGenerated = line.startswith(commentPrefix + "++Autogenerated")
        if copying and not isStartGenerated:
            output.append(line)
        if isStartGenerated:
            if retainDefs:
                output.append(line)
            copying = 0
            definition = ""
        elif not copying and line.startswith(commentPrefix + "**"):
            if retainDefs:
                output.append(line)
            definition = line[len(commentPrefix + "**"):]
            if (commentPrefix == "<!--") and (" -->" in definition):
                definition = definition.replace(" -->", "")
            listid = 0
            if definition[0] in string.digits:
                listid = int(definition[:1])
                definition = definition[2:]
            # Hide double slashes as a control character
            definition = definition.replace("\\\\", "\001")
            # Do some normal C style transforms
            definition = definition.replace("\\n", "\n")
            definition = definition.replace("\\t", "\t")
            # Get the doubled backslashes back as single backslashes
            definition = definition.replace("\001", "\\")
            startRepeat = definition.find("\\(")
            endRepeat = definition.find("\\)")
            intro = definition[:startRepeat]
            out = ""
            if intro.endswith("\n"):
                pos = 0
            else:
                pos = len(intro)
            out += intro
            middle = definition[startRepeat+2:endRepeat]
            for i in lists[listid]:
                item = middle.replace("\\*", i)
                if pos and (pos + len(item) >= 80):
                    out += "\\\n"
                    pos = 0
                out += item
                pos += len(item)
                if item.endswith("\n"):
                    pos = 0
            outro = definition[endRepeat+2:]
            out += outro
            out = out.replace("\n", eolType) # correct EOLs in generated content
            output.append(out)
        elif line.startswith(commentPrefix + "--Autogenerated"):
            copying = 1
            if retainDefs:
                output.append(line)
    output = [line.rstrip(" \t") for line in output] # trim trailing whitespace
    return eolType.join(output) + eolType

def UpdateFile(filename, updated):
    """ If the file is different to updated then copy updated
    into the file else leave alone so CVS and make don't treat
    it as modified. """
    try:
        infile = open(filename, "rb")
    except IOError:	# File is not there yet
        out = open(filename, "wb")
        out.write(updated.encode('utf-8'))
        out.close()
        print("New %s" % filename)
        return
    original = infile.read()
    infile.close()
    original = original.decode('utf-8')
    if updated != original:
        os.unlink(filename)
        out = open(filename, "wb")
        out.write(updated.encode('utf-8'))
        out.close()
        print("Changed %s " % filename)
    #~ else:
        #~ print "Unchanged", filename

def Generate(inpath, outpath, commentPrefix, eolType, *lists):
    """Generate 'outpath' from 'inpath'.

        "eolType" indicates the type of EOLs to use in the generated
            file. It should be one of following constants: LF, CRLF,
            CR, or NATIVE.
    """
    #print "generate '%s' -> '%s' (comment prefix: %r, eols: %r)"\
    #      % (inpath, outpath, commentPrefix, eolType)
    try:
        infile = open(inpath, "rb")
    except IOError:
        print("Can not open %s" % inpath)
        return
    original = infile.read()
    infile.close()
    original = original.decode('utf-8')
    updated = CopyWithInsertion(original, commentPrefix,
        inpath == outpath, eolType, *lists)
    UpdateFile(outpath, updated)

def Regenerate(filename, commentPrefix, eolType, *lists):
    """Regenerate the given file.

        "eolType" indicates the type of EOLs to use in the generated
            file. It should be one of following constants: LF, CRLF,
            CR, or NATIVE.
    """
    Generate(filename, filename, commentPrefix, eolType, *lists)

def FindModules(lexFile):
    modules = []
    f = open(lexFile)
    for l in f.readlines():
        if l.startswith("LexerModule"):
            l = l.replace("(", " ")
            modules.append(l.split()[1])
    return modules

# Properties that start with lexer. or fold. are automatically found but there are some
# older properties that don't follow this pattern so must be explicitly listed.
knownIrregularProperties = [
    "fold",
    "styling.within.preprocessor",
    "tab.timmy.whinge.level",
    "asp.default.language",
    "html.tags.case.sensitive",
    "ps.level",
    "ps.tokenize",
    "sql.backslash.escapes",
    "nsis.uservars",
    "nsis.ignorecase"
]

def FindProperties(lexFile):
    properties = {}
    f = open(lexFile)
    for l in f.readlines():
        if ("GetProperty" in l or "DefineProperty" in l) and "\"" in l:
            l = l.strip()
            if not l.startswith("//"):	# Drop comments
                propertyName = l.split("\"")[1]
                if propertyName.lower() == propertyName:
                    # Only allow lower case property names
                    if propertyName in knownIrregularProperties or \
                        propertyName.startswith("fold.") or \
                        propertyName.startswith("lexer."):
                        properties[propertyName] = 1
    return properties

def FindPropertyDocumentation(lexFile):
    documents = {}
    f = open(lexFile)
    name = ""
    for l in f.readlines():
        l = l.strip()
        if "// property " in l:
            propertyName = l.split()[2]
            if propertyName.lower() == propertyName:
                # Only allow lower case property names
                name = propertyName
                documents[name] = ""
        elif "DefineProperty" in l and "\"" in l:
            propertyName = l.split("\"")[1]
            if propertyName.lower() == propertyName:
                # Only allow lower case property names
                name = propertyName
                documents[name] = ""
        elif name:
            if l.startswith("//"):
                if documents[name]:
                    documents[name] += " "
                documents[name] += l[2:].strip()
            elif l.startswith("\""):
                l = l[1:].strip()
                if l.endswith(";"):
                    l = l[:-1].strip()
                if l.endswith(")"):
                    l = l[:-1].strip()
                if l.endswith("\""):
                    l = l[:-1]
                # Fix escaped double quotes
                l = l.replace("\\\"", "\"")
                documents[name] += l
            else:
                name = ""
    for name in list(documents.keys()):
        if documents[name] == "":
            del documents[name]
    return documents

def ciCompare(a,b):
    return cmp(a.lower(), b.lower())

def ciKey(a):
    return a.lower()

def sortListInsensitive(l):
    try:    # Try key function
        l.sort(key=ciKey)
    except TypeError:    # Earlier version of Python, so use comparison function
        l.sort(ciCompare)

def UpdateLineInFile(path, linePrefix, lineReplace):
    lines = []
    with open(path, "r") as f:
        for l in f.readlines():
            l = l.rstrip()
            if l.startswith(linePrefix):
                lines.append(lineReplace)
            else:
                lines.append(l)
    contents = NATIVE.join(lines) + NATIVE
    UpdateFile(path, contents)

host = "prdownloads.sourceforge.net/"
def UpdateDownloadLinks(path, version):
    lines = []
    with open(path, "r") as f:
        for l in f.readlines():
            l = l.rstrip()
            if host in l:
                start, prd, rest = l.partition(host)
                pth, dot, ending = rest.partition(".")
                pthNew = pth[:-3] + version.rstrip()
                lineWithNewVersion = start + prd +pthNew + dot + ending
                lines.append(lineWithNewVersion)
            else:
                lines.append(l)
    contents = NATIVE.join(lines) + NATIVE
    UpdateFile(path, contents)

def UpdateVersionNumbers(root):
    with open(root + "scintilla/version.txt") as f:
        version = f.read()
    versionDotted = version[0] + '.' + version[1] + '.' + version[2]
    versionCommad = version[0] + ', ' + version[1] + ', ' + version[2] + ', 0'

    UpdateLineInFile(root + "scintilla/win32/ScintRes.rc", "#define VERSION_SCINTILLA",
        "#define VERSION_SCINTILLA \"" + versionDotted + "\"")
    UpdateLineInFile(root + "scintilla/win32/ScintRes.rc", "#define VERSION_WORDS", 
        "#define VERSION_WORDS " + versionCommad)
    UpdateLineInFile(root + "scintilla/qt/ScintillaEditBase/ScintillaEditBase.pro",
        "VERSION =", 
        "VERSION = " + versionDotted)
    UpdateLineInFile(root + "scintilla/qt/ScintillaEdit/ScintillaEdit.pro",
        "VERSION =", 
        "VERSION = " + versionDotted)
    UpdateLineInFile(root + "scintilla/doc/ScintillaDownload.html", "       Release", 
        "       Release " + versionDotted)
    UpdateDownloadLinks(root + "scintilla/doc/ScintillaDownload.html", version)
    UpdateLineInFile(root + "scintilla/doc/index.html",
        '          <font color="#FFCC99" size="3"> Release version', 
        '          <font color="#FFCC99" size="3"> Release version ' + versionDotted + '<br />') 

    if os.path.exists(root + "scite"):
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define VERSION_SCITE", 
            "#define VERSION_SCITE \"" + versionDotted + "\"")
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define VERSION_WORDS", 
            "#define VERSION_WORDS " + versionCommad)
        UpdateLineInFile(root + "scite/doc/SciTEDownload.html", "       Release", 
            "       Release " + versionDotted)
        UpdateDownloadLinks(root + "scite/doc/SciTEDownload.html", version)
        UpdateLineInFile(root + "scite/doc/SciTE.html",
            '          <font color="#FFCC99" size="3"> Release version', 
            '          <font color="#FFCC99" size="3"> Release version ' + versionDotted + '<br />') 

def RegenerateAll():
    root="../../"

    # Find all the lexer source code files
    lexFilePaths = glob.glob(root + "scintilla/lexers/Lex*.cxx")
    sortListInsensitive(lexFilePaths)
    lexFiles = [os.path.basename(f)[:-4] for f in lexFilePaths]
    print(lexFiles)
    lexerModules = []
    lexerProperties = {}
    propertyDocuments = {}
    for lexFile in lexFilePaths:
        lexerModules.extend(FindModules(lexFile))
        for k in FindProperties(lexFile).keys():
            lexerProperties[k] = 1
        documents = FindPropertyDocumentation(lexFile)
        for k in documents.keys():
            if k not in propertyDocuments:
                propertyDocuments[k] = documents[k]
    sortListInsensitive(lexerModules)
    lexerProperties = list(lexerProperties.keys())
    sortListInsensitive(lexerProperties)

    # Generate HTML to document each property
    # This is done because tags can not be safely put inside comments in HTML
    documentProperties = list(propertyDocuments.keys())
    sortListInsensitive(documentProperties)
    propertiesHTML = []
    for k in documentProperties:
        propertiesHTML.append("\t<tr id='property-%s'>\n\t<td>%s</td>\n\t<td>%s</td>\n\t</tr>" %
            (k, k, propertyDocuments[k]))

    # Find all the SciTE properties files
    otherProps = ["abbrev.properties", "Embedded.properties", "SciTEGlobal.properties", "SciTE.properties"]
    if os.path.exists(root + "scite"):
        propFilePaths = glob.glob(root + "scite/src/*.properties")
        sortListInsensitive(propFilePaths)
        propFiles = [os.path.basename(f) for f in propFilePaths if os.path.basename(f) not in otherProps]
        sortListInsensitive(propFiles)
        print(propFiles)

    Regenerate(root + "scintilla/src/Catalogue.cxx", "//", NATIVE, lexerModules)
    Regenerate(root + "scintilla/win32/scintilla.mak", "#", NATIVE, lexFiles)
    if os.path.exists(root + "scite"):
        Regenerate(root + "scite/win32/makefile", "#", NATIVE, propFiles)
        Regenerate(root + "scite/win32/scite.mak", "#", NATIVE, propFiles)
        Regenerate(root + "scite/src/SciTEProps.cxx", "//", NATIVE, lexerProperties)
        Regenerate(root + "scite/doc/SciTEDoc.html", "<!--", NATIVE, propertiesHTML)
        Generate(root + "scite/boundscheck/vcproj.gen",
         root + "scite/boundscheck/SciTE.vcproj", "#", NATIVE, lexFiles)

    UpdateVersionNumbers(root)

RegenerateAll()
