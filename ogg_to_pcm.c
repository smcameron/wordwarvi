/* OggDec
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2002, Michael Smith <msmith@xiph.org>
 *
 */

/* 
 *
 * This code was hacked off of the carcass of oggdec.c, from
 * the vorbistools-1.2.0 package, and is copyrighted as above, 
 * with the modifications made by me, 
 * (c) Copyright Stephen M. Cameron, 2008,
 * (and of course also released under the GNU General Public License, version 2.)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#if !defined(__APPLE__)
/* Apple gets what it needs for malloc from stdlib.h */
#include <malloc.h>
#endif

#include <vorbis/vorbisfile.h>

static int bits = 16;

/* returns true on a big endian machine, false elsewhere.
 * compiler ought to be smart enough to optimize this to nothing.
 */
static inline int we_are_big_endian()
{
	const uint32_t z = 0x01020304;
	const unsigned char *y = (unsigned char *) &z;
	return (y[0] == 0x01);
}

/* Reads an ogg vorbis file, infile, and dumps the data into
   a big buffer, *pcmbuffer (which it allocates via malloc)
   and returns the number of samples in *nsamples, and the
   samplesize in *samplesize. and etc.
*/
int ogg_to_pcm(char *infile, int16_t **pcmbuffer,
	int *samplesize, int *sample_rate, int *nchannels, 
	uint64_t *nsamples)
{
	int endian;
	OggVorbis_File vf;
	int bs = 0;
	char buf[8192];
	char *p_outbuf;
	int buflen = 8192;
	unsigned int written = 0;
	int ret;
	ogg_int64_t length = 0;
	int size = 0;
	int seekable = 0;
	int channels;
	int samplerate;
	unsigned char *bufferptr;
	int sign = 1;
	FILE *in;
	int count=0;

	in = fopen(infile, "r");
	if (in == NULL) {
		fprintf(stderr, "%s:%d ERROR: Failed to open '%s' for read: '%s'\n", 
			__FILE__, __LINE__, infile, strerror(errno));
		return -1;
	}

	if(ov_open(in, &vf, NULL, 0) < 0) {
		fprintf(stderr, "%s:%d: ERROR: Failed to open '%s' as vorbis\n", 
			__FILE__, __LINE__, infile);
		fclose(in);
		return -1;
	}

	channels = ov_info(&vf,0)->channels;
	samplerate = ov_info(&vf,0)->rate;
	*sample_rate = samplerate;
	*nchannels = channels;

	if (ov_seekable(&vf)) {
		int link;
		int chainsallowed = 0;
		for(link = 0; link < ov_streams(&vf); link++) {
			if (ov_info(&vf, link)->channels == channels && 
			    ov_info(&vf, link)->rate == samplerate) {
				chainsallowed = 1;
			}
		}

		seekable = 1;
		if (chainsallowed)
			length = ov_pcm_total(&vf, -1);
		else
			length = ov_pcm_total(&vf, 0);
		size = bits/8 * channels;
	}

	*nsamples = length;

	*pcmbuffer = (void *) malloc(sizeof(int16_t) * length * channels);
	memset(*pcmbuffer, 0, sizeof(int16_t) * length * channels);
	if (*pcmbuffer == NULL) {
		fprintf(stderr, "%s:%d: Failed to allocate memory for '%s'\n",
			__FILE__, __LINE__, infile);
		fclose(in);
		return -1;
	}
	bufferptr = (unsigned char *) *pcmbuffer;

	endian = we_are_big_endian();

	while ((ret = ov_read(&vf, buf, buflen, endian, bits/8, sign, &bs)) != 0) {
		if (bs != 0) {
			vorbis_info *vi = ov_info(&vf, -1);
			if (channels != vi->channels || samplerate != vi->rate) {
				fprintf(stderr, "%s:%d: Logical bitstreams with changing "
					"parameters are not supported\n",
					__FILE__, __LINE__);
				break;
			}
		}

		if(ret < 0 ) {
			fprintf(stderr, "%s:%d: Warning: hole in data (%d)\n",
				__FILE__, __LINE__, ret);
			continue;
		}

		p_outbuf = buf;

		/* copy the data to the pcmbuffer. */
		memcpy(bufferptr, p_outbuf, ret);
		bufferptr += ret; 
		count+=ret;
		written += ret;
	}

	/* ov_clear closes the file, so don't fclose here, even though we fopen()ed.
	 * libvorbis is weird that way.
 	 */
	ov_clear(&vf);

	return 0;
}
