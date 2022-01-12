/* Program that pretends to be AFL-instrumented. Logic:

   1. Get SHM segment ID from appropriate env var (maybe need to include
      config.h or something)
   2. Map the SHM segment.
   3. Write some junk to the segment to make parent think we're instrumented
      (not sure whether this is required; try leaving it out first).
   4. Maybe some forkserver mumbo jumbo, IDK.
   5. Cleanup!
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* use `gcc -DNO_FAKE_AFL` to turn off fake AFL layer */
#ifndef NO_FAKE_AFL
  #define FAKE_AFL
#endif

#ifdef FAKE_AFL
#include "config.h"
/* This forkserver logic adapted from afl-qemu-cpu-inl.h. I removed the logic
   that mirrors translation requests in the parent. */
static void afl_forkserver() {
  /* Tell the parent that we're alive. If the parent doesn't want
     to talk, assume that we're not running in forkserver mode. */
  static unsigned char tmp[4];
  /* Note that FORKSRV_FD is already open because `afl-fuzz` opened it before
     fork(). FORKSRV_FD is a constant defined in config.h, so we can compile it
     into child programs without worry. Also, tmp is never used in this program,
     it's just a dummy 4-byte message that we can send across the pipe to signal
     our readiness to the parent. All messages that this program sends or
     receives are 4 bytes. */
  if (write(FORKSRV_FD + 1, tmp, 4) != 4) return;

  int afl_forksrv_pid = getpid();

  /* All right, let's await orders... */
  while (1) {
    /* Whoops, parent dead? */
    if (read(FORKSRV_FD, tmp, 4) != 4) exit(2);

    pid_t child_pid = fork();
    if (child_pid < 0) exit(4);

    if (!child_pid) {
      /* Child process. Close descriptors and run free. */
      /* afl_fork_child = 1; (I believe this was for deugging purposes in
                              afl-qemu-cpu-inl.h) */
      close(FORKSRV_FD);
      close(FORKSRV_FD + 1);
      return;
    }

    if (write(FORKSRV_FD + 1, &child_pid, 4) != 4) exit(5);

    /* Get and relay exit status to parent. */
    int status;
    if (waitpid(child_pid, &status, 0) < 0) exit(6);
    if (write(FORKSRV_FD + 1, &status, 4) != 4) exit(7);
  }
}

#endif /* end FAKE_AFL */

char *afl_map_region() {
  char *shm_id_str = getenv(SHM_ENV_VAR);
  if (!shm_id_str) {
    if (errno) {
      perror("getenv");
    } else {
      fprintf(stderr, "no environment variable '%s'\n", SHM_ENV_VAR);
    }
    exit(8);
  }
  int shm_id = atoi(shm_id_str);
  if (!shm_id) {
    fprintf(stderr, "could not convert %s='%s' to integer\n", SHM_ENV_VAR, shm_id_str);
    exit(9);
  }
  char *shm_block = shmat(shm_id, NULL, 0);
  if (shm_block == (char*)-1) {
    perror("shmat");
    exit(10);
  }
  return shm_block;
}

int main(int argc, char **argv) {

#ifdef FAKE_AFL

  printf("Pretending to be AFL program.\n");

  /* first map region that we need to write to */
  char *shm_block = afl_map_region();

  /* Now launch a fork server. If we're the parent in the forkserver, then this
     never returns. If we're the child, then this returns & we can continue
     executing. */
  afl_forkserver();

  /* If we're the child then we let the input completely control the contents of
     shm_block. This isn't meant to do anything useful, it's just meant to make
     AFL happy that it's finding lots of new control paths. */
  char input[1024];
  ssize_t read_len = 0;
  uint64_t loc = 0;
  while ((read_len = read(0, &input, 1024)) > 0) {
    for (int i = 0; i < read_len; ++i) {
      loc <<= 4;
      loc |= input[i];
      shm_block[loc & (MAP_SIZE - 1)] += 1;
    }
  }

#else /* end FAKE_AFL */

  printf("Not pretending to be AFL program, so AFL should complain.\n")

#endif /* end !FAKE_AFL */

  printf("All done.\n");

  return 0;
}
