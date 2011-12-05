dnl# -*- sh -*-
dnl# locate lsf
dnl#
dnl# Prototype:
dnl#     CHECK_DIR(dir_value, file_in_dir)
# -----------------------------------------------------------------------------
AC_DEFUN([CHECK_DIR],[
    AS_IF([test ! -d "$1"],
        [AC_MSG_RESULT([not found])
            AC_MSG_WARN([Directory $1 not found])
            AC_MSG_ERROR([Cannot continue])],
        [AS_IF([test -f "$1/$2"],
            [AC_MSG_RESULT([ok ($1)])],
            [AC_MSG_RESULT([file not found $1/$2])
                AC_MSG_WARN([Expected file $1/$2 not found])
                AC_MSG_ERROR([Cannot continue])]
            )
        ]
    )
])dnl
dnl# Prototype:
dnl#     CHECK_LSF(prefix, [action-if-found], [action-if-not-found])
dnl# --------------------------------------------------------------------------
AC_DEFUN([CHECK_LSF],[
    AC_ARG_WITH([lsf],
        [AC_HELP_STRING([--with-lsf(=DIR)],
             [Build LSF support (enabled by default)])])
    AC_ARG_WITH([lsf-envdir],
       [AC_HELP_STRING([--with-lsf-envdir=DIR],
             [Find lsf.conf in DIR instead of $LSF_ENVDIR])])
    AC_ARG_WITH([lsf-libdir],
       [AC_HELP_STRING([--with-lsf-libdir=DIR],
             [Search for LSF libraries in DIR])])

    # Defaults
    check_lsf_dir=
    check_lsf_envdir=
    check_lsf_libdir=
    check_lsf_dir_msg="default"
    check_lsf_envdir_msg="default"
    check_lsf_libdir_msg="default"

    dnl# Enable LSF by default
    AS_IF(
        [test -z "$with_lsf" -o "$with_lsf" = "yes" ],
        [AC_MSG_RESULT([LSF is enabled])
          with_lsf="yes"
        ],
        [test "$with_lsf" = "no" ],
        [AC_MSG_RESULT([LSF is disabled])
          with_lsf_envdir="no"
          with_lsf_libdir="no"
        ],
        [
          # Save directory name supplied
          check_lsf_dir="$with_lsf"
          check_lsf_dir_msg="$check_lsf_dir (from --with-lsf)"
          AC_MSG_CHECKING([--with-lsf directory $with_lsf])
          CHECK_DIR([$with_lsf], [include/lsf/lsbatch.h])
        ]
    )

    AS_IF(
        [test ! -z "$with_lsf_envdir" -a "with_lsf_envdir" != "yes" -a "$with_lsf_envdir" != "no"],
        [
          # Save directory name supplied
          check_lsf_envdir="$with_lsf_envdir"
          check_lsf_envdir_msg="$check_lsf_envdir (from --with-lsf-envdir)"

          AC_CHECKING([--with-lsf-envdir directory $with_lsf_envdir])
          CHECK_DIR([$with_lsf_envdir], [lsf.conf])
          AC_MSG_RESULT([ok])
        ]
    )

    AS_IF(
        [test ! -z "$with_lsf_libdir" -a "with_lsf_libdir" != "yes" -a "$with_lsf_libdir" != "no"],
        [
          # Save directory name supplied
          check_lsf_libdir="$with_lsf_libdir"
          check_lsf_libdir_msg="$check_lsf_libdir (from --with-lsf-libdir)"

          AC_CHECKING([--with-lsf-libdir directory $with_lsf_libdir])
          CHECK_DIR([$with_lsf_libdir], [libbat.a])
        ]
    )

    # If no directory specified, verify LSF_ENVDIR
    AS_IF([test -z "$check_lsf_envdir" -a "$with_lsf" != "no" ],
          [AS_IF([test ! -z "$LSF_ENVDIR" -a -f "$LSF_ENVDIR/lsf.conf"],
                 [AC_MSG_CHECKING([checking for LSF env dir])
                   AC_MSG_RESULT($LSF_ENVDIR)
                 ],
                 [AC_MSG_WARN([LSF support requested but $LSF_ENVDIR/lsf.conf not found. Use --without-lsf to disable.])
                   AC_MSG_ERROR([Aborting.])
                 ])
    ])

    # If no directory specified, look for LSF_INCLUDEDIR, and/or LSF_ENVDIR.
    AS_IF([test -z "$check_lsf_dir" -a "$with_lsf" != "no" ],
          [AS_IF([test ! -z "$LSF_ENVDIR" -a -z "$LSF_INCLUDEDIR" -a -f "$LSF_ENVDIR/lsf.conf"],
                 [LSF_INCLUDEDIR=`egrep ^LSF_INCLUDEDIR= $LSF_ENVDIR/lsf.conf | cut -d= -f2-`])
           AS_IF([test ! -z "$LSF_INCLUDEDIR"],
                 [check_lsf_dir=`dirname $LSF_INCLUDEDIR`
                  check_lsf_dir_msg="$check_lsf_dir (parent from \$LSF_INCLUDEDIR)"])
    ])

    # If no directory specified, look for LSF_LIBDIR, and/or LSF_ENVDIR.
    AS_IF([test -z "$check_lsf_libdir" -a "$with_lsf_libdir" != "no" ],
          [AS_IF([test ! -z "$LSF_ENVDIR" -a -z "$LSF_LIBDIR" -a -f "$LSF_ENVDIR/lsf.conf"],
                 [LSF_LIBDIR=`egrep ^LSF_LIBDIR= $LSF_ENVDIR/lsf.conf | cut -d= -f2-`])
           AS_IF([test ! -z "$LSF_LIBDIR"],
                 [check_lsf_libdir=$LSF_LIBDIR
                  check_lsf_libdir_msg="$LSF_LIBDIR (from \$LSF_LIBDIR)"])
    ])

    # verify that all the directories actually exist
    AS_IF([test "$with_lsf" = "no"],
          [check_lsf_ok="no"],
          [check_lsf_ok="yes"]
          )

    AS_IF([test "$check_lsf_ok" = "yes"],
          [AC_MSG_CHECKING([for LSF dir])
           AC_MSG_RESULT([$check_lsf_dir_msg])
           AC_MSG_CHECKING([for LSF library dir])
           AC_MSG_RESULT([$check_lsf_libdir_msg])
           AS_IF([test ! -d "$check_lsf_dir"], [check_lsf_ok="no"])
           AS_IF([test ! -d "$check_lsf_libdir"], [check_lsf_ok="no"])
           ])

    # Reset for the next time we're called
    check_lsf_dir=
    check_lsf_libdir=

    AS_IF([test "$check_lsf_ok" = "no"],
          [AS_IF(
              [test ! -z "$with_lsf" -a "$with_lsf" != "no"],
              [AC_MSG_WARN([LSF support requested but not found. Use --without-lsf to disable.])
                AC_MSG_ERROR([Aborting.])
              ])
          ])

    AC_SUBST(LSF_ENVDIR)
    AC_SUBST(LSF_INCLUDEDIR)
    AC_SUBST(LSF_LIBDIR)
])

CHECK_LSF()

dnl# -------------------------------------------------------------- end-of-file
