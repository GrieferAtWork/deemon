/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_DOCTEXT_H
#define GUARD_DEEMON_COMPILER_DOCTEXT_H 1

#include "../api.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "../string.h" /* Dee_unicode_printer */

DECL_BEGIN

/* +=============================================================================+
 * | OVERVIEW                                                                    |
 * +=============================================================================+
 *
 * Documentation text. - i.e. the user-visible portion of documentation strings
 * >> import * from deemon;
 * >>
 * >> @@My Documentation text
 * >> function foo(x: int): int { }
 * >>
 * >> print repr foo.__doc__;
 * Output:
 *    "(:?Dint)->?Dint\nMy Documentation text"
 *
 * The user-visible doc portion here is "My Documentation text", as already seen
 * in the original source code. However, it's not quite as simple as that, since
 * documentation strings may contain a number of special formating indicators.
 * This form of the documentation text is called "Fully Encoded"
 *
 * First off, before any additional formating is done, Encoded Documentation Texts
 * to-be embedded within __doc__ strings are escaped as follows in order to turn
 * them into Fully Encoded Documentation Texts:
 *  #1 Any instance of \ is replaced with \\
 *     >> replace("\\", "\\\\");
 *  #2 Multiple consecutive line-feeds (i.e. sequences of at least 1 empty line) are
 *     escaped such that every \n-character except for the first is prefixed by \:
 *     >> ...
 *     >> replace("\n\n\n\n", "\n\\\n\\\n\\\n");
 *     >> replace("\n\n\n",   "\n\\\n\\\n");
 *     >> replace("\n\n",     "\n\\\n");
 *  #3 Any line that starts with a (-character is escaped by inserting a \ at the
 *     start of the same line:
 *     >> replace("\n(", "\n\\(");
 *  #4 Any instance of -> is replaced with -\>:
 *     >> replace("->", "-\\>");
 *  #5 If the doc text starts with a \n-character, make it start with \\\n instead:
 *     >> if (startswith("\n")) this = "\\" + this;
 *
 * The resulting text is then appended after the documentation's declaration string,
 * the format of which is documented in `/include/deemon/compiler/symbol.h', with
 * an additional \n-character inserted inserted in-between.
 *
 * This encoding method can be reversed at runtime to re-gain the original documentation
 * text, the format of which will be described next, and who's encoding mechanism is
 * implemented within this file.
 *
 * Naming:
 *   - Fully Encoded Documentation Text:
 *         The raw strings found in __doc__ strings
 *
 *   - Encoded Documentation Text:
 *         This text's format is produced by compiling the "Raw Documentation Text",
 *         and encoding it using the the methods described in the section `ENCODING'
 *
 *   - Raw Documentation Text:
 *         The raw list of strings preceding some declaration in user-code
 *         after being stripped of each line's leading "@@", and joined by
 *         line-feeds ("\n".join(lines.each.lsstrip("@@")))
 *         This text's format is described in the next section `FORMAT'
 *
 */

