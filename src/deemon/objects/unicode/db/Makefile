
db.dat: ctype.dee UnicodeData.txt CaseFolding.txt DerivedCoreProperties.txt
	../../../../../deemon ctype.dee

# "--no-check-certificate" is here because (at least right now), the SSL cert that
# the unicode guys are using for "ftp.unicode.org" is only valid for "unicode.org",
# but not for the "ftp." sub-domain...
UnicodeData.txt:
	wget --no-check-certificate https://ftp.unicode.org/Public/UNIDATA/UnicodeData.txt
CaseFolding.txt:
	wget --no-check-certificate https://ftp.unicode.org/Public/UNIDATA/CaseFolding.txt
DerivedCoreProperties.txt:
	wget --no-check-certificate https://ftp.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt
