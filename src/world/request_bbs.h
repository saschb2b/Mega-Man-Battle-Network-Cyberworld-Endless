/* The visit's requests on BN6's own Request BBS, the board on AsterLand's
 * wall (request_bbs.c, docs/world/home/jobs.md). */
#ifndef CW_REQUEST_BBS_H
#define CW_REQUEST_BBS_H

/* The askers' requests as the visit has them (job_talk) posted on the
 * board: each one offered, and the one held, BN6's request k asker k's.
 * At each visit, and again as a request is taken or settled. */
void request_bbs_post(void);
/* The asker whose post the board just took for flag `flag` set (BN6's
 * "Take this request?", Yes), -1 where it is none of theirs. */
int request_bbs_taken(int flag);

#endif
