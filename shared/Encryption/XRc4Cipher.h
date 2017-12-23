#pragma once

struct XRC4Cipher
{
public:

	XRC4Cipher();
	virtual ~XRC4Cipher();

	void SetKey( const char* pKey );

	// pSource 와 pTarget 이 같아도 무방함.
	virtual void Encode( const void* pSource, void* pTarget, unsigned len, bool bIsPeek = false );
	virtual void Decode( const void* pSource, void* pTarget, unsigned len, bool bIsPeek = false );
	virtual void Clear();

private:

	inline void tryCipher( const void* pSource, void* pTarget, unsigned len );
	inline void doCipher( const void* pSource, void* pTarget, unsigned len );

	struct TImpl;
	TImpl* m_pImpl;
};

