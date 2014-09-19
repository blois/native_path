// Copyright (c) 2014, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "native/platform/globals.h"
#include "native/platform/assert.h"
#include "native/log.h"
#include "native/snapshotter/path.h"
#include "native/snapshotter/directory.h"

namespace dart {
namespace snapshotter {

void PosixRootPrefixTests() {
  const Path& path = Path::kPosix;

  EXPECT_EQ(path.RootPrefix(""), "");
  EXPECT_EQ(path.RootPrefix("a"), "");
  EXPECT_EQ(path.RootPrefix("a/b"), "");
  EXPECT_EQ(path.RootPrefix("/a/c"), "/");
  EXPECT_EQ(path.RootPrefix("/"), "/");
}

void PosixIsAbsoluteTests() {
  const Path& path = Path::kPosix;

  EXPECT_EQ(path.IsAbsolute(""), false);
  EXPECT_EQ(path.IsAbsolute("a"), false);
  EXPECT_EQ(path.IsAbsolute("a/b"), false);
  EXPECT_EQ(path.IsAbsolute("/a"), true);
  EXPECT_EQ(path.IsAbsolute("/a/b"), true);
  EXPECT_EQ(path.IsAbsolute("~"), false);
  EXPECT_EQ(path.IsAbsolute("."), false);
  EXPECT_EQ(path.IsAbsolute(".."), false);
  EXPECT_EQ(path.IsAbsolute(".foo"), false);
  EXPECT_EQ(path.IsAbsolute("../a"), false);
  EXPECT_EQ(path.IsAbsolute("C:/a"), false);
  EXPECT_EQ(path.IsAbsolute("C:\\a"), false);
  EXPECT_EQ(path.IsAbsolute("\\\\a"), false);
}

void PosixDirnameTests() {
  const Path& path = Path::kPosix;

  EXPECT_EQ(path.Dirname(""), ".");
  EXPECT_EQ(path.Dirname("."), ".");
  EXPECT_EQ(path.Dirname(".."), ".");
  EXPECT_EQ(path.Dirname("../.."), "..");
  EXPECT_EQ(path.Dirname("a"), ".");
  EXPECT_EQ(path.Dirname("a/b"), "a");
  EXPECT_EQ(path.Dirname("a/b/c"), "a/b");
  EXPECT_EQ(path.Dirname("a/b.c"), "a");
  EXPECT_EQ(path.Dirname("a/"), ".");
  EXPECT_EQ(path.Dirname("a/."), "a");
  EXPECT_EQ(path.Dirname("a/.."), "a");
  EXPECT_EQ(path.Dirname("a\\b/c"), "a\\b");
  EXPECT_EQ(path.Dirname("/a"), "/");
  EXPECT_EQ(path.Dirname("///a"), "/");
  EXPECT_EQ(path.Dirname("/"), "/");
  EXPECT_EQ(path.Dirname("///"), "/");
  EXPECT_EQ(path.Dirname("a/b/"), "a");
  EXPECT_EQ(path.Dirname("a/b\\c"), "a");
  EXPECT_EQ(path.Dirname("a//"), ".");
  EXPECT_EQ(path.Dirname("a/b//"), "a");
  EXPECT_EQ(path.Dirname("a//b"), "a");
}

void PosixNormalizeTests() {
  const Path& path = Path::kPosix;

  EXPECT_EQ(path.Normalize(""), ".");
  EXPECT_EQ(path.Normalize("."), ".");
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("a"), "a");
  EXPECT_EQ(path.Normalize("/"), "/");
  EXPECT_EQ(path.Normalize("\\"), "\\");
  EXPECT_EQ(path.Normalize("C:/"), "C:");
  EXPECT_EQ(path.Normalize("C:\\"), "C:\\");
  EXPECT_EQ(path.Normalize("\\\\"), "\\\\");

  // collapses redundant separators
  EXPECT_EQ(path.Normalize("a/b/c"), "a/b/c");
  EXPECT_EQ(path.Normalize("a//b///c////d"), "a/b/c/d");

  // does not collapse separators for other platform
  EXPECT_EQ(path.Normalize("a\\b\\\\\\c"), "a\\b\\\\\\c");

  // eliminates "." parts
  EXPECT_EQ(path.Normalize("./"), ".");
  EXPECT_EQ(path.Normalize("/."), "/");
  EXPECT_EQ(path.Normalize("/./"), "/");
  EXPECT_EQ(path.Normalize("./."), ".");
  EXPECT_EQ(path.Normalize("a/./b"), "a/b");
  EXPECT_EQ(path.Normalize("a/.b/c"), "a/.b/c");
  EXPECT_EQ(path.Normalize("a/././b/./c"), "a/b/c");
  EXPECT_EQ(path.Normalize("././a"), "a");
  EXPECT_EQ(path.Normalize("a/./."), "a");

