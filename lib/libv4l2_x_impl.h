/* -*- c++ -*- */
/* 
 * Copyright 2013 Antti Palosaari <crope@iki.fi>
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

#ifndef INCLUDED_KERNEL_LIBV4L2_X_IMPL_H
#define INCLUDED_KERNEL_LIBV4L2_X_IMPL_H

#include <gnuradio/kernel/libv4l2_x.h>

namespace gr {
	namespace kernel {
	// v4l2_mmap
	struct buffer {
		void *start;
		size_t length;
	};

	class libv4l2_x_impl : public libv4l2_x
	{
	private:
		// v4l2 device file handle
		int fd;

		// stream / sample format
		uint32_t pixelformat;

		struct buffer *buffers;
		unsigned int n_buffers;

		// for processing mmap buffer
		void *recebuf_ptr;
		unsigned int recebuf_len;
		unsigned int recebuf_mmap_index;

	public:
		libv4l2_x_impl(const char *filename, double samp_rate, double freq, double bandwidth, double gain);
		~libv4l2_x_impl();

		void set_samp_rate(double samp_rate);
		void set_center_freq(double freq);
		void set_bandwidth(double bandwidth);
		void set_tuner_gain(double gain);

		// Where all the action really happens
		int work(int noutput_items,
				gr_vector_const_void_star &input_items,
				gr_vector_void_star &output_items);
	};

	} // namespace kernel
} // namespace gr

#endif /* INCLUDED_KERNEL_LIBV4L2_X_IMPL_H */

