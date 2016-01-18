//
//  ProcFS_RootDirTests.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/15/16.
//
//  Procfs root directory tests.
//
#include <inttypes.h>
#include <gtest/gtest.h>
#include "Procfs_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

using namespace std;
using namespace testing;

static AssertionResult check_proc_file_name(const char * const file_name);
static AssertionResult check_proc_files_names_are_valid();
static AssertionResult check_proc_file_properties(const char * const file_name);
static AssertionResult check_proc_files_properties_are_valid();

TEST_F(ProcFSTestFixture, CheckRootDirPerms) {
    // Check that the root directory has the correct type and permissions.
    EXPECT_TRUE(check_type_and_permissions("/", S_IFDIR, 0555));
}

TEST_F(ProcFSTestFixture, CheckRootDirContent) {
    // Check that the root directory contains "curproc" and "byname", allowing others.
    EXPECT_TRUE(check_directory_contains("/", vector<string>({"curproc", "byname"}), true));
}

TEST_F(ProcFSTestFixture, CheckByNameType) {
    // Check that "byname" is a directory.
    EXPECT_TRUE(check_type_and_permissions("byname", S_IFDIR, 0550));
}

TEST_F(ProcFSTestFixture, CheckCurprocProperties) {
    // Check that "curproc" is a symbolic link
    EXPECT_TRUE(check_type_and_permissions("curproc", S_IFLNK, 0777));
}

TEST_F(ProcFSTestFixture, CheckCurprocTarget) {
    // Check that "curproc" refers to the current process.
    pid_t pid = getpid();
    string content = to_string(pid);
    EXPECT_TRUE(check_symlink_content("curproc", content));
}

TEST_F(ProcFSTestFixture, CheckRootFileNames) {
    // Check that all of the other entries have numeric names.
    EXPECT_TRUE(check_proc_files_names_are_valid());
}

TEST_F(ProcFSTestFixture, CheckRootFileProperties) {
    // Check that all of the other entries have correct types and permission.
    EXPECT_TRUE(check_proc_files_properties_are_valid());
}

// Vallidates a file from the root directory. If it's not one of the special
// cases, its name mustbe numeric.
static AssertionResult
check_proc_file_name(const char * const file_name) {
    if (non_process_directory_entry(file_name)) {
        return AssertionSuccess();
    }
    
    char *endptr;
    auto result = strtol(file_name, &endptr, 10);
    return result >= 0 && *endptr == '\0' ? AssertionSuccess()
                            : (AssertionFailure() << "Invalid file name: " << file_name);
}

static AssertionResult
check_proc_files_names_are_valid() {
    return iterate_all_files("/", check_proc_file_name);
}

// Checks that the non-special case files in the root directory are all
// directories with the correct permissions.
static AssertionResult
check_proc_file_properties(const char * const file_name) {
    if (non_process_directory_entry(file_name)) {
        return AssertionSuccess();
    }
    
    string full_path(ROOTPATH + "/" + file_name);
    
    struct stat st;
    int error = lstat(full_path.c_str(), &st);
    if (error != 0) {
        return AssertionFailure() << "Failed to lstat() " << full_path;
    }
    
    if ((st.st_mode & S_IFMT) != S_IFDIR) {
        return AssertionFailure() << "Incorrect file type for " << full_path << ": " << (st.st_mode & S_IFMT) << " not S_IFDIR";
    }
    
    if ((st.st_mode & ALLPERMS) != 0550) {
        return AssertionFailure() << "Incorrect permission for " << full_path << ": " << (st.st_mode & ALLPERMS) << " not 0550";
    }
    return AssertionSuccess();
}


static AssertionResult
check_proc_files_properties_are_valid() {
    return iterate_all_files("/", check_proc_file_properties);
}

