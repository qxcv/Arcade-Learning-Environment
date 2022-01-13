// Support for AFL
#ifndef __ALE_AFL_HPP__
#define __ALE_AFL_HPP__

#ifdef ALE_AFL_SUPPORT
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include "../AFL/config.h"

namespace ale {
  class AFLIntegration {
  public:
    AFLIntegration() {
      aflAreaPtr = aflMapRegion();
      aflInstRMS = aflGetRMS();
    }

    /* taken from afl_maybe_log, which is in AFL's afl-qemu-cpu-inl.h */
    inline void maybeLog(uint16_t PC) {
      if (!aflAreaPtr)
        return;

      /* Mangling addresses to make them quasi-uniform even the compiler aligned
        them. (I don't think there was any reason for compilers to do this on a
        6502, so I took it out. Should log addresses to verify my assumption.) */
      // PC  = (PC >> 4) ^ (PC << 8);
      PC &= MAP_SIZE - 1;

      /* Implement probabilistic instrumentation by looking at scrambled block
        address. This keeps the instrumented locations stable across runs. */
      if (PC >= aflInstRMS) return;

      aflAreaPtr[PC ^ lastPC]++;
      lastPC = PC >> 1;
    }

    bool launchForkServer() {
      // Tell parent we're alive. Note that FORKSRV_FD is already open because
      // `afl-fuzz` opened it before fork().
      static unsigned char tmp[4];
      if (write(FORKSRV_FD + 1, tmp, 4) != 4) {
        std::cerr << "Parent write failed, no AFL fork server." << std::endl;
        return false;
      };

      /* All right, let's await orders... */
      while (1) {
        /* Whoops, parent dead? */
        if (read(FORKSRV_FD, tmp, 4) != 4) {
          std::cerr << "AFL parent silent, aborting fork server" << std::endl;
          break;
        }

        pid_t childPID = fork();
        if (childPID < 0) {
          if (errno) {
            perror("fork");
          }
          std::cerr << "AFL parent silent, aborting fork server" << std::endl;
          break;
        }

        if (!childPID) {
          /* Child process. Close descriptors and run free. */
          /* afl_fork_child = 1; (I believe this was for deugging purposes in
                                  afl-qemu-cpu-inl.h) */
          close(FORKSRV_FD);
          close(FORKSRV_FD + 1);
          return true;
        }

        if (write(FORKSRV_FD + 1, &childPID, 4) != 4) {
          std::cerr << "Couldn't send child PID to AFL, aborting" << std::endl;
          break;
        }

        /* Get and relay exit status to parent. */
        int status;
        if (waitpid(childPID, &status, 0) < 0) {
          std::cerr << "Child wait failed, forkserver aborting" << std::endl;
          break;
        }
        if (write(FORKSRV_FD + 1, &status, 4) != 4) {
          std::cerr << "Couldn't send status to AFL, aborting" << std::endl;
          break;
        }
      }
      // we only get here on error
      return false;
    }

  private:
    char *aflAreaPtr = NULL;
    uint16_t lastPC = 0;
    int aflInstRMS = 0;

    static int aflGetRMS() {
      // Determine what fraction of branches get instrumented. Copied from
      // afl_setup in afl-qemu-cpu-inl.h.
      char *instR = getenv("AFL_INSTRATIO");
      unsigned int r = 100;
      if (instR) {
        r = atoi(instR);
      } else if (errno) {
        // don't bail out, just log error
        perror("getenv");
      }
      if (r > 100) r = 100;
      if (!r) r = 1;
      return MAP_SIZE * r / 100;
    }

    static char *aflMapRegion() {
      char *shmIdStr = getenv(SHM_ENV_VAR);
      if (!shmIdStr) {
        if (errno) {
          perror("getenv");
        } else {
          fprintf(stderr, "no environment variable '%s'\n", SHM_ENV_VAR);
        }
        return NULL;
      }
      int shmId = atoi(shmIdStr);
      if (!shmId) {
        fprintf(stderr, "could not convert %s='%s' to integer\n", SHM_ENV_VAR, shmIdStr);
        return NULL;
      }
      char *shmBlock = (char*)shmat(shmId, NULL, 0);
      if (shmBlock == (char*)-1) {
        perror("shmat");
        return NULL;
      }
      return shmBlock;
    }
  };
} // namespace ale

#endif // ALE_AFL_SUPPORT
#endif // __ALE_AFL_HPP__
