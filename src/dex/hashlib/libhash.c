/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HASHLIB_LIBHASH_C
#define GUARD_DEX_HASHLIB_LIBHASH_C 1
#define CONFIG_BUILDING_LIBHASH 1
#define DEE_SOURCE 1

#include "libhash.h"
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>

DECL_BEGIN


/*[[[deemon
import * from deemon;
import * from fs;
import util;
local algo_folder = "algorithms";

function DEEMON_REFLECT(x, n_bits) {
	local xt = x.class;
	local result = (xt)0;
	for (local i = 0; i < (n_bits); ++i) {
		if (x & (xt(1) << i))
			result |= xt(1) << (((n_bits) - 1) - i);
	}
	return result;
}


// function DEEMON_GENERATE_CRC(int width, Integer polynom, bool reflect_input) -> list
//    Returns a 256 element list of crc values, based on a given polynom
function DEEMON_GENERATE_CRC(width, polynom, reflect_input) {
	local width_mask = 1 << (width - 1);
	local type_mask  = 1;
	for (local i = 1; i < width; ++i) {
		type_mask <<= 1;
		type_mask |= 0x1;
	}
	local hash_width = width;
	if (hash_width > 32)
		hash_width = 64;
	else if (hash_width > 16)
		hash_width = 32;
	else if (hash_width > 8)
		hash_width = 16;
	else {
		hash_width = 8;
	}
	local crc_shift = 0;
	if (width < 8)
		crc_shift = 8 - width;
	for (local i = 0; i < 256; ++i) {
		local reg = reflect_input ? DEEMON_REFLECT(i, 8) : i;
		reg <<= (width - 8) + crc_shift;
		for (local j = 0; j < 8; ++j) {
			if (reg & (width_mask << crc_shift)) {
				reg = (reg << 1) ^ (polynom << crc_shift);
			} else {
				reg <<= 1;
			}
		}
		if (reflect_input) {
			reg >>= crc_shift;
			reg = DEEMON_REFLECT(reg,width);
			reg <<= crc_shift;
		}
		reg >>= crc_shift;
		reg &= type_mask;
		reg <<= (hash_width - width);
		yield reg;
	}
}

try {
	mkdir(algo_folder);
} catch (...) {
}

crc_algos = [];

known_hashfuncs = Dict {};
function get_hashfunc(width, revin, revout) {
	local key = pack(width, revin, revout);
	if (key in known_hashfuncs)
		return known_hashfuncs[key];
	local name = "_hashimpl_" + width;
	if (revin && revout)
		name += "_ioref";
	else if (revout)
		name += "_oref";
	else if (revin) {
		name += "_iref";
	}
	print "#define WIDTH", width;
	print "#define IN_REFLECTED", revin ? "1" : "0";
	print "#define OUT_REFLECTED", revout ? "1" : "0";
	print "#include", repr "hashfunc.c.inl";
	print;
	known_hashfuncs[key] = name;
	return name;
}

function def_crc(names,width,poly,args...) {
	local init = 0x0;
	local refin = false;
	local refout = false;
	local xorout = 0x0;
	switch (#args) {
	case 4: xorout   = args[3]; // fallthrough
	case 3: refout   = args[2]; // fallthrough
	case 2: refin    = args[1]; // fallthrough
	case 1: init     = args[0]; // fallthrough
	default: break;
	}
	local size = (width / 8) + ((width % 8) ? 1 : 0);
	switch (size) {
	case 3: size = 4; break;
	case 5: case 6: case 7: size = 8; break;
	default: break;
	}
	local main_name = names.pop_front().upper();
	print File.stderr: "Calculating:", main_name;
	local algo_name = "dhash_" + main_name.lower().replace("-", "_").replace("/", "__");
	crc_algos.append(algo_name);
	local hashfunc = get_hashfunc(width, refin, refout);
	local alias_names = "NULL";
	if (names) {
		alias_names = algo_name+"_alias_names";
		print "PRIVATE char const *const", alias_names,;
		print "[] = {", ", ".join(for(local name: names) repr name.upper()) + ", NULL };";
	}
	print "PRIVATE struct dhashalgo" + (size * 8) + " const", algo_name, "= {";
#define CODEINT(x) (("UINT" + (size * 8) + "_C(%#." + (size*2) + "I" + (size * 8) + "x)") % (x))
//	local width_mask = 1;
//	for (local i = 1; i < width; ++i) {
//		width_mask <<= 1;
//		width_mask |= 0x1;
//	}
	print("\t{");
	print("\t\t", repr main_name, ",");
	print("\t\t", alias_names, ",");
	print("\t\t", width, ",");
	print("\t\t", size, ",");
	print("\t\t", "HASHALGO_FNORMAL", ",");
	print("\t\t{");
	print("\t\t\t(dhashfuncn_t)&", hashfunc);
	print("\t\t}");
	print("\t},");
	print("\t", CODEINT(init), ",");
	print("\t", CODEINT(xorout), ",");
	local algo_filename = joinpath(algo_folder, "algorithm." + main_name.replace("\\", "_").replace("/", "_") + ".c.inl");
	with (local algo_file = File.open(algo_filename,"w")) {
		print algo_file: "{";
		for (local i, x: util.enumerate(DEEMON_GENERATE_CRC(width, poly, refin))) {
			if ((i % 8) == 0)
				print algo_file: "\t",;
			print algo_file: CODEINT(x),;
			print algo_file: ",",;
			if ((i % 8) == 7) {
				print algo_file:;
			} else {
				print algo_file: " ",;
			}
		}
		print algo_file: "}";
	}
	print "#include",repr(algo_filename.replace("\\", "/"));
	print "};";
	print;
}

// Define all whole bunch of crc algorithms
def_crc(["CRC-1"],                     1, 0x01);
def_crc(["CRC-3/ROHC"],                3, 0x03, 0x7, true, true, 0x0);
def_crc(["CRC-4/INTERLAKEN"],          4, 0x03, 0xf, false, false, 0xf);
def_crc(["CRC-4/ITU"],                 4, 0x03, 0x0, true, true, 0x0);
def_crc(["CRC-5/EPC"],                 5, 0x09, 0x9, false, false, 0x00);
def_crc(["CRC-5/ITU"],                 5, 0x15, 0x0, true, true, 0x00);
def_crc(["CRC-5/USB"],                 5, 0x05, 0x1f, true, true, 0x1f);
def_crc(["CRC-6/CDMA2000-A"],          6, 0x27, 0x3f, false, false, 0x00);
def_crc(["CRC-6/CDMA2000-B"],          6, 0x07, 0x3f, false, false, 0x00);
def_crc(["CRC-6/DARC"],                6, 0x19, 0x00, true, true, 0x00);
def_crc(["CRC-6/ITU"],                 6, 0x03, 0x00, true, true, 0x00);
def_crc(["CRC-7"],                     7, 0x09, 0x00, false, false, 0x00);
def_crc(["CRC-7/ROHC"],                7, 0x4f, 0x7f, true, true, 0x00);
def_crc(["CRC-7/UMTS"],                7, 0x45, 0x00, false, false, 0x00);
def_crc(["CRC-8"],                     8, 0x07, 0x00, false, false, 0x00);
def_crc(["CRC-8/AUTOSAR"],             8, 0x2f, 0xff, false, false, 0xff);
def_crc(["CRC-8/CDMA2000"],            8, 0x9b, 0xff, false, false, 0x00);
def_crc(["CRC-8/DARC"],                8, 0x39, 0x00, true, true, 0x00);
def_crc(["CRC-8/DVB-S2"],              8, 0xd5, 0x00, false, false, 0x00);
def_crc(["CRC-8/EBU","CRC-8/AES"],     8, 0x1d, 0xff, true, true, 0x00);
def_crc(["CRC-8/I-CODE"],              8, 0x1d, 0xfd, false, false, 0x00);
def_crc(["CRC-8/ITU"],                 8, 0x07, 0x00, false, false, 0x55);
def_crc(["CRC-8/LTE"],                 8, 0x9b, 0x00, false, false, 0x00);

// NOTE: 'CRC-8/DALLAS' was added because of it being mentioned here:
// >> https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
def_crc(["CRC-8/MAXIM", "DOW-CRC", "CRC-8/DALLAS"], 8, 0x31, 0x00, true, true, 0x00);

def_crc(["CRC-8/OPENSAFETY"],           8, 0x2f, 0x00, false, false, 0x00);
def_crc(["CRC-8/ROHC"],                 8, 0x07, 0xff, true, true, 0x00);
def_crc(["CRC-8/SAE-J1850"],            8, 0x1d, 0xff, false, false, 0xff);
def_crc(["CRC-8/WCDMA"],                8, 0x9b, 0x00, true, true, 0x00);
def_crc(["CRC-10"],                    10, 0x233, 0x000, false, false, 0x000);
def_crc(["CRC-10/CDMA2000"],           10, 0x3d9, 0x3ff, false, false, 0x000);
def_crc(["CRC-11"],                    11, 0x385, 0x01a, false, false, 0x000);
def_crc(["CRC-11/UMTS"],               11, 0x307, 0x000, false, false, 0x000);
def_crc(["CRC-12/CDMA2000"],           12, 0xf13, 0xfff, false, false, 0x000);
def_crc(["CRC-12/DECT","X-CRC-12"],    12, 0x80f, 0x000, false, false, 0x000);
def_crc(["CRC-12/UMTS","CRC-12/3GPP"], 12, 0x80f, 0x000, false, true, 0x000);
def_crc(["CRC-13/BBC"],                13, 0x1cf5, 0x0000, false, false, 0x0000);
def_crc(["CRC-14/DARC"],               14, 0x0805, 0x0000, true, true, 0x0000);
def_crc(["CRC-15"],                    15, 0x4599, 0x0000, false, false, 0x0000);
def_crc(["CRC-15/MPT1327"],            15, 0x6815, 0x0000, false, false, 0x0001);

// Added 'CRC-16/IBM' alias, as referenced here: https://users.ece.cmu.edu/~koopman/crc/crc16.html
def_crc(["ARC", "CRC-16", "CRC-IBM", "CRC-16/ARC", "CRC-16/LHA", "CRC-16/IBM"], 16, 0x8005, 0x0000, true, true, 0x0000);

// this was is named 'CRC-CCITT (0x1D0F)' on https://www.lammertbies.nl/comm/info/crc-calculation.html
//   >> With the crc-name compare function and the listed aliases, "CRC-CCITT (0x1D0F)" is covered
def_crc(["CRC-16/AUG-CCITT", "CRC-16/SPI-FUJITSU",
            "CRC-CCITT/0x1D0F", "CRC-CCITT/1D0F",
         "CRC-16/CCITT-0x1D0F", "CRC-16/CCITT-1D0F",
        ], 16, 0x1021, 0x1d0f, false, false, 0x0000);

def_crc(["CRC-16/BUYPASS", "CRC-16/VERIFONE", "CRC-16/UMTS"], 16, 0x8005, 0x0000, false, false, 0x0000);

// v This one is listed as 'CRC-CCITT (0xFFFF)' under https://www.lammertbies.nl/comm/info/crc-calculation.html
//   >> With the crc-name compare function and the listed aliases, "CRC-CCITT (0xFFFF)" is covered
def_crc(["CRC-16/CCITT-FALSE",
            "CRC-CCITT/0xFFFF",    "CRC-CCITT/FFFF",
         "CRC-16/CCITT-0xFFFF", "CRC-16/CCITT-FFFF",
        ], 16, 0x1021, 0xffff, false, false, 0x0000);

def_crc(["CRC-16/CDMA2000"],           16, 0xc867, 0xffff, false, false, 0x0000);
def_crc(["CRC-16/CMS"],                16, 0x8005, 0xffff, false, false, 0x0000);
def_crc(["CRC-16/DDS-110"],            16, 0x8005, 0x800d, false, false, 0x0000);
def_crc(["CRC-16/DECT-R", "R-CRC-16"], 16, 0x0589, 0x0000, false, false, 0x0001);
def_crc(["CRC-16/DECT-X", "X-CRC-16"], 16, 0x0589, 0x0000, false, false, 0x0000);

// v On https://www.lammertbies.nl/comm/info/crc-calculation.html this one was named "CRC-DNP"
//   Just like with 'KERMIT', it seemed like this one needed to get its bytes swapped.
//   EDIT: After writing test cases, it seems like that web-page has an endian problem...
def_crc(["CRC-16/DNP", "CRC-DNP"],     16, 0x3d65, 0x0000, true, true, 0xffff);

def_crc(["CRC-16/EN-13757"],          16, 0x3d65, 0x0000, false, false, 0xffff);
def_crc(["CRC-16/GENIBUS", "CRC-16/EPC", "CRC-16/I-CODE", "CRC-16/DARC"], 16, 0x1021, 0xffff, false, false, 0xffff);
def_crc(["CRC-16/LJ1200"],            16, 0x6f63, 0x0000, false, false, 0x0000);
def_crc(["CRC-16/MAXIM"],             16, 0x8005, 0x0000, true, true, 0xffff);
def_crc(["CRC-16/MCRF4XX"],           16, 0x1021, 0xffff, true, true, 0x0000);
def_crc(["CRC-16/OPENSAFETY-A"],      16, 0x5935, 0x0000, false, false, 0x0000);
def_crc(["CRC-16/OPENSAFETY-B"],      16, 0x755b, 0x0000, false, false, 0x0000);
def_crc(["CRC-16/PROFIBUS", "CRC-16/IEC-61158-2"], 16, 0x1dcf, 0xffff, false, false, 0xffff);

// v This one gave me quite the headache...
//   - While the best source for all the other algorithms was
//     http://reveng.sourceforge.net/crc-catalogue/ It has a wrong init hash listed.
//     But since that page tells you that it can't be trusted, I guess it's ok...
//   - After quite some extensive searching, I found the correct init hash here:
//     http://crcmod.sourceforge.net/crcmod.predefined.html
//   NOTE: The wrong version is commented out, but kept as a reminder.
//   NOTE: After looking at the numbers, I noticed that '0x554d
//         is '0xb2aa' when reading the bits in reverse...
//         Dunno why someone did that, though, but it might be 
//   EDIT: Thank goodness I noticed that bit-order, because as
//         it turns out, that bug is present in a bunch of algos.
assert DEEMON_REFLECT(0xb2aa, 16) == 0x554d; // Just so you know...
def_crc(["CRC-16/RIELLO"],            16, 0x1021, 0x554d, true, true, 0x0000);
//f_crc(["CRC-16/RIELLO"],            16, 0x1021, 0xb2aa, true, true, 0x0000);

def_crc(["CRC-16/T10-DIF"],           16, 0x8bb7, 0x0000, false, false, 0x0000);
def_crc(["CRC-16/TELEDISK"],          16, 0xa097, 0x0000, false, false, 0x0000);

// Again: Same incorrect bit-order as with 'CRC-16/RIELLO'
assert DEEMON_REFLECT(0x89ec, 16) == 0x3791; // Just so you know...
def_crc(["CRC-16/TMS37157"],          16, 0x1021, 0x3791, true, true, 0x0000);
//f_crc(["CRC-16/TMS37157"],          16, 0x1021, 0x89ec, true, true, 0x0000);

def_crc(["CRC-16/USB"],               16, 0x8005, 0xffff, true, true, 0xffff);

// After figuring out what the deal was with 'CRC-16/RIELLO', I tried the
// same thing with this one's intial value. And lookie-lookie: it worked
assert DEEMON_REFLECT(0xc6c6, 16) == 0x6363; // Just so you know...
def_crc(["CRC-A"],                    16, 0x1021, 0x6363, true, true, 0x0000);
//f_crc(["CRC-A"],                    16, 0x1021, 0xc6c6, true, true, 0x0000);

// v After tinkering with https://www.lammertbies.nl/comm/info/crc-calculation.html,
//   it seemed like this one needed to get its bytes swapped.
//   EDIT: After writing test cases, it seems like that web-page has an endian problem...
def_crc(["KERMIT", "CRC-16/CCITT", "CRC-16/CCITT-TRUE", "CRC-CCITT"], 16, 0x1021, 0x0000, true, true, 0x0000);

def_crc(["MODBUS"],                    16, 0x8005, 0xffff, true, true, 0x0000);
def_crc(["X-25", "CRC-16/IBM-SDLC", "CRC-16/ISO-HDLC", "CRC-B"], 16, 0x1021, 0xffff, true, true, 0xffff);
def_crc(["XMODEM", "ZMODEM", "CRC-16/ACORN", "CRC-16/LTE"], 16, 0x1021, 0x0000, false, false, 0x0000);
def_crc(["CRC-24", "CRC-24/OPENPGP"],   24, 0x864cfb, 0xb704ce, false, false, 0x000000);
def_crc(["CRC-24/BLE"],                24, 0x00065b, 0x555555, true, true, 0x000000);
def_crc(["CRC-24/FLEXRAY-A"],          24, 0x5d6dcb, 0xfedcba, false, false, 0x000000);
def_crc(["CRC-24/FLEXRAY-B"],          24, 0x5d6dcb, 0xabcdef, false, false, 0x000000);
def_crc(["CRC-24/INTERLAKEN"],         24, 0x328b63, 0xffffff, false, false, 0xffffff);
def_crc(["CRC-24/LTE-A"],              24, 0x864cfb, 0x000000, false, false, 0x000000);
def_crc(["CRC-24/LTE-B"],              24, 0x800063, 0x000000, false, false, 0x000000);
def_crc(["CRC-30/CDMA"],               30, 0x2030b9c7, 0x3fffffff, false, false, 0x3fffffff);
def_crc(["CRC-31/PHILIPS"],            31, 0x04c11db7, 0x7fffffff, false, false, 0x7fffffff);

// >> https://users.ece.cmu.edu/~koopman/crc/crc32.html (referenced this as 'CRC-32/IEEE-802.3')
// >> https://www.autosar.org/fileadmin/files/releases/4-2/software-architecture/safety-and-security/standard/AUTOSAR_SWS_CRCLibrary.pdf (contains freaking readable IEEE-802.3 specs on page 25/48)
def_crc(["CRC-32","CRC-32/ADCCP","PKZIP","CRC-32/IEEE-802.3",
         "IEEE-802.3"], 32, 0x04c11db7, 0xffffffff, true, true, 0xffffffff);

def_crc(["CRC-32/AUTOSAR"],            32, 0xf4acfb13, 0xffffffff, true, true, 0xffffffff);
def_crc(["CRC-32/BZIP2", "CRC-32/AAL5", "CRC-32/DECT-B", "B-CRC-32"], 32, 0x04c11db7, 0xffffffff, false, false, 0xffffffff);
def_crc(["CRC-32C"],                   32, 0x1edc6f41, 0xffffffff, true, true, 0xffffffff);
def_crc(["CRC-32D"],                   32, 0xa833982b, 0xffffffff, true, true, 0xffffffff);
def_crc(["CRC-32/MPEG-2"],             32, 0x04c11db7, 0xffffffff, false, false, 0x00000000);

// NOTE: The 'POSIX' alternate name was assigned because of the name given here:
//       >> http://crcmod.sourceforge.net/crcmod.predefined.html
def_crc(["CRC-32/POSIX", "POSIX"],      32, 0x04c11db7, 0x00000000, false, false, 0xffffffff);

def_crc(["CRC-32Q"],                   32, 0x814141ab, 0x00000000, false, false, 0x00000000);
def_crc(["JAMCRC"],                    32, 0x04c11db7, 0xffffffff, true, true, 0x00000000);
def_crc(["XFER"],                      32, 0x000000af, 0x00000000, false, false, 0x00000000);
def_crc(["CRC-40/GSM"],                40, 0x0004820009, 0x0000000000, false, false, 0xffffffffff);
def_crc(["CRC-64"],                    64, 0x42f0e1eba9ea3693, 0x0000000000000000, false, false, 0x0000000000000000);
def_crc(["CRC-64/WE"],                 64, 0x42f0e1eba9ea3693, 0xffffffffffffffff, false, false, 0xffffffffffffffff);
def_crc(["CRC-64/XZ"],                 64, 0x42f0e1eba9ea3693, 0xffffffffffffffff, true, true, 0xffffffffffffffff);
//def_crc(["CRC-82/DARC"],             82, 0x0308c0111011401440411, 0x000000000000000000000, true, true, 0x000000000000000000000);

// Some more unspecific crcs from here: http://crcmod.sourceforge.net/crcmod.predefined.html
// NOTE: These we fitted to match the reveng name format
//crc-64 	0x1000000000000001B 	True 	0x0000000000000000 	0x0000000000000000 	0x46A5A9388A5BEFFE
def_crc(["CRC-64/JONES"],             64,0xAD93D23594C935A9,0xffffffffffffffff, true, true,0x0000000000000000);

// Some more asorted CRC algorithms from across the internet

// >> https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
// >> https://github.com/culvertsoft/mgen/blob/master/mgen-api/src/main/java/se/culvertsoft/mgen/api/util/CRC64.java (code indicates reflection)
def_crc(["CRC-64/ECMA", "CRC-64/ECMA-182"], 64, 0x42f0e1eba9ea3693, 0x0000000000000000, true, true, 0x0000000000000000);

// >> https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
// >> http://de.slideshare.net/cloudflare/cloud-flare-jgc-bigo-meetup-rolling-hashes  (barely readable code example indicates reflection)
// >> https://users.ece.cmu.edu/~koopman/crc/crc64.html  (alternative name: 'FP-64')
def_crc(["CRC-64/ISO", "FP-64"],        64, 0x000000000000001b, 0x0000000000000000, true, true, 0x0000000000000000);




print;
print "/" "* NULL-terminated list of known hash algorithms *" "/";
print "PRIVATE struct dhashalgo const *const dhash_algorithms[] = {";
for (local algo: crc_algos) {
	print "\t&",;
	print algo,;
	print ".ha_base",;
	print ",";
}
print "\tNULL";
print "};";
]]]*/
#define WIDTH 1
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_1 = {
	{
		"CRC-1",
		NULL,
		1,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_1
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-1.c.inl"
};

#define WIDTH 3
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_3__rohc = {
	{
		"CRC-3/ROHC",
		NULL,
		3,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_3_ioref
		}
	},
	UINT8_C(0x07),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-3_ROHC.c.inl"
};

