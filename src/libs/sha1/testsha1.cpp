/* testsha1.cpp

Copyright (c) 2005 Michael D. Leonhard

http://tamale.net/

This file is licensed under the terms described in the
accompanying LICENSE file.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "sha1.h"

int main(int argc, char *argv[])
{
	SHA1* sha1;
	unsigned char* digest;
	int i;
	#define BUFFERSIZE 8192
	
	if( argc == 2 )
	{
		assert( argv[1] );
		/* open the file */
		int fd = open( argv[1], O_RDONLY , 0 );
		/* handle open failure */
		if( fd == -1 ) {
			fprintf( stderr, "cannot open file %s\n", argv[1] );
			return 1;
			}
		
		/* prepare to calculate the SHA-1 hash */
		sha1 = new SHA1();
		char* buffer = (char*)malloc( BUFFERSIZE );
		assert( buffer );
		
		/* loop through the file */
		int ret;
		while( true ) {
			/* read a chunk of data */
			ret = read( fd, buffer, BUFFERSIZE );
			/* check for error and end of file */
			if( ret < 1 ) break;
			/* run this data through the hash function */
			sha1->addBytes( buffer, ret );
			}
		
		/* close the file */
		close( fd );
		
		/* there was an error reading the file */
		if( ret == -1 ) {
			fprintf( stderr, "error reading %s.\n", argv[1] );
			return 1;
			}
		
		/* get the digest */
		digest = sha1->getDigest();
		assert( digest );
		/* print it out */
		printf( "%s:", argv[1] );
		sha1->hexPrinter( digest, 20 );
		printf( "\n" );
		fflush( stdout );
		delete sha1;
		free( digest );
		return 0;
	}
	
	// these example text blocks are taken from RFC3174
				
	#define TEXT1 "abc"
	#define DIGEST1 " a9 99 3e 36 47 06 81 6a ba 3e 25 71 78 50 c2 6c 9c d0 d8 9d"
	printf( "%s:\n%s\n", TEXT1, DIGEST1 );
	sha1 = new SHA1();
	sha1->addBytes( TEXT1, strlen( TEXT1 ) );
	digest = sha1->getDigest();
	sha1->hexPrinter( digest, 20 );
	delete sha1;
	free( digest );

	#define TEXT2 "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
	#define DIGEST2 " 84 98 3e 44 1c 3b d2 6e ba ae 4a a1 f9 51 29 e5 e5 46 70 f1"
	printf( "\n%s:\n%s\n", TEXT2, DIGEST2 );
	sha1 = new SHA1();
	sha1->addBytes( TEXT2, strlen( TEXT2 ) );
	digest = sha1->getDigest();
	sha1->hexPrinter( digest, 20 );
	delete sha1;
	free( digest );
	
	#define TEXT3 "a"
	#define INFO3 "a X 1000000"
	#define DIGEST3 " 34 aa 97 3c d4 c4 da a4 f6 1e eb 2b db ad 27 31 65 34 01 6f"
	printf( "\n%s:\n%s\n", INFO3, DIGEST3 );
	sha1 = new SHA1();
	for( i = 0; i < 1000000; i++ ) sha1->addBytes( TEXT3, 1 );
	digest = sha1->getDigest();
	sha1->hexPrinter( digest, 20 );
	delete sha1;
	free( digest );
	
	#define TEXT4 "0123456701234567012345670123456701234567012345670123456701234567"
	#define INFO4 "0123456701234567012345670123456701234567012345670123456701234567 X 10"
	#define DIGEST4 " de a3 56 a2 cd dd 90 c7 a7 ec ed c5 eb b5 63 93 4f 46 04 52"
	printf( "\n%s:\n%s\n", INFO4, DIGEST4 );
	sha1 = new SHA1();
	for( int i = 0; i < 10; i++ ) sha1->addBytes( TEXT4, strlen( TEXT4 ) );
	digest = sha1->getDigest();
	sha1->hexPrinter( digest, 20 );
	delete sha1;
	free( digest );
	
	return 0;
}
