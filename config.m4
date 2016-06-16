dnl $Id$

PHP_ARG_ENABLE(mathstat, whether to enable mathstat support,
[  --enable-mathstat           Enable mathstat support])

if test "$PHP_MATHSTAT" != "no"; then
  PHP_NEW_EXTENSION(mathstat, mathstat.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
