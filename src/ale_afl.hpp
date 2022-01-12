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

      afl_area_ptr[PC ^ lastPC]++;
      lastPC = PC >> 1;
    }

  private:
    char *aflAreaPtr = NULL;
    uint16_t lastPC = 0;
    int aflInstRMS = 0;

    static int aflGetRMS() {
      // Determine what fraction of branches get instrumented. Copied from
      // afl_setup in afl-qemu-cpu-inl.h.
      char *instR = getenv("AFL_INSTRATIO");
      int r = 100;
      if (instR) {
        unsigned int r;
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
        goto err;
      }
      int shmId = atoi(shmIdStr);
      if (!shmId) {
        fprintf(stderr, "could not convert %s='%s' to integer\n", SHM_ENV_VAR, shmIdStr);
        goto err;
      }
      char *shmBlock = shmat(shmId, NULL, 0);
      if (shmBlock == (char*)-1) {
        perror("shmat");
        goto err;
      }
      return shmBlock;
    err:
      // error message for all the above failure cases
      // TODO(sam): convert this into an exception or something
      fprintf(stderr, "AFL disabled due to error (see above)");
      return NULL;
    }
  };
} // namespace ale

#endif // ALE_AFL_SUPPORT
#endif // __ALE_AFL_HPP__