/* +=============================================================================+
 * | FORMAT (encoding scheme)                                                    |
 * +=============================================================================+
 *
 * This section details how "Raw Documentation Text" is compiled into "Encoded Documentation Text"
 *
 * Since this format is designed to compile from a human-readable documentation text format
 * into something that is easier parsed by tools such as `doc-browser.dee', this format is
 * designed with redundancy in mind.
 * This means that anything that would normally be considered a compiler error in a normal
 * compiler will instead cause anything that had already been specially encoded to be discarded,
 * before re-processing the original code of the failed component such that the outer-most
 * instance of the failed component will not be triggered, but will instead produce raw text.
 * e.g.:
 * >> +-----+--------------------+
 * >> | A   | B                  |
 * >> +-----+--------------------+
 * >> | @10 | @(int from deemon) |
 * >> +--------------------------+
 * This table is not well-formed (its footer is lacking a `+' where there is a `-' instead).
 * As such, the produced output text will not contain a table, but will instead contain the
 * raw +, - and | characters as they appear in this example the same way they would appear
 * if encoding tables in this form wasn't actually a feature of doc texts.
 * However, @10 and @(int from deemon) will still be encoded and high-lit properly, the later
 * also being clickable when the documentation string is viewed in `doc-browser.dee'.
 *
 * Any character that would be a controlling part of a special formating construct can be
 * prefixed by a \-character in order to disable the recognition of that construct, and have
 * the \-character be removed in the final output. When what follows a \-character isn't
 * actually a special construct start character, the \-character will not be removed, with
 * the exception of a \\-sequence, which will instead cause a single \-character to be written
 * to the Encoded Documentation Text output.
 *
 * As such, \ can be used to escape the following characters such that \ is removed,
 * and the escaped character looses any special meaning:
 *     \ _ @ ` [ ] ( ) - + | = ~ * # : >
 *
 *     0 1 2 3 4 5 6 7 8 9 .  (Only if the character appeared at the beginning of a line, or
 *                            was preceded by only other decimal, \, . or : characters (i.e.
 *                            would have been apart of a possibly already broken ordered list))
 *                            HINT: Any character matching `DeeUni_IsDigit()' is considered
 *
 * Additionally, ' ' (or any other ) can be escaped to force the insertion of an additional
 * or specific space character.
 *
 * Lastly, since the @-character is also a vital part of Raw Documentation Text, it can also
 * be escaped in a variety of ways:
 *   - As already stated above, by causing a "compiler error" in the encoded expression that
 *     follows, the @-character can be escaped by something as simple as having it be followed
 *     by space, one of ')', '}', ']', '\', '`', ',', or a number of other characters that are
 *     not allowed to start an expression.
 *   - When written as \@, the already stated rule of using \ for escaping special formating
 *     constructs kicks in, causing a raw @-character to appear within the Encoded Documentation
 *     Text output
 *   - Lastly, you can write @@, which has the same effect as writing \@
 *
 *
 * A set of general raw text formating operations is performed in order to re-flow text into
 * a form that does not contain explicit line-feed at unintended locations:
 *
 *   - When taking the entirety of the "Raw Documentation Text", count the number of
 *     leading space characters for each non-empty line that also contains anything that
 *     isn't a space character, and take the lowest number from this set.
 *     If that number is non-zero, strip that many characters from every line before doing
 *     any additional processing:
 *     >> @@   First line
 *     >> @@
 *     >> @@ Second line
 *     Parsed the same way as:
 *     >> @@  First line
 *     >> @@
 *     >> @@Second line
 *
 *   - Singular line-feeds are replaced by a single space-character
 *     One exception to this rule is the case where a line that originally began
 *     with some leading white-space is followed by a non-empty line that starts
 *     with less leading space characters. In this case, the line-feed is kept:
 *     >> @@      First line
 *     >> @@      Second line
 *     >> @@Third line
 *     Output:
 *     >> First line Second line
 *     >> Third line
 *
 *   - When one line is followed by another line with more indentation, a line-feed
 *     is also inserted, and the indentation is kept:
 *     >> @@First line
 *     >> @@      Second line
 *     >> @@      Third line
 *     Output:
 *     >> First line
 *     >>         Second line Third line
 *     An exception to this rule is when the line prior to the indented line contains
 *     a `:' character preceded by `.', <space> or <issymcont>:
 *     >> @@First line
 *     >> @@
 *     >> @@NOTE: Second line
 *     >> @@      Third line
 *     Output:
 *     >> First line
 *     >> NOTE: Second line Third line
 *
 *   - Multiple consecutive line-feeds are replaced by having one of the line-feeds be
 *     removed, while the remainder of line-feeds is kept (though the one removed line-
 *     feed is _NOT_ replaced by a space character!)
 *     >> @@First line
 *     >> @@Second line
 *     >> @@
 *     >> @@Third line
 *     Output:
 *     >> First line Second line
 *     >> Third line
 *
 *   - Multiple consecutive space characters are replaced with a single space-character
 *     This also includes space characters only created by the removal of line-feeds
 *
 *
 *
 *
 * Recognized source code formating options (most of these are based on markdown):
 *
 *   - Headers
 *     >> @@# H1
 *     >> @@## H2
 *     >> @@### H3
 *     >> @@#### H4
 *     >> @@##### H5
 *     >> @@###### H6
 *     Where headers with more leading #-characters become smaller each time.
 *      - The #-character-sequence must appear at the start of a line (or only be preceded by whitespace)
 *      - The #-character-sequence must be followed by at least 1 space character
 *
 *
 *   - Emphasis/italics, Strong emphasis/bold and strikethrough
 *     >> @@*italic*, _bold_ and ~strikethrough~.
 *      - The first *, _ or ~ must be at the start of a line, or be preceded by whitespace
 *      - The second *, _ or ~ must follow on the same line and no whitespace must exist between it and the first
 *     >> @@**italic with spaces**, __bold with spaces__ and ~~strikethrough with spaces~~.
 *      - The first **, __ or ~~ must be at the start of a line, or be preceded by whitespace
 *      - The second **, __ or ~~ must follow on the same line, through whitespace is allowed
 *        to exist between it and the first
 *     Note that these rules somewhat differ from markdown, uses *foo* and _bar_ for italics,
 *     and **foo** and __foo__ for bold, while deemon documentation text changes this to use
 *     * and ** for italics, and _ and __ for bold (meaning that *foo* is a portable italic,
 *     and __bar__ is a portable bold)
 *
 *
 *   - Lists
 *     >> 1. First item
 *     >> 2. Second item
 *     >> - First unordered item
 *     >> - Second unordered item
 *     Lists come in 2 forms:
 *       - Numbered:  each line begins with a decimal number, followed by (the same) . or :
 *                    These numbers don't necessarily have to be incremental, nor does there
 *                    need to be only a single decimal number (so-long as decimals are only
 *                    ever separated by `.' characters; i.e. `1.1.3:' is a valid list item start)
 *       - Unordered: each line beings with (the same) character, which is one of - + or *
 *     A list sequence is continuous starting with the first item, after which all following
 *     items must use the same ordering format (i.e. if the first item uses -, the next item
 *     must also use -, and can't just go ahead and use * or +). Additionally, if a list continues
 *     depends on the number of leading whitespace character of a following line, when compared to
 *     the number of leading whitespace character from the previous (list-item) line:
 *       - The first list item must appear at the start of a line (or only be preceded by whitespace)
 *       - Next line contains less whitespace characters -> End the current list and begin a new one,
 *         or resume a previous, not-yet-completed list:
 *         >> @@ - First item
 *         >> @@ - Second item
 *         >> @@   Second line of second item
 *         >> @@    - Inner list item 1
 *         >> @@    - Inner list item 2
 *         >> @@ - Third item
 *         The last line here contains less whitespace than the previous line. As such, the inner
 *         list is terminated, and the next line is part of the outer list.
 *       - Next line is empty, or contains only space characters -> Scan ahead for the next non-empty
 *         line that contains more than just space characters. If this line's formating indicates that
 *         the current list is to-be terminated, terminate the list and _DONT_ parse the empty lines
 *         as part of the current (last) list item. - These empty lines should be parsed as part of
 *         the list's parent container.
 *         With this rule, it also becomes possible to force-insert new lines into list items:
 *         >> @@ - First line
 *         >> @@   Second line
 *         >> @@
 *         >> @@   Third line
 *         List item body:
 *             " First line\n Second line\n\n Third line"
 *       - Next line contains more, or equal the number of whitespace characters, as well as
 *         non-space characters -> Add the line to the list item's body to-be re-parsed.
 *     With the general body of a list item determined, that body is then processed for further,
 *     recursive formating components after striping the number of characters until after the end
 *     of the list control indicator of the first line from all other lines:
 *     >> @@ - First line
 *     >> @@   Second line
 *     Here, the list item body gets parsed as:
 *          " First line\n Second line"
 *
 *
 *   - Hyper-links
 *     >> @@[Click me](https://foo.bar)
 *     Same as in markdown. Note that the [, ], ( and ) characters are all control characters,
 *     such that any one of them being escaped causes the link not to be recognized as such.
 *     The following formating rules apply:
 *       - The number of (non-scaped) [- and ]-characters in the first portion must match:
 *         >> @@ [foo[bar]](https://foo.bar)
 *         >> @@ [foo\[bar](https://foo.bar)
 *         >> @@ [foo\]bar](https://foo.bar)
 *         All of these forms are allowed, and the same also applies to the second portion
 *         when dealing with (- and )-characters.
 *       - The first portion ending with a ]-character must immediately be followed by the
 *         second portion starting with a (-character. No other characters must be present
 *         in-between, including white-space characters.
 *
 *
 *   - Inline code (with- and without deemon syntax highlighting)
 *     >> @@Highlight `code`, `code', ``code`` and ```code``` differently
 *     >> @@
 *     >> @@```
 *     >> @@Highlight this part the same as code above
 *     >> @@```
 *     >> @@
 *     >> @@```deemon
 *     >> @@if (true)
 *     >> @@    print "Highlight as deemon";
 *     >> @@```
 *     >> @@
 *     >> @@> if (true)
 *     >> @@>     print "Also highlight as deemon";
 *     >> @@
 *     >> @@ >> if (true)
 *     >> @@ >>     print "Same deal: Highlight as deemon";
 *     >> @@
 *     >> @@ - I'm a list item
 *     >> @@   >> print "Just to show it as part of a list";
 *     Formating rules:
 *       - `foo`-like is the easiest, in that it behaves just like **foo** and __foo__,
 *         meaning that the contained text may contain space characters, but must terminate
 *         on the same line.
 *       - START_OF_LINE + OPTIONAL_WHITESPACE + `>' (optionally repeated) is also pretty simple:
 *         line-feed followed by any number of >-characters (though at least one), with any line
 *         there-after that starts with the same number of >-characters also belonging to the same
 *         source representation block.
 *         The embedded code is also stripped for leading space characters common to all lines, such
 *         that the following two examples produce the same output:
 *         >> @@ >  Foobar
 *         >> @@ >  Barfoo
 *         And
 *         >> @@ >Foobar
 *         >> @@ >Barfoo
 *         Note however that keywords in code such as this are not clickable, in that the compiler
 *         does not embed additional meta-data the way it does for the @<expr> construct
 *       - ```deemon
 *         ...
 *         ```
 *         This one behaves the same as the previous one (`@@> ...'), in that it allows for deemon
 *         syntax highlighting within the eventual output. However, unlike the `@@> ...' form, this
 *         one allows the first line (containing "```deemon\n") to not appear at the start of its own
 *         line, and also unlike the `@@>...' form, this form strip leading and trailing whitespace
 *         from the embedded code, which also includes line-feeds.
 *         Note that forms such as "```deemon\n if (true) none;```" are also allowed, so-long as the
 *         "```deemon\n" portion is followed by at least 1 whitespace character, or a linefeed.
 *       - ```...```
 *         This form behaves the same as ```deemon\n...```, but doesn't include deemon-specific syntax
 *         highlighting, but instead uses the same highlighting as `...` does.
 *
 *
 *   - Tables
 *     >> @@+--------------+---------------+
 *     >> @@| First Column | Second Column |
 *     >> @@+--------------+---------------+
 *     >> @@| Left 1.1     | Right 1.1     |
 *     >> @@| Left 1.2     |               |
 *     >> @@|              |               |
 *     >> @@| Left 2.1     | Right 2.1     |
 *     >> @@|              | Right 2.2     |
 *     >> @@|              |               |
 *     >> @@| Left 3.1     | Right 3.1     |
 *     >> @@+--------------+---------------+
 *     Produced table (approximation)
 *          +==============+===============+
 *          | First Column | Second Column |
 *          +==============+===============+
 *          | Left 1.1     | Right 1.1     |
 *          | Left 1.2     |               |
 *          +--------------+---------------+
 *          | Left 2.1     | Right 2.1     |
 *          |              | Right 2.2     |
 *          +--------------+---------------+
 *          | Left 3.1     | Right 3.1     |
 *          +==============+===============+
 *     Formating rules:
 *       - The top-left corner of the table must appear at the start of a line (or only be preceded by whitespace)
 *       - All table borders must consistently use the same convention throughout the same table.
 *         i.e. If at least one corner uses +-characters, the all corners must use them.
 *              If at least one horizontal line uses = instead of -, all - must become =
 *              ...
 *       - Vertical lines must be drawn using one of: |
 *       - Horizontal lines must be drawn using one of: - =
 *         Additionally, space characters are allowed, so-long as each segment contains
 *         at least one non-space character, that character being the chosen horizontal
 *         line character.
 *       - Corners must be drawn using one of: + |
 *       - The number of columns cannot be altered half-way into a table. - In other words,
 *         the number of corner characters must remain consistent for every border throughout.
 *       - The actual number of Horizontal line characters 
 *       - A new row with a thin separator can be started by inserting a line where all
 *         columns contain only space characters. If multiple such lines appear after
 *         one-another, all but the first line are simply ignored.
 *       - A new row with a thick separator can be starts by using horizontal line characters,
 *         alongside optional space characters for all columns.
 *       - The first and last line are optional, and their presence/absence doesn't alter the
 *         representation of the produced table.
 *
 *
 *   - Symbol references
 *     >> @@Click the following @(List from deemon)
 *     Will be rendered such that a clickable link for `List' appears within flow-text
 *     Formating rules:
 *       - The format starts with an @-character that is followed by one of:
 *         - "- or '-character (normal string; allow \-escape):
 *           @"foo"  Encoded as '```deemon\n"Foo"```'  (except that no new-line is inserted)
 *           @'foo'  Encoded as '```deemon\n'Foo'```'  (except that no new-line is inserted)
 *         - r, followed by a "- or '-character (raw string; no \-escape):
 *           @r"foo"  Encoded as '```deemon\n"Foo"```'  (except that no new-line is inserted)
 *           @r'foo'  Encoded as '```deemon\n'Foo'```'  (except that no new-line is inserted)
 *         - A digit:
 *           @123    Encoded as "```deemon\n123```"  (except that no new-line is inserted)
 *           @123.   Encoded as "```deemon\n123```."  (except that no new-line is inserted)
 *           @123.4  Encoded as "```deemon\n123.4```"  (except that no new-line is inserted)
 *           @123. 4 Encoded as "```deemon\n123```. 4"  (except that no new-line is inserted)
 *         - A - or + followed by a digit:
 *           @-123    Encoded as "```deemon\n-123```"  (except that no new-line is inserted)
 *           @-123.   Encoded as "```deemon\n-123```."  (except that no new-line is inserted)
 *           @-123.4  Encoded as "```deemon\n-123.4```"  (except that no new-line is inserted)
 *           @-123. 4 Encoded as "```deemon\n-123```. 4"  (except that no new-line is inserted)
 *         - A keyword:
 *           @foo    Resolves to a clickable symbol `foo' in the context of the component
 *                   being annotated by the documentation text. Note that in the case of
 *                   a function, this also allows function arguments to be annotated!
 *           @foo()  Same as a pure keyword, but annotate as a function-call
 *         - A ( [ or {-character:
 *           @(foo)          Same as `@foo'
 *           @(foo, bar)     A tuple expression (```deemon\n(foo, bar)```)
 *           @[foo]          An array expression (```deemon\n[foo]```)
 *           @{foo}          An sequence expression (```deemon\n{foo}```)
 *     In general, any deemon expression can be high-lit/annotated using this scheme, whilst
 *     also allowing keywords to be clicked in the associated representation, as well as making
 *     special provisioning for external symbol references to show up properly.
 *     This formating component is the one used most often and it's main purpose is to have code
 *     point at something particular that should be clickable for easy reference inside of text.
 *
 *
 *   - Parameter/return/throws descriptions
 *     >> @@@param foo This is the description of foo
 *     >> @@@return This explains what the function returns and why
 *     >> @@@throws Error This is the description of why @Error may get thrown
 *     >> @@@interrupt
 *      - The @-character must appear at the start of a line (or only be preceded by whitespace)
 *      - An optional ':' character may appear after the tag, and once again after the referenced argument/type
 *        >> @@@throws: Error: This time with ':'-characters
 *      - In the case of `@param' and `@throws', the referenced
 *        >> @@@throws: Error: This time with ':'-characters
 *     
 *
 */

