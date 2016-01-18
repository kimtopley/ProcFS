//
//  ProcFS_TestHelpers.hpp
//  ProcFS
//
//  Created by Kim Topley on 1/16/16.
//
//

#ifndef ProcFS_TestHelpers_hpp
#define ProcFS_TestHelpers_hpp

#include "gtest/gtest.h"
#include <string>

extern std::string ROOTPATH;
extern std::string DOT;
extern std::string DOTDOT;

// Signature of iterator function for directory scans.
typedef testing::AssertionResult (*iterator_fn)(const char * const file_name);

// Gets the mounted on path for the file system.
const char *get_mounted_on_path();

// Ensures that procfs is mounted at a given path.
testing::AssertionResult check_procfs_mounted(const std::string &path);

// Ensures a directory contains all of a given set of paths.
// If "allowOthers" is true, it is permissible for the directory
// to contain additional paths. If not, the directory must contain
// only the named paths. The paths "." and ".." must not be listed,
// but the current test will fail if either is not present.
testing::AssertionResult check_directory_contains(const std::string &rel_dir_path, const std::vector<std::string> &paths, const bool allowOthers);

// Checks that the type and permissions of an object are as expected.
testing::AssertionResult check_type_and_permissions(const std::string &path, int type, int perms);

// Iterates over all of the files in a directory, calling a given function.
// If any call returns false, returns an AssertionFailure. If all files are
// processed without error, return AssertionSuccess;
testing::AssertionResult iterate_all_files(const std::string &rel_dir_path, const iterator_fn fn);

// Checks whether a name represents a non-process entry in a process directory
// (i.e, ".", "..", "byname" and "curproc").
bool non_process_directory_entry(const char *name);

// Checks whether a name represents a special entry in a directory
// (i.e, ".", "..").
bool is_special_directory_entry(const char *name);

// Checks that a symlink has given content.
testing::AssertionResult check_symlink_content(const std::string &rel_path, const std::string content);

// Counts the number of entries in a directory, including "." and "..".
int count_directory_entries(const std::string &rel_dir_path);

// Gets the path of the directory for the current process, relative to
// the file system root.
std::string current_process_directory_path();

// Gets the size of a file or directory.
size_t file_size(const std::string &rel_file_path);

// Reads the content of a file into a given location, verifying that it has the
// expected size.
bool read_file_content(const std::string &rel_file_path, void *buffer, size_t size);

// Reads the content of a file. If the file could not be read, an
// empty string is returned and error is set to true.
std::string read_file(const std::string &rel_file_path, bool &error);

// Checks whether a file is empty by reading its content.
bool check_file_empty(const std::string &rel_file_path);

// Checks whether a file exists. If the file is a symbolic
// link, checks whether the link itself exists, ignoring the
// validity of the target. The file path is relative to the
// "/proc" directory.
bool check_file_exists(const std::string &rel_file_path);

// Checks whether a file exists. If the file is a symbolic
// link, checks whether the link itself exists, ignoring the
// validity of the target. The file path is absolute.
bool check_absolute_file_exists(const std::string &file_path);

#endif /* ProcFS_TestHelpers_hpp */
