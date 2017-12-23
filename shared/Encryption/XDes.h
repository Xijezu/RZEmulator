#pragma once

// 어디선가 긁어온 des 암호화 알고리즘.
// 8byte 이하는 encrypt 안됨.

class XDes
{
public:
	XDes( const char *password = NULL );
	virtual ~XDes();

	void Init( const char *password );
	bool Encrypt( void *buf, int len );
	bool Decrypt( void *buf, int len );

	// 이하는 사실 XDes( password ).Encrypt( buf, len ); 식으로 쓰는거나 차이 없음.
	static bool Encrypt( const char *password, void *buf, int len );
	static bool Decrypt( const char *password, void *buf, int len );

private:

	// End of DES algorithm (except for calling desinit below)
	void DesMem( void *buf, int mlen, int isencrypting );

	// 32-bit permutation at end
	void perm32(char *inblock, char *outblock);

	// contract f from 48 to 32 bits
	void contract(char *in48, char *out32);                   

	// critical cryptographic trans
	void f(char *right, int num, char *fret);                 

	// 1 churning operation
	void iter(int num, char *inblock, char *outblock);        

	// initialize s1-s8 arrays
	void sinit(void);

	// initialize key schedule array
	void kinit(char *key64bit);

	// initialize 32-bit permutation
	void p32init(void);

	// encrypt 64-bit inblock
	void endes(char *inblock, char *outblock);

	// decrypt 64-bit inblock
	void dedes(char *inblock, char *outblock);

	// inital and final permutations
	char m_iperm[16][16][8];
	char m_fperm[16][16][8];
	
	char m_s[4][4096];				// S1 thru S8 precomputed
	char m_p32[4][256][4];			// for permuting 32-bit f output
	char m_kn[16][6];				// key selections

};