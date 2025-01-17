/* -*- c++ -*- */
/* 
 * Copyright 2013 Antti Palosaari <crope@iki.fi>
 * Copyright 2022 Albert Briscoe
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libv4l2.h>
#include <fcntl.h>
#include "libv4l2_x_impl.h"
#include <gnuradio/io_signature.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

/* Control classes */
#define V4L2_CTRL_CLASS_USER          0x00980000 /* Old-style 'user' controls */
/* User-class control IDs */
#define V4L2_CID_BASE                 (V4L2_CTRL_CLASS_USER | 0x900)
#define V4L2_CID_USER_BASE            V4L2_CID_BASE

#define CID_SAMPLING_MODE             ((V4L2_CID_USER_BASE | 0xf000) +  0)
#define CID_SAMPLING_RATE             ((V4L2_CID_USER_BASE | 0xf000) +  1)
#define CID_SAMPLING_RESOLUTION       ((V4L2_CID_USER_BASE | 0xf000) +  2)
#define CID_TUNER_RF                  ((V4L2_CID_USER_BASE | 0xf000) + 10)
#define CID_TUNER_BW                  ((V4L2_CID_USER_BASE | 0xf000) + 11)
#define CID_TUNER_IF                  ((V4L2_CID_USER_BASE | 0xf000) + 12)
#define CID_TUNER_GAIN                ((V4L2_CID_USER_BASE | 0xf000) + 13)

#define V4L2_PIX_FMT_SDR_U8     v4l2_fourcc('C', 'U', '0', '8') /* unsigned 8-bit */
#define V4L2_PIX_FMT_SDR_U16LE  v4l2_fourcc('C', 'U', '1', '6') /* unsigned 16-bit LE */

#define CLEAR(x) memset(&(x), 0, sizeof(x))

