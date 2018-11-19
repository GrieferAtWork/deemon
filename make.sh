#!/bin/bash

# Build deemon without it already being installed.
#
# Usage:
# $ ./make.sh  [OPTIONS]
#
# OPTIONS may specify a set of variables in the syntax
# `name=value', which can also be passed through environ.
# A list of recognized options can be found below.
#
# Example:
# $ ./make.sh  TARGET_WINDOWS TARGET_ARCH=i686

# Recognized options:
#	LD                = ${CROSS_PREFIX}gcc
#	READELF           = ${CROSS_PREFIX}readelf
#	MKDIR             = mkdir
#	ADDR2LINE         = ${CROSS_PREFIX}addr2line
#	CPP               = ${CROSS_PREFIX}cpp
#	CC                = ${CROSS_PREFIX}gcc
#	TARGET_CPU        = ${TARGET_ARCH}
#	AR                = ${CROSS_PREFIX}ar
#	BUILDPATH         = build/${TARGET_CPU}-deemon
#	AS                = ${CROSS_PREFIX}as
#	CROSS_PREFIX      =
#	CXX               = ${CROSS_PREFIX}g++
#	BINPATH           = .
#	NM                = ${CROSS_PREFIX}nm
#	TARGET_ARCH       =
#	CONFIG_DEBUG
#	CONFIG_NDEBUG
#	CONFIG_NO_DEX
#	CONFIG_NO_THREADS
#	CONFIG_OPTIMIZE
#	TARGET_WINDOWS

# Automatically generated using:
#	$ deemon -DCONFIG_PRINTCMD magic.dee

function yopt { [[ -v $1 ]]; return $?; }
function nopt { if [[ -v $1 ]]; then return 1; else return 0; fi; }
function iyopt { if [[ -v $1 ]]; then shift; echo $*; fi; }
function inopt { if ! [[ -v $1 ]]; then shift; echo $*; fi; }
function iexpr { if [ $1 ]; then shift; echo $*; fi; }
function run { echo $*; $*; }
function req { $* || exit $?; }
case "$(uname -s)" in
	*CYGWIN* | *Cygwin* | *cygwin*)
		TARGET_WINDOWS=1
		;;
	*)
		;;
esac
case "$(uname -a)" in
	*x86_64* | *x86-64*)
		TARGET_ARCH=x86_64
		;;
	*386*)
		TARGET_ARCH=i386
		;;
	*486*)
		TARGET_ARCH=i486
		;;
	*586*)
		TARGET_ARCH=i586
		;;
	*686*)
		TARGET_ARCH=i686
		;;
	*)
		;;
esac
while [[ $# -gt 0 ]]; do
	case $1 in
		--no-auto-target)
			unset TARGET_WINDOWS
			unset TARGET_ARCH
			unset TARGET_CPU
			;;
		-*)
			echo "Unrecognized option: '$1'"
			exit 1
			;;
		*)
			echo "Option: $1"
			if [[ "$1" == *"="* ]]; then
				export $1
			else
				export $1=""
			fi
			;;
	esac
	shift
done
if ! [[ -v TARGET_ARCH ]]; then TARGET_ARCH=""; fi
if ! [[ -v BINPATH ]]; then BINPATH="."; fi
if ! [[ -v CROSS_PREFIX ]]; then CROSS_PREFIX=""; fi
if ! [[ -v AS ]]; then AS="${CROSS_PREFIX}as"; fi
if ! [[ -v AR ]]; then AR="run ${CROSS_PREFIX}ar"; fi
if ! [[ -v TARGET_CPU ]]; then TARGET_CPU="${TARGET_ARCH}"; fi
if ! [[ -v CC ]]; then CC="run ${CROSS_PREFIX}gcc"; fi
if ! [[ -v CPP ]]; then CPP="run ${CROSS_PREFIX}cpp"; fi
if ! [[ -v ADDR2LINE ]]; then ADDR2LINE="run ${CROSS_PREFIX}addr2line"; fi
if ! [[ -v READELF ]]; then READELF="run ${CROSS_PREFIX}readelf"; fi
if ! [[ -v LD ]]; then LD="run ${CROSS_PREFIX}gcc"; fi
if ! [[ -v MKDIR ]]; then MKDIR="run mkdir"; fi
if ! [[ -v BUILDPATH ]]; then BUILDPATH="build/${TARGET_CPU}-deemon"; fi
if ! [[ -v NM ]]; then NM="run ${CROSS_PREFIX}nm"; fi
if ! [[ -v CXX ]]; then CXX="run ${CROSS_PREFIX}g++"; fi

if [[ $TARGET_ARCH == i?86 ]]; then TARGET_ARCH="i386"; fi

${MKDIR} -p ${BINPATH}
${MKDIR} -p lib
${MKDIR} -p ${BUILDPATH}
${MKDIR} -p ${BUILDPATH}/deemon
${MKDIR} -p ${BUILDPATH}/math
${MKDIR} -p ${BUILDPATH}/time
${MKDIR} -p ${BUILDPATH}/net
${MKDIR} -p ${BUILDPATH}/disassembler
${MKDIR} -p ${BUILDPATH}/threading
${MKDIR} -p ${BUILDPATH}/rt
${MKDIR} -p ${BUILDPATH}/fs
${MKDIR} -p ${BUILDPATH}/files
${MKDIR} -p ${BUILDPATH}/ctypes
${MKDIR} -p ${BUILDPATH}/ipc
${MKDIR} -p ${BUILDPATH}/collections

