#!/usr/local/bin/python
# testsha1.py
# under Python 2.4
#
# Copyright (c) 2005 Michael D. Leonhard
# 
# http://tamale.net/
# 
# This file is licensed under the terms described in the
# accompanying LICENSE file.

import sha1, sys

print "sha1, Copyright (C) 2005 Michael Leonhard (http://tamale.net/)"
if len( sys.argv ) > 1:
	shaHash = sha1.sha1()
	try:
		print "Reading " + sys.argv[1] + "..."
		theFile = file( sys.argv[1], 'rb' )
		while True:
			# read a chunk
			chunk = theFile.read( 8192 )
			# reached end of file, return the header
			if len( chunk ) == 0: break;
			# process the chunk
			shaHash.update( chunk )
		theFile.close()
		print "SHA-1: " + shaHash.hexdigest()
	except IOError, e: print e
else:
	print "Testing module with examples from RFC 3174"
	TEXT1 = "abc"
	DIGEST1 = " a9993e36 4706816a ba3e2571 7850c26c 9cd0d89d"
	shaHash = sha1.sha1()
	shaHash.update( TEXT1 )
	print TEXT1 + ":\n " + shaHash.hexdigest() + "\n" + DIGEST1

	TEXT2 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
	DIGEST2 = " 84983e44 1c3bd26e baae4aa1 f95129e5 e54670f1"
	shaHash = sha1.sha1()
	shaHash.update( TEXT2 )
	print TEXT2 + ":\n " + shaHash.hexdigest() + "\n" + DIGEST2

	TEXT3 = "a"
	INFO3 = "a X 1000000"
	DIGEST3 = " 34aa973c d4c4daa4 f61eeb2b dbad2731 6534016f"
	shaHash = sha1.sha1()
	x = 0;
	while x < 1000000:
		x += 1
		shaHash.update( TEXT3 )
	print INFO3 + ":\n " + shaHash.hexdigest() + "\n" + DIGEST3

	TEXT4 = "0123456701234567012345670123456701234567012345670123456701234567"
	INFO4 = "0123456701234567012345670123456701234567012345670123456701234567 X 10"
	DIGEST4 = " dea356a2 cddd90c7 a7ecedc5 ebb56393 4f460452"
	shaHash = sha1.sha1()
	x = 0;
	while x < 10:
		x += 1
		shaHash.update( TEXT4 )
	print INFO4 + ":\n " + shaHash.hexdigest() + "\n" + DIGEST4