  // eliminates ".." parts
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("../"), "..");
  EXPECT_EQ(path.Normalize("../../.."), "../../..");
  EXPECT_EQ(path.Normalize("../../../"), "../../..");
  EXPECT_EQ(path.Normalize("/.."), "/");
  EXPECT_EQ(path.Normalize("/../../.."), "/");
  EXPECT_EQ(path.Normalize("/../../../a"), "/a");
  EXPECT_EQ(path.Normalize("c:/.."), ".");
  EXPECT_EQ(path.Normalize("A:/../../.."), "../..");
  EXPECT_EQ(path.Normalize("a/.."), ".");
  EXPECT_EQ(path.Normalize("a/b/.."), "a");
  EXPECT_EQ(path.Normalize("a/../b"), "b");
  EXPECT_EQ(path.Normalize("a/./../b"), "b");
  EXPECT_EQ(path.Normalize("a/b/c/../../d/e/.."), "a/d");
  EXPECT_EQ(path.Normalize("a/b/../../../../c"), "../../c");
  EXPECT_EQ(path.Normalize("z/a/b/../../..\\../c"), "z/..\\../c");
  EXPECT_EQ(path.Normalize("a/b\\c/../d"), "a/d");

  // does not walk before root on absolute paths
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("../"), "..");
  EXPECT_EQ(path.Normalize("http://dartlang.org/.."), "http:");
  EXPECT_EQ(path.Normalize("http://dartlang.org/../../a"), "a");
  EXPECT_EQ(path.Normalize("file:///.."), ".");
  EXPECT_EQ(path.Normalize("file:///../../a"), "../a");
  EXPECT_EQ(path.Normalize("/.."), "/");
  EXPECT_EQ(path.Normalize("a/.."), ".");
  EXPECT_EQ(path.Normalize("../a"), "../a");
  EXPECT_EQ(path.Normalize("/../a"), "/a");
  EXPECT_EQ(path.Normalize("c:/../a"), "a");
  EXPECT_EQ(path.Normalize("/../a"), "/a");
  EXPECT_EQ(path.Normalize("a/b/.."), "a");
  EXPECT_EQ(path.Normalize("../a/b/.."), "../a");
  EXPECT_EQ(path.Normalize("a/../b"), "b");
  EXPECT_EQ(path.Normalize("a/./../b"), "b");
  EXPECT_EQ(path.Normalize("a/b/c/../../d/e/.."), "a/d");
  EXPECT_EQ(path.Normalize("a/b/../../../../c"), "../../c");
  EXPECT_EQ(path.Normalize("a/b/c/../../..d/./.e/f././"), "a/..d/.e/f.");

  // removes trailing separators
  EXPECT_EQ(path.Normalize("./"), ".");
  EXPECT_EQ(path.Normalize(".//"), ".");
  EXPECT_EQ(path.Normalize("a/"), "a");
  EXPECT_EQ(path.Normalize("a/b/"), "a/b");
  EXPECT_EQ(path.Normalize("a/b\\"), "a/b\\");
  EXPECT_EQ(path.Normalize("a/b///"), "a/b");
}

void PosixJoinTests() {
  const Path& path = Path::kPosix;
   // allows up to eight parts
  EXPECT_EQ(path.Join("a"), "a");
  EXPECT_EQ(path.Join("a", "b"), "a/b");
  EXPECT_EQ(path.Join("a", "b", "c"), "a/b/c");
  EXPECT_EQ(path.Join("a", "b", "c", "d"), "a/b/c/d");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e"), "a/b/c/d/e");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f"), "a/b/c/d/e/f");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g"), "a/b/c/d/e/f/g");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g", "h"),
      "a/b/c/d/e/f/g/h");

  // does not add separator if a part ends in one
  EXPECT_EQ(path.Join("a/", "b", "c/", "d"), "a/b/c/d");
  EXPECT_EQ(path.Join("a\\", "b"), "a\\/b");

  // ignores parts before an absolute path
  EXPECT_EQ(path.Join("a", "/", "b", "c"), "/b/c");
  EXPECT_EQ(path.Join("a", "/b", "/c", "d"), "/c/d");
  EXPECT_EQ(path.Join("a", "c:\\b", "c", "d"), "a/c:\\b/c/d");
  EXPECT_EQ(path.Join("a", "\\\\b", "c", "d"), "a/\\\\b/c/d");

  // ignores empty strings
  EXPECT_EQ(path.Join(""), "");
  EXPECT_EQ(path.Join("", ""), "");
  EXPECT_EQ(path.Join("", "a"), "a");
  EXPECT_EQ(path.Join("a", "", "b", "", "", "", "c"), "a/b/c");
  EXPECT_EQ(path.Join("a", "b", ""), "a/b");

  // join does not modify internal ., .., or trailing separators
  EXPECT_EQ(path.Join("a/", "b/c/"), "a/b/c/");
  EXPECT_EQ(path.Join("a/b/./c/..//", "d/.././..//e/f//"),
         "a/b/./c/..//d/.././..//e/f//");
  EXPECT_EQ(path.Join("a/b", "c/../../../.."), "a/b/c/../../../..");
  EXPECT_EQ(path.Join("a", "b/"), "a/b/");
}

