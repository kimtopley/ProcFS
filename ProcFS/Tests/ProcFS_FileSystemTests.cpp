//
//  ProcFS_FileSystemTests.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/16/16.
//
//  Procfs file system tests.
//
#include <sys/mount.h>
#include <gtest/gtest.h>
#include "ProcFS_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

using namespace std;

// File system block size.
#define BLOCK_SIZE 4096

// File system id - hard coded in the kernel.
#define PROCFS_FS_ID 21

TEST_F(ProcFSTestFixture, CheckStatFS) {
    // Check that statfs(2) returns correct values.
    struct statfs fs;
    int error = statfs(get_mounted_on_path(), &fs);
    ASSERT_EQ(0, error) << "statfs failed";
    
    // Not much to check - most fields are not valid for procfs
    ASSERT_STREQ("proc", fs.f_mntfromname) << "Mount from name incorrect: " << fs.f_mntfromname;
    ASSERT_EQ(BLOCK_SIZE, fs.f_bsize) << "File system block size incorrect: " << fs.f_bsize;
    ASSERT_EQ(BLOCK_SIZE, fs.f_iosize) << "File system I/O size incorrect: " << fs.f_iosize;
    ASSERT_EQ(PROCFS_FS_ID, fs.f_fsid.val[1]) << "Fle system id incorrect: " << fs.f_fsid.val[1];
}