/* +=============================================================================+
 * | ENCODING (of compiled documentation text)                                   |
 * +=============================================================================+
 *
 * This section details how the compiled "Raw Documentation Text" is encoded
 * to form the "Encoded Documentation Text" that is then embedded in doc strings.
 *
 * All flow-text is encoded normally, with the exception that the following characters
 * are all escaped by being prefixed with a #-character, should they not be intended
 * as a special symbol used for encoding embedded format options.
 * All other characters are part of raw flow-text and will be encoded on a 1-on-1 basis.
 *
 * Characters that need escaping in raw text (by being prefixed by a #-character)
 *    # $ % & ~ ^ { } [ ] | ? * @ - + :
 *
 * Note that some of these characters aren't currently used by the encoding syntax,
 * however may find use in the future, so they already having to be escaped is done
 * in order to guaranty forward-compatibility.
 *
 * NOTE: The encoding format documented here must also be used by hard-coded documentation
 *       strings, such as found throughout the deemon core, as well as dex modules.
 *
 * NOTE: Unless otherwise noted, the BODY part of each of these can always contain other
 *       formating options recursively.
 *
 *
 *   - Headers
 *         #Hn{BODY}
 *         #HnBODY\n
 *             Where `n' is one of 1 2 3 4 5 or 6, and BODY is the text used as header.
 *
 *   - Emphasis/italics, Strong emphasis/bold and strikethrough
 *         #B{BODY}   Bold for BODY
 *         #BBODY     Bold for BODY; BODY starts with a issymstrt() character, and ends at the first !issymcont() char
 *         #I{BODY}   Italic for BODY
 *         #IBODY     Italic for BODY; BODY starts with a issymstrt() character, and ends at the first !issymcont() char
 *         #S{BODY}   Strikethrough for BODY
 *         #SBODY     Strikethrough for BODY; BODY starts with a issymstrt() character, and ends at the first !issymcont() char
 *
 *   - Lists
 *         #L-{BODY1|BODY2|BODY3}
 *         #L+{BODY1|BODY2|BODY3}
 *         #L*{BODY1|BODY2|BODY3}
 *         #L{{1.}BODY1|{2.}BODY2|{3.}BODY3}
 *             Syntax:
 *               - List lines are separated by |-characters
 *               - The list element character is written after the #L and before the {
 *               - When no element character is given, a per-item character can instead
 *                 be given by having list elements start with {ELEM_PREFIX}
 *
 *   - Hyper-links
 *         #A{BODY|LINK}
 *             Syntax:
 *               - The LINK text is not processed recursively, but is the actual
 *                 hyper-link location (with things like |-characters escaped as #|)
 *               - BODY will be clickable to go to the given indicated link, and
 *                 is processed recursively for more format specifiers
 *
 *
 *   - Inline code (with- and without deemon syntax highlighting)
 *         #C{BODY}           - BODY is high-lit as abstract code
 *         #C[deemon]{BODY}   - BODY is high-lit as deemon code
 *         #CBODY             - Same as #C{BODY}, but BODY is must start with issymstrt() and ends on the first !issymcont()
 *         #C[deemon]BODY     - Same as #C[deemon]{BODY}, but BODY must start with issymstrt() and ends on the first !issymcont()
 *         ${BODY}            - Same as #C[deemon]{BODY}
 *         $BODY              - Same as #C[deemon]BODY
 *             Syntax:
 *               - More language-specific highlighting may be supported by doc text renderers
 *                 than only the anonymous code renderer, and the deemon code renderer. To
 *                 ensure forward compatibility, unknown languages such as #C[bash]{echo Hello}
 *                 should be high-lit the same way as #C{echo Hello}, meaning that unknown names
 *                 should appear as abstract code
 *               - The $BODY form accepts the following strings for BODY:
 *                 - $foo    -- symstrt + symcont...                       (Symbol name)
 *                 - $1.2    -- decimal + (decimal | '.')...               (Integer or float constant)
 *                 - $-1.2   -- ('-' | '+') + decimal + (decimal | '.')... (Integer or float constant)
 *                 - $"foo"  -- '"' + (('\\' any) | (any & !'"')) + '"'    (String constant)
 *                 - $'foo'  -- '\'' + (('\\' any) | (any & !'\'')) + '\'' (String constant)
 *                 - $r"foo" -- 'r' + '"' + (any & !'"') + '"'             (Raw string constant)
 *                 - $r'foo' -- 'r' + '\'' + (any & !'\'') + '\''          (Raw string constant)
 *
 *   - Tables
 *         #T{BODY11|BODY12|BODY13~BODY21|BODY22|BODY23&BODY31|BODY32|BODY33}
 *             Produced list (approximation)
 *                  +========+========+========+
 *                  | BODY11 | BODY12 | BODY13 |
 *                  +========+========+========+
 *                  | BODY21 | BODY22 | BODY23 |
 *                  +--------+--------+--------+
 *                  | BODY31 | BODY32 | BODY33 |
 *                  +========+========+========+
 *             Syntax:
 *               - #T{ begins the list and } ends the list
 *               - Cells are filled left->right, top->bottom
 *               - The next horizontal cells is started with a |-character
 *               - The next line is started by a...
 *                  - ... ~-character for a thick line
 *                  - ... &-character for a thin line
 *               - A list where certain lines contain a different # of cells than other lines
 *                 that came before or come after is encoded, rendering that list causes weak
 *                 undefined behavior, in that it is up to the renderer what to do with superfluous
 *                 cells, or cells that are missing.
 *               - At the start of a cell, an additional {...} block may be placed where ...
 *                 is a ,-separated list of additional renderer options. - The following list
 *                 of options is recognized:
 *                 - tl, t, tr, l, c, r, bl, b, br -- Set the alignment of the cell's contents. (default: tl)
 *                 - ...                           -- Any other rendering option should silently
 *                                                    by ignored by the rendering engine in order
 *                                                    to allow for forward-compatibility with
 *                                                    future options.
 *
 *   - Parameter/return/throws descriptions
 *         #pNAME{BODY}         Description on parameter `NAME' (NAME is handled similarly to `@NAME')
 *         #p{NAME}{BODY}       ...
 *
 *         #r{BODY}             Description on function return value
 *
 *         #tNAME{BODY}         Description on exception NAME (which is usually a symbol reference)
 *         #t{NAME}{BODY}       Note that #tNAME{BODY} is treated the same as #t{:NAME}{BODY}.
 *         #t{NAME}             When there is no body, either show a default body, or
 *                              only indicate that objects of this type can be thrown.
 *
 *
 *   - Symbol references
 *         ?.                   The current type in the documentation of a type-header, operator, or this-function
 *                              For global (non-class) symbols, this references the current module the same as `?M{.}'
 *         ?Mposix              Module reference to `import("posix")'
 *         ?M{posix}            ...
 *         ?M{.}                Module reference to the current module
 *         ?Afoo?<ref>          Reference to an attribute `foo' of ?<ref> (where `ref' must be one of those that start with `?')
 *         ?A{foo}?<ref>        ...   (e.g. ?Aid?O for `deemon.Object.id')
 *         ?Eposix:errno        Same as ?Aerrno?Mposix
 *         ?E{posix}:errno      Same as ?Aerrno?M{posix}
 *         ?Eposix:{errno}      Same as ?A{errno}?Mposix
 *         ?E{posix}:{errno}    Same as ?A{errno}?M{posix}
 *         ?#foo                Same as ?Afoo?.
 *         ?#{foo}              Same as ?A{foo}?.
 *         ?#{op:call}          Same as ?A{op:call}?.
 *         ?Gfoo                Same as ?Afoo?M{.}
 *         ?G{foo}              Same as ?A{foo}?M{.}
 *         ?Dint                Same as ?Edeemon:int
 *         ?D{int}              Same as ?Edeemon:{int}
 *         ?O                   Same as ?Edeemon:Object
 *         ?N                   Same as ?Edeemon:none
 *         ?t                   Same as ?Edeemon?t
 *         ?f                   Same as ?Edeemon?f
 *         ?R!Afoo]             Same as @foo     (argument reference)
 *         ?R!A{foo}]           Same as @{foo}   (argument reference)
 *         ?R!t]                Same as ?t
 *         ?R!f]                Same as ?f
 *         ?R!Dint]             Same as ?Dint
 *         ?R!D{int}]           Same as ?D{int}
 *         ?R!Eposix:errno]     Same as ?Eposix:errno
 *         ?R!E{posix}:errno]   Same as ?E{posix}:errno
 *         ?R!Eposix:{errno}]   Same as ?Eposix:{errno}
 *         ?R!E{posix}:{errno}] Same as ?E{posix}:{errno}
 *         ?R!Gfoo]             Same as ?Gfoo
 *         ?R!G{foo}]           Same as ?G{foo}
 *         ?R!Mposix]           Same as ?Mposix
 *         ?R!M{posix}]         Same as ?M{posix}
 *         ?R!M{.}]             Same as ?M{.}
 *         ?R!N]                Same as ?N
 *         ?R!#foo]             Same as ?#foo
 *         ?R!#{foo}]           Same as ?#{foo}
 *         ?R!#{op:call}]       Same as ?#{op:call}
 *         @foo                 Reference to an argument `foo' of the current function (only allowed for function doc texts)
 *         @{foo}               ...
 *         @this                Reference to the hidden this-argument
 *         @{this}              ...
 *         :foo                 Multi-purpose. Same as the first match from (in order):
 *                               - ?Dfoo         (Globals within the deemon module)
 *                               - ?Eerrors:foo  (The name of a builtin Error or Signal class)
 *         :{foo}               ...
 *
 *     When no {}-block is used, the string must allow for issymbol().
 *     Otherwise, when a {}-block is used, the contained string `name'
 *     is escaped as follows:
 *     >> if (!name.issymbol()) {
 *     >>     for (local x: r'#$%&~^{}[]|?*@-+')
 *     >>          name = name.replace(x, r'#' + x);
 *     >>     name = "{" + name + "}";
 *     >> }
 *
 * NOTE: The !-encoding is fully documented under EXPR-ENCODING in "deemon/compiler/symbol.h"
 */

/* Compile documentation text in `doctext' into itself.
 * This function scans `doctext' according to `FORMAT',
 * then re-writes `doctext' to contain the equivalent
 * as described by `ENCODING'.
 * NOTE: This function should be called by the compiler
 *       in the context of the declaration being annotated,
 *       such that in the case of a function being annotated,
 *       argument variables are visible. */
INTDEF WUNUSED NONNULL((1)) int DCALL
doctext_compile(struct Dee_unicode_printer *__restrict doctext);

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_DOCTEXT_H */