void PosixTests() {
  PosixRootPrefixTests();
  PosixIsAbsoluteTests();
  PosixDirnameTests();
  PosixNormalizeTests();
  PosixJoinTests();
}

void WindowsRootPrefixTests() {
  const Path& path = Path::kWindows;

  EXPECT_EQ(path.RootPrefix(""), "");
  EXPECT_EQ(path.RootPrefix("a"), "");
  EXPECT_EQ(path.RootPrefix("a\\b"), "");
  EXPECT_EQ(path.RootPrefix("C:\\a\\c"), "C:\\");
  EXPECT_EQ(path.RootPrefix("C:\\"), "C:\\");
  EXPECT_EQ(path.RootPrefix("C:/"), "C:/");
  EXPECT_EQ(path.RootPrefix("\\\\server\\share\\a\\b"), "\\\\server\\share");
  EXPECT_EQ(path.RootPrefix("\\\\server\\share"), "\\\\server\\share");
  EXPECT_EQ(path.RootPrefix("\\\\server\\"), "\\\\server\\");
  EXPECT_EQ(path.RootPrefix("\\\\server"), "\\\\server");
  EXPECT_EQ(path.RootPrefix("\\a\\b"), "\\");
  EXPECT_EQ(path.RootPrefix("/a/b"), "/");
  EXPECT_EQ(path.RootPrefix("\\"), "\\");
  EXPECT_EQ(path.RootPrefix("/"), "/");
}

void WindowsIsAbsoluteTests() {
  const Path& path = Path::kWindows;

  EXPECT_EQ(path.IsAbsolute(""), false);
  EXPECT_EQ(path.IsAbsolute("."), false);
  EXPECT_EQ(path.IsAbsolute(".."), false);
  EXPECT_EQ(path.IsAbsolute("a"), false);
  EXPECT_EQ(path.IsAbsolute("a\\b"), false);
  EXPECT_EQ(path.IsAbsolute("\\a\\b"), true);
  EXPECT_EQ(path.IsAbsolute("\\"), true);
  EXPECT_EQ(path.IsAbsolute("/a/b"), true);
  EXPECT_EQ(path.IsAbsolute("/"), true);
  EXPECT_EQ(path.IsAbsolute("~"), false);
  EXPECT_EQ(path.IsAbsolute("."), false);
  EXPECT_EQ(path.IsAbsolute("..\\a"), false);
  EXPECT_EQ(path.IsAbsolute("a:/a\\b"), true);
  EXPECT_EQ(path.IsAbsolute("D:/a/b"), true);
  EXPECT_EQ(path.IsAbsolute("c:\\"), true);
  EXPECT_EQ(path.IsAbsolute("B:\\"), true);
  EXPECT_EQ(path.IsAbsolute("c:\\a"), true);
  EXPECT_EQ(path.IsAbsolute("C:\\a"), true);
  EXPECT_EQ(path.IsAbsolute("\\\\server\\share"), true);
  EXPECT_EQ(path.IsAbsolute("\\\\server\\share\\path"), true);
}

void WindowsIsRootRelativeTests() {
  const PathStyle& style = Path::kWindowsStyle;

  EXPECT_EQ(style.IsRootRelative(""), false);
  EXPECT_EQ(style.IsRootRelative("."), false);
  EXPECT_EQ(style.IsRootRelative(".."), false);
  EXPECT_EQ(style.IsRootRelative("a"), false);
  EXPECT_EQ(style.IsRootRelative("a\\b"), false);
  EXPECT_EQ(style.IsRootRelative("\\a\\b"), true);
  EXPECT_EQ(style.IsRootRelative("\\"), true);
  EXPECT_EQ(style.IsRootRelative("/a/b"), true);
  EXPECT_EQ(style.IsRootRelative("/"), true);
  EXPECT_EQ(style.IsRootRelative("~"), false);
  EXPECT_EQ(style.IsRootRelative("."), false);
  EXPECT_EQ(style.IsRootRelative("..\\a"), false);
  EXPECT_EQ(style.IsRootRelative("a:/a\\b"), false);
  EXPECT_EQ(style.IsRootRelative("D:/a/b"), false);
  EXPECT_EQ(style.IsRootRelative("c:\\"), false);
  EXPECT_EQ(style.IsRootRelative("B:\\"), false);
  EXPECT_EQ(style.IsRootRelative("c:\\a"), false);
  EXPECT_EQ(style.IsRootRelative("C:\\a"), false);
  EXPECT_EQ(style.IsRootRelative("\\\\server\\share"), false);
  EXPECT_EQ(style.IsRootRelative("\\\\server\\share\\path"), false);
}

void WindowsDirnameTests() {
  const Path& path = Path::kWindows;

  EXPECT_EQ(path.Dirname(""), ".");
  EXPECT_EQ(path.Dirname("a"), ".");
  EXPECT_EQ(path.Dirname("a\\b"), "a");
  EXPECT_EQ(path.Dirname("a\\b\\c"), "a\\b");
  EXPECT_EQ(path.Dirname("a\\b.c"), "a");
  EXPECT_EQ(path.Dirname("a\\"), ".");
  EXPECT_EQ(path.Dirname("a/"), ".");
  EXPECT_EQ(path.Dirname("a\\."), "a");
  EXPECT_EQ(path.Dirname("a\\b/c"), "a\\b");
  EXPECT_EQ(path.Dirname("C:\\a"), "C:\\");
  EXPECT_EQ(path.Dirname("C:\\\\\\a"), "C:\\");
  EXPECT_EQ(path.Dirname("C:\\"), "C:\\");
  EXPECT_EQ(path.Dirname("C:\\\\\\"), "C:\\");
  EXPECT_EQ(path.Dirname("a\\b\\"), "a");
  EXPECT_EQ(path.Dirname("a/b\\c"), "a/b");
  EXPECT_EQ(path.Dirname("a\\\\"), ".");
  EXPECT_EQ(path.Dirname("a\\b\\\\"), "a");
  EXPECT_EQ(path.Dirname("a\\\\b"), "a");
  EXPECT_EQ(path.Dirname("foo bar\\gule fisk"), "foo bar");
  EXPECT_EQ(path.Dirname("\\\\server\\share"), "\\\\server\\share");
  EXPECT_EQ(path.Dirname("\\\\server\\share\\dir"), "\\\\server\\share");
  EXPECT_EQ(path.Dirname("\\a"), "\\");
  EXPECT_EQ(path.Dirname("/a"), "/");
  EXPECT_EQ(path.Dirname("\\"), "\\");
  EXPECT_EQ(path.Dirname("/"), "/");
}

void WindowsNormalizeTests() {
  const Path& path = Path::kWindows;

  EXPECT_EQ(path.Normalize(""), ".");
  EXPECT_EQ(path.Normalize("."), ".");
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("a"), "a");
  EXPECT_EQ(path.Normalize("/a/b"), "\\a\\b");
  EXPECT_EQ(path.Normalize("\\"), "\\");
  EXPECT_EQ(path.Normalize("\\a\\b"), "\\a\\b");
  EXPECT_EQ(path.Normalize("/"), "\\");
  EXPECT_EQ(path.Normalize("C:/"), "C:\\");
  EXPECT_EQ(path.Normalize("C:\\"), "C:\\");
  EXPECT_EQ(path.Normalize("\\\\server\\share"), "\\\\server\\share");

  // collapses redundant separators
  EXPECT_EQ(path.Normalize("a\\b\\c"), "a\\b\\c");
  EXPECT_EQ(path.Normalize("a\\\\b\\\\\\c\\\\\\\\d"), "a\\b\\c\\d");

  // eliminates "." parts"
  EXPECT_EQ(path.Normalize(".\\"), ".");
  EXPECT_EQ(path.Normalize("c:\\."), "c:\\");
  EXPECT_EQ(path.Normalize("B:\\.\\"), "B:\\");
  EXPECT_EQ(path.Normalize("\\\\server\\share\\."), "\\\\server\\share");
  EXPECT_EQ(path.Normalize(".\\."), ".");
  EXPECT_EQ(path.Normalize("a\\.\\b"), "a\\b");
  EXPECT_EQ(path.Normalize("a\\.b\\c"), "a\\.b\\c");
  EXPECT_EQ(path.Normalize("a\\./.\\b\\.\\c"), "a\\b\\c");
  EXPECT_EQ(path.Normalize(".\\./a"), "a");
  EXPECT_EQ(path.Normalize("a/.\\."), "a");
  EXPECT_EQ(path.Normalize("\\."), "\\");
  EXPECT_EQ(path.Normalize("/."), "\\");

  // eliminates ".." parts
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("..\\"), "..");
  EXPECT_EQ(path.Normalize("..\\..\\.."), "..\\..\\..");
  EXPECT_EQ(path.Normalize("../..\\..\\"), "..\\..\\..");
  EXPECT_EQ(path.Normalize("\\\\server\\share\\.."), "\\\\server\\share");
  EXPECT_EQ(path.Normalize("\\\\server\\share\\..\\../..\\a"),
      "\\\\server\\share\\a");
  EXPECT_EQ(path.Normalize("c:\\.."), "c:\\");
  EXPECT_EQ(path.Normalize("A:/..\\..\\.."), "A:\\");
  EXPECT_EQ(path.Normalize("b:\\..\\..\\..\\a"), "b:\\a");
  EXPECT_EQ(path.Normalize("b:\\r\\..\\..\\..\\a\\c\\.\\.."), "b:\\a");
  EXPECT_EQ(path.Normalize("a\\.."), ".");
  EXPECT_EQ(path.Normalize("..\\a"), "..\\a");
  EXPECT_EQ(path.Normalize("c:\\..\\a"), "c:\\a");
  EXPECT_EQ(path.Normalize("\\..\\a"), "\\a");
  EXPECT_EQ(path.Normalize("a\\b\\.."), "a");
  EXPECT_EQ(path.Normalize("..\\a\\b\\.."), "..\\a");
  EXPECT_EQ(path.Normalize("a\\..\\b"), "b");
  EXPECT_EQ(path.Normalize("a\\.\\..\\b"), "b");
  EXPECT_EQ(path.Normalize("a\\b\\c\\..\\..\\d\\e\\.."), "a\\d");
  EXPECT_EQ(path.Normalize("a\\b\\..\\..\\..\\..\\c"), "..\\..\\c");
  EXPECT_EQ(path.Normalize("a/b/c/../../..d/./.e/f././"), "a\\..d\\.e\\f.");

  // removes trailing separators
  EXPECT_EQ(path.Normalize(".\\"), ".");
  EXPECT_EQ(path.Normalize(".\\\\"), ".");
  EXPECT_EQ(path.Normalize("a/"), "a");
  EXPECT_EQ(path.Normalize("a\\b\\"), "a\\b");
  EXPECT_EQ(path.Normalize("a\\b\\\\\\"), "a\\b");

  // normalizes separators
  EXPECT_EQ(path.Normalize("a/b\\c"), "a\\b\\c");
}

