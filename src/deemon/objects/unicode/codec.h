/* Copyright (c) 2019 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CODEC_H
#define GUARD_DEEMON_OBJECTS_UNICODE_CODEC_H 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/string.h>

DECL_BEGIN

/*[[[deemon
import List from deemon;
local codecs = List {
	("ascii\0",            "CODEC_ASCII"),
	("646\0",              "CODEC_ASCII"),
	("us-ascii\0",         "CODEC_ASCII"),
	("latin-1\0",          "CODEC_LATIN1"),
	("iso8859-1\0",        "CODEC_LATIN1"),
	("iso8859\0",          "CODEC_LATIN1"),
	("8859\0",             "CODEC_LATIN1"),
	("cp819\0",            "CODEC_LATIN1"),
	("latin\0",            "CODEC_LATIN1"),
	("latin1\0",           "CODEC_LATIN1"),
	("l1\0",               "CODEC_LATIN1"),
	("utf-8\0",            "CODEC_UTF8"),
	("utf8\0",             "CODEC_UTF8"),
	("u8\0",               "CODEC_UTF8"),
	("utf\0",              "CODEC_UTF8"),
	("utf-16\0",           "CODEC_UTF16"),
	("utf16\0",            "CODEC_UTF16"),
	("u16\0",              "CODEC_UTF16"),
	("utf-16-le\0",        "CODEC_UTF16_LE"),
	("utf16-le\0",         "CODEC_UTF16_LE"),
	("u16-le\0",           "CODEC_UTF16_LE"),
	("utf-16le\0",         "CODEC_UTF16_LE"),
	("utf16le\0",          "CODEC_UTF16_LE"),
	("u16le\0",            "CODEC_UTF16_LE"),
	("utf-16-be\0",        "CODEC_UTF16_BE"),
	("utf16-be\0",         "CODEC_UTF16_BE"),
	("u16-be\0",           "CODEC_UTF16_BE"),
	("utf-16be\0",         "CODEC_UTF16_BE"),
	("utf16be\0",          "CODEC_UTF16_BE"),
	("u16be\0",            "CODEC_UTF16_BE"),
	("utf-32\0",           "CODEC_UTF32"),
	("utf32\0",            "CODEC_UTF32"),
	("u32\0",              "CODEC_UTF32"),
	("utf-32-le\0",        "CODEC_UTF32_LE"),
	("utf32-le\0",         "CODEC_UTF32_LE"),
	("u32-le\0",           "CODEC_UTF32_LE"),
	("utf-32le\0",         "CODEC_UTF32_LE"),
	("utf32le\0",          "CODEC_UTF32_LE"),
	("u32le\0",            "CODEC_UTF32_LE"),
	("utf-32-be\0",        "CODEC_UTF32_BE"),
	("utf32-be\0",         "CODEC_UTF32_BE"),
	("u32-be\0",           "CODEC_UTF32_BE"),
	("utf-32be\0",         "CODEC_UTF32_BE"),
	("utf32be\0",          "CODEC_UTF32_BE"),
	("u32be\0",            "CODEC_UTF32_BE"),
	("string-escape\0",    "CODEC_C_ESCAPE"),
	("backslash-escape\0", "CODEC_C_ESCAPE"),
	("c-escape\0",         "CODEC_C_ESCAPE"),
};
codecs.sort([](x) -> x[0]);

local inset = "\t";

function gen_switch(prefix, indent) {
	import HashSet from deemon;
	local cases = [];
	local suffix_chars = HashSet();
	for (local x: codecs) {
		if (x[0] == prefix) {
			print indent,;
			print x[1],;
			print "; /" "* ",;
			print repr x[0].rstrip("\0"),;
			print " *" "/ \\";
			return;
		}
		if (!x[0].startswith(prefix))
			continue;
		suffix_chars.insert(x[0][#prefix]);
		cases.append(x);
	}
	if (#suffix_chars <= 5) { // Use IF
		local prev_ch = none;
		for (local name, target: cases) {
			local ch = name[#prefix];
			if (prev_ch is none || prev_ch != ch) {
				if (prev_ch !is none) {
					print "else ",;
				} else {
					print indent,;
				}
				prev_ch = ch;
				if (#cases == 1) {
					local suffix = name[#prefix:];
					if (suffix.endswith("\0"))
						suffix = suffix[:-1];
					if (#suffix <= 1) goto do_as_if;
					print "if (memcmp(name + ",;
					print #prefix,;
					print ", ",;
					print repr suffix,;
					print ", ",;
					print #suffix,;
					print "*sizeof(char)",;
					print ") == 0) { \\";
					print indent,;
					print inset,;
					print target,;
					print "; /" "* ",;
					print repr name.rstrip("\0"),;
					print " *" "/ \\";
					print indent,;
					print "} ",;
				} else {
do_as_if:
					print "if ((uint8_t)name[",;
					print #prefix,;
					print "] == ",;
					print ch.ord(),;
					print ") { /" "* ",;
					print repr ch,;
					print " *" "/ \\";
					gen_switch(prefix + ch, indent + inset);
					print indent,;
					print "} ",;
				}
			}
		}
		if (prev_ch !is none)
			print "\\";
		return;
	}
	print indent,;
	print "switch ((uint8_t)name[",;
	print #prefix,;
	print "]) { \\";
	local prev_ch = none;
	for (local name, target: cases) {
		local ch = name[#prefix];
		if (prev_ch is none || prev_ch != ch) {
			if (prev_ch !is none) {
				print indent,;
				print inset,;
				print "break; \\";
			}
			prev_ch = ch;
			print indent,;
			print "case ",;
			print ch.ord(),;
			print ": /" "* ",;
			print repr ch,;
			print " *" "/ \\";
			gen_switch(prefix + ch, indent + inset);
		}
	}
	print indent,;
	print inset,;
	print "break; \\";
	print indent,;
	print "default: break; \\";
	print indent,;
	print "} \\";
    
}
local target_names = [];
for (local alias, target: codecs) {
	if (target !in target_names)
		target_names.append(target);
}
target_names.sort();

print "#ifdef __INTELLISENSE__";
print "#define SWITCH_BUILTIN_CODECS(name \\";
for (local x: target_names) {
	print inset,;
	print ", ",;
	print x,;
	print " \\";
}
print ") {  }";
print "#else";
print "#define SWITCH_BUILTIN_CODECS(name \\";
for (local x: target_names) {
	print inset,;
	print ", ",;
	print x,;
	print " \\";
}
print ") { \\";
gen_switch("", inset);
print "}";
print "#endif";
]]]*/
#ifdef __INTELLISENSE__
#define SWITCH_BUILTIN_CODECS(name \
	, CODEC_ASCII \
	, CODEC_C_ESCAPE \
	, CODEC_LATIN1 \
	, CODEC_UTF16 \
	, CODEC_UTF16_BE \
	, CODEC_UTF16_LE \
	, CODEC_UTF32 \
	, CODEC_UTF32_BE \
	, CODEC_UTF32_LE \
	, CODEC_UTF8 \
) {  }
#else
#define SWITCH_BUILTIN_CODECS(name \
	, CODEC_ASCII \
	, CODEC_C_ESCAPE \
	, CODEC_LATIN1 \
	, CODEC_UTF16 \
	, CODEC_UTF16_BE \
	, CODEC_UTF16_LE \
	, CODEC_UTF32 \
	, CODEC_UTF32_BE \
	, CODEC_UTF32_LE \
	, CODEC_UTF8 \
) { \
	switch ((uint8_t)name[0]) { \
	case 54: /* "6" */ \
		if (memcmp(name + 1, "46", 2*sizeof(char)) == 0) { \
			CODEC_ASCII; /* "646" */ \
		} \
		break; \
	case 56: /* "8" */ \
		if (memcmp(name + 1, "859", 3*sizeof(char)) == 0) { \
			CODEC_LATIN1; /* "8859" */ \
		} \
		break; \
	case 97: /* "a" */ \
		if (memcmp(name + 1, "scii", 4*sizeof(char)) == 0) { \
			CODEC_ASCII; /* "ascii" */ \
		} \
		break; \
	case 98: /* "b" */ \
		if (memcmp(name + 1, "ackslash-escape", 15*sizeof(char)) == 0) { \
			CODEC_C_ESCAPE; /* "backslash-escape" */ \
		} \
		break; \
	case 99: /* "c" */ \
		if ((uint8_t)name[1] == 45) { /* "-" */ \
			if (memcmp(name + 2, "escape", 6*sizeof(char)) == 0) { \
				CODEC_C_ESCAPE; /* "c-escape" */ \
			} \
		} else if ((uint8_t)name[1] == 112) { /* "p" */ \
			if (memcmp(name + 2, "819", 3*sizeof(char)) == 0) { \
				CODEC_LATIN1; /* "cp819" */ \
			} \
		} \
		break; \
	case 105: /* "i" */ \
		if ((uint8_t)name[1] == 115) { /* "s" */ \
			if ((uint8_t)name[2] == 111) { /* "o" */ \
				if ((uint8_t)name[3] == 56) { /* "8" */ \
					if ((uint8_t)name[4] == 56) { /* "8" */ \
						if ((uint8_t)name[5] == 53) { /* "5" */ \
							if ((uint8_t)name[6] == 57) { /* "9" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_LATIN1; /* "iso8859" */ \
								} else if ((uint8_t)name[7] == 45) { /* "-" */ \
									if ((uint8_t)name[8] == 49) { /* "1" */ \
										if ((uint8_t)name[9] == 0) { /* "\0" */ \
											CODEC_LATIN1; /* "iso8859-1" */ \
										} \
									} \
								} \
							} \
						} \
					} \
				} \
			} \
		} \
		break; \
	case 108: /* "l" */ \
		if ((uint8_t)name[1] == 49) { /* "1" */ \
			if ((uint8_t)name[2] == 0) { /* "\0" */ \
				CODEC_LATIN1; /* "l1" */ \
			} \
		} else if ((uint8_t)name[1] == 97) { /* "a" */ \
			if ((uint8_t)name[2] == 116) { /* "t" */ \
				if ((uint8_t)name[3] == 105) { /* "i" */ \
					if ((uint8_t)name[4] == 110) { /* "n" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_LATIN1; /* "latin" */ \
						} else if ((uint8_t)name[5] == 45) { /* "-" */ \
							if ((uint8_t)name[6] == 49) { /* "1" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_LATIN1; /* "latin-1" */ \
								} \
							} \
						} else if ((uint8_t)name[5] == 49) { /* "1" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_LATIN1; /* "latin1" */ \
							} \
						} \
					} \
				} \
			} \
		} \
		break; \
	case 115: /* "s" */ \
		if (memcmp(name + 1, "tring-escape", 12*sizeof(char)) == 0) { \
			CODEC_C_ESCAPE; /* "string-escape" */ \
		} \
		break; \
	case 117: /* "u" */ \
		if ((uint8_t)name[1] == 49) { /* "1" */ \
			if ((uint8_t)name[2] == 54) { /* "6" */ \
				if ((uint8_t)name[3] == 0) { /* "\0" */ \
					CODEC_UTF16; /* "u16" */ \
				} else if ((uint8_t)name[3] == 45) { /* "-" */ \
					if ((uint8_t)name[4] == 98) { /* "b" */ \
						if ((uint8_t)name[5] == 101) { /* "e" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF16_BE; /* "u16-be" */ \
							} \
						} \
					} else if ((uint8_t)name[4] == 108) { /* "l" */ \
						if ((uint8_t)name[5] == 101) { /* "e" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF16_LE; /* "u16-le" */ \
							} \
						} \
					} \
				} else if ((uint8_t)name[3] == 98) { /* "b" */ \
					if ((uint8_t)name[4] == 101) { /* "e" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF16_BE; /* "u16be" */ \
						} \
					} \
				} else if ((uint8_t)name[3] == 108) { /* "l" */ \
					if ((uint8_t)name[4] == 101) { /* "e" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF16_LE; /* "u16le" */ \
						} \
					} \
				} \
			} \
		} else if ((uint8_t)name[1] == 51) { /* "3" */ \
			if ((uint8_t)name[2] == 50) { /* "2" */ \
				if ((uint8_t)name[3] == 0) { /* "\0" */ \
					CODEC_UTF32; /* "u32" */ \
				} else if ((uint8_t)name[3] == 45) { /* "-" */ \
					if ((uint8_t)name[4] == 98) { /* "b" */ \
						if ((uint8_t)name[5] == 101) { /* "e" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF32_BE; /* "u32-be" */ \
							} \
						} \
					} else if ((uint8_t)name[4] == 108) { /* "l" */ \
						if ((uint8_t)name[5] == 101) { /* "e" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF32_LE; /* "u32-le" */ \
							} \
						} \
					} \
				} else if ((uint8_t)name[3] == 98) { /* "b" */ \
					if ((uint8_t)name[4] == 101) { /* "e" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF32_BE; /* "u32be" */ \
						} \
					} \
				} else if ((uint8_t)name[3] == 108) { /* "l" */ \
					if ((uint8_t)name[4] == 101) { /* "e" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF32_LE; /* "u32le" */ \
						} \
					} \
				} \
			} \
		} else if ((uint8_t)name[1] == 56) { /* "8" */ \
			if ((uint8_t)name[2] == 0) { /* "\0" */ \
				CODEC_UTF8; /* "u8" */ \
			} \
		} else if ((uint8_t)name[1] == 115) { /* "s" */ \
			if (memcmp(name + 2, "-ascii", 6*sizeof(char)) == 0) { \
				CODEC_ASCII; /* "us-ascii" */ \
			} \
		} else if ((uint8_t)name[1] == 116) { /* "t" */ \
			if ((uint8_t)name[2] == 102) { /* "f" */ \
				if ((uint8_t)name[3] == 0) { /* "\0" */ \
					CODEC_UTF8; /* "utf" */ \
				} else if ((uint8_t)name[3] == 45) { /* "-" */ \
					if ((uint8_t)name[4] == 49) { /* "1" */ \
						if ((uint8_t)name[5] == 54) { /* "6" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF16; /* "utf-16" */ \
							} else if ((uint8_t)name[6] == 45) { /* "-" */ \
								if ((uint8_t)name[7] == 98) { /* "b" */ \
									if ((uint8_t)name[8] == 101) { /* "e" */ \
										if ((uint8_t)name[9] == 0) { /* "\0" */ \
											CODEC_UTF16_BE; /* "utf-16-be" */ \
										} \
									} \
								} else if ((uint8_t)name[7] == 108) { /* "l" */ \
									if ((uint8_t)name[8] == 101) { /* "e" */ \
										if ((uint8_t)name[9] == 0) { /* "\0" */ \
											CODEC_UTF16_LE; /* "utf-16-le" */ \
										} \
									} \
								} \
							} else if ((uint8_t)name[6] == 98) { /* "b" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF16_BE; /* "utf-16be" */ \
									} \
								} \
							} else if ((uint8_t)name[6] == 108) { /* "l" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF16_LE; /* "utf-16le" */ \
									} \
								} \
							} \
						} \
					} else if ((uint8_t)name[4] == 51) { /* "3" */ \
						if ((uint8_t)name[5] == 50) { /* "2" */ \
							if ((uint8_t)name[6] == 0) { /* "\0" */ \
								CODEC_UTF32; /* "utf-32" */ \
							} else if ((uint8_t)name[6] == 45) { /* "-" */ \
								if ((uint8_t)name[7] == 98) { /* "b" */ \
									if ((uint8_t)name[8] == 101) { /* "e" */ \
										if ((uint8_t)name[9] == 0) { /* "\0" */ \
											CODEC_UTF32_BE; /* "utf-32-be" */ \
										} \
									} \
								} else if ((uint8_t)name[7] == 108) { /* "l" */ \
									if ((uint8_t)name[8] == 101) { /* "e" */ \
										if ((uint8_t)name[9] == 0) { /* "\0" */ \
											CODEC_UTF32_LE; /* "utf-32-le" */ \
										} \
									} \
								} \
							} else if ((uint8_t)name[6] == 98) { /* "b" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF32_BE; /* "utf-32be" */ \
									} \
								} \
							} else if ((uint8_t)name[6] == 108) { /* "l" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF32_LE; /* "utf-32le" */ \
									} \
								} \
							} \
						} \
					} else if ((uint8_t)name[4] == 56) { /* "8" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF8; /* "utf-8" */ \
						} \
					} \
				} else if ((uint8_t)name[3] == 49) { /* "1" */ \
					if ((uint8_t)name[4] == 54) { /* "6" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF16; /* "utf16" */ \
						} else if ((uint8_t)name[5] == 45) { /* "-" */ \
							if ((uint8_t)name[6] == 98) { /* "b" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF16_BE; /* "utf16-be" */ \
									} \
								} \
							} else if ((uint8_t)name[6] == 108) { /* "l" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF16_LE; /* "utf16-le" */ \
									} \
								} \
							} \
						} else if ((uint8_t)name[5] == 98) { /* "b" */ \
							if ((uint8_t)name[6] == 101) { /* "e" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_UTF16_BE; /* "utf16be" */ \
								} \
							} \
						} else if ((uint8_t)name[5] == 108) { /* "l" */ \
							if ((uint8_t)name[6] == 101) { /* "e" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_UTF16_LE; /* "utf16le" */ \
								} \
							} \
						} \
					} \
				} else if ((uint8_t)name[3] == 51) { /* "3" */ \
					if ((uint8_t)name[4] == 50) { /* "2" */ \
						if ((uint8_t)name[5] == 0) { /* "\0" */ \
							CODEC_UTF32; /* "utf32" */ \
						} else if ((uint8_t)name[5] == 45) { /* "-" */ \
							if ((uint8_t)name[6] == 98) { /* "b" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF32_BE; /* "utf32-be" */ \
									} \
								} \
							} else if ((uint8_t)name[6] == 108) { /* "l" */ \
								if ((uint8_t)name[7] == 101) { /* "e" */ \
									if ((uint8_t)name[8] == 0) { /* "\0" */ \
										CODEC_UTF32_LE; /* "utf32-le" */ \
									} \
								} \
							} \
						} else if ((uint8_t)name[5] == 98) { /* "b" */ \
							if ((uint8_t)name[6] == 101) { /* "e" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_UTF32_BE; /* "utf32be" */ \
								} \
							} \
						} else if ((uint8_t)name[5] == 108) { /* "l" */ \
							if ((uint8_t)name[6] == 101) { /* "e" */ \
								if ((uint8_t)name[7] == 0) { /* "\0" */ \
									CODEC_UTF32_LE; /* "utf32le" */ \
								} \
							} \
						} \
					} \
				} else if ((uint8_t)name[3] == 56) { /* "8" */ \
					if ((uint8_t)name[4] == 0) { /* "\0" */ \
						CODEC_UTF8; /* "utf8" */ \
					} \
				} \
			} \
		} \
		break; \
	default: break; \
	} \
}
#endif
//[[[end]]]


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CODEC_H */
