NO NEW CONTENT MUST BE ADDED TO THIS FOLDER

The files in here are _ONLY_ meant for backwards compatibility with deemon 1.0+,
and new system functions should be added as modules stored in the folder one level
above the folder that this file is located inside of.

In v200, deemon 2.0+ has been overhauled when it comes to how scripts should
interface with functionality provided by other files. - Gone is the approach
of using c-style headers with no way of even declaring deemon functions in a
way that would make them behave extern, requiring everything to be implemented
as inline or builtins.
Now deemon uses a module-based approach, similar to what can also be seen in
python, meaning that inter-connecting multiple script files is actually possible
now, as well as a drastic simplification of how (and which) components should be
used from system libraries.
The downside of this is that this makes the existing system become obsolete.
Note however that any library path passed to deemon via the -L option is also
implicity a #include-path after appending `/include' to that path, which is how
compatibility with legacy code can be provided, and also the reason why this
folder is called `include'. However, since new code should not strive to maintain
backwards compatibility, no new functionality will be provided in the form of
files that need to be #include-ed.

