//
// DES
//
// $Revision: $
// $Date: 10/27/01 9:14a $
// $Author: $

/* des: duplicate the NBS Data Encryption Standard in software.
 * usage: des <file>
 * prompts for the password
 * If the filename ends in ".n" it will be decrypted with the key;
 * otherwise it will be encrypted.
 *
 * Permutation algorithm:
 *      The permutation is defined by its effect on each of the 16 nibbles
 *      of the 64-bit input.  For each nibble we give an 8-byte bit array
 *      that has the bits in the input nibble distributed correctly.  The
 *      complete permutation involves ORing the 16 sets of 8 bytes designated
 *      by the 16 input nibbles.  Uses 16*16*8 = 2K bytes of storage for
 *      each 64-bit permutation.  32-bit permutations (P) and expansion (E)
 *      are done similarly, but using bytes instead of nibbles.
 *      Should be able to use long ints, adding the masks, at a
 *      later pass.  Tradeoff: can speed 64-bit perms up at cost of slowing
 *      down expansion or contraction operations by using 8K tables here and
 *      decreasing the size of the other tables.
 * The compressions are pre-computed in 12-bit chunks, combining 2 of the
 *      6->4 bit compressions.
 * The key schedule is also precomputed.
 * Compile with VALIDATE defined to run the NBS validation suite.
 *
 * Jim Gillogly, May 1977
 * Modified 8/84 by Jim Gillogly and Lauren Weinstein to compile with
 *   post-1977 C compilers and systems
 *
 * This program is now officially in the public domain, and is available for
 * any non-profit use as long as the authorship line is retained.
 */

/*#define VALIDATE      */ /* define to check the NBS validation suite */
/*#define DEBUG         */
/*#define LATTICE       */ /* define for Lattice C on IBM PC */


#include <signal.h>
#ifdef _WIN32
#include <sys/types.h>                  /* for local timer */
#include <sys/timeb.h>                  /* ditto */
#endif	// _WIN32
#include <assert.h>
#include <string.h>
#include "XDes.h"

const static char ip[]                                /* initial permutation P        */
= {
 58, 50, 42, 34, 26, 18, 10, 2,
 60, 52, 44, 36, 28, 20, 12, 4,
 62, 54, 46, 38, 30, 22, 14, 6,
 64, 56, 48, 40, 32, 24, 16, 8,
 57, 49, 41, 33, 25, 17, 9, 1,
 59, 51, 43, 35, 27, 19, 11, 3,
 61, 53, 45, 37, 29, 21, 13, 5,
 63, 55, 47, 39, 31, 23, 15, 7
};

const static char fp[]                                /* final permutation F    */
= {
 40, 8, 48, 16, 56, 24, 64, 32,
 39, 7, 47, 15, 55, 23, 63, 31,
 38, 6, 46, 14, 54, 22, 62, 30,
 37, 5, 45, 13, 53, 21, 61, 29,
 36, 4, 44, 12, 52, 20, 60, 28,
 35, 3, 43, 11, 51, 19, 59, 27,
 34, 2, 42, 10, 50, 18, 58, 26,
 33, 1, 41, 9, 49, 17, 57, 25
};


const static char pc1[]                               /* permuted choice packetHandler (key)  */
= {
 57, 49, 41, 33, 25, 17, 9,
 1, 58, 50, 42, 34, 26, 18,
 10, 2, 59, 51, 43, 35, 27,
 19, 11, 3, 60, 52, 44, 36,

 63, 55, 47, 39, 31, 23, 15,
 7, 62, 54, 46, 38, 30, 22,
 14, 6, 61, 53, 45, 37, 29,
 21, 13, 5, 28, 20, 12, 4
};

/* number left rotations of pc1 */
const static char totrot[] = {1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28};

