/* second_mail.h. BN6's E-Mail lists each mail's subject and sender, four
 * at a time, and opens a mail in its box with the face its body sets up.
 * As BN5 DS reads a mail with its sender's face (issue #72's research),
 * the PET beside it shows the mail under the cursor, or open: its sender's
 * face (BN6's mugshot, Dad's for the lab's mails; a board has none) and
 * name, its subject, and how many of the whole list's mails are new. */
#include "second_mail.h"

#include "second_frame.h"
#include "second_home.h"
#include "second_state.h"
#include "second_text.h"

void second_mail_draw(SDL_Rect b) {
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
	if (!S2.pet_ready) return;
	int x = b.x + 6, y = b.y + 6;
	if (S2.mail_id < 0) {
		second_wrapped(second_mail_none(), x, y, b.w - 12, 2, PET_CYAN_HI);
		return;
	}
	int tx = x;
	if (S2.mail_face >= 0) {
		second_mugshot(S2.mail_face, x, y);
		tx = x + SECOND_FACE_W + 10;
	}
	text_draw_scaled(tx, y, S2.mail_from, PET_GOLD, TEXT_LEFT, 2);
	SecondRow r[2];
	int n = second_mail_rows(r, 2, S2.mail_subject, S2.mail_unread, S2.mail_shown);
	for (int i = 0; i < n; ++i) second_row(tx, y + 30 + i * SECOND_ROW_H, b.x + b.w - 6 - tx, r[i].name, r[i].value);
}