#define WIDTH 4
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_4__interlaken = {
	{
		"CRC-4/INTERLAKEN",
		NULL,
		4,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_4
		}
	},
	UINT8_C(0x0f),
	UINT8_C(0x0f),
#include "algorithms/algorithm.CRC-4_INTERLAKEN.c.inl"
};

#define WIDTH 4
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_4__itu = {
	{
		"CRC-4/ITU",
		NULL,
		4,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_4_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-4_ITU.c.inl"
};

#define WIDTH 5
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_5__epc = {
	{
		"CRC-5/EPC",
		NULL,
		5,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_5
		}
	},
	UINT8_C(0x09),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-5_EPC.c.inl"
};

#define WIDTH 5
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_5__itu = {
	{
		"CRC-5/ITU",
		NULL,
		5,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_5_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-5_ITU.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_5__usb = {
	{
		"CRC-5/USB",
		NULL,
		5,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_5_ioref
		}
	},
	UINT8_C(0x1f),
	UINT8_C(0x1f),
#include "algorithms/algorithm.CRC-5_USB.c.inl"
};

#define WIDTH 6
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_6__cdma2000_a = {
	{
		"CRC-6/CDMA2000-A",
		NULL,
		6,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_6
		}
	},
	UINT8_C(0x3f),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-6_CDMA2000-A.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_6__cdma2000_b = {
	{
		"CRC-6/CDMA2000-B",
		NULL,
		6,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_6
		}
	},
	UINT8_C(0x3f),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-6_CDMA2000-B.c.inl"
};