const static char pc2[]                               /* permuted choice key (packetHandler)  */
= {
 14, 17, 11, 24, 1, 5,
 3, 28, 15, 6, 21, 10,
 23, 19, 12, 4, 26, 8,
 16, 7, 27, 20, 13, 2,
 41, 52, 31, 37, 47, 55,
 30, 40, 51, 45, 33, 48,
 44, 49, 39, 56, 34, 53,
 46, 42, 50, 36, 29, 32
};

const static char si[8][64]                           /* 48->32 bit compression tables*/
= {
/* S[1]                  */
	14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
	0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
	4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
	15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13,
/* S[2]                  */
	15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
	3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
	0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
	13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9,
/* S[3]                  */
	10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
	13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
	1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12,
/* S[4]                  */
	7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
	13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
	10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
	3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14,
/* S[5]                  */
	2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
	14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
	4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
	11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3,
/* S[6]                  */
	12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
	10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
	9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
	4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13,
/* S[7]                  */
	4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
	13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
	1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
	6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12,
/* S[8]                  */
	13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
	1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
	7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
	2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11
};

const static char p32i[]                              /* 32-bit permutation function  */
= {
 16, 7, 20, 21,
 29, 12, 28, 17,
 1, 15, 23, 26,
 5, 18, 31, 10,
 2, 8, 24, 14,
 32, 27, 3, 9,
 19, 13, 30, 6,
 22, 11, 4, 25
};

/* bit 0 is left-most in byte   */
const static int bytebit[] = {0200, 0100, 040, 020, 010, 04, 02, 01};
const static int nibblebit[] = {010, 04, 02, 01};



#define REGISTER

/* permute inblock with perm : inblock, perm, outblock) */
/*    char perm[16][16][8],     2K bytes defining perm. */
/*    char *outblock)            result into outblock,64 bits */
void permute( char *inblock, char perm[16][16][8], char *outblock )
{
	REGISTER int i, j;
	REGISTER char *ib, *ob;             /* ptr to input or output block */
	REGISTER char *p, *q;

	for (i = 0, ob = outblock; i < 8; i++)
		*ob++ = 0;                      /* clear output block           */
	ib = inblock;
	for (j = 0; j < 16; j += 2, ib++) { /* for each input nibble        */
		ob = outblock;
		p = perm[j][(*ib >> 4) & 0x0f];
		q = perm[j + 1][*ib & 0x0f];
		for (i = 0; i < 8; i++)         /* and each output byte */
			*ob++ |= *p++ | *q++;       /* OR the masks together*/
	}
}


//    char *inblock, *outblock;          of the f crypto function
void XDes::perm32(char *inblock, char *outblock)               /* 32-bit permutation at end    */
{
	REGISTER int j;

	REGISTER char *ib, *ob;
	REGISTER char *q;

	ob = outblock;                      /* clear output block           */
	*ob++ = 0;
	*ob++ = 0;
	*ob++ = 0;
	*ob++ = 0;
	ib = inblock;                       /* ptr to 1st byte of input     */
	for (j = 0; j < 4; j++, ib++) {     /* for each input byte          */
		q = m_p32[j][*ib & 0xff];
		ob = outblock;                  /* and each output byte         */
		*ob++ |= *q++;                  /* OR the 16 masks together     */
		*ob++ |= *q++;
		*ob++ |= *q++;
		*ob++ |= *q++;
	}
}

