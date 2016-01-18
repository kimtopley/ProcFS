//
//  ProcFS_ProcessDirTests.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/16/16.
//
//  Tests for process directories (/proc/NNN).
//
#include <gtest/gtest.h>
#include <sys/proc_info.h>
#include "ProcFS_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

using namespace std;
using namespace testing;

// Checks that the reported size of a process directory is the
// same as the number of objects in that directory. Uses the
// current process directory because it's guaranteed not to disappear.
TEST_F(ProcFSTestFixture, CheckProcessDirectorySize) {
    auto dir_path = current_process_directory_path();
    ASSERT_EQ(file_size(dir_path), count_directory_entries(dir_path)) << "Invalid directory size for " << dir_path;
}

// Checks that each process directory has the correct set of
// subdirectories. Uses the current process directory because it's
// guaranteed not to disappear.
TEST_F(ProcFSTestFixture, CheckProcessSubdirectories) {
    auto dir_path = current_process_directory_path();
    EXPECT_TRUE(check_directory_contains(dir_path,
                    vector<string>({"fd", "info", "pgid",
                                    "pid", "ppid", "sid", "taskinfo",
                                    "threads", "tty"}),
                    false));
}

// Verifies the content of the "pid" file for a process.
TEST_F(ProcFSTestFixture, CheckPidFileContent) {
    auto dir_path = current_process_directory_path();
    
    pid_t pid;
    ASSERT_TRUE(read_file_content(dir_path + "/pid", &pid, sizeof(pid))) << "Failed to read 'pid' file content";
    ASSERT_EQ(getpid(), pid) << "Incorrect process id in 'pid' file";
}

// Verifies the content of the "ppid" file for a process.
TEST_F(ProcFSTestFixture, CheckPpidFileContent) {
    auto dir_path = current_process_directory_path();
    
    pid_t ppid;
    ASSERT_TRUE(read_file_content(dir_path + "/ppid", &ppid, sizeof(ppid))) << "Failed to read 'ppid' file content";
    ASSERT_EQ(getppid(), ppid) << "Incorrect process id in 'ppid' file";
}

// Verifies the content of the "pgid" file for a process.
TEST_F(ProcFSTestFixture, CheckPgidFileContent) {
    auto dir_path = current_process_directory_path();
    
    pid_t pgid;
    ASSERT_TRUE(read_file_content(dir_path + "/pgid", &pgid, sizeof(pgid))) << "Failed to read 'pgid' file content";
    ASSERT_EQ(getpgid(getpid()), pgid) << "Incorrect process id in 'pgid' file";
}

// Verifies the content of the "sid" file for a process.
TEST_F(ProcFSTestFixture, CheckSidFileContent) {
    auto dir_path = current_process_directory_path();
    
    pid_t sid;
    ASSERT_TRUE(read_file_content(dir_path + "/sid", &sid, sizeof(sid))) << "Failed to read 'sid' file content";
    ASSERT_EQ(getsid(getpid()), sid) << "Incorrect process id in 'sid' file";
}

// Verifies the content of the "tty" file for a process.
TEST_F(ProcFSTestFixture, CheckTtyFileContent) {
    auto dir_path = current_process_directory_path();
    
    bool error;
    string tty = read_file(dir_path + "/tty", error);
    ASSERT_FALSE(error) << "Failed to read the 'tty' file";
    
    // The tty field is empty of the process does not have a controlling
    // terminal. If it's not empty, it must contain the name of a file
    // that exists.
    if (tty.empty()) {
        SUCCEED();
    } else {
        ASSERT_TRUE(check_absolute_file_exists(tty));
    }
}

// Verifies the content of the "info" file for a process.
TEST_F(ProcFSTestFixture, CheckInfoFileContent) {
    auto dir_path = current_process_directory_path();
    
    // Check that we get a structure of the correct size.
    proc_bsdinfo info;
    ASSERT_TRUE(read_file_content(dir_path + "/info", &info, sizeof(info))) << "Failed to read 'info' file content";
    
    // Check a few fields.
    ASSERT_EQ(getpid(), info.pbi_pid) << "Incorrect process id in 'info' file";
    ASSERT_EQ(getppid(), info.pbi_ppid) << "Incorrect parent process id in 'info' file";
    ASSERT_EQ(geteuid(), info.pbi_uid) << "Incorrect uid in 'info' file";
    ASSERT_EQ(getegid(), info.pbi_gid) << "Incorrect gid in 'info' file";
}

// Verifies the content of the "taskinfo" file for a process.
TEST_F(ProcFSTestFixture, CheckTaskInfoFileContent) {
    auto dir_path = current_process_directory_path();
    
    // Check that we get a structure of the correct size.
    proc_taskinfo taskinfo;
    ASSERT_TRUE(read_file_content(dir_path + "/taskinfo", &taskinfo, sizeof(taskinfo))) << "Failed to read 'taskinfo' file content";
    
    // Not much we can check . Just do some sanity checks.
    ASSERT_TRUE(taskinfo.pti_virtual_size > 0) << "unlikely virtual size value";
    ASSERT_TRUE(taskinfo.pti_resident_size > 0) << "unlikely resident size value";
    ASSERT_TRUE(taskinfo.pti_virtual_size >= taskinfo.pti_resident_size) << "unlikely virtual/resident size values";
    ASSERT_TRUE(taskinfo.pti_threads_user > 0) << "unlikely pti_threads_user value";
    ASSERT_TRUE(taskinfo.pti_syscalls_unix > 0) << "unlikely pti_syscalls_unix value";
}
