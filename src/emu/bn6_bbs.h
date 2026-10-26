/* BN6's Request BBS, the board on AsterLand's wall (bn6f reqBBS.s; docs/rom/world.md,
 * The Request BBS): what its screen lists and reads, and the flags it keeps. Its
 * screen unpacks the requests' titles and posts (LZ77 each, through two literals)
 * as it opens, lists the requests posted newest first, reads a post picked, asks
 * "Take this request?" and takes one at a time. */
#ifndef CW_BN6_BBS_H
#define CW_BN6_BBS_H

#define BN6_BBS_TITLES_PTR    0x08141158u /* the word the titles' archive is read through, LZ77 (bn6f off_813F378) */
#define BN6_BBS_POSTS_PTR     0x0814115Cu /* ... and the posts' */
#define BN6_BBS_TITLES        0x087ED244u /* BN6's titles (script n: request n's, 14 letters) */
#define BN6_BBS_POSTS         0x087ED504u /* BN6's posts (script n: request n's) */
#define BN6_BBS_LIST          0x02000290u /* the requests posted, a byte each in the order posted (0x80 bytes) */
#define BN6_BBS_COUNT         0x020065B0u /* u32 how many */
#define BN6_BBS_SEEN          0x02000770u /* u32 the new posts counted as the BBS was last left */
#define BN6_BBS_STATE         0x02005780u /* the screen's state (bn6f eReqBBSGui), and in it: */
#define BN6_BBS_STATE_TAKEN   0x0F        /* ... u8 the request taken, while one is */
#define BN6_BBS_REQUESTS      48          /* the requests its flags count */
#define BN6_FLAG_BBS_POSTED   0x1B60      /* + request: on the board */
#define BN6_FLAG_BBS_NEW      0x1BA0      /* + request: not read yet */
#define BN6_FLAG_BBS_TAKEN    0x1BE0      /* + request: taken ("Take this request?", Yes) */
#define BN6_FLAG_BBS_FINISHED 0x1C20      /* + request: finished ("already finished!") */
#define BN6_FLAG_BBS_DONE     0x1C60      /* + request: done for its client, finished as the board next opens */
#define BN6_FLAG_BBS_HOLDING  0x173A      /* a request taken and not finished: "You can only pick one request at a time." */

#endif