//    char *right, *bigright;           right is 32, bigright 48
void expand(char *right, char *bigright)                 /* 32 to 48 bits with E oper    */
{
	REGISTER char *bb, *r, r0, r1, r2, r3;

	bb = bigright;
	r = right;
	r0 = *r++;
	r1 = *r++;
	r2 = *r++;
	r3 = *r++;
	*bb++ = ((r3 & 0001) << 7) |        /* 32                           */
		((r0 & 0370) >> 1) |            /* 1 2 3 4 5                    */
		((r0 & 0030) >> 3);             /* 4 5                          */
	*bb++ = ((r0 & 0007) << 5) |        /* 6 7 8                        */
		((r1 & 0200) >> 3) |            /* 9                            */
		((r0 & 0001) << 3) |            /* 8                            */
		((r1 & 0340) >> 5);             /* 9 10 11                      */
	*bb++ = ((r1 & 0030) << 3) |        /* 12 13                        */
		((r1 & 0037) << 1) |            /* 12 13 14 15 16               */
		((r2 & 0200) >> 7);             /* 17                           */
	*bb++ = ((r1 & 0001) << 7) |        /* 16                           */
		((r2 & 0370) >> 1) |            /* 17 18 19 20 21               */
		((r2 & 0030) >> 3);             /* 20 21                        */
	*bb++ = ((r2 & 0007) << 5) |        /* 22 23 24                     */
		((r3 & 0200) >> 3) |            /* 25                           */
		((r2 & 0001) << 3) |            /* 24                           */
		((r3 & 0340) >> 5);             /* 25 26 27                     */
	*bb++ = ((r3 & 0030) << 3) |        /* 28 29                        */
		((r3 & 0037) << 1) |            /* 28 29 30 31 32               */
		((r0 & 0200) >> 7);             /* 1                            */
}

//    char *in48, *out32;                        using 12-bit pieces into bytes
void XDes::contract(char *in48, char *out32)                   /* contract f from 48 to 32 bits*/
{
	REGISTER char *c;
	REGISTER char *i;
	REGISTER int i0, i1, i2, i3, i4, i5;

	i = in48;
	i0 = *i++;
	i1 = *i++;
	i2 = *i++;
	i3 = *i++;
	i4 = *i++;
	i5 = *i++;
	c = out32;                          /* do output a byte at a time   */
	*c++ = m_s[0][0xfff & ((i0 << 4) | ((i1 >> 4) & 0x0f))];
	*c++ = m_s[1][0xfff & ((i1 << 8) | (i2 & 0xff))];
	*c++ = m_s[2][0xfff & ((i3 << 4) | ((i4 >> 4) & 0x0f))];
	*c++ = m_s[3][0xfff & ((i4 << 8) | (i5 & 0xff))];
}


//    char *right, *fret;                        32 bits each
//    int num;                          index number of this iter
void XDes::f(char *right, int num, char *fret)                 /* critical cryptographic trans */
{
	REGISTER char *kb, *rb, *bb;        /* ptr to key selection &c      */
	char bigright[6];                   /* right expanded to 48 bits    */
	char result[6];                     /* expand(R) XOR keyselect[num] */
	char preout[4];                     /* result of 32-bit permutation */

	kb = m_kn[num];                       /* fast version of iteration    */
	bb = bigright;
	rb = result;
	expand(right, bb);                  /* expand to 48 bits            */
	*rb++ = *bb++ ^ *kb++;              /* expanded R XOR chunk of key  */
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	contract(result, preout);           /* use S fns to get 32 bits     */
	perm32(preout, fret);               /* and do final 32-bit perm     */
}


/*
	int num;                             i.e. the num-th one
	char *inblock, *outblock;            64 bits each
*/

void XDes::iter(int num, char *inblock, char *outblock)        /* 1 churning operation         */
{
	char fret[4];                       /* return from f(R[i-1],key)    */
	REGISTER char *ib, *ob, *fb;

	ob = outblock;
	ib = &inblock[4];
	f(ib, num, fret);                   /* the primary transformation   */
	*ob++ = *ib++;                      /* L[i] = R[i-1]                */
	*ob++ = *ib++;
	*ob++ = *ib++;
	*ob++ = *ib++;
	ib = inblock;
	fb = fret;                          /* R[i]=L[i] XOR f(R[i-1],key)  */
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
}

int getcomp(int k, int v)                                /* 1 compression value for sinit*/
{
	REGISTER int i, j;                  /* correspond to i and j in FIPS*/

	i = ((v & 040) >> 4) | (v & 1);     /* first and last bits make row */
	j = (v & 037) >> 1;                 /* middle 4 bits are column     */
	return (int) si[k][(i << 4) + j];   /* result is ith row, jth col   */
}

