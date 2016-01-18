# ProcFS
An implementation of the /proc file system for OS X

## What is procfs?
*procfs* lets you view the processes running on a UNIX system as nodes in the file system, where each process is represented by a single directory named from its process id. Typically, the file system is mounted at `/proc`, so the directory for process 1 would be called `/proc/1`. Beneath a process’ directory are further directories and files that give more information about the process, such as its process id, its active threads, the files that it has open, and so on. *procfs* first appeared in an early version of AT&T’s UNIX and was later implemented in various forms in System V, BSD, Solaris and Linux. You can find a history of the implementation of *procfs* at https://en.wikipedia.org/wiki/Procfs.

In addition to letting you visualize running processes, *procfs* also allows some measure of control over them, at least to suitably privileged users. By writing specific data structures to certain files, you could do such things as set breakpoints and read and write process memory and registers. In fact, on some systems, this was how debugging facilities were provided. However, more modern operating systems do this differently, so some UNIX variants no longer include an implementation of *procfs*. In particular, OS X doesn’t provide *procfs* so, although it’s not strictly needed, I thought that implementing it would be an interesting side project. The code in this repository provides a very basic implementation of *procfs* for OS X. You can use it to see what processes and threads are running on the system and what files they have open. Later, I plan to add more features, such as the ability to inspect a thread’s address space to see which executable it is running and what shared libraries it has loaded. 

If you install *procfs* on your system, mount it at `/proc` and take a look at it with Finder, you’ll see a hierarchy of files that looks something like this:

![ProcFS in Finder](ProcFS_Finder.png)

Each directory in the left column represents one process on the system. By default you can only see your own processes, although it is possible to set an option when mounting the file system that will let you see every process. Obviously this is a security risk, so it’s not the default mode of operation. Within each process directory are seven files and two further directories, shown in the second column of the screenshot. All of the files can be read in the normal way, but the data that they contain is not text, so they are really intended to be used in applications rather than for direct human consumption. The following table summarizes what’s in each file. You’ll find definitions of the structures in this table in the file */usr/include/sys/proc_info.h*.

| File    | Summary                          | Structure                     |
|---------|----------------------------------|-------------------------------|
|pid      | Process id                       | pid_t                         |
|ppid     | Parent process id                | pid_t                         |
|pgid     | Process group id                 | pid_t                         |
|sid      | Session id                       | pid_t                         |
|tty      | Controlling tty                  | string, such as `/dev/tty000` |
|info     | Basic process info               | struct proc_bsdinfo           |
|taskinfo | Info for the process’s Mach task | struct proc_taskinfo          |

The `fd` directory contains one entry for each file that the process has open. Each entry is a directory that’s numbered for the corresponding file descriptor. Most processes will have at least entries 0, 1 and 2 for standard input, output and error respectively. Within each subdirectory you’ll find two files called `details` and `socket`. The `details` file contains a `vnode_fdinfowithpath` structure, which contains information about the file including its path name if it is a file system file. If the file is a socket endpoint, you can read a `socket_fdinfo` structure from the `socket` file.

The `threads` directory contains a subdirectory for each of the process’ threads. The process in the screenshot above has two threads with ids 550 and 1284. Each thread directory contains a single file called `info` the contains thread-specific information in the form of a `proc_threadinfo` structure.

## Installing procfs for OS X


## Why isn’t procfs a Kernel Extension?

For the first few weeks of development, *procfs* was implemented as a kernel extension. Developing s kernel extensions is much easier than adding code to the kernel itself because an extension can be installed, loaded for testing, unloaded and then replaced without rebooting—provided, of course, that the extension doesn’t cause a kernel panic or a lock up. 


## Testing the procfs Implementation