//
//  ProcFS_TestFixture.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/15/16.
//
//

#include "ProcFS_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

// Overrides setup to ensure that procfs is mounted.
// All subclasses must call this method if they
//override SetUp() themselves.
void
ProcFSTestFixture::SetUp() {
    EXPECT_TRUE(check_procfs_mounted(get_mounted_on_path()));
}


