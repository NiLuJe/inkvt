#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "tracexec.hpp"

std::string get_cmdline(pid_t pid) {
    // /proc/{pid}/cmdline: arguments separated by \0's, and ends with an extra \0
    std::ostringstream path_cmdline;
    path_cmdline << "/proc/" << (int)pid << "/cmdline";
    std::ifstream f(path_cmdline.str());
    std::string cmdline((std::istreambuf_iterator<char>(f)),
                       (std::istreambuf_iterator<char>()));
    if (cmdline.length() == 0) {
        return cmdline; // in case of a zombie
    }
    std::replace(cmdline.begin(), cmdline.end(), (char)0, ' ');
    cmdline.resize(cmdline.length() - 1);
    return cmdline;
}

struct RivalApp {
    pid_t pid;
    std::vector<int> fds = { };
    std::string cmdline;

    tracexec te;

    void takeover() {
        long err;
        te.pid = pid;
        kill(pid, SIGSTOP);
        try_ptrace(PTRACE_SEIZE, pid, 0, 0);
        waitpid(pid, 0, 0);
        try_ptrace(PTRACE_GETREGS, pid, 0, &te.reset_regs);
        for (int fd : fds) {
            err = te.ioctl(fd, EVIOCGRAB, 0);
        }
    }

    void give_back_control() {
        long err;
        for (int fd : fds) {
            int nevents = 0;
            int fsflags = te.fcntl(fd, F_GETFL, 0);
            if (!(fsflags & O_NONBLOCK)) {
                err = te.fcntl(fd, F_SETFL, fsflags | O_NONBLOCK);
            }
            while(te.read(fd, (void*)(SP(te.regs)-2048), sizeof(struct input_event)) > 0) {
                nevents += 1;
            }
            printf("flushed %d events\n", nevents);
            fcntl(fd, F_SETFL, fsflags);
            // err = te.ioctl(fd, EVIOCGRAB, 1); // we should only grab the onces back which we ungrabbed
        }
        try_ptrace(PTRACE_SETREGS, pid, 0, &te.reset_regs);
        try_ptrace(PTRACE_DETACH, pid, 0, SIGCONT);
    }

    static std::vector<RivalApp> search(std::vector<const char*> targets = { "/dev/input/event0", "/dev/input/event1", "/dev/input/event2" }) {
        char buf[1024];
        glob_t pglob = { 0 };
        glob("/proc/*/fd/*", GLOB_NOSORT, 0, &pglob);
        std::vector<RivalApp> procs;
        for (unsigned int i = 0; i < pglob.gl_pathc; i++) {
            ssize_t nbuf = readlink(pglob.gl_pathv[i], buf, sizeof(buf)-1);
            buf[nbuf] = 0;
            bool found = 0;
            for (const char * target : targets) {
                if (strcmp(buf, target) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) continue;
            (void)strtok(pglob.gl_pathv[i], "/"); // "proc"
            char * str_pid = strtok(0, "/");
            (void)strtok(0, "/"); // "fd"
            char * str_fd = strtok(0, "/");
            char * endptr;
            long pid = strtol(str_pid, &endptr, 10);
            if (*endptr) continue; /* pid is not an integer, like "self" */
            long fd = strtol(str_fd, &endptr, 10);
            if (*endptr) continue; /* fd is not an integer. when would this ever happen? */
            decltype(procs)::iterator it = std::find_if(procs.begin(), procs.end(), [&pid](const RivalApp & x) {
                return pid == x.pid;
            });
            if (it == procs.end()) {
                RivalApp proc;
                proc.pid = pid;
                proc.fds.push_back(fd);
                proc.cmdline = get_cmdline(pid);
                procs.push_back(proc);
            } else {
                it->fds.push_back(fd);
            }
        }
        globfree(&pglob);
        return procs;
    }
};