//
//  ProcFS_TestHelpers.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/16/16.
//
//
#include <sys/mount.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "procfs.h"
#include <fstream>
#include <memory>
#include <unordered_set>
#include "ProcFS_TestHelpers.hpp"

using namespace std;
using namespace testing;

string ROOTPATH("/proc");
string DOT(".");
string DOTDOT("..");

// Gets the mounted on path for the file system.
const char *
get_mounted_on_path() {
    return ROOTPATH.c_str();
}

// Ensures that procfs is mounted at a given path.
AssertionResult
check_procfs_mounted(const string &path) {
    struct statfs *mounts;
    
    // Get mount information for all mounted file systems.
    int count = getmntinfo(&mounts, 0);
    if (count == 0) {
        return AssertionFailure() << "Failed to get mount information";
    }
    
    // Look for a procfs mount and check that it is at the
    // specified path.
    bool found = false;
    bool correct_path = false;
    const char *root_path = path.c_str();
    for (int i = 0; i < count; i++) {
        struct statfs *fs = &mounts[i];
        if (::strcmp(fs->f_fstypename, PROCFS_FSNAME) == 0) {
            found = true;
            if (::strcmp(fs->f_mntonname, root_path) == 0) {
                correct_path = true;
                break;
            }
        }
    }
    
    if (!found) {
        return AssertionFailure() << "No procfs mounts found";
    }
    
    if (!correct_path) {
        return AssertionFailure() << "procfs mounted, but not at " << path;
    }
    return AssertionSuccess();
}

// Ensures a directory contains all of a given set of paths.
// If "allowOthers" is true, it is permissible for the directory
// to contain additional paths. If not, the directory must contain
// only the named paths. The paths "." and ".." must not be listed,
// but the current test will fail if either is not present.
// NOTE: the rel_dir_path is relative to the root of the file system,
// so "/" would be the top-level directory, mapped to "/proc".
testing::AssertionResult
check_directory_contains(const string &rel_dir_path,
                         const vector<string> &paths,
                         const bool allowOthers) {
    string dir_path(ROOTPATH + "/" + rel_dir_path);
    DIR *dir = opendir(dir_path.c_str());
    if (dir == (DIR *)NULL) {
        return AssertionFailure() << "Failed to open directory " << dir_path;
    }
    
    
    // Create an unordered set containing all of the paths in the directory.
    unordered_set<string> paths_in_directory;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        string name(entry->d_name);
        paths_in_directory.insert(name);
    }
    closedir(dir);
    
    // First, check for "." and ".." which are always required.
    if (paths_in_directory.erase(DOT) != 1) {
        return AssertionFailure() << "'.' entry missing in directory " << dir_path;
    }
    if (paths_in_directory.erase(DOTDOT) != 1) {
        return AssertionFailure() << "'..' entry missing in directory " << dir_path;
    }
    
    // Iterate over the paths we are given and ensure that they are all there.
    for (auto path : paths) {
        if (paths_in_directory.erase(path) != 1) {
            return AssertionFailure() << "Entry '" << path << "' missing in directory " << dir_path;
        }
    }
    
    // If we are not allowed to have other entries, paths_in_directory must now be empty.
    if (!paths_in_directory.empty() && !allowOthers) {
        return AssertionFailure() << "Unexpected entries in directory " << dir_path << ": " << paths_in_directory;
    }
    return AssertionSuccess();
}


// Checks that the type and permissions of an object are as expected.
testing::AssertionResult
check_type_and_permissions(const string &path, int type, int perms) {
    string full_path(ROOTPATH + "/" + path);
    
    struct stat st;
    int error = lstat(full_path.c_str(), &st);
    if (error != 0) {
        return AssertionFailure() << "lstat() for " << full_path << " failed: " << strerror(errno);
    }
    
    if ((st.st_mode & S_IFMT) != type) {
        return AssertionFailure() << "Incorrect file type for " << full_path;
    }
    
    if ((st.st_mode & ALLPERMS) != perms) {
        return AssertionFailure() << "Incorrect permissions for " << full_path;
    }
    return AssertionSuccess();
}


// Iterates over all of the files in a directory, calling a given function. Returns NULL
// if all of the calls returned true. As soon as any call returns false, returns the
// name of the file that was passed to the failing call.
testing::AssertionResult
iterate_all_files(const string &rel_dir_path, const iterator_fn fn) {
    string dir_path(ROOTPATH + "/" + rel_dir_path);
    DIR *dir = opendir(dir_path.c_str());
    if (dir != NULL) {
        // Invoke the function "fn" for each directory entry, until we
        // reach the end or a call returns false.
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            AssertionResult result = fn(entry->d_name);
            if (!result) {
                // Failed - return the AssertionFailure to the caller.
                closedir(dir);
                return result;
            }
        }
        closedir(dir);
    } else {
        return AssertionFailure() << "Unable to open directory " << dir_path;
    }
    
    return AssertionSuccess();
}

// Checks whether a name represents a non-process entry in a process directory
// (i.e, ".", "..", "byname" and "curproc")
bool
non_process_directory_entry(const char *name) {
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || strcmp(name, "byname") == 0
            || strcmp(name, "curproc") == 0;
}

// Checks whether a name represents a special entry in a directory
// (i.e, ".", "..").
bool
is_special_directory_entry(const char *name) {
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

// Checks that a symlink has given content.
AssertionResult
check_symlink_content(const string &rel_path, const string content) {
    string symlink_path(ROOTPATH + "/" + rel_path);
    char buf[PATH_MAX];
    ssize_t length = readlink(symlink_path.c_str(), buf, sizeof(buf));
    if (length > 0) {
        buf[length] = '\0';
        return strcmp(buf, content.c_str()) == 0 ? AssertionSuccess()
                        : (AssertionFailure() << "Unexpected content: " << buf);
    } else {
        return AssertionFailure() << "Failed to read symlink content for " << symlink_path;
    }
}

// Counts the number of entries in a directory, including "." and "..".
int
count_directory_entries(const string &rel_dir_path) {
    int count = 0;
    string dir_path(ROOTPATH + "/" + rel_dir_path);
    DIR *dir = opendir(dir_path.c_str());
    if (dir != NULL) {
        // Invoke the function "fn" for each directory entry, until we
        // reach the end or a call returns false.
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            count++;
        }
        closedir(dir);
    }
    
    return count;
}

// Gets the path of the directory for the current process, relative to
// the file system root.
string
current_process_directory_path() {
    return to_string(getpid());
}

// Gets the size of a file or directory.
size_t
file_size(const string &rel_file_path) {
    size_t size = 0;
    struct stat st;
    string full_path(ROOTPATH + "/" + rel_file_path);
    if (stat(full_path.c_str(), &st) == 0) {
        size = st.st_size;
    }
    return size;
}


// Reads the content of a file into a given location, verifying that it has the
// expected size.
bool
read_file_content(const std::string &rel_file_path, void * const buffer, const size_t size) {
    string file_path(ROOTPATH + "/" + rel_file_path);
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }
    
    // Read the file content, verifying that we get exactly the right
    // amount of data.
    char *ptr = static_cast<char *>(buffer);
    size_t size_read = 0;
    while(size_read < size) {
        ssize_t n = read(fd, ptr + size_read, size - size_read);
        if (n < 0) {
            close(fd);
            return false;
        }
        if (n == 0) {
            break;
        }
        size_read += n;
    }
    
    // We got all the data. Read once more to make sure there is no more data available.
    char c;
    bool result = read(fd, &c, 1)  == 0;
    close(fd);
    
    return result;
}

// Reads the content of a file. If the file could not be read, an
// empty string is returned and error is set to true.
std::string
read_file(const std::string &rel_file_path, bool &error) {
    string file_path(ROOTPATH + "/" + rel_file_path);

    auto ss = ifstream(file_path);
    string result;
    ss >> result;
    error = ss.bad() || ss.fail();
    
    return result;
}

// Checks whether a file is empty by reading its content.
bool
check_file_empty(const std::string &rel_file_path) {
    string file_path(ROOTPATH + "/" + rel_file_path);
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

    char c;
    bool result = read(fd, &c, 1) == 0;
    close(fd);
    
    return result;
}

// Checks whether a file exists. If the file is a symbolic
// link, checks whether the link itself exists, ignoring the
// validity of the target,
bool
check_file_exists(const std::string &rel_file_path) {
    string file_path(ROOTPATH + "/" + rel_file_path);
    struct stat st;
    return lstat(file_path.c_str(), &st) == 0;
}

// Checks whether a file exists. If the file is a symbolic
// link, checks whether the link itself exists, ignoring the
// validity of the target. The file path is absolute.
bool
check_absolute_file_exists(const std::string &file_path) {
    struct stat st;
    return lstat(file_path.c_str(), &st) == 0;
}