void WindowsJoinTests() {
  const Path& path = Path::kWindows;

  // allows up to eight parts
  EXPECT_EQ(path.Join("a"), "a");
  EXPECT_EQ(path.Join("a", "b"), "a\\b");
  EXPECT_EQ(path.Join("a", "b", "c"), "a\\b\\c");
  EXPECT_EQ(path.Join("a", "b", "c", "d"), "a\\b\\c\\d");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e"), "a\\b\\c\\d\\e");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f"), "a\\b\\c\\d\\e\\f");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g"),
      "a\\b\\c\\d\\e\\f\\g");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g", "h"),
      "a\\b\\c\\d\\e\\f\\g\\h");

  // does not add separator if a part ends or begins in one
  EXPECT_EQ(path.Join("a\\", "b", "c\\", "d"), "a\\b\\c\\d");
  EXPECT_EQ(path.Join("a/", "b"), "a/b");

  // ignores parts before an absolute path
  EXPECT_EQ(path.Join("a", "\\b", "\\c", "d"), "\\c\\d");
  EXPECT_EQ(path.Join("a", "/b", "/c", "d"), "/c\\d");
  EXPECT_EQ(path.Join("a", "c:\\b", "c", "d"), "c:\\b\\c\\d");
  EXPECT_EQ(path.Join("a", "\\b\\c", "\\\\d\\e", "f"), "\\\\d\\e\\f");
  EXPECT_EQ(path.Join("a", "c:\\b", "\\c", "d"), "c:\\c\\d");
  EXPECT_EQ(path.Join("a", "\\\\b\\c\\d", "\\e", "f"), "\\\\b\\c\\e\\f");

  // ignores empty strings
  EXPECT_EQ(path.Join(""), "");
  EXPECT_EQ(path.Join("", ""), "");
  EXPECT_EQ(path.Join("", "a"), "a");
  EXPECT_EQ(path.Join("a", "", "b", "", "", "", "c"), "a\\b\\c");
  EXPECT_EQ(path.Join("a", "b", ""), "a\\b");


  // join does not modify internal ., .., or trailing separators
  EXPECT_EQ(path.Join("a/", "b/c/"), "a/b/c/");
  EXPECT_EQ(path.Join("a\\b\\./c\\..\\\\", "d\\..\\.\\..\\\\e\\f\\\\"),
         "a\\b\\./c\\..\\\\d\\..\\.\\..\\\\e\\f\\\\");
  EXPECT_EQ(path.Join("a\\b", "c\\..\\..\\..\\.."), "a\\b\\c\\..\\..\\..\\..");
  EXPECT_EQ(path.Join("a", "b\\"), "a\\b\\");
}

void WindowsTests() {
  WindowsRootPrefixTests();
  WindowsIsAbsoluteTests();
  WindowsIsRootRelativeTests();
  WindowsDirnameTests();
  WindowsNormalizeTests();
  WindowsJoinTests();
}

void UrlRootPrefixTests() {
  const Path& path = Path::kUrl;

  EXPECT_EQ(path.RootPrefix(""), "");
  EXPECT_EQ(path.RootPrefix("a"), "");
  EXPECT_EQ(path.RootPrefix("a/b"), "");
  EXPECT_EQ(path.RootPrefix("http://dartlang.org/a/c"),
      "http://dartlang.org");
  EXPECT_EQ(path.RootPrefix("file:///a/c"), "file://");
  EXPECT_EQ(path.RootPrefix("/a/c"), "/");
  EXPECT_EQ(path.RootPrefix("http://dartlang.org/"), "http://dartlang.org");
  EXPECT_EQ(path.RootPrefix("file:///"), "file://");
  EXPECT_EQ(path.RootPrefix("http://dartlang.org"), "http://dartlang.org");
  EXPECT_EQ(path.RootPrefix("file://"), "file://");
  EXPECT_EQ(path.RootPrefix("/"), "/");
  EXPECT_EQ(path.RootPrefix("foo/bar://"), "");
}

