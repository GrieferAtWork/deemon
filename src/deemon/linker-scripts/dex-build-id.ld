__dex_build_id__ = ADDR(.note.gnu.build-id);
ASSERT(SIZEOF(.note.gnu.build-id) == 32, "Should be 16-byte header + 16-byte MD5");
