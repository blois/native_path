// Copyright (c) 2014, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "native/snapshotter/path.h"

#include "native/platform/assert.h"

#include <algorithm>
#include <sstream>

namespace dart {
namespace snapshotter {

std::string PathStyle::GetRoot(const std::string& path) const {
  size_t length = RootLength(path);
  if (length != std::string::npos && length > 0) return path.substr(0, length);

  return IsRootRelative(path) ? path.substr(0, 1) : std::string();
}

size_t PosixPathStyle::RootLength(const std::string& path) const {
  if (path.find("/") == 0) {
    return 1;
  }
  return 0;
}

bool PosixPathStyle::NeedsSeparator(const std::string& path) const {
  return !path.empty() && !IsSeparator(*path.rbegin());
}

static bool IsAlphabetic(char c) {
  return (c >= L'A' && c <= L'Z') ||
      (c >= L'a' && c <= L'z');
}

size_t WindowsPathStyle::RootLength(const std::string& path) const {
  if (path.empty()) return 0;
  if (path[0] == '/') return 1;
  if (path[0] == '\\') {
    if (path.length() < 2 || path[1] != '\\') return 1;
    // The path is a network share. Search for up to two '\'s, as they are
    // the server and share - and part of the root part.
    size_t index = path.find('\\', 2);
    if (index != std::string::npos) {
      index = path.find('\\', index + 1);
      if (index != std::string::npos) return index;
    }
    return path.length();
  }
  // If the path is of the form 'C:/' or 'C:\', with C being any letter, it's
  // a root part.
  if (path.length() < 3) return 0;
  // Check for the letter.
  if (!IsAlphabetic(path[0])) return 0;
  // Check for the ':'.
  if (path[1] != ':') return 0;
  // Check for either '/' or '\'.
  if (!IsSeparator(path[2])) return 0;

  return 3;
}

bool WindowsPathStyle::IsRootRelative(const std::string& path) const {
  return RootLength(path) == 1;
}

bool WindowsPathStyle::NeedsSeparator(const std::string& path) const {
  if (path.empty()) return false;
  return !IsSeparator(*path.rbegin());
}

size_t UrlPathStyle::RootLength(const std::string& path) const {
  if (path.empty()) return 0;
  if (IsSeparator(path[0])) return 1;

  size_t index = path.find("/");
  if (index != std::string::npos && path.find("://", index - 1) == index - 1) {
    // The root part is up until the next '/', or the full path. Skip
    // '://' and search for '/' after that.
    index = path.find("/", index + 2);
    if (index != std::string::npos) return index;
    return path.length();
  }
  index = 0;
  for (std::string::const_iterator pos = path.begin(); pos != path.end(); ++pos, ++index) {
    if (*pos == L':') {
      return index + 1;
    }
    if (!IsAlphabetic(*pos)) break;
  }

  return 0;
}

bool UrlPathStyle::IsRootRelative(const std::string& path) const {
  return !path.empty() && IsSeparator(path[0]);
}

bool UrlPathStyle::NeedsSeparator(const std::string& path) const {
  if (path.empty()) return false;

  // A URL that doesn't end in "/" always needs a separator.
  if (!IsSeparator(*path.rbegin())) return true;

  // A URI that's just "scheme://" needs an extra separator, despite ending
  // with "/".
  return path.rfind("://") == path.length() - 3 &&
      RootLength(path) == path.length();
}

const PosixPathStyle Path::kPosixStyle;
const UrlPathStyle Path::kUrlStyle;
const WindowsPathStyle Path::kWindowsStyle;

const Path Path::kPosix(kPosixStyle);
const Path Path::kUrl(kUrlStyle);
const Path Path::kWindows(kWindowsStyle);

const Path& Path::current() {
#if defined(TARGET_OS_WINDOWS)
  return kWindows;
#else
  return kPosix;
#endif
}

bool Path::IsAbsolute(const std::string& path) const {
  return style_.RootLength(path) != 0;
}

std::string Path::RootPrefix(const std::string& path) const {
  return path.substr(0, style_.RootLength(path));
}

std::string Path::Dirname(const std::string& path) const {
  ParsedPath parsed(path, style_);

  parsed.RemoveTrailingSeparators();
  if (parsed.parts_.empty()) return parsed.root_.empty() ? "." : parsed.root_;
  if (parsed.parts_.size() == 1) {
    return parsed.root_.empty() ? "." : parsed.root_;
  }
  parsed.parts_.pop_back();
  parsed.separators_.pop_back();
  parsed.RemoveTrailingSeparators();
  return parsed.str();
}

std::string Path::Normalize(const std::string& path) const {
  ParsedPath parsed(path, style_);
  parsed.Normalize();
  return parsed.str();
}

std::string Path::Join(const std::string& part0,
                       const std::string& part1,
                       const std::string& part2,
                       const std::string& part3,
                       const std::string& part4,
                       const std::string& part5,
                       const std::string& part6,
                       const std::string& part7,
                       const std::string& part8) const {
  std::vector<std::string> parts(8);
  parts[0] = part0;
  parts[1] = part1;
  parts[2] = part2;
  parts[3] = part3;
  parts[4] = part4;
  parts[5] = part5;
  parts[6] = part6;
  parts[7] = part7;
  return JoinAll(parts);
}

std::string Path::JoinAll(const std::vector<std::string>& parts) const {
  std::stringstream buffer;
  bool needs_separator = false;
  bool is_absolute_and_not_root_relative = false;

  for (std::vector<std::string>::const_iterator pos = parts.begin();
      pos != parts.end(); ++pos) {
    const std::string& part = *pos;
    if (part.empty()) continue;

    if (style_.IsRootRelative(part) && is_absolute_and_not_root_relative) {
      // If the new part is root-relative, it preserves the previous root but
      // replaces the path after it.
      ParsedPath parsed(part, style_);
      parsed.root_ = RootPrefix(buffer.str());
      if (style_.NeedsSeparator(parsed.root_)) {
        parsed.separators_[0] = style_.separator();
      }
      buffer.str(std::string());
      buffer << parsed.str();
    } else if (IsAbsolute(part)) {
      is_absolute_and_not_root_relative = !style_.IsRootRelative(part);
      // An absolute path discards everything before it.
      buffer.str(std::string());
      buffer << part;
    } else {
      if (!part.empty() && style_.IsSeparator(part[0])) {
        // The part starts with a separator, so we don't need to add one.
      } else if (needs_separator) {
        buffer << style_.separator();
      }

      buffer << part;
    }
    // Unless this part ends with a separator, we'll need to add one before
    // the next part.
    needs_separator = style_.NeedsSeparator(part);
  }

  return buffer.str();
}

bool IsEmpty(const std::string& str) { return str.empty(); }

std::vector<std::string> Path::Split(const std::string& path) const {
  ParsedPath parsed(path, style_);
  // Filter out empty parts that exist due to multiple separators in a row.
  std::vector<std::string>::iterator end = std::remove_if(parsed.parts_.begin(),
                                                          parsed.parts_.end(),
                                                          IsEmpty);
  parsed.parts_.resize(end - parsed.parts_.begin());

  if (!parsed.root_.empty()) parsed.parts_.insert(parsed.parts_.begin(),
                                                  parsed.root_);
  return parsed.parts_;
}

Path::ParsedPath::ParsedPath(const std::string& before, const PathStyle& style)
    : style_(&style) {
  std::string path = before;

  // Remove the root prefix, if any.
  root_ = style.GetRoot(path);
  is_root_relative_ = style.IsRootRelative(path);
  if (!root_.empty()) path = path.substr(root_.length());

  // Split the parts on path separators.
  size_t start = 0;
  if (!path.empty() && style_->IsSeparator(path[0])) {
    separators_.push_back(path[0]);
    start = 1;
  } else {
    separators_.push_back(0);
  }

  for (size_t i = start; i < path.length(); ++i) {
    if (style_->IsSeparator(path[i])) {
      parts_.push_back(path.substr(start, i - start));
      separators_.push_back(path[i]);
      start = i + 1;
    }
  }

  // Add the final part, if any.
  if (start < path.length()) {
    parts_.push_back(path.substr(start));
    separators_.push_back(0);
  }
}

void Path::ParsedPath::RemoveTrailingSeparators() {
  while (!parts_.empty() && parts_.back().empty()) {
    parts_.pop_back();
    separators_.pop_back();
  }
  if (!separators_.empty()) separators_.back() = 0;
}

void Path::ParsedPath::Normalize() {
  // Handle '.', '..', and empty parts.
  size_t leading_doubles = 0;
  StrList new_parts;
  for (StrList::iterator part = parts_.begin(); part != parts_.end(); ++part) {
    if (*part == "." || *part == "") {
      // Do nothing. Ignore it.
    } else if (*part == "..") {
      // Pop the last part off.
      if (!new_parts.empty()) {
        new_parts.pop_back();
      } else {
        // Backed out past the beginning, so preserve the "..".
        leading_doubles++;
      }
    } else {
      new_parts.push_back(*part);
    }
  }

  // A relative path can back out from the start directory.
  if (!IsAbsolute()) {
    for (size_t i = 0; i < leading_doubles; ++i) {
      new_parts.insert(new_parts.begin(), "..");
    }
  }

  // If we collapsed down to nothing, do ".".
  if (new_parts.empty() && !IsAbsolute()) {
    new_parts.push_back(".");
  }

  // Canonicalize separators.
  std::vector<char> new_separators(new_parts.size(), style_->separator());
  new_separators.insert(new_separators.begin(),
      IsAbsolute() && !new_parts.empty() && style_->NeedsSeparator(root_) ?
      style_->separator() : 0);

  parts_ = new_parts;
  separators_ = new_separators;

  // Normalize the Windows root if needed.
  if (!root_.empty() && style_->IsWindows()) {
    std::replace(root_.begin(), root_.end(), '/', '\\');
  }
  RemoveTrailingSeparators();
}

std::string Path::ParsedPath::str() const {
  std::stringstream strstr;
  if (!root_.empty()) strstr << root_;
  for (size_t i = 0; i < parts_.size(); i++) {
    if (separators_[i] != 0) {
      strstr << separators_[i];
    }
    strstr << parts_[i];
  }
  if (separators_.back() != 0) {
    strstr << separators_.back();
  }

  return strstr.str();
}

}  // namespace snapshotter
}  // namespace dart