void UrlIsAbsoluteTests() {
  const Path& path = Path::kUrl;

  EXPECT_EQ(path.IsAbsolute(""), false);
  EXPECT_EQ(path.IsAbsolute("a"), false);
  EXPECT_EQ(path.IsAbsolute("a/b"), false);
  EXPECT_EQ(path.IsAbsolute("http://dartlang.org/a"), true);
  EXPECT_EQ(path.IsAbsolute("file:///a"), true);
  EXPECT_EQ(path.IsAbsolute("/a"), true);
  EXPECT_EQ(path.IsAbsolute("http://dartlang.org/a/b"), true);
  EXPECT_EQ(path.IsAbsolute("file:///a/b"), true);
  EXPECT_EQ(path.IsAbsolute("/a/b"), true);
  EXPECT_EQ(path.IsAbsolute("http://dartlang.org/"), true);
  EXPECT_EQ(path.IsAbsolute("file:///"), true);
  EXPECT_EQ(path.IsAbsolute("http://dartlang.org"), true);
  EXPECT_EQ(path.IsAbsolute("file://"), true);
  EXPECT_EQ(path.IsAbsolute("/"), true);
  EXPECT_EQ(path.IsAbsolute("~"), false);
  EXPECT_EQ(path.IsAbsolute("."), false);
  EXPECT_EQ(path.IsAbsolute("../a"), false);
  // EXPECT_EQ(path.IsAbsolute("C:/a"), false); // URI handling
  // EXPECT_EQ(path.IsAbsolute("C:\\a"), false);// URI handling
  EXPECT_EQ(path.IsAbsolute("\\\\a"), false);
}

void UrlIsRootRelativeTests() {
  const PathStyle& style = Path::kUrlStyle;

  EXPECT_EQ(style.IsRootRelative(""), false);
  EXPECT_EQ(style.IsRootRelative("a"), false);
  EXPECT_EQ(style.IsRootRelative("a/b"), false);
  EXPECT_EQ(style.IsRootRelative("http://dartlang.org/a"), false);
  EXPECT_EQ(style.IsRootRelative("file:///a"), false);
  EXPECT_EQ(style.IsRootRelative("/a"), true);
  EXPECT_EQ(style.IsRootRelative("http://dartlang.org/a/b"), false);
  EXPECT_EQ(style.IsRootRelative("file:///a/b"), false);
  EXPECT_EQ(style.IsRootRelative("/a/b"), true);
  EXPECT_EQ(style.IsRootRelative("http://dartlang.org/"), false);
  EXPECT_EQ(style.IsRootRelative("file:///"), false);
  EXPECT_EQ(style.IsRootRelative("http://dartlang.org"), false);
  EXPECT_EQ(style.IsRootRelative("file://"), false);
  EXPECT_EQ(style.IsRootRelative("/"), true);
  EXPECT_EQ(style.IsRootRelative("~"), false);
  EXPECT_EQ(style.IsRootRelative("."), false);
  EXPECT_EQ(style.IsRootRelative("../a"), false);
  EXPECT_EQ(style.IsRootRelative("C:/a"), false);
  EXPECT_EQ(style.IsRootRelative("C:\\a"), false);
  EXPECT_EQ(style.IsRootRelative("\\\\a"), false);
}

void UrlDirnameTests() {
  const Path& path = Path::kUrl;

  EXPECT_EQ(path.Dirname(""), ".");
  EXPECT_EQ(path.Dirname("."), ".");
  EXPECT_EQ(path.Dirname(".."), ".");
  EXPECT_EQ(path.Dirname("../.."), "..");
  EXPECT_EQ(path.Dirname("a"), ".");
  EXPECT_EQ(path.Dirname("a/b"), "a");
  EXPECT_EQ(path.Dirname("a/b/c"), "a/b");
  EXPECT_EQ(path.Dirname("a/b.c"), "a");
  EXPECT_EQ(path.Dirname("a/"), ".");
  EXPECT_EQ(path.Dirname("a/."), "a");
  EXPECT_EQ(path.Dirname("a/.."), "a");
  EXPECT_EQ(path.Dirname("a\\b/c"), "a\\b");
  EXPECT_EQ(path.Dirname("/a"), "/");
  EXPECT_EQ(path.Dirname("///a"), "/");
  EXPECT_EQ(path.Dirname("/"), "/");
  EXPECT_EQ(path.Dirname("///"), "/");
  EXPECT_EQ(path.Dirname("a/b/"), "a");
  EXPECT_EQ(path.Dirname("a/b\\c"), "a");
  EXPECT_EQ(path.Dirname("a//"), ".");
  EXPECT_EQ(path.Dirname("a/b//"), "a");
  EXPECT_EQ(path.Dirname("a//b"), "a");
}

