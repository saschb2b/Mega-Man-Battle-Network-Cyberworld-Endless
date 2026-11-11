The knowledge base's log, entries waiting to be folded in (docs/project/merging.md, The log).

A change to docs/ logs itself here, not in docs/log.md: a file DATE-WORDS.txt
(2026-11-12-merging-in-parallel.txt) that says who writes and when, then the
log's line as it is to stand:

by: claude-code/claude-fable-5-2
at: 2026-11-12T09:30:00Z
* **Update**: project/merging.md (what changed, in a clause), code/tools.md (its row).

dev's workflow folds each file into docs/log.md under its date, writes
`generated: { by, at }` on each concept the line names (a path ending .md
outside parentheses), and removes the file (tools/log_fold.py). No branch
edits docs/log.md, nor the `generated` line of a concept that exists.