#define WIDTH 6
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_6__darc = {
	{
		"CRC-6/DARC",
		NULL,
		6,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_6_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-6_DARC.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_6__itu = {
	{
		"CRC-6/ITU",
		NULL,
		6,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_6_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-6_ITU.c.inl"
};

#define WIDTH 7
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_7 = {
	{
		"CRC-7",
		NULL,
		7,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_7
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-7.c.inl"
};

#define WIDTH 7
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_7__rohc = {
	{
		"CRC-7/ROHC",
		NULL,
		7,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_7_ioref
		}
	},
	UINT8_C(0x7f),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-7_ROHC.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_7__umts = {
	{
		"CRC-7/UMTS",
		NULL,
		7,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_7
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-7_UMTS.c.inl"
};

#define WIDTH 8
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_8 = {
	{
		"CRC-8",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__autosar = {
	{
		"CRC-8/AUTOSAR",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0xff),
	UINT8_C(0xff),
#include "algorithms/algorithm.CRC-8_AUTOSAR.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__cdma2000 = {
	{
		"CRC-8/CDMA2000",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0xff),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_CDMA2000.c.inl"
};

#define WIDTH 8
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo8 const dhash_crc_8__darc = {
	{
		"CRC-8/DARC",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_DARC.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__dvb_s2 = {
	{
		"CRC-8/DVB-S2",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_DVB-S2.c.inl"
};

PRIVATE char const *const dhash_crc_8__ebu_alias_names[] = { "CRC-8/AES", NULL };
PRIVATE struct dhashalgo8 const dhash_crc_8__ebu = {
	{
		"CRC-8/EBU",
		dhash_crc_8__ebu_alias_names,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8_ioref
		}
	},
	UINT8_C(0xff),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_EBU.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__i_code = {
	{
		"CRC-8/I-CODE",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0xfd),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_I-CODE.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__itu = {
	{
		"CRC-8/ITU",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x55),
#include "algorithms/algorithm.CRC-8_ITU.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__lte = {
	{
		"CRC-8/LTE",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_LTE.c.inl"
};

PRIVATE char const *const dhash_crc_8__maxim_alias_names[] = { "DOW-CRC", "CRC-8/DALLAS", NULL };
PRIVATE struct dhashalgo8 const dhash_crc_8__maxim = {
	{
		"CRC-8/MAXIM",
		dhash_crc_8__maxim_alias_names,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_MAXIM.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__opensafety = {
	{
		"CRC-8/OPENSAFETY",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_OPENSAFETY.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__rohc = {
	{
		"CRC-8/ROHC",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8_ioref
		}
	},
	UINT8_C(0xff),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_ROHC.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__sae_j1850 = {
	{
		"CRC-8/SAE-J1850",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8
		}
	},
	UINT8_C(0xff),
	UINT8_C(0xff),
#include "algorithms/algorithm.CRC-8_SAE-J1850.c.inl"
};

PRIVATE struct dhashalgo8 const dhash_crc_8__wcdma = {
	{
		"CRC-8/WCDMA",
		NULL,
		8,
		1,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_8_ioref
		}
	},
	UINT8_C(0x00),
	UINT8_C(0x00),
#include "algorithms/algorithm.CRC-8_WCDMA.c.inl"
};

#define WIDTH 10
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_10 = {
	{
		"CRC-10",
		NULL,
		10,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_10
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-10.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_10__cdma2000 = {
	{
		"CRC-10/CDMA2000",
		NULL,
		10,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_10
		}
	},
	UINT16_C(0x03ff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-10_CDMA2000.c.inl"
};

#define WIDTH 11
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_11 = {
	{
		"CRC-11",
		NULL,
		11,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_11
		}
	},
	UINT16_C(0x001a),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-11.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_11__umts = {
	{
		"CRC-11/UMTS",
		NULL,
		11,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_11
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-11_UMTS.c.inl"
};

#define WIDTH 12
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_12__cdma2000 = {
	{
		"CRC-12/CDMA2000",
		NULL,
		12,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_12
		}
	},
	UINT16_C(0x0fff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-12_CDMA2000.c.inl"
};

PRIVATE char const *const dhash_crc_12__dect_alias_names[] = { "X-CRC-12", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_12__dect = {
	{
		"CRC-12/DECT",
		dhash_crc_12__dect_alias_names,
		12,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_12
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-12_DECT.c.inl"
};

#define WIDTH 12
#define IN_REFLECTED 0
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_crc_12__umts_alias_names[] = { "CRC-12/3GPP", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_12__umts = {
	{
		"CRC-12/UMTS",
		dhash_crc_12__umts_alias_names,
		12,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_12_oref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-12_UMTS.c.inl"
};

#define WIDTH 13
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_13__bbc = {
	{
		"CRC-13/BBC",
		NULL,
		13,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_13
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-13_BBC.c.inl"
};

#define WIDTH 14
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_14__darc = {
	{
		"CRC-14/DARC",
		NULL,
		14,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_14_ioref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-14_DARC.c.inl"
};

#define WIDTH 15
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo16 const dhash_crc_15 = {
	{
		"CRC-15",
		NULL,
		15,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_15
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-15.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_15__mpt1327 = {
	{
		"CRC-15/MPT1327",
		NULL,
		15,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_15
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0001),
#include "algorithms/algorithm.CRC-15_MPT1327.c.inl"
};

#define WIDTH 16
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_arc_alias_names[] = { "CRC-16", "CRC-IBM", "CRC-16/ARC", "CRC-16/LHA", "CRC-16/IBM", NULL };
PRIVATE struct dhashalgo16 const dhash_arc = {
	{
		"ARC",
		dhash_arc_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.ARC.c.inl"
};

#define WIDTH 16
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_crc_16__aug_ccitt_alias_names[] = { "CRC-16/SPI-FUJITSU", "CRC-CCITT/0X1D0F", "CRC-CCITT/1D0F", "CRC-16/CCITT-0X1D0F", "CRC-16/CCITT-1D0F", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__aug_ccitt = {
	{
		"CRC-16/AUG-CCITT",
		dhash_crc_16__aug_ccitt_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x1d0f),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_AUG-CCITT.c.inl"
};

PRIVATE char const *const dhash_crc_16__buypass_alias_names[] = { "CRC-16/VERIFONE", "CRC-16/UMTS", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__buypass = {
	{
		"CRC-16/BUYPASS",
		dhash_crc_16__buypass_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_BUYPASS.c.inl"
};

PRIVATE char const *const dhash_crc_16__ccitt_false_alias_names[] = { "CRC-CCITT/0XFFFF", "CRC-CCITT/FFFF", "CRC-16/CCITT-0XFFFF", "CRC-16/CCITT-FFFF", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__ccitt_false = {
	{
		"CRC-16/CCITT-FALSE",
		dhash_crc_16__ccitt_false_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_CCITT-FALSE.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__cdma2000 = {
	{
		"CRC-16/CDMA2000",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_CDMA2000.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__cms = {
	{
		"CRC-16/CMS",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_CMS.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__dds_110 = {
	{
		"CRC-16/DDS-110",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x800d),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_DDS-110.c.inl"
};

PRIVATE char const *const dhash_crc_16__dect_r_alias_names[] = { "R-CRC-16", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__dect_r = {
	{
		"CRC-16/DECT-R",
		dhash_crc_16__dect_r_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0001),
#include "algorithms/algorithm.CRC-16_DECT-R.c.inl"
};

PRIVATE char const *const dhash_crc_16__dect_x_alias_names[] = { "X-CRC-16", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__dect_x = {
	{
		"CRC-16/DECT-X",
		dhash_crc_16__dect_x_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_DECT-X.c.inl"
};

PRIVATE char const *const dhash_crc_16__dnp_alias_names[] = { "CRC-DNP", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__dnp = {
	{
		"CRC-16/DNP",
		dhash_crc_16__dnp_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_DNP.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__en_13757 = {
	{
		"CRC-16/EN-13757",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_EN-13757.c.inl"
};

PRIVATE char const *const dhash_crc_16__genibus_alias_names[] = { "CRC-16/EPC", "CRC-16/I-CODE", "CRC-16/DARC", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__genibus = {
	{
		"CRC-16/GENIBUS",
		dhash_crc_16__genibus_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_GENIBUS.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__lj1200 = {
	{
		"CRC-16/LJ1200",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_LJ1200.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__maxim = {
	{
		"CRC-16/MAXIM",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_MAXIM.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__mcrf4xx = {
	{
		"CRC-16/MCRF4XX",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_MCRF4XX.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__opensafety_a = {
	{
		"CRC-16/OPENSAFETY-A",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_OPENSAFETY-A.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__opensafety_b = {
	{
		"CRC-16/OPENSAFETY-B",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_OPENSAFETY-B.c.inl"
};

PRIVATE char const *const dhash_crc_16__profibus_alias_names[] = { "CRC-16/IEC-61158-2", NULL };
PRIVATE struct dhashalgo16 const dhash_crc_16__profibus = {
	{
		"CRC-16/PROFIBUS",
		dhash_crc_16__profibus_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_PROFIBUS.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__riello = {
	{
		"CRC-16/RIELLO",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x554d),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_RIELLO.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__t10_dif = {
	{
		"CRC-16/T10-DIF",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_T10-DIF.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__teledisk = {
	{
		"CRC-16/TELEDISK",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_TELEDISK.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__tms37157 = {
	{
		"CRC-16/TMS37157",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x3791),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-16_TMS37157.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_16__usb = {
	{
		"CRC-16/USB",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0xffff),
#include "algorithms/algorithm.CRC-16_USB.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_crc_a = {
	{
		"CRC-A",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x6363),
	UINT16_C(0x0000),
#include "algorithms/algorithm.CRC-A.c.inl"
};

PRIVATE char const *const dhash_kermit_alias_names[] = { "CRC-16/CCITT", "CRC-16/CCITT-TRUE", "CRC-CCITT", NULL };
PRIVATE struct dhashalgo16 const dhash_kermit = {
	{
		"KERMIT",
		dhash_kermit_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.KERMIT.c.inl"
};

PRIVATE struct dhashalgo16 const dhash_modbus = {
	{
		"MODBUS",
		NULL,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0x0000),
#include "algorithms/algorithm.MODBUS.c.inl"
};

PRIVATE char const *const dhash_x_25_alias_names[] = { "CRC-16/IBM-SDLC", "CRC-16/ISO-HDLC", "CRC-B", NULL };
PRIVATE struct dhashalgo16 const dhash_x_25 = {
	{
		"X-25",
		dhash_x_25_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16_ioref
		}
	},
	UINT16_C(0xffff),
	UINT16_C(0xffff),
#include "algorithms/algorithm.X-25.c.inl"
};

PRIVATE char const *const dhash_xmodem_alias_names[] = { "ZMODEM", "CRC-16/ACORN", "CRC-16/LTE", NULL };
PRIVATE struct dhashalgo16 const dhash_xmodem = {
	{
		"XMODEM",
		dhash_xmodem_alias_names,
		16,
		2,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_16
		}
	},
	UINT16_C(0x0000),
	UINT16_C(0x0000),
#include "algorithms/algorithm.XMODEM.c.inl"
};

#define WIDTH 24
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_crc_24_alias_names[] = { "CRC-24/OPENPGP", NULL };
PRIVATE struct dhashalgo32 const dhash_crc_24 = {
	{
		"CRC-24",
		dhash_crc_24_alias_names,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00b704ce),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24.c.inl"
};

#define WIDTH 24
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo32 const dhash_crc_24__ble = {
	{
		"CRC-24/BLE",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24_ioref
		}
	},
	UINT32_C(0x00555555),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24_BLE.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_24__flexray_a = {
	{
		"CRC-24/FLEXRAY-A",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00fedcba),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24_FLEXRAY-A.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_24__flexray_b = {
	{
		"CRC-24/FLEXRAY-B",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00abcdef),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24_FLEXRAY-B.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_24__interlaken = {
	{
		"CRC-24/INTERLAKEN",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00ffffff),
	UINT32_C(0x00ffffff),
#include "algorithms/algorithm.CRC-24_INTERLAKEN.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_24__lte_a = {
	{
		"CRC-24/LTE-A",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00000000),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24_LTE-A.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_24__lte_b = {
	{
		"CRC-24/LTE-B",
		NULL,
		24,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_24
		}
	},
	UINT32_C(0x00000000),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-24_LTE-B.c.inl"
};

#define WIDTH 30
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo32 const dhash_crc_30__cdma = {
	{
		"CRC-30/CDMA",
		NULL,
		30,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_30
		}
	},
	UINT32_C(0x3fffffff),
	UINT32_C(0x3fffffff),
#include "algorithms/algorithm.CRC-30_CDMA.c.inl"
};

#define WIDTH 31
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo32 const dhash_crc_31__philips = {
	{
		"CRC-31/PHILIPS",
		NULL,
		31,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_31
		}
	},
	UINT32_C(0x7fffffff),
	UINT32_C(0x7fffffff),
#include "algorithms/algorithm.CRC-31_PHILIPS.c.inl"
};

#define WIDTH 32
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_crc_32_alias_names[] = { "CRC-32/ADCCP", "PKZIP", "CRC-32/IEEE-802.3", "IEEE-802.3", NULL };
PRIVATE struct dhashalgo32 const dhash_crc_32 = {
	{
		"CRC-32",
		dhash_crc_32_alias_names,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32_ioref
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_32__autosar = {
	{
		"CRC-32/AUTOSAR",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32_ioref
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32_AUTOSAR.c.inl"
};

#define WIDTH 32
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE char const *const dhash_crc_32__bzip2_alias_names[] = { "CRC-32/AAL5", "CRC-32/DECT-B", "B-CRC-32", NULL };
PRIVATE struct dhashalgo32 const dhash_crc_32__bzip2 = {
	{
		"CRC-32/BZIP2",
		dhash_crc_32__bzip2_alias_names,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32_BZIP2.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_32c = {
	{
		"CRC-32C",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32_ioref
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32C.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_32d = {
	{
		"CRC-32D",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32_ioref
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32D.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_32__mpeg_2 = {
	{
		"CRC-32/MPEG-2",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-32_MPEG-2.c.inl"
};

PRIVATE char const *const dhash_crc_32__posix_alias_names[] = { "POSIX", NULL };
PRIVATE struct dhashalgo32 const dhash_crc_32__posix = {
	{
		"CRC-32/POSIX",
		dhash_crc_32__posix_alias_names,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32
		}
	},
	UINT32_C(0x00000000),
	UINT32_C(0xffffffff),
#include "algorithms/algorithm.CRC-32_POSIX.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_crc_32q = {
	{
		"CRC-32Q",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32
		}
	},
	UINT32_C(0x00000000),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.CRC-32Q.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_jamcrc = {
	{
		"JAMCRC",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32_ioref
		}
	},
	UINT32_C(0xffffffff),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.JAMCRC.c.inl"
};

PRIVATE struct dhashalgo32 const dhash_xfer = {
	{
		"XFER",
		NULL,
		32,
		4,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_32
		}
	},
	UINT32_C(0x00000000),
	UINT32_C(0x00000000),
#include "algorithms/algorithm.XFER.c.inl"
};

#define WIDTH 40
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo64 const dhash_crc_40__gsm = {
	{
		"CRC-40/GSM",
		NULL,
		40,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_40
		}
	},
	UINT64_C(0x0000000000000000),
	UINT64_C(0x000000ffffffffff),
#include "algorithms/algorithm.CRC-40_GSM.c.inl"
};

#define WIDTH 64
#define IN_REFLECTED 0
#define OUT_REFLECTED 0
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo64 const dhash_crc_64 = {
	{
		"CRC-64",
		NULL,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64
		}
	},
	UINT64_C(0x0000000000000000),
	UINT64_C(0x0000000000000000),
#include "algorithms/algorithm.CRC-64.c.inl"
};

PRIVATE struct dhashalgo64 const dhash_crc_64__we = {
	{
		"CRC-64/WE",
		NULL,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64
		}
	},
	UINT64_C(0xffffffffffffffff),
	UINT64_C(0xffffffffffffffff),
#include "algorithms/algorithm.CRC-64_WE.c.inl"
};

#define WIDTH 64
#define IN_REFLECTED 1
#define OUT_REFLECTED 1
#include "hashfunc.c.inl"

PRIVATE struct dhashalgo64 const dhash_crc_64__xz = {
	{
		"CRC-64/XZ",
		NULL,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64_ioref
		}
	},
	UINT64_C(0xffffffffffffffff),
	UINT64_C(0xffffffffffffffff),
#include "algorithms/algorithm.CRC-64_XZ.c.inl"
};

PRIVATE struct dhashalgo64 const dhash_crc_64__jones = {
	{
		"CRC-64/JONES",
		NULL,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64_ioref
		}
	},
	UINT64_C(0xffffffffffffffff),
	UINT64_C(0x0000000000000000),
#include "algorithms/algorithm.CRC-64_JONES.c.inl"
};

PRIVATE char const *const dhash_crc_64__ecma_alias_names[] = { "CRC-64/ECMA-182", NULL };
PRIVATE struct dhashalgo64 const dhash_crc_64__ecma = {
	{
		"CRC-64/ECMA",
		dhash_crc_64__ecma_alias_names,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64_ioref
		}
	},
	UINT64_C(0x0000000000000000),
	UINT64_C(0x0000000000000000),
#include "algorithms/algorithm.CRC-64_ECMA.c.inl"
};

PRIVATE char const *const dhash_crc_64__iso_alias_names[] = { "FP-64", NULL };
PRIVATE struct dhashalgo64 const dhash_crc_64__iso = {
	{
		"CRC-64/ISO",
		dhash_crc_64__iso_alias_names,
		64,
		8,
		HASHALGO_FNORMAL,
		{
			(dhashfuncn_t)&_hashimpl_64_ioref
		}
	},
	UINT64_C(0x0000000000000000),
	UINT64_C(0x0000000000000000),
#include "algorithms/algorithm.CRC-64_ISO.c.inl"
};


/* NULL-terminated list of known hash algorithms */
PRIVATE struct dhashalgo const *const dhash_algorithms[] = {
	&dhash_crc_1.ha_base,
	&dhash_crc_3__rohc.ha_base,
	&dhash_crc_4__interlaken.ha_base,
	&dhash_crc_4__itu.ha_base,
	&dhash_crc_5__epc.ha_base,
	&dhash_crc_5__itu.ha_base,
	&dhash_crc_5__usb.ha_base,
	&dhash_crc_6__cdma2000_a.ha_base,
	&dhash_crc_6__cdma2000_b.ha_base,
	&dhash_crc_6__darc.ha_base,
	&dhash_crc_6__itu.ha_base,
	&dhash_crc_7.ha_base,
	&dhash_crc_7__rohc.ha_base,
	&dhash_crc_7__umts.ha_base,
	&dhash_crc_8.ha_base,
	&dhash_crc_8__autosar.ha_base,
	&dhash_crc_8__cdma2000.ha_base,
	&dhash_crc_8__darc.ha_base,
	&dhash_crc_8__dvb_s2.ha_base,
	&dhash_crc_8__ebu.ha_base,
	&dhash_crc_8__i_code.ha_base,
	&dhash_crc_8__itu.ha_base,
	&dhash_crc_8__lte.ha_base,
	&dhash_crc_8__maxim.ha_base,
	&dhash_crc_8__opensafety.ha_base,
	&dhash_crc_8__rohc.ha_base,
	&dhash_crc_8__sae_j1850.ha_base,
	&dhash_crc_8__wcdma.ha_base,
	&dhash_crc_10.ha_base,
	&dhash_crc_10__cdma2000.ha_base,
	&dhash_crc_11.ha_base,
	&dhash_crc_11__umts.ha_base,
	&dhash_crc_12__cdma2000.ha_base,
	&dhash_crc_12__dect.ha_base,
	&dhash_crc_12__umts.ha_base,
	&dhash_crc_13__bbc.ha_base,
	&dhash_crc_14__darc.ha_base,
	&dhash_crc_15.ha_base,
	&dhash_crc_15__mpt1327.ha_base,
	&dhash_arc.ha_base,
	&dhash_crc_16__aug_ccitt.ha_base,
	&dhash_crc_16__buypass.ha_base,
	&dhash_crc_16__ccitt_false.ha_base,
	&dhash_crc_16__cdma2000.ha_base,
	&dhash_crc_16__cms.ha_base,
	&dhash_crc_16__dds_110.ha_base,
	&dhash_crc_16__dect_r.ha_base,
	&dhash_crc_16__dect_x.ha_base,
	&dhash_crc_16__dnp.ha_base,
	&dhash_crc_16__en_13757.ha_base,
	&dhash_crc_16__genibus.ha_base,
	&dhash_crc_16__lj1200.ha_base,
	&dhash_crc_16__maxim.ha_base,
	&dhash_crc_16__mcrf4xx.ha_base,
	&dhash_crc_16__opensafety_a.ha_base,
	&dhash_crc_16__opensafety_b.ha_base,
	&dhash_crc_16__profibus.ha_base,
	&dhash_crc_16__riello.ha_base,
	&dhash_crc_16__t10_dif.ha_base,
	&dhash_crc_16__teledisk.ha_base,
	&dhash_crc_16__tms37157.ha_base,
	&dhash_crc_16__usb.ha_base,
	&dhash_crc_a.ha_base,
	&dhash_kermit.ha_base,
	&dhash_modbus.ha_base,
	&dhash_x_25.ha_base,
	&dhash_xmodem.ha_base,
	&dhash_crc_24.ha_base,
	&dhash_crc_24__ble.ha_base,
	&dhash_crc_24__flexray_a.ha_base,
	&dhash_crc_24__flexray_b.ha_base,
	&dhash_crc_24__interlaken.ha_base,
	&dhash_crc_24__lte_a.ha_base,
	&dhash_crc_24__lte_b.ha_base,
	&dhash_crc_30__cdma.ha_base,
	&dhash_crc_31__philips.ha_base,
	&dhash_crc_32.ha_base,
	&dhash_crc_32__autosar.ha_base,
	&dhash_crc_32__bzip2.ha_base,
	&dhash_crc_32c.ha_base,
	&dhash_crc_32d.ha_base,
	&dhash_crc_32__mpeg_2.ha_base,
	&dhash_crc_32__posix.ha_base,
	&dhash_crc_32q.ha_base,
	&dhash_jamcrc.ha_base,
	&dhash_xfer.ha_base,
	&dhash_crc_40__gsm.ha_base,
	&dhash_crc_64.ha_base,
	&dhash_crc_64__we.ha_base,
	&dhash_crc_64__xz.ha_base,
	&dhash_crc_64__jones.ha_base,
	&dhash_crc_64__ecma.ha_base,
	&dhash_crc_64__iso.ha_base,
	NULL
};
//[[[end]]]

/* Execute the given hash algorithm to hash `data...+=datasize'
 * When given, `start' is used as the initial hash value (which
 * may be the hash result of a previous call), but when set to
 * NULL, the algorythm's default start-value is used instead. */
INTERN WUNUSED NONNULL((1, 3)) DREF /*Int*/ DeeObject *DCALL
dhashalgo_exec(struct dhashalgo const *__restrict self,
               /*Int*/ DeeObject *start,
               void const *__restrict data,
               size_t datasize) {
	DREF /*Int*/ DeeObject *result;
	switch (self->ha_size) {

	case 1: {
		uint8_t state;
		struct dhashalgo8 const *me;
		me = (struct dhashalgo8 const *)self;
		if (start) {
			if (DeeObject_AsUInt8(start, &state))
				goto err;
			state ^= me->ha_outmod;
		} else {
			state = me->ha_start;
		}
		state  = (*me->ha_base.ha_hash8)(me, state, data, datasize);
		state ^= me->ha_outmod;
		result = DeeInt_NewU8(state);
	}	break;

	case 2: {
		uint16_t state;
		struct dhashalgo16 const *me;
		me = (struct dhashalgo16 const *)self;
		if (start) {
			if (DeeObject_AsUInt16(start, &state))
				goto err;
			state ^= me->ha_outmod;
		} else {
			state = me->ha_start;
		}
		state  = (*me->ha_base.ha_hash16)(me, state, data, datasize);
		state ^= me->ha_outmod;
		result = DeeInt_NewU16(state);
	}	break;

	case 4: {
		uint32_t state;
		struct dhashalgo32 const *me;
		me = (struct dhashalgo32 const *)self;
		if (start) {
			if (DeeObject_AsUInt32(start, &state))
				goto err;
			state ^= me->ha_outmod;
		} else {
			state = me->ha_start;
		}
		state  = (*me->ha_base.ha_hash32)(me, state, data, datasize);
		state ^= me->ha_outmod;
		result = DeeInt_NewU32(state);
	}	break;

	case 8: {
		uint64_t state;
		struct dhashalgo64 const *me;
		me = (struct dhashalgo64 const *)self;
		if (start) {
			if (DeeObject_AsUInt64(start, &state))
				goto err;
			state ^= me->ha_outmod;
		} else {
			state = me->ha_start;
		}
		state  = (*me->ha_base.ha_hash64)(me, state, data, datasize);
		state ^= me->ha_outmod;
		result = DeeInt_NewU64(state);
	}	break;

	default: {
		struct dhashalgon const *me;
		me = (struct dhashalgon const *)self;
		if (!start)
			start = me->ha_start;
		result = (*me->ha_base.ha_hashn)(me, start, data, datasize);
	}	break;

	}
	return result;
err:
	return NULL;
}


#define dhashname_is_nondef_sep(ch)  \
	((ch) == '\t' || (ch) == ' ' || \
	 (ch) == '_' || (ch) == '/' || (ch) == '\\')

PRIVATE bool DCALL dhashname_equals(char const *a, char const *b) {
	char cha, chb;
	for (;;) {
		cha = *a++;
		do {
			chb = *b++;
		} while (chb == '(' || chb == ')');
		if (chb >= 'a' && chb <= 'z')
			chb -= ('a' - 'A');
		if (cha != chb) {
			if (cha == '/')
				cha = '-';
			if (dhashname_is_nondef_sep(chb)) {
				for (;;) {
					chb = *b;
					if (!dhashname_is_nondef_sep(chb))
						break;
					++b;
				}
				chb = '-';
			}
			if (cha != chb)
				return 0;
		}
		if (!cha)
			break;
	}
	return 1;
}

/* Try to find the hash algorithm associated with `name', returning
 * NULL (but not throwing an error) if no such algorithm exists. */
INTERN WUNUSED NONNULL((1)) struct dhashalgo const *DCALL
dhashalgo_tryfind(char const *__restrict name) {
	struct dhashalgo const *const *iter, *algo;
	iter = dhash_algorithms;
	while ((algo = *iter++) != NULL) {
		char const *const *alias_iter;
		if (dhashname_equals(algo->ha_name, name))
			goto done;
		if ((alias_iter = algo->ha_alias) != NULL)
			while (*alias_iter) {
				if (dhashname_equals(*alias_iter, name))
					goto done;
				++alias_iter;
			}
	}
done:
	return algo;
}

/* Same as `dhashalgo_tryfind()', but throw an error if the algorithm wasn't found. */
INTERN WUNUSED NONNULL((1)) struct dhashalgo const *DCALL
dhashalgo_find(char const *__restrict name) {
	struct dhashalgo const *result;
	result = dhashalgo_tryfind(name);
	if unlikely(!result) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Unknown hash algorithm: %q",
		                name);
	}
	return result;
}


PRIVATE DREF DeeObject *DCALL
dhashmain_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	char const *name;
	DeeObject *data, *start = NULL;
	struct dhashalgo const *algo;
	DREF DeeObject *result;
	PRIVATE DEFINE_KWLIST(kwlist, { K(name), K(data), K(start), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "so|o:hash", &name, &data, &start))
		goto err;
	algo = dhashalgo_find(name);
	if unlikely(!algo)
		goto err;
	if (DeeString_Check(data)) {
		char const *utf8 = DeeString_AsUtf8(data);
		if unlikely(!utf8)
			goto err;
		result = dhashalgo_exec(algo, start, utf8, WSTR_LENGTH(utf8));
	} else {
		DeeBuffer buf;
		if (DeeObject_GetBuf(data, &buf, Dee_BUFFER_FREADONLY))
			goto err;
		result = dhashalgo_exec(algo, start,
		                        buf.bb_base,
		                        buf.bb_size);
		DeeObject_PutBuf(data, &buf, Dee_BUFFER_FREADONLY);
	}
	return result;
err:
	return NULL;
}

PRIVATE DEFINE_KWCMETHOD(dhashmain, &dhashmain_f);



PRIVATE struct dex_symbol symbols[] = {
	{ "hash", (DeeObject *)&dhashmain, MODSYM_FNORMAL,
	  DOC("(name:?Dstring,data:?X2?Dstring?DBytes,start?:?Dint)->?Dint\n"
	      "Calculate the hash of @data using the given hash function @name") },
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END


#endif /* !GUARD_DEX_HASHLIB_LIBHASH_C */