static void xioctl(int fh, unsigned long int request, void *arg)
{
	int ret;

	do {
		ret = v4l2_ioctl(fh, request, arg);
	} while (ret == -1 && ((errno == EINTR) || (errno == EAGAIN)));
	if (ret == -1) {
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

namespace gr {
	namespace kernel {

	libv4l2_x::sptr
	libv4l2_x::make(const char *filename, double samp_rate, double freq, double bandwidth, double gain)
	{
		return gnuradio::make_block_sptr<libv4l2_x_impl>(filename, samp_rate, freq, bandwidth, gain);
	}

	/*
	 * The private constructor
	 */
	libv4l2_x_impl::libv4l2_x_impl(const char *filename,
	                               double samp_rate,
	                               double freq,
	                               double bandwidth,
	                               double gain)
		: gr::sync_block("libv4l2_x",
				gr::io_signature::make(0, 0, 0),
				gr::io_signature::make(1, 1, sizeof (gr_complex)))
	{
		struct v4l2_format fmt;
		struct v4l2_buffer buf;
		struct v4l2_requestbuffers req;
		enum v4l2_buf_type type;
		unsigned int i;

		recebuf_len = 0;

		// libv4l2 does not know SDR yet
		// fd = v4l2_open(filename, O_RDWR | O_NONBLOCK, 0);
		fd = open(filename, O_RDWR | O_NONBLOCK, 0);
		if (fd < 0) {
			perror("Cannot open device");
			exit(EXIT_FAILURE);
		}

		pixelformat = V4L2_PIX_FMT_SDR_U8;
		//pixelformat = V4L2_PIX_FMT_SDR_U16LE;

		CLEAR(fmt);
		fmt.type = V4L2_BUF_TYPE_SDR_CAPTURE;
		fmt.fmt.sdr.pixelformat = pixelformat;
		xioctl(fd, VIDIOC_S_FMT, &fmt);
		if (fmt.fmt.sdr.pixelformat != pixelformat) {
			printf("Libv4l didn't accept FLOAT format. Cannot proceed. Pixelformat %4.4s\n",
					(char *)&fmt.fmt.sdr.pixelformat);
			exit(EXIT_FAILURE);
		}

		printf("Selected stream format: %4.4s\n", (char *)&fmt.fmt.sdr.pixelformat);

		CLEAR(req);
		req.count = 8;
		req.type = V4L2_BUF_TYPE_SDR_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;
		xioctl(fd, VIDIOC_REQBUFS, &req);

		buffers = (struct buffer*) calloc(req.count, sizeof(*buffers));
		for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_SDR_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = n_buffers;
			xioctl(fd, VIDIOC_QUERYBUF, &buf);

			buffers[n_buffers].length = buf.length;
			buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					fd, buf.m.offset);

			if (buffers[n_buffers].start == MAP_FAILED) {
				perror("mmap");
				exit(EXIT_FAILURE);
			}
		}

		for (i = 0; i < n_buffers; i++) {
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_SDR_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			xioctl(fd, VIDIOC_QBUF, &buf);
		}

		// start streaming
		type = V4L2_BUF_TYPE_SDR_CAPTURE;
		xioctl(fd, VIDIOC_STREAMON, &type);

		// set variables on startup
		libv4l2_x_impl::set_samp_rate(samp_rate);
		libv4l2_x_impl::set_center_freq(freq);
		libv4l2_x_impl::set_bandwidth(bandwidth);
		libv4l2_x_impl::set_tuner_gain(gain);
	}

	/*
	 * Our virtual destructor.
	 */
	libv4l2_x_impl::~libv4l2_x_impl()
	{
		unsigned int i;
		enum v4l2_buf_type type;

		// stop streaming
		type = V4L2_BUF_TYPE_SDR_CAPTURE;
		xioctl(fd, VIDIOC_STREAMOFF, &type);

		for (i = 0; i < n_buffers; i++)
			v4l2_munmap(buffers[i].start, buffers[i].length);

		v4l2_close(fd);
	}

	void
	libv4l2_x_impl::set_samp_rate(double samp_rate)
	{
		struct v4l2_frequency frequency;

		memset (&frequency, 0, sizeof(frequency));
		frequency.tuner = 0;
		frequency.type = V4L2_TUNER_ADC;
		frequency.frequency = samp_rate / 1;

		if (v4l2_ioctl(fd, VIDIOC_S_FREQUENCY, &frequency) == -1)
			perror("VIDIOC_S_FREQUENCY");

		return;
	}

	void
	libv4l2_x_impl::set_center_freq(double freq)
	{
		struct v4l2_frequency frequency;

		memset (&frequency, 0, sizeof(frequency));
		frequency.tuner = 1;
		frequency.type = V4L2_TUNER_RF;
		frequency.frequency = freq;

		if (v4l2_ioctl(fd, VIDIOC_S_FREQUENCY, &frequency) == -1)
			perror("VIDIOC_S_FREQUENCY");

		return;
	}

	void
	libv4l2_x_impl::set_bandwidth(double bandwidth)
	{
		struct v4l2_ext_controls ext_ctrls;
		struct v4l2_ext_control ext_ctrl;

		memset (&ext_ctrl, 0, sizeof(ext_ctrl));
		ext_ctrl.id = CID_TUNER_BW;
		ext_ctrl.value = bandwidth;

		memset (&ext_ctrls, 0, sizeof(ext_ctrls));
		ext_ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
		ext_ctrls.count = 1;
		ext_ctrls.controls = &ext_ctrl;

		if (v4l2_ioctl(fd, VIDIOC_S_EXT_CTRLS, &ext_ctrls) == -1)
			perror("VIDIOC_S_EXT_CTRLS");

		return;
	}

	void
	libv4l2_x_impl::set_tuner_gain(double gain)
	{
		struct v4l2_ext_controls ext_ctrls;
		struct v4l2_ext_control ext_ctrl;

		memset (&ext_ctrl, 0, sizeof(ext_ctrl));
		ext_ctrl.id = CID_TUNER_GAIN;
		ext_ctrl.value = gain;

		memset (&ext_ctrls, 0, sizeof(ext_ctrls));
		ext_ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
		ext_ctrls.count = 1;
		ext_ctrls.controls = &ext_ctrl;

		if (v4l2_ioctl(fd, VIDIOC_S_EXT_CTRLS, &ext_ctrls) == -1)
			perror("VIDIOC_S_EXT_CTRLS");

		return;
	}

	int
	libv4l2_x_impl::work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items)
	{
		gr_complex *out = (gr_complex *) output_items[0];
		int ret;
		struct timeval tv;
		struct v4l2_buffer buf;
		fd_set fds;
		unsigned int i, items = 0;
		float *fptr = (float *) output_items[0];

process_buf:
		/* process received mmap buffer */
		uint8_t *u8src = (uint8_t *) recebuf_ptr;
		uint16_t *u16src = (uint16_t *) recebuf_ptr;

		while (recebuf_len) {
			if (pixelformat == V4L2_PIX_FMT_SDR_U8) {
				*fptr++ = (*u8src++ - 127.5f) / 127.5f;
				*fptr++ = (*u8src++ - 127.5f) / 127.5f;
				recebuf_len -= 2;
				items++;
				recebuf_ptr = u8src;
			} else if (pixelformat == V4L2_PIX_FMT_SDR_U16LE) {
				*fptr++ = (*u16src++ - 32767.5f) / 32767.5f;
				*fptr++ = (*u16src++ - 32767.5f) / 32767.5f;
				recebuf_len -= 4;
				items++;
				recebuf_ptr = u16src;
			} else {
				recebuf_len = 0;
			}

			if (items == noutput_items)
				break;
		}

		/* enqueue mmap buf after it is processed */
		if (recebuf_len == 0 && items != 0) {
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_SDR_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = recebuf_mmap_index;
			xioctl(fd, VIDIOC_QBUF, &buf);
		}

		/* signal DSP we have some samples to offer */
		if (items)
			return items;

		/* Read data from device */
		do {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			// Timeout
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			ret = select(fd + 1, &fds, NULL, NULL, &tv);
		} while ((ret == -1 && (errno = EINTR)));
		if (ret == -1) {
			perror("select");
			return errno;
		}

		/* dequeue mmap buf (receive data) */
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_SDR_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		xioctl(fd, VIDIOC_DQBUF, &buf);

		/* store buffer in order to handle it during next call */
		recebuf_ptr = buffers[buf.index].start;
		recebuf_len = buf.bytesused;
		recebuf_mmap_index = buf.index;
		/* FIXME: */
		goto process_buf;

		// Tell runtime system how many output items we produced.
		return 0;
	}

	} /* namespace kernel */
} /* namespace gr */

