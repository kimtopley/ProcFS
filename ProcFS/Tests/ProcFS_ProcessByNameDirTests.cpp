//
//  ProcFS_ProcessByNameDirTests.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/16/16.
//
//  Tests for the /proc/byname directory.
//
#include <gtest/gtest.h>
#include "ProcFS_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

using namespace std;
using namespace testing;

static AssertionResult check_byname_subdirectory(const char * const file_name);

// Checks the content of the /proc/byname directory.
TEST_F(ProcFSTestFixture, CheckProcByNameContent) {
    auto result = iterate_all_files("byname", check_byname_subdirectory);
    EXPECT_TRUE(result);
}

// Checks one entry in /proc/byname. It must be a symlink with modes 0777,
// its name must start with a number followed by a space and its content
// must be "../NNN" where NNN is the numeric part of its name.
static AssertionResult
check_byname_subdirectory(const char * const file_name) {
    if (is_special_directory_entry(file_name)) {
        // "." and ".." are OK.
        return AssertionSuccess();
    }
    
    // Any other kind of file must be a symlink with mode 0777.
    string full_name = string("byname/") + file_name;
    AssertionResult result = check_type_and_permissions(full_name, S_IFLNK, 0777);
    if (result) {
        // The path name must start with a number. We can't check that the number is
        // the pid for an existing process, because that process may exit at any time.
        int digits = 0;
        char next = ' ';
        ssize_t length = strlen(file_name);
        for (int i = 0; i < length; i++) {
            next = file_name[i];
            if (!isdigit(next)) {
                break;
            }
            digits++;
        }
        
        // We must have at least one digit, and the following character
        // must be a space.
        if (digits == 0 || next != ' ') {
            result = AssertionFailure() << "byname entry " << file_name << " is not valid";
        }
        
        // The symnlink must point to "../NNN" where NNN is the number that we got
        // from the name.
        string expected_content = string("../");
        for (int i = 0; i < digits; i++) {
            expected_content += file_name[i];
        }

        string symlink_path(ROOTPATH + "/" + full_name);
        char buf[PATH_MAX];
        length = readlink(symlink_path.c_str(), buf, sizeof(buf));
        buf[length] = '\0';
        string actual_content = string(buf);
        if (actual_content != expected_content) {
            return AssertionFailure() << "byname symlink content should be '" << expected_content
                                      << "' but is '" << actual_content;
        }
    }
    return result;
}

