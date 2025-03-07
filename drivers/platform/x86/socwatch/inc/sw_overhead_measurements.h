/* SPDX-License-Identifier: GPL-2.0 AND BSD-3-Clause
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2014 - 2019 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Contact Information:
 * SoC Watch Developer Team <socwatchdevelopers@intel.com>
 * Intel Corporation,
 * 1300 S Mopac Expwy,
 * Austin, TX 78746
 *
 * BSD LICENSE
 *
 * Copyright(c) 2014 - 2019 Intel Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Description: file containing overhead measurement
 * routines used by the power driver.
 */

#ifndef _PW_OVERHEAD_MEASUREMENTS_H_
#define _PW_OVERHEAD_MEASUREMENTS_H_

/*
 * Helper macro to declare variables required
 * for conducting overhead measurements.
 */
/*
 * For each function that you want to profile,
 * do the following (e.g. function 'foo'):
 * **************************************************
 * DECLARE_OVERHEAD_VARS(foo);
 * **************************************************
 * This will declare the two variables required
 * to keep track of overheads incurred in
 * calling/servicing 'foo'. Note that the name
 * that you declare here *MUST* match the function name!
 */

#if DO_OVERHEAD_MEASUREMENTS

#ifndef __get_cpu_var
	/*
	 * Kernels >= 3.19 don't include a definition
	 * of '__get_cpu_var'. Create one now.
	 */
	#define __get_cpu_var(var) (*this_cpu_ptr(&var))
#endif /* __get_cpu_var */
#ifndef __raw_get_cpu_var
	/*
	 * Kernels >= 3.19 don't include a definition
	 * of '__raw_get_cpu_var'. Create one now.
	 */
	#define __raw_get_cpu_var(var) (*raw_cpu_ptr(&var))
#endif /* __get_cpu_var */

extern u64 sw_timestamp(void);

#define DECLARE_OVERHEAD_VARS(name)					\
	static DEFINE_PER_CPU(u64, name##_elapsed_time);		\
	static DEFINE_PER_CPU(local_t, name##_num_iters) =		\
							 LOCAL_INIT(0);	\
	static inline u64 get_my_cumulative_elapsed_time_##name(void)	\
	{								\
		return *(&__get_cpu_var(name##_elapsed_time));		\
	}								\
	static inline int get_my_cumulative_num_iters_##name(void)	\
	{								\
		return local_read(&__get_cpu_var(name##_num_iters));	\
	}								\
	static inline u64 name##_get_cumulative_elapsed_time_for(	\
							int cpu)	\
	{								\
		return *(&per_cpu(name##_elapsed_time, cpu));		\
	}								\
	static inline int name##_get_cumulative_num_iters_for(int cpu)	\
	{								\
		return local_read(&per_cpu(name##_num_iters, cpu));	\
	}								\
	static inline void name##_get_cumulative_overhead_params(	\
							u64 *time,	\
							int *iters)	\
	{								\
		int cpu = 0;						\
		*time = 0; *iters = 0;					\
		for_each_online_cpu(cpu) {				\
			*iters += name##_get_cumulative_num_iters_for(	\
								cpu);	\
			*time += name##_get_cumulative_elapsed_time_for(\
								cpu);	\
		}							\
		return;							\
	}								\
	static inline void name##_print_cumulative_overhead_params(	\
							const char *str)\
	{								\
		int num = 0;						\
		u64 time = 0;						\
		name##_get_cumulative_overhead_params(&time, &num);	\
		pw_pr_error("%s: %d iters took %llu nano seconds!\n",	\
			str, num, time);				\
	}

#define DO_PER_CPU_OVERHEAD_FUNC(func, ...) do {			\
	u64 *__v = &__raw_get_cpu_var(func##_elapsed_time);		\
	u64 tmp_1 = 0, tmp_2 = 0;					\
	local_inc(&__raw_get_cpu_var(func##_num_iters));		\
	tmp_1 = sw_timestamp();						\
	{								\
		func(__VA_ARGS__);						\
	}								\
	tmp_2 = sw_timestamp();						\
	*(__v) += (tmp_2 - tmp_1);					\
} while (0)

#define DO_PER_CPU_OVERHEAD_FUNC_RET(type, func, ...) ({		\
	type __ret;							\
	u64 *__v = &__raw_get_cpu_var(func##_elapsed_time);		\
	u64 tmp_1 = 0, tmp_2 = 0;					\
	local_inc(&__raw_get_cpu_var(func##_num_iters));		\
	tmp_1 = sw_timestamp();						\
	{								\
		__ret = func(__VA_ARGS__);					\
	}								\
	tmp_2 = sw_timestamp();						\
	*(__v) += (tmp_2 - tmp_1);					\
	__ret;								\
})

#else /* !DO_OVERHEAD_MEASUREMENTS */
#define DECLARE_OVERHEAD_VARS(name)					\
	static inline void name##_print_cumulative_overhead_params(	\
							const char *str)\
		{ /* NOP */ }

#define DO_PER_CPU_OVERHEAD_FUNC(func, ...) func(__VA_ARGS__)
#define DO_PER_CPU_OVERHEAD_FUNC_RET(type, func, ...) func(__VA_ARGS__)

#endif /* DO_OVERHEAD_MEASUREMENTS */

#define PRINT_CUMULATIVE_OVERHEAD_PARAMS(name, str)			\
	name##_print_cumulative_overhead_params(str)

#endif /* _PW_OVERHEAD_MEASUREMENTS_H_ */
