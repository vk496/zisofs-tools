#ident "$Id$"
/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 2001-2006 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * workers.c
 *
 * Parallel job maintenance
 */

#include "mkzftree.h" /* Must be included first! */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

/* Global option */
extern int parallel; /* Number of parallel jobs */

/* Functions related to parallel execution */
static volatile int work_threads = 0;
static int is_worker = 0;

/* This waits for one worker to finish */
static void wait_for_one_worker(void)
{
  int status;

  if (wait(&status) > 0)
  {
    work_threads--;

    if (WIFSIGNALED(status))
    {
      kill(getpid(), WTERMSIG(status));
      exit(EX_SOFTWARE);
    }
    else if (WEXITSTATUS(status))
    {
      exit(WEXITSTATUS(status));
    }
  }
}

/* This waits for *all* workers to finish */
void wait_for_all_workers(void)
{
  while (work_threads)
    wait_for_one_worker();
}

/* This returns 1 if the "job" at hand should be performed */
int spawn_worker(void)
{
  pid_t f;

  if (opt.parallel == 0)
    return 1;

  fflush(NULL);

  /* Wait for a work slot */
  while (work_threads >= opt.parallel)
    wait_for_one_worker();

  /* Spawn worker process */
  work_threads++; /* Avoids race conditions */
  f = fork();
  if (f == -1)
  {
    work_threads--;
    return 1; /* Do it ourselves */
  }

  if (f == 0)
  {
    /* Worker process */
    is_worker = 1;
    return 1;
  }
  else
  {
    /* Control process */
    return 0;
  }
}

/* Routine to perform at the end of the job */
void end_worker(int err)
{
  if (is_worker)
  {
    exit(err);
  }
}
