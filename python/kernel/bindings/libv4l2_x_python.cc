/*
 * Copyright 2022 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(libv4l2_x.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(1564f4c2f89e6c6cae3c15a9394fc829)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/kernel/libv4l2_x.h>
// pydoc.h is automatically generated in the build directory
#include <libv4l2_x_pydoc.h>

void bind_libv4l2_x(py::module& m)
{

    using libv4l2_x = ::gr::kernel::libv4l2_x;


    py::class_<libv4l2_x,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<libv4l2_x>>(m, "libv4l2_x", D(libv4l2_x))

        .def(py::init(&libv4l2_x::make),
             py::arg("filename"),
             py::arg("samp_rate"),
             py::arg("freq"),
             py::arg("bandwidth"),
             py::arg("gain"),
             D(libv4l2_x, make))


        .def("set_samp_rate",
             &libv4l2_x::set_samp_rate,
             py::arg("samp_rate"),
             D(libv4l2_x, set_samp_rate))


        .def("set_center_freq",
             &libv4l2_x::set_center_freq,
             py::arg("freq"),
             D(libv4l2_x, set_center_freq))


        .def("set_bandwidth",
             &libv4l2_x::set_bandwidth,
             py::arg("bandwidth"),
             D(libv4l2_x, set_bandwidth))


        .def("set_tuner_gain",
             &libv4l2_x::set_tuner_gain,
             py::arg("gain"),
             D(libv4l2_x, set_tuner_gain))

        ;
}