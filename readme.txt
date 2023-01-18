=====a65=====

This project is essentially me modifying William Colley's "a65" assembler
for my weird niche purposes. If you came across this repo because you have an
existing codebase that depends on a65 v0.1, don't use this because it'll break
your workflow.

=====Why not to use this assembler=====
- The code has that "old UNIX program" stank to it, where you have a lot of
  buffers with hard-coded sizes and if you exceed them the program will
  (hopefully) crash without telling you why. See a65.h if you want to fiddle
  with the buffer sizes.
- It's fairly opinionated regarding syntax. It doesn't support a ton of 6502
  "dialects" like all the popular assemblers do
- It's an absolute assembler, so no linking
- No macro support
- I'm like 90% sure that it won't like Unicode characters (but who knows, it
  might...)

=====Why to use this assembler=====
- The code is well laid out and easy to understand and modify in case you want
  to fork it and use it for your own weird niche purposes.
- It'll basically run on anything with a C compiler (although I took out the
  K&R function declarations, so it'll have to be at least C89)
- It assembles stuff pretty fast on today's computers

=====Goals for release 0.2n=====
- Reformat code for ANSI style function declarations - done
- Change "FCB", "FCC", "FDB" to "DB", "DS", "DW" - done
- Change the syntax so "DB" accepts string(s) as arguments - done
- Change the syntax so labels can have colons after them - done
- Give each .c file its own header file - done
- Change the assembler to output binary files instead of Intel HEX - done
- Allow for forcing loads/stores to not be optimized for zero page - done (! operator)
- Add a "MSG" statement that prints out arbitrary strings and symbols
- Print the error messages to stdout instead of the assembler listing - done
- Add a makefile for our GNU/Linux and Mac OS X using friends - done
- Update A65.DOC with the new changes

=====Future goals=====
- Add a "BASE" statement that changes the PC but not the output position in
  the file

=====License=====

Copyright (c) 1986 William C. Colley, III

This package may be used for any commercial or non-commercial purpose.  It may
be copied and distributed freely provided that any fee charged by the
distributor of the copy does not exceed the sum of:
1) the cost of the media the copy is written on,
2) any required costs of shipping the copy, and
3) a nominal handling fee.  
Any other distribution requires the written permission of the author.  Also, the
author's copyright notices shall not be removed from the program source, the
program object, or the program documentation.