void XDes::sinit(void)
{                                       /* initialize s1-s8 arrays              */
	REGISTER int i, j;

	for (i = 0; i < 4; i++)             /* each 12-bit position         */
		for (j = 0; j < 4096; j++)      /* each possible 12-bit value   */
			m_s[i][j] = (getcomp(i * 2, j >> 6) << 4) |
				(0x0f & getcomp(i * 2 + 1, j & 077));
	/* store 2 compressions per char*/
}


//    char *key;                                64 bits (will use only 56)
void XDes::kinit(char *key64bit)       /* initialize key schedule array*/
{
	REGISTER int i, j, l;
	int m;
	char pc1m[56];                           /* place to modify pc1 into     */
	char pcr[56];                            /* place to rotate pc1 into     */

	for (j = 0; j < 56; j++) {          /* convert pc1 to bits of key   */
		l = pc1[j] - 1;                 /* integer bit location         */
		m = l & 07;                     /* find bit                     */
		pc1m[j] = ( ( key64bit[l >> 3] & bytebit[m] ) ? 1 : 0 );
		/* find which key byte l is in and which bit of that byte and store 1-bit result  */
	}
	for (i = 0; i < 16; i++){           /* for each key sched section   */
		for (j = 0; j < 6; j++)         /* and each byte of the kn      */
			m_kn[i][j] = 0;               /* clear it for accumulation    */
	}
	for (i = 0; i < 16; i++) {          /* key chunk for each iteration */
		for (j = 0; j < 56; j++)        /* rotate pc1 the right amount  */
			pcr[j] = pc1m[(l = j + totrot[i]) < (j < 28 ? 28 : 56) ? l : l - 28];
		/* rotate left and right halves independently   */
		for (j = 0; j < 48; j++)        /* select bits individually     */
			if (pcr[pc2[j] - 1]) {      /* check bit that goes to m_kn[j] */
				l = j & 07;
				m_kn[i][j >> 3] |= bytebit[l];
			}                           /* mask it in if it's there     */
	}
}

void XDes::p32init(void)
{                                       /* initialize 32-bit permutation*/
	REGISTER int l, j, k;
	int i, m;

	for (i = 0; i < 4; i++)             /* each input byte position     */
		for (j = 0; j < 256; j++)       /* all possible input bytes     */
			for (k = 0; k < 4; k++)     /* each byte of the mask        */
				m_p32[i][j][k] = 0;       /* clear permutation array      */
	for (i = 0; i < 4; i++)             /* each input byte position     */
		for (j = 0; j < 256; j++)       /* each possible input byte     */
			for (k = 0; k < 32; k++) {  /* each output bit position     */
				l = p32i[k] - 1;        /* invert this bit (0-31)       */
				if ((l >> 3) != i)      /* does it come from input posn?*/
					continue;           /* if not, bit k is 0           */
				if (!(j & bytebit[l & 07]))
					continue;           /* any such bit in input?       */
				m = k & 07;             /* which bit is it?             */
				m_p32[i][j][k >> 3] |= bytebit[m];
			}
}

//    char perm[16][16][8];             64-bit, either init or final
//    char p[64];
void perminit(char perm[16][16][8], const char p[64])                  /* initialize a perm array      */
{
	REGISTER int l, j, k;
	int i, m;

	for (i = 0; i < 16; i++)            /* each input nibble position   */
		for (j = 0; j < 16; j++)        /* all possible input nibbles   */
			for (k = 0; k < 8; k++)     /* each byte of the mask        */
				perm[i][j][k] = 0;      /* clear permutation array      */
	for (i = 0; i < 16; i++)            /* each input nibble position   */
		for (j = 0; j < 16; j++)        /* each possible input nibble   */
			for (k = 0; k < 64; k++) {  /* each output bit position     */
				l = p[k] - 1;           /* where does this bit come from*/
				if ((l >> 2) != i)      /* does it come from input posn?*/
					continue;           /* if not, bit k is 0           */
				if (!(j & nibblebit[l & 3]))
					continue;           /* any such bit in input?       */
				m = k & 07;             /* which bit is this in the byte*/
				perm[i][j][k >> 3] |= bytebit[m];
			}
}


void XDes::endes(char *inblock, char *outblock)                /* encrypt 64-bit inblock       */
{
	char iters[17][8];                  /* workspace for each iteration */
	char swap[8];                       /* place to interchange L and R */
	REGISTER int i;
	REGISTER char *s, *t;

	permute(inblock, m_iperm, iters[0]);  /* apply initial permutation    */
	for (i = 0; i < 16; i++)            /* 16 churning operations       */
		iter(i, iters[i], iters[i + 1]);
	/* don't re-copy to save space  */
	s = swap;
	t = &iters[16][4];                  /* interchange left             */
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	t = &iters[16][0];                  /* and right                    */
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	permute(swap, m_fperm, outblock);     /* apply final permutation      */
}

//    char *inblock, *outblock;
void XDes::dedes(char *inblock, char *outblock)                /* decrypt 64-bit inblock       */
{
	char iters[17][8];                  /* workspace for each iteration */
	char swap[8];                       /* place to interchange L and R */
	REGISTER int i;
	REGISTER char *s, *t;

	permute(inblock, m_iperm, iters[0]);  /* apply initial permutation    */
	for (i = 0; i < 16; i++)            /* 16 churning operations       */
		iter(15 - i, iters[i], iters[i + 1]);
	/* reverse order from encrypting*/
	s = swap;
	t = &iters[16][4];                  /* interchange left             */
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	t = &iters[16][0];                  /* and right                    */
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	*s++ = *t++;
	permute(swap, m_fperm, outblock);     /* apply final permutation      */
}

// End of DES algorithm (except for calling desinit below)
void XDes::DesMem( void *buf, int mlen, int isencrypting )
{                                       // process the file
	char *b;

	for (b = (char*)buf; mlen > 0; mlen -= 8, b += 8) {                /* encrypt/decrypt 1 buffer-full 8-byte blocks */
		if (isencrypting) {
			endes(b, b);                /* don't need diff input, output*/
		} else {                        /* decrypting                   */
			dedes(b, b);                /* decrypt and output block     */
		}
	}
}

bool XDes::Encrypt(void *buf, int len)
{
	if( !(len & 0xfffffff8) ) return false;
	
	DesMem(buf, len & 0xfffffff8, 1);

	return true;
}

bool XDes::Decrypt(void *buf, int len)
{
	if( !(len & 0xfffffff8) ) return false;

	DesMem(buf, len & 0xfffffff8, 0);

	return true;
}

void XDes::Init( const char *password )
{                                       /* initialize all des arrays    */
	char    key64bit[8];
	char   *key ;
	int     i ;

	memset(key64bit, 0, sizeof(key64bit));

	key = key64bit ;
	for(i = 0; *password && i < 40; i++) {
		key[i % 8] ^= *password ;
		password++ ;
	}
	perminit(m_iperm, ip);                /* initial permutation          */
	perminit(m_fperm, fp);                /* final permutation            */
	kinit(key64bit);                    /* key schedule                 */
	sinit();                            /* compression functions        */
	p32init();                          /* 32-bit permutation in f      */
}


XDes::XDes( const char *password )
{
	if( password ) Init( password );
}

XDes::~XDes()
{
}

bool XDes::Encrypt( const char *password, void *buf, int len )
{
	return XDes( password ).Encrypt( buf, len );
}

bool XDes::Decrypt( const char *password, void *buf, int len )
{
	return XDes( password ).Decrypt( buf, len );
}