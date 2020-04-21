#pragma once
#if !defined(_HASH)
# define _HASH                      "@rev_hash@"
#endif
#if !defined(_DATE)
# define _DATE                      "@rev_date@"
#endif
#if !defined(_BRANCH)
# define _BRANCH                    "@rev_branch@"
#endif
#if !defined(VER_COMPANYNAME_STR)
# define VER_COMPANYNAME_STR        "NGemity"
#endif
#if !defined(VER_LEGALCOPYRIGHT_STR)
# define VER_LEGALCOPYRIGHT_STR     "(c)2016-2020 NGemity"
#endif
#if !defined(VER_FILEVERSION)
# define VER_FILEVERSION            0,0,0
#endif
#if !defined(VER_FILEVERSION_STR)
# define VER_FILEVERSION_STR        "@rev_hash@ @rev_date@ (@rev_branch@ branch)"
#endif
#if !defined(VER_PRODUCTVERSION)
# define VER_PRODUCTVERSION         VER_FILEVERSION
#endif
#if !defined(VER_PRODUCTVERSION_STR)
# define VER_PRODUCTVERSION_STR     VER_FILEVERSION_STR
#endif