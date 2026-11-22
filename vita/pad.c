/* Room for vita-elf-create (the Makefile's Vita block): 8 KB of read-only
 * data, linked in only where the code ends too close to the data's 64 KB
 * boundary for the module data that tool writes between them. */
const char cw_vita_pad[0x2000] = { 1 };
