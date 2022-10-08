/* -*- c++ -*- */
/*
 * Copyright 2022 Albert Briscoe.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_KERNEL_LIBV4L2_X_H
#define INCLUDED_KERNEL_LIBV4L2_X_H

#include <gnuradio/kernel/api.h>
#include <gnuradio/sync_block.h>
#include <libv4l2.h>

namespace gr {
  namespace kernel {

    /*!
     * \brief <+description of block+>
     * \ingroup kernel
     *
     */
    class KERNEL_API libv4l2_x : virtual public gr::sync_block
    {
    public:
       typedef std::shared_ptr<libv4l2_x> sptr;

       /*!
        * \brief Return a shared_ptr to a new instance of kernel::libv4l2_x.
        *
        * To avoid accidental use of raw pointers, kernel::libv4l2_x's
        * constructor is in a private implementation
        * class. kernel::libv4l2_x::make is the public interface for
        * creating new instances.
        */
       static sptr make(const char *filename, double samp_rate, double freq, double bandwidth, double gain);
       virtual void set_samp_rate(double samp_rate) = 0;
	   virtual void set_center_freq(double freq) = 0;
       virtual void set_bandwidth(double bandwidth) = 0;
       virtual void set_tuner_gain(double gain) = 0;
    };

  } // namespace kernel
} // namespace gr

#endif /* INCLUDED_KERNEL_LIBV4L2_X_H */
