// Copyright (c) 2014, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#ifndef SRC_NATIVE_SNAPSHOTTER_PATH_H_
#define SRC_NATIVE_SNAPSHOTTER_PATH_H_

#include <string>
#include <vector>

#include "native/platform/globals.h"

namespace dart {
namespace snapshotter {

class PathStyle {
 public:
  virtual ~PathStyle() {}

  virtual char separator() const = 0;
  virtual size_t RootLength(const std::string& path) const = 0;
  virtual bool IsRootRelative(const std::string& path) const = 0;
  virtual bool IsSeparator(char c) const = 0;
  virtual bool NeedsSeparator(const std::string& root) const = 0;
  virtual bool IsWindows() const = 0;

  std::string GetRoot(const std::string& path) const;

 protected:
  PathStyle() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(PathStyle);
};

class PosixPathStyle: public PathStyle {
 public:
  PosixPathStyle() {}
  virtual ~PosixPathStyle() {}

  virtual char separator() const { return '/'; }
  virtual size_t RootLength(const std::string& path) const;
  virtual bool IsRootRelative(const std::string& path) const { return false; }
  virtual bool IsSeparator(char c) const { return c == L'/'; }
  virtual bool NeedsSeparator(const std::string& root) const;
  virtual bool IsWindows() const { return false; }

 private:
  DISALLOW_COPY_AND_ASSIGN(PosixPathStyle);
};

class WindowsPathStyle: public PathStyle {
 public:
  WindowsPathStyle() {}
  virtual ~WindowsPathStyle() {}

  virtual char separator() const { return '\\'; }
  virtual size_t RootLength(const std::string& path) const;
  virtual bool IsRootRelative(const std::string& path) const;
  virtual bool IsSeparator(char c) const { return c == L'/' || c == L'\\'; }
  virtual bool NeedsSeparator(const std::string& root) const;
  virtual bool IsWindows() const { return true; }

 private:
  DISALLOW_COPY_AND_ASSIGN(WindowsPathStyle);
};

class UrlPathStyle: public PathStyle {
 public:
  UrlPathStyle() {}
  virtual ~UrlPathStyle() {}

  virtual char separator() const { return '/'; }
  virtual size_t RootLength(const std::string& path) const;
  virtual bool IsRootRelative(const std::string& path) const;
  virtual bool IsSeparator(char c) const { return c == L'/'; }
  virtual bool NeedsSeparator(const std::string& root) const;
  virtual bool IsWindows() const { return false; }

 private:
  DISALLOW_COPY_AND_ASSIGN(UrlPathStyle);
};

class Path {
 public:
  static const Path kPosix;
  static const Path kUrl;
  static const Path kWindows;

  static const PosixPathStyle kPosixStyle;
  static const UrlPathStyle kUrlStyle;
  static const WindowsPathStyle kWindowsStyle;

  static const Path& current();

  bool IsAbsolute(const std::string& path) const;
  std::string RootPrefix(const std::string& path) const;
  std::string Dirname(const std::string& path) const;
  std::string Normalize(const std::string& path) const;
  std::string Join(const std::string& part0,
                   const std::string& part1 = "",
                   const std::string& part2 = "",
                   const std::string& part3 = "",
                   const std::string& part4 = "",
                   const std::string& part5 = "",
                   const std::string& part6 = "",
                   const std::string& part7 = "",
                   const std::string& part8 = "") const;
  std::string JoinAll(const std::vector<std::string>& parts) const;
  std::vector<std::string> Split(const std::string& path) const;

 private:
  Path(const PathStyle& style) : style_(style) {}

  class ParsedPath {
   public:
    ParsedPath(const std::string& path, const PathStyle& style);

    void RemoveTrailingSeparators();
    void Normalize();
    bool IsAbsolute() const { return !root_.empty(); }

    std::string str() const;

   private:
    friend class dart::snapshotter::Path;
    void Parse(const std::string& path);

    std::string root_;
    bool is_root_relative_;
    typedef std::vector<std::string> StrList;
    StrList parts_;
    std::vector<char> separators_;
    const PathStyle* style_;

    DISALLOW_COPY_AND_ASSIGN(ParsedPath);
  };

  const PathStyle& style_;

  DISALLOW_COPY_AND_ASSIGN(Path);
};

}  // namespace snapshotter
}  // namespace dart

#endif  // SRC_NATIVE_SNAPSHOTTER_PATH_H_