void UrlNormalizeTests() {
  const Path& path = Path::kUrl;

  EXPECT_EQ(path.Normalize(""), ".");
  EXPECT_EQ(path.Normalize("."), ".");
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("a"), "a");
  EXPECT_EQ(path.Normalize("http://dartlang.org/"), "http://dartlang.org");
  EXPECT_EQ(path.Normalize("http://dartlang.org"), "http://dartlang.org");
  EXPECT_EQ(path.Normalize("file://"), "file://");
  EXPECT_EQ(path.Normalize("file:///"), "file://");
  EXPECT_EQ(path.Normalize("/"), "/");
  EXPECT_EQ(path.Normalize("\\"), "\\");
  EXPECT_EQ(path.Normalize("C:/"), "C:");
  // EXPECT_EQ(path.Normalize("C:\\"), "C:\\"); // URI scheme handling
  EXPECT_EQ(path.Normalize("\\\\"), "\\\\");

  // collapses redundant separators
  EXPECT_EQ(path.Normalize("a/b/c"), "a/b/c");
  EXPECT_EQ(path.Normalize("a//b///c////d"), "a/b/c/d");

  // does not collapse separators for other platform
  EXPECT_EQ(path.Normalize("a\\\\b\\\\\\c"), "a\\\\b\\\\\\c");

  // eliminates "." parts
  EXPECT_EQ(path.Normalize("./"), ".");
  EXPECT_EQ(path.Normalize("http://dartlang.org/."), "http://dartlang.org");
  EXPECT_EQ(path.Normalize("file:///."), "file://");
  EXPECT_EQ(path.Normalize("/."), "/");
  EXPECT_EQ(path.Normalize("http://dartlang.org/./"),
      "http://dartlang.org");
  EXPECT_EQ(path.Normalize("file:///./"), "file://");
  EXPECT_EQ(path.Normalize("/./"), "/");
  EXPECT_EQ(path.Normalize("./."), ".");
  EXPECT_EQ(path.Normalize("a/./b"), "a/b");
  EXPECT_EQ(path.Normalize("a/.b/c"), "a/.b/c");
  EXPECT_EQ(path.Normalize("a/././b/./c"), "a/b/c");
  EXPECT_EQ(path.Normalize("././a"), "a");
  EXPECT_EQ(path.Normalize("a/./."), "a");

  // eliminates ".." parts
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("../"), "..");
  EXPECT_EQ(path.Normalize("../../.."), "../../..");
  EXPECT_EQ(path.Normalize("../../../"), "../../..");
  EXPECT_EQ(path.Normalize("http://dartlang.org/.."),
      "http://dartlang.org");
  EXPECT_EQ(path.Normalize("file:///.."), "file://");
  EXPECT_EQ(path.Normalize("/.."), "/");
  EXPECT_EQ(path.Normalize("http://dartlang.org/../../.."),
      "http://dartlang.org");
  EXPECT_EQ(path.Normalize("file:///../../.."), "file://");
  EXPECT_EQ(path.Normalize("/../../.."), "/");
  EXPECT_EQ(path.Normalize("http://dartlang.org/../../../a"),
      "http://dartlang.org/a");
  EXPECT_EQ(path.Normalize("file:///../../../a"), "file:///a");
  EXPECT_EQ(path.Normalize("/../../../a"), "/a");
  // EXPECT_EQ(path.Normalize("c:/.."), "."); // URI scheme handling
  // EXPECT_EQ(path.Normalize("A:/../../.."), "../.."); // URI scheme handling
  EXPECT_EQ(path.Normalize("a/.."), ".");
  EXPECT_EQ(path.Normalize("a/b/.."), "a");
  EXPECT_EQ(path.Normalize("a/../b"), "b");
  EXPECT_EQ(path.Normalize("a/./../b"), "b");
  EXPECT_EQ(path.Normalize("a/b/c/../../d/e/.."), "a/d");
  EXPECT_EQ(path.Normalize("a/b/../../../../c"), "../../c");
  EXPECT_EQ(path.Normalize("z/a/b/../../..\\../c"), "z/..\\../c");
  EXPECT_EQ(path.Normalize("a/b\\c/../d"), "a/d");

  // does not walk before root on absolute paths
  EXPECT_EQ(path.Normalize(".."), "..");
  EXPECT_EQ(path.Normalize("../"), "..");
  EXPECT_EQ(path.Normalize("http://dartlang.org/.."),
      "http://dartlang.org");
  EXPECT_EQ(path.Normalize("http://dartlang.org/../a"),
         "http://dartlang.org/a");
  EXPECT_EQ(path.Normalize("file:///.."), "file://");
  EXPECT_EQ(path.Normalize("file:///../a"), "file:///a");
  EXPECT_EQ(path.Normalize("/.."), "/");
  EXPECT_EQ(path.Normalize("a/.."), ".");
  EXPECT_EQ(path.Normalize("../a"), "../a");
  EXPECT_EQ(path.Normalize("/../a"), "/a");
  // EXPECT_EQ(path.Normalize("c:/../a"), "a");  // URI scheme handling
  EXPECT_EQ(path.Normalize("/../a"), "/a");
  EXPECT_EQ(path.Normalize("a/b/.."), "a");
  EXPECT_EQ(path.Normalize("../a/b/.."), "../a");
  EXPECT_EQ(path.Normalize("a/../b"), "b");
  EXPECT_EQ(path.Normalize("a/./../b"), "b");
  EXPECT_EQ(path.Normalize("a/b/c/../../d/e/.."), "a/d");
  EXPECT_EQ(path.Normalize("a/b/../../../../c"), "../../c");
  EXPECT_EQ(path.Normalize("a/b/c/../../..d/./.e/f././"), "a/..d/.e/f.");

  // removes trailing separators
  EXPECT_EQ(path.Normalize("./"), ".");
  EXPECT_EQ(path.Normalize(".//"), ".");
  EXPECT_EQ(path.Normalize("a/"), "a");
  EXPECT_EQ(path.Normalize("a/b/"), "a/b");
  EXPECT_EQ(path.Normalize("a/b\\"), "a/b\\");
  EXPECT_EQ(path.Normalize("a/b///"), "a/b");
}

