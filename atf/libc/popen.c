/*	$NetBSD: popen.c,v 1.4 2008/07/21 14:33:31 lukem Exp $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Matthias Scheler.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT("@(#) Copyright (c) 1999\
 The NetBSD Foundation, Inc.  All rights reserved.");
#endif /* not lint */

#ifndef lint
__RCSID("$NetBSD: popen.c,v 1.4 2008/07/21 14:33:31 lukem Exp $");
#endif /* not lint */

#include <sys/param.h>

#include <atf-c.h>
#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define _PATH_CAT	"/bin/cat"
#define BUFSIZE		(640 * 1024)
			/* 640KB ought to be enough for everyone. */
#define DATAFILE	"popen.data"

/* Test case -- popen() */
ATF_TC(test_popen);
ATF_TC_HEAD(test_popen, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the popen(3) function");
}
ATF_TC_BODY(test_popen, tc)
{
        char *buffer, command[MAXPATHLEN];
        int index, in;
        FILE *pipe;

        /* Allocate memory for buffer */
        if ((buffer = malloc(BUFSIZE)) == NULL)
                atf_tc_fail("malloc() couldn't allocate %u bytes of memory",
                            BUFSIZE);
        /* err(1, NULL); */

    /* Initialize random number generator */
    srand((unsigned int)time(NULL));

    /* Populate buffer with random data */
    for (index = 0 ; index < BUFSIZE; index++)
        buffer[index] = (char)rand();

    /*
     * Construct the shell command line, e.g:
     * /bin/cat > popen.data
     */
    (void)snprintf(command, sizeof(command), "%s > %s",
                   _PATH_CAT, DATAFILE);

    /* Pipe magic begins here */
    if ((pipe = popen(command, "w")) == NULL)
        atf_tc_fail("popen() failed to open pipe in write mode");
       /* err(1, "popen write"); */

    if (fwrite(buffer, 1, BUFSIZE, pipe) != BUFSIZE)
        atf_tc_fail("fwrite() failed to write to pipe");
        /* err(1, "write"); */

    if (pclose(pipe) == -1)
        atf_tc_fail("pclose() failed to close pipe");
        /* err(1, "pclose"); */

    /* */
    (void)snprintf(command, sizeof(command), "%s %s",
                   _PATH_CAT, DATAFILE);
    if ((pipe = popen(command, "r")) == NULL)
        atf_tc_fail("popen read");
        /* err(1, "popen read"); */

    index = 0;
    while ((in = fgetc(pipe)) != EOF)
        if (index == BUFSIZE) {
            errno = EFBIG;
            atf_tc_fail("read");
            /* err(1, "read"); */
        }
        else
            if ((char)in != buffer[index++]) {
                errno = EINVAL;
                atf_tc_fail("read");
                /* err(1, "read"); */
            }

    if (index < BUFSIZE) {
        errno = EIO;
        atf_tc_fail("read");
        /* err(1, "read"); */
    }

    if (pclose(pipe) == -1)
        atf_tc_fail("pclose");
        /* err(1, "pclose"); */

    (void)unlink(DATAFILE);
}

/* Add test case to test program */
ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, test_popen);

    return atf_no_error();
}
