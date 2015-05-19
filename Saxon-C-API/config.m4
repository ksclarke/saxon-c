PHP_ARG_ENABLE(saxon,
    [Whether to enable the "saxon" extension],
    [  --enable-saxon      Enable "saxon" extension support])

if test $PHP_SAXON != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(SAXON_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, SAXON_SHARED_LIBADD)
    PHP_ADD_LIBRARY(saxon, 1, SAXON_SHARED_LIBADD)
    PHP_ADD_LIBRARY(dl, 1, SAXON_SHARED_LIBADD)
    PHP_NEW_EXTENSION(saxon, xsltProcessor.cc, $ext_shared)
fi