void UrlJoinTests() {
  const Path& path = Path::kUrl;

  // allows up to eight parts
  EXPECT_EQ(path.Join("a"), "a");
  EXPECT_EQ(path.Join("a", "b"), "a/b");
  EXPECT_EQ(path.Join("a", "b", "c"), "a/b/c");
  EXPECT_EQ(path.Join("a", "b", "c", "d"), "a/b/c/d");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e"), "a/b/c/d/e");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f"), "a/b/c/d/e/f");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g"), "a/b/c/d/e/f/g");
  EXPECT_EQ(path.Join("a", "b", "c", "d", "e", "f", "g", "h"),
      "a/b/c/d/e/f/g/h");

  // does not add separator if a part ends in one
  EXPECT_EQ(path.Join("a/", "b", "c/", "d"), "a/b/c/d");
  EXPECT_EQ(path.Join("a\\\\", "b"), "a\\\\/b");

  // ignores parts before an absolute path
  EXPECT_EQ(path.Join("a", "http://dartlang.org", "b", "c"),
      "http://dartlang.org/b/c");
  EXPECT_EQ(path.Join("a", "file://", "b", "c"), "file:///b/c");
  EXPECT_EQ(path.Join("a", "/", "b", "c"), "/b/c");
  EXPECT_EQ(path.Join("a", "/b", "http://dartlang.org/c", "d"),
      "http://dartlang.org/c/d");
  EXPECT_EQ(path.Join(
          "a", "http://google.com/b", "http://dartlang.org/c", "d"),
      "http://dartlang.org/c/d");
  EXPECT_EQ(path.Join("a", "/b", "/c", "d"), "/c/d");
  // URI scheme handling
  // EXPECT_EQ(path.Join("a", "c:\\b", "c", "d"), "a/c:\\b/c/d");
  EXPECT_EQ(path.Join("a", "\\\\b", "c", "d"), "a/\\\\b/c/d");

  // preserves roots before a root-relative path
  EXPECT_EQ(path.Join("http://dartlang.org", "a", "/b", "c"),
      "http://dartlang.org/b/c");
  EXPECT_EQ(path.Join("file://", "a", "/b", "c"), "file:///b/c");
  EXPECT_EQ(path.Join("file://", "a", "/b", "c", "/d"), "file:///d");

  // ignores empty strings
  EXPECT_EQ(path.Join(""), "");
  EXPECT_EQ(path.Join("", ""), "");
  EXPECT_EQ(path.Join("", "a"), "a");
  EXPECT_EQ(path.Join("a", "", "b", "", "", "", "c"), "a/b/c");
  EXPECT_EQ(path.Join("a", "b", ""), "a/b");

  // Join does not modify internal ., .., or trailing separators
  EXPECT_EQ(path.Join("a/", "b/c/"), "a/b/c/");
  EXPECT_EQ(path.Join("a/b/./c/..//", "d/.././..//e/f//"),
         "a/b/./c/..//d/.././..//e/f//");
  EXPECT_EQ(path.Join("a/b", "c/../../../.."), "a/b/c/../../../..");
  EXPECT_EQ(path.Join("a", "b/"), "a/b/");
}

void UrlTests() {
  UrlRootPrefixTests();
  UrlIsAbsoluteTests();
  UrlIsRootRelativeTests();
  UrlDirnameTests();
  UrlNormalizeTests();
  UrlJoinTests();
}

extern void ExecutePathTests() {
  PosixTests();
  WindowsTests();
  UrlTests();
}

}  // namespace snapshotter
}  // namespace dart
