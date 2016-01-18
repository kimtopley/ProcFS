//
//  ProcFS_TestFixture.hpp
//  ProcFS
//
//  Created by Kim Topley on 1/15/16.
//
//

#ifndef ProcFS_TestFixture_hpp
#define ProcFS_TestFixture_hpp

#include "gtest/gtest.h"

/*
 * A fixture class that ensures that the procfs
 * file system is mounted before any tests are run.
 */
class ProcFSTestFixture : public testing::Test {
protected:
    // Override setup to perform check for mounted procfs.
    virtual void SetUp() override;
};

#endif /* ProcFS_TestFixture_hpp */