if nopt CONFIG_NDEBUG; then
CONFIG_DEBUG=1
fi
if yopt TARGET_WINDOWS && [[ $TARGET_CPU == i?86 ]]; then
CROSS_PREFIX=i686-w64-mingw32-
fi

# Project: math
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/math/src.dex.math.libmath.c.o src/dex/math/libmath.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/math.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/math.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/math.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/math/src.dex.math.libmath.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: time
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/time/src.dex.time.libtime.c.o src/dex/time/libtime.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/time.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/time.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/time.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/time/src.dex.time.libtime.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: net
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/net/src.dex.net.error.c.o src/dex/net/error.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/net/src.dex.net.libnet.c.o src/dex/net/libnet.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/net/src.dex.net.sockaddr.c.o src/dex/net/sockaddr.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/net/src.dex.net.socket.c.o src/dex/net/socket.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/net.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/net.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/net.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/net/src.dex.net.error.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/net/src.dex.net.libnet.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/net/src.dex.net.sockaddr.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/net/src.dex.net.socket.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-lWs2_32"))
fi

# Project: disassembler
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/disassembler/src.dex.disassembler.libdisasm.c.o src/dex/disassembler/libdisasm.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/disassembler/src.dex.disassembler.printcode.c.o src/dex/disassembler/printcode.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/disassembler/src.dex.disassembler.printinstr.c.o src/dex/disassembler/printinstr.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/disassembler.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/disassembler.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/disassembler.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/disassembler/src.dex.disassembler.libdisasm.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/disassembler/src.dex.disassembler.printcode.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/disassembler/src.dex.disassembler.printinstr.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: threading
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/threading/src.dex.threading.host.c.o src/dex/threading/host.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/threading/src.dex.threading.libthreading.c.o src/dex/threading/libthreading.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/threading/src.dex.threading.tls.c.o src/dex/threading/tls.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/threading.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/threading.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/threading.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/threading/src.dex.threading.host.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/threading/src.dex.threading.libthreading.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/threading/src.dex.threading.tls.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: rt
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/rt/src.dex.rt.librt.c.o src/dex/rt/librt.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/rt.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/rt.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/rt.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/rt/src.dex.rt.librt.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: fs
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/fs/src.dex.fs.host.c.o src/dex/fs/host.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/fs/src.dex.fs.libfs.c.o src/dex/fs/libfs.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/fs/src.dex.fs.path.c.o src/dex/fs/path.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/fs.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/fs.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/fs.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/fs/src.dex.fs.host.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/fs/src.dex.fs.libfs.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/fs/src.dex.fs.path.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: files
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/files/src.dex.files.joined.c.o src/dex/files/joined.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/files/src.dex.files.libfiles.c.o src/dex/files/libfiles.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/files.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/files.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/files.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/files/src.dex.files.joined.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/files/src.dex.files.libfiles.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: ctypes
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.array.c.o src/dex/ctypes/array.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.builtins.c.o src/dex/ctypes/builtins.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.cfunction.c.o src/dex/ctypes/cfunction.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.core.c.o src/dex/ctypes/core.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.c_malloc.c.o src/dex/ctypes/c_malloc.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.c_string.c.o src/dex/ctypes/c_string.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.libctypes.c.o src/dex/ctypes/libctypes.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.pointer-math.c.o src/dex/ctypes/pointer-math.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.pointer.c.o src/dex/ctypes/pointer.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.shlib.c.o src/dex/ctypes/shlib.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -DCONFIG_NO_CFUNCTION -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ctypes/src.dex.ctypes.struct.c.o src/dex/ctypes/struct.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/ctypes.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/ctypes.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/ctypes.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.array.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.builtins.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.cfunction.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.core.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.c_malloc.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.c_string.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.libctypes.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.pointer-math.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.pointer.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.shlib.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ctypes/src.dex.ctypes.struct.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: ipc
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/ipc/src.dex.ipc.libipc.c.o src/dex/ipc/libipc.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/ipc.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/ipc.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/ipc.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/ipc/src.dex.ipc.libipc.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi

# Project: collections
if nopt CONFIG_NO_DEX; then
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/collections/src.dex.collections.deque.c.o src/dex/collections/deque.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "TARGET_WINDOWS" "-D__PE__") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_DEBUG" "-DNDEBUG") -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits $(iyopt "CONFIG_NO_THREADS" "-DCONFIG_NO_THREADS") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" "-DCONFIG_BUILDING_DEX") -shared -Iinclude -Iinclude/deemon/kos-headers -c -o ${BUILDPATH}/collections/src.dex.collections.libcollections.c.o src/dex/collections/libcollections.c
req $(inopt "CONFIG_NO_DEX" "${CXX}") $(iyopt "CONFIG_OPTIMIZE" "-O3") $(iyopt "CONFIG_DEBUG" "-fstack-protector-strong -g") $(inopt "CONFIG_NO_THREADS" "-pthread") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-o lib/collections.dll")) $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-Wl,--out-implib=/lib/collections.dll.a")) $(inopt "CONFIG_NO_DEX" $(inopt "TARGET_WINDOWS" "-o lib/collections.so")) $(inopt "CONFIG_NO_DEX" "-L.") $(inopt "CONFIG_NO_DEX" "-shared") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-static-libgcc")) $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/collections/src.dex.collections.deque.c.o") $(inopt "CONFIG_NO_DEX" "${BUILDPATH}/collections/src.dex.collections.libcollections.c.o") $(inopt "CONFIG_NO_DEX" $(iyopt "TARGET_WINDOWS" "-ldeemon"))
fi
