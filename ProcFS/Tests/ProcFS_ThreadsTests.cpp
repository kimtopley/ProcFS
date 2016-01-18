//
//  ProcFS_ThreadsTests.cpp
//  ProcFS
//
//  Created by Kim Topley on 1/17/16.
//
//  Tests for the /proc/NNN/threads directory.
//
#include <gtest/gtest.h>
#include <dirent.h>
#include <sys/proc_info.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ProcFS_TestFixture.hpp"
#include "ProcFS_TestHelpers.hpp"

using namespace std;

// Test fixture that creates some threads at setup time
// and terminates them at tear down.
class ProcFSThreadTestFixture : public ProcFSTestFixture {
public:
    // Override the SetUp() method to create some threads.
    virtual void SetUp() override {
        // Ensure that the file system is mounted.
       // ProcFSTestFixture::SetUp();
        
        // Create the threads. They do nothing but wait
        // to be told to terminate, which will happen
        // when the test is complete.
        for (int i = 0; i < thread_count(); i++) {
            threads.push_back(thread([this] {
                // Increment count of running threads and signal.
                unique_lock<mutex> lock(start_m);
                started_threads++;
                start_cv.notify_all();
                lock.unlock();
                
                // Wait for the condition variable to be signalled.
                while (!terminate) {
                    unique_lock<mutex> lock(end_m);
                    if (!terminate) {
                        stop_cv.wait(lock);
                    }
                    lock.unlock();
                }
            }));
        }
        
        // Wait for all threads to start.
        while (started_threads != thread_count()) {
            unique_lock<mutex> lock(start_m);
            if (started_threads != thread_count()) {
                start_cv.wait(lock);
                lock.unlock();
            }
        }
    }
    
    // Override the TearDown() method to terminate all threads.
    virtual void TearDown() override {
        // Signal all threads to terminate and wait until
        // they have all done so.
        unique_lock<mutex> lock(end_m);
        terminate = true;
        stop_cv.notify_all();
        lock.unlock();
        
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
    
    // Gets the number of threads that are created.
    int thread_count() {
        return 10;
    }
    
private:
    mutex start_m;
    mutex end_m;
    condition_variable start_cv;
    condition_variable stop_cv;
    atomic<int> started_threads{0};
    atomic<bool> terminate;
    vector<thread> threads;
};

TEST_F(ProcFSThreadTestFixture, CheckThreadDirectoryEntryCount) {
    // The test setup creates threads in addition to the
    // current one. Check that the threads directory has
    // an entry for each thread, plus the usual "." and
    // "..".
    auto proc_dir_path = current_process_directory_path();
    int count = 0;
    string dir_path(ROOTPATH + "/" + proc_dir_path + "/threads");
    DIR *dir = opendir(dir_path.c_str());
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (!is_special_directory_entry(entry->d_name)) {
                // The thread name must be numeric. Fail if not.
                char *endp;
                long result = strtol(entry->d_name, &endp, 10);
                if (result == 0 || *endp != '\0') {
                    closedir(dir);
                    FAIL() << "Invalid directory name: " << entry->d_name;
                }
                count++;
            }
        }
        closedir(dir);

        ASSERT_EQ(thread_count() + 1, count) << "Wrong number of directories: " << count << " instead of " << (thread_count() + 1);
    } else {
        FAIL() << "Unable to open threads directory " << dir_path;
    }
}

TEST_F(ProcFSThreadTestFixture, CheckThreadDirectoryEntries) {
    // Read the "info" entry for each thread. Check that is has
    // the correct size. We don't look at the content because there
    // isn't anything we can meaningfully check for an exact value.
    auto proc_dir_path = current_process_directory_path();
    string dir_path(ROOTPATH + "/" + proc_dir_path + "/threads");
    DIR *dir = opendir(dir_path.c_str());
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (!is_special_directory_entry(entry->d_name)) {
                struct proc_threadinfo info;
                string info_path = proc_dir_path + "/threads/" + entry->d_name + "/info";
                if (!read_file_content(info_path, &info, sizeof(info))) {
                    closedir(dir);
                    FAIL() << "Incorrect size of thread info";
                }
            }
        }
        closedir(dir);
    } else {
        FAIL() << "Unable to open threads directory " << dir_path;
    }
}