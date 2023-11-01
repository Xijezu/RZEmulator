#include <iostream>
#include <fstream>
#include "NGInit.h"
#include <string.h>
#include <vector>

#define STR_DATE_BUFFER 128
struct RDB_HEADER
{
	RDB_HEADER()
	{
		memset( szDate, 0, sizeof(szDate) );
		nCount = 0;
	}

	char szDate[STR_DATE_BUFFER]; 
	int  nCount;
};

std::streampos fileSize( const char* filePath ){

    std::streampos fsize = 0;
    std::ifstream file( filePath, std::ios::binary );

    fsize = file.tellg();
    file.seekg( 0, std::ios::end );
    fsize = file.tellg() - fsize;
    file.close();

    return fsize;
}

int main(int argc, char **argv)
{
    auto [bInitialized, ioContext] = NGemity::InitFramework("mononoke.conf", "mononoke", argc, argv);

    if (!bInitialized)
        return -1;

    std::ifstream instream("db_skill.rdb", std::ios::binary);
    if(!instream.is_open()) {
        NG_LOG_ERROR("ngrdb", "Error opening file.");
        return -1;
    }
    RDB_HEADER db_hdr;
	auto file_size = fileSize("db_skill.rdb") - sizeof(db_hdr);
    instream.read(reinterpret_cast<char*>(&db_hdr), sizeof(db_hdr));
	int32_t entrySize = (int32_t)(file_size / db_hdr.nCount);
	NG_LOG_DEBUG("ngrdb", "There are %d entries. File Size is %d, total size per entry is %d", db_hdr.nCount, (int32_t)file_size, entrySize);
	std::vector<char*> list;
	list.reserve(db_hdr.nCount);
	
	for(int i = 0; i < db_hdr.nCount; i++) {
		char* entry = new char[entrySize];
		instream.read(entry, entrySize);
		list.emplace_back(entry);
	}

	for(auto entry : list) {
		delete[] entry;
	}


	NG_LOG_INFO("ngrdb", "Parsing done.");
	return 0;
}