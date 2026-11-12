/* visitors.h: a family's role by its placements, and the telegraph. */
#include "visitors.h"

#include "visitor_families.h"

int visitor_role(int front, int middle, int back) {
	int n = front + middle + back;
	if (n <= 0) return VISITOR_FREE;
	if (4 * back >= 3 * n) return VISITOR_BACK;
	return 2 * front > n ? VISITOR_FRONT : VISITOR_FREE;
}

int visitor_telegraph(int home, int net) {
	bool older = net == VISIT_BN5 || net == VISIT_BN4;
	if (home == VISIT_BN6) return older ? VISITOR_OLDER_NETS : VISITOR_UNSAID;
	return home == VISIT_BN5 && net == VISIT_BN6 ? VISITOR_OURS : VISITOR_UNSAID;
}
