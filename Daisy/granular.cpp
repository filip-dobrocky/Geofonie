#include "granular.h"

namespace granular {

/****************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER 200k USD in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below 200k USD annual revenue or funding.

For entities with OVER 200k USD in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing (at) cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/360050779193-Gen-Code-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
****************************************************************************************/

// global noise generator
Noise noise;
static const int GENLIB_LOOPCOUNT_BAIL = 100000;


// The State struct contains all the state and procedures for the gendsp kernel
typedef struct State {
	CommonState __commonstate;
	Data m_loop_wav_16;
	Data m_intervals_2;
	Data m_info_5;
	Delay m_delay_4;
	Delta __m_delta_21;
	Phasor __m_phasor_73;
	Phasor __m_phasor_22;
	Phasor __m_phasor_20;
	Phasor __m_phasor_62;
	PlusEquals __m_pluseq_81;
	PlusEquals __m_pluseq_80;
	SineCycle __m_cycle_78;
	SineCycle __m_cycle_79;
	SineCycle __m_cycle_77;
	SineData __sinedata;
	int __loopcount;
	int __exception;
	int vectorsize;
	t_sample m_midi_cc_15;
	t_sample __m_slide_74;
	t_sample m_midi_cc_7;
	t_sample m_midi_cc_6;
	t_sample m_history_3;
	t_sample m_init_1;
	t_sample m_midi_cc_8;
	t_sample samplerate;
	t_sample m_midi_cc_14;
	t_sample __m_slide_59;
	t_sample __m_carry_25;
	t_sample samples_to_seconds;
	t_sample __m_slide_17;
	t_sample m_midi_cc_13;
	t_sample m_midi_cc_9;
	t_sample m_midi_cc_12;
	t_sample m_midi_cc_10;
	t_sample m_midi_cc_11;
	t_sample __m_count_23;
	t_sample __m_slide_82;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_init_1 = ((int)0);
		m_intervals_2.reset("intervals", ((int)6), ((int)1));
		m_history_3 = ((int)0);
		m_delay_4.reset("m_delay_4", ((int)1000));
		m_info_5.reset("info", ((int)10), ((int)5));
		m_midi_cc_6 = 0;
		m_midi_cc_7 = 1;
		m_midi_cc_8 = 1;
		m_midi_cc_9 = 0.5;
		m_midi_cc_10 = 1;
		m_midi_cc_11 = 0;
		m_midi_cc_12 = 0.5;
		m_midi_cc_13 = 0.1;
		m_midi_cc_14 = 1;
		m_midi_cc_15 = 0;
		m_loop_wav_16.reset("loop_wav", ((int)3131843), ((int)1));
		__m_slide_17 = 0;
		samples_to_seconds = (1 / samplerate);
		__m_phasor_20.reset(0);
		__m_delta_21.reset(0);
		__m_phasor_22.reset(0);
		__m_count_23 = 0;
		__m_carry_25 = 0;
		__m_slide_59 = 0;
		__m_phasor_62.reset(0);
		__m_phasor_73.reset(0);
		__m_slide_74 = 0;
		__m_cycle_77.reset(samplerate, 0);
		__m_cycle_78.reset(samplerate, 0);
		__m_cycle_79.reset(samplerate, 0);
		__m_pluseq_80.reset(0);
		__m_pluseq_81.reset(0);
		__m_slide_82 = 0;
		genlib_reset_complete(this);
		
	};
	// the signal processing routine;
	inline int perform(t_sample ** __ins, t_sample ** __outs, int __n) {
		vectorsize = __n;
		const t_sample * __in1 = __ins[0];
		t_sample * __out1 = __outs[0];
		t_sample * __out2 = __outs[1];
		t_sample * __out3 = __outs[2];
		if (__exception) {
			return __exception;
			
		} else if (( (__in1 == 0) || (__out1 == 0) || (__out2 == 0) || (__out3 == 0) )) {
			__exception = GENLIB_ERR_NULL_BUFFER;
			return __exception;
			
		};
		t_sample pow_10035 = safepow(m_midi_cc_9, ((t_sample)0.6));
		t_sample dbtoa_10014 = dbtoa(m_midi_cc_11);
		samples_to_seconds = (1 / samplerate);
		t_sample orange_10267 = (m_midi_cc_13 - m_midi_cc_15);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample floor_9946 = floor(m_midi_cc_7);
		t_sample min_63 = (-0.99);
		t_sample clamp_10027 = ((m_midi_cc_6 <= min_63) ? min_63 : ((m_midi_cc_6 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_6));
		t_sample mode = floor(m_midi_cc_10);
		t_sample mstosamps_9972 = (((t_sample)0.5) * (samplerate * 0.001));
		t_sample iup_75 = (1 / maximum(1, abs(mstosamps_9972)));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample mstosamps_9934 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_9933 = (((int)600) * (samplerate * 0.001));
			t_sample iup_18 = (1 / maximum(1, abs(mstosamps_9934)));
			t_sample idown_19 = (1 / maximum(1, abs(mstosamps_9933)));
			__m_slide_17 = fixdenorm((__m_slide_17 + (((m_midi_cc_12 > __m_slide_17) ? iup_18 : idown_19) * (m_midi_cc_12 - __m_slide_17))));
			t_sample slide_9937 = __m_slide_17;
			t_sample gen_9938 = slide_9937;
			t_sample sub_10255 = (gen_9938 - ((int)0));
			t_sample scale_10252 = ((safepow((sub_10255 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_10043 = scale_10252;
			t_sample sub_10259 = (m_midi_cc_9 - ((int)0));
			t_sample scale_10256 = ((safepow((sub_10259 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_10045 = scale_10256;
			t_sample mul_10054 = (scale_10045 * (-1));
			t_sample sub_10263 = (gen_9938 - ((int)0));
			t_sample scale_10260 = ((safepow((sub_10263 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_10044 = scale_10260;
			if ((((int)0) != 0)) {
				__m_phasor_20.phase = 0;
				
			};
			t_sample phasor_10055 = __m_phasor_20(scale_10044, samples_to_seconds);
			t_sample add_10265 = (gen_9938 + phasor_10055);
			t_sample sub_10268 = (add_10265 - ((int)0));
			t_sample scale_10264 = ((safepow((sub_10268 * ((t_sample)1)), ((int)1)) * orange_10267) + m_midi_cc_15);
			t_sample scale_10056 = scale_10264;
			t_sample rsub_10038 = (((int)1) - gen_9938);
			t_sample mul_10039 = (rsub_10038 * ((int)200));
			t_sample add_10037 = (mul_10039 + ((int)200));
			t_sample orange_10271 = (add_10037 - mul_10039);
			t_sample sub_10272 = (pow_10035 - ((int)0));
			t_sample scale_10269 = ((safepow((sub_10272 * ((t_sample)1)), ((int)1)) * orange_10271) + mul_10039);
			t_sample scale_10042 = scale_10269;
			if ((((int)0) != 0)) {
				__m_phasor_22.phase = 0;
				
			};
			int p = (__m_delta_21(__m_phasor_22(scale_10043, samples_to_seconds)) < ((int)0));
			__m_count_23 = (((int)0) ? 0 : (fixdenorm(__m_count_23 + p)));
			int carry_24 = 0;
			if ((((int)0) != 0)) {
				__m_count_23 = 0;
				__m_carry_25 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_23 >= ((int)10)))) {
				int wraps_26 = (__m_count_23 / ((int)10));
				__m_carry_25 = (__m_carry_25 + wraps_26);
				__m_count_23 = (__m_count_23 - (wraps_26 * ((int)10)));
				carry_24 = 1;
				
			};
			int c = __m_count_23;
			t_sample sum = ((int)0);
			// for loop initializer;
			int i = ((int)0);
			// for loop condition;
			// abort processing if an infinite loop is suspected;
			if (((__loopcount--) <= 0)) {
				__exception = GENLIB_ERR_LOOP_OVERFLOW;
				break ;
				
			};
			while ((i < ((int)10))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				int trigger = get_trigger_i_i_dat(c, i, m_info_5);
				t_sample count = get_count_dat_i_i(m_info_5, i, trigger);
				t_sample sz = (scale_10042 * (samplerate * 0.001));
				t_sample sz_10066 = latchy_i_d_dat_i_i(trigger, sz, m_info_5, i, ((int)2));
				t_sample minb_42 = safediv(count, sz_10066);
				t_sample phase = ((minb_42 < ((int)1)) ? minb_42 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_10054 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_10067 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_5, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_16.dim;
				int loop_wav_channels = m_loop_wav_16.channels;
				t_sample start_pos = ((scale_10056 + spry) * loop_wav_dim);
				t_sample start_pos_10068 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_5, i, ((int)4));
				t_sample playhead = (start_pos_10068 + (count * pitch_hz_10067));
				int index_trunc_43 = fixnan(floor(playhead));
				double index_fract_44 = (playhead - index_trunc_43);
				int index_trunc_45 = (index_trunc_43 - 1);
				int index_trunc_46 = (index_trunc_43 + 1);
				int index_trunc_47 = (index_trunc_43 + 2);
				int index_wrap_48 = ((index_trunc_45 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_45 + 1) % loop_wav_dim)) : (index_trunc_45 % loop_wav_dim));
				int index_wrap_49 = ((index_trunc_43 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_43 + 1) % loop_wav_dim)) : (index_trunc_43 % loop_wav_dim));
				int index_wrap_50 = ((index_trunc_46 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_46 + 1) % loop_wav_dim)) : (index_trunc_46 % loop_wav_dim));
				int index_wrap_51 = ((index_trunc_47 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_47 + 1) % loop_wav_dim)) : (index_trunc_47 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_52 = ((int)0);
				bool chan_ignore_53 = ((chan_52 < 0) || (chan_52 >= loop_wav_channels));
				double read_loop_wav_54 = (chan_ignore_53 ? 0 : m_loop_wav_16.read(index_wrap_48, chan_52));
				double read_loop_wav_55 = (chan_ignore_53 ? 0 : m_loop_wav_16.read(index_wrap_49, chan_52));
				double read_loop_wav_56 = (chan_ignore_53 ? 0 : m_loop_wav_16.read(index_wrap_50, chan_52));
				double read_loop_wav_57 = (chan_ignore_53 ? 0 : m_loop_wav_16.read(index_wrap_51, chan_52));
				double readinterp_58 = cubic_interp(index_fract_44, read_loop_wav_54, read_loop_wav_55, read_loop_wav_56, read_loop_wav_57);
				t_sample smp = readinterp_58;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_10069 = sum;
			t_sample mstosamps_9940 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_9939 = (((int)600) * (samplerate * 0.001));
			t_sample iup_60 = (1 / maximum(1, abs(mstosamps_9940)));
			t_sample idown_61 = (1 / maximum(1, abs(mstosamps_9939)));
			__m_slide_59 = fixdenorm((__m_slide_59 + (((m_midi_cc_8 > __m_slide_59) ? iup_60 : idown_61) * (m_midi_cc_8 - __m_slide_59))));
			t_sample slide_9943 = __m_slide_59;
			t_sample sub_10008 = (m_midi_cc_14 - ((int)1));
			t_sample sub_10276 = (gen_9938 - ((int)0));
			t_sample scale_10273 = ((safepow((sub_10276 * ((t_sample)1)), ((int)1)) * ((t_sample)19.9)) + ((t_sample)0.1));
			int gt_10011 = (m_midi_cc_14 > ((int)1));
			int add_10010 = (gt_10011 + ((int)1));
			if ((((int)0) != 0)) {
				__m_phasor_62.phase = 0;
				
			};
			t_sample phasor_10053 = __m_phasor_62(scale_10043, samples_to_seconds);
			t_sample mul_10034 = (phasor_10053 * ((t_sample)0.5));
			t_sample pow_10052 = safepow(mul_10034, ((int)2));
			t_sample rsub_10051 = (((int)1) - pow_10052);
			t_sample out3 = rsub_10051;
			t_sample mtof_9968 = mtof(floor_9946, ((int)440));
			t_sample rdiv_10025 = safediv(((int)1), mtof_9968);
			t_sample mul_10024 = (rdiv_10025 * ((int)1000));
			t_sample mstosamps_10026 = (mul_10024 * (samplerate * 0.001));
			t_sample tap_10031 = m_delay_4.read_linear(mstosamps_10026);
			t_sample mul_10029 = (tap_10031 * clamp_10027);
			t_sample add_10028 = (expr_10069 + mul_10029);
			t_sample gen_10033 = add_10028;
			t_sample mul_10017 = (mtof_9968 * ((int)10));
			t_sample abs_10018 = fabs(mul_10017);
			t_sample mul_10020 = (abs_10018 * safediv((-6.2831853071796), samplerate));
			t_sample exp_10022 = exp(mul_10020);
			t_sample clamp_10023 = ((exp_10022 <= ((int)0)) ? ((int)0) : ((exp_10022 >= ((int)1)) ? ((int)1) : exp_10022));
			t_sample mix_10277 = (add_10028 + (clamp_10023 * (m_history_3 - add_10028)));
			t_sample mix_10019 = mix_10277;
			t_sample history_10021_next_10032 = fixdenorm(mix_10019);
			t_sample mul_9918 = (gen_9938 * ((int)10));
			t_sample mod_9994 = safemod(floor_9946, ((int)12));
			t_sample mstosamps_10049 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_10048 = (((int)300) * (samplerate * 0.001));
			t_sample rsub_9945 = (((int)1) - m_midi_cc_9);
			t_sample pow_9932 = safepow(((int)1500), rsub_9945);
			t_sample add_10016 = (pow_9932 + mtof_9968);
			t_sample root = floor(mod_9994);
			if ((m_init_1 == ((int)0))) {
				int intervals_dim = m_intervals_2.dim;
				int intervals_channels = m_intervals_2.channels;
				m_intervals_2.write(((int)2), 0, 0);
				bool index_ignore_64 = (((int)1) >= intervals_dim);
				if ((!index_ignore_64)) {
					m_intervals_2.write(((int)2), ((int)1), 0);
					
				};
				bool index_ignore_65 = (((int)2) >= intervals_dim);
				if ((!index_ignore_65)) {
					m_intervals_2.write(((int)1), ((int)2), 0);
					
				};
				bool index_ignore_66 = (((int)3) >= intervals_dim);
				if ((!index_ignore_66)) {
					m_intervals_2.write(((int)2), ((int)3), 0);
					
				};
				bool index_ignore_67 = (((int)4) >= intervals_dim);
				if ((!index_ignore_67)) {
					m_intervals_2.write(((int)2), ((int)4), 0);
					
				};
				bool index_ignore_68 = (((int)5) >= intervals_dim);
				if ((!index_ignore_68)) {
					m_intervals_2.write(((int)2), ((int)5), 0);
					
				};
				m_init_1 = ((int)1);
				
			};
			t_sample note = (ftom(add_10016, ((int)440)) - root);
			t_sample octave = floor((note * ((t_sample)0.083333333333333)));
			t_sample degree = safemod(note, ((int)12));
			t_sample min_dif = ((int)127);
			t_sample q_deg = ((int)0);
			t_sample prev = ((int)0);
			// for loop initializer;
			int i_10071 = ((int)0);
			// for loop condition;
			while ((i_10071 < ((int)7))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				t_sample d = ((int)0);
				if ((i_10071 > ((int)0))) {
					int intervals_dim = m_intervals_2.dim;
					int intervals_channels = m_intervals_2.channels;
					int index_trunc_69 = fixnan(floor(safemod(((i_10071 - ((int)1)) + mode), ((int)6))));
					bool index_ignore_70 = ((index_trunc_69 >= intervals_dim) || (index_trunc_69 < 0));
					// samples intervals channel 1;
					int chan_71 = ((int)0);
					bool chan_ignore_72 = ((chan_71 < 0) || (chan_71 >= intervals_channels));
					t_sample interval = ((chan_ignore_72 || index_ignore_70) ? 0 : m_intervals_2.read(index_trunc_69, chan_71));
					d = (prev + interval);
					
				};
				t_sample dif = fabs((degree - d));
				if ((dif < min_dif)) {
					min_dif = dif;
					q_deg = d;
					
				};
				prev = d;
				// for loop increment;
				i_10071 = (i_10071 + ((int)1));
				
			};
			t_sample expr_10006 = mtof(((root + q_deg) + (octave * ((int)12))), ((int)440));
			t_sample mstosamps_9971 = (((int)1) * (samplerate * 0.001));
			t_sample clamp_9986 = ((scale_10273 <= ((t_sample)0.1)) ? ((t_sample)0.1) : ((scale_10273 >= ((int)1000)) ? ((int)1000) : scale_10273));
			if ((((int)0) != 0)) {
				__m_phasor_73.phase = 0;
				
			};
			t_sample phasor_9990 = __m_phasor_73(clamp_9986, samples_to_seconds);
			t_sample rsub_9987 = (((int)1) - phasor_9990);
			t_sample pow_9988 = safepow(rsub_9987, ((int)2));
			t_sample idown_76 = (1 / maximum(1, abs(mstosamps_9971)));
			__m_slide_74 = fixdenorm((__m_slide_74 + (((pow_9988 > __m_slide_74) ? iup_75 : idown_76) * (pow_9988 - __m_slide_74))));
			t_sample slide_9973 = __m_slide_74;
			t_sample mul_9974 = (scale_10273 * ((int)10));
			__m_cycle_77.freq(mul_9974);
			t_sample cycle_9979 = __m_cycle_77(__sinedata);
			t_sample cycleindex_9980 = __m_cycle_77.phase();
			t_sample sub_10281 = (cycle_9979 - (-1));
			t_sample scale_10278 = ((safepow((sub_10281 * ((t_sample)0.5)), ((int)1)) * ((int)2000)) + ((int)0));
			t_sample floor_9976 = floor(scale_10273);
			t_sample mul_9977 = (expr_10006 * floor_9976);
			t_sample clamp_9975 = ((mul_9977 <= ((int)1000)) ? ((int)1000) : ((mul_9977 >= ((int)20000)) ? ((int)20000) : mul_9977));
			__m_cycle_78.freq(clamp_9975);
			t_sample cycle_9982 = __m_cycle_78(__sinedata);
			t_sample cycleindex_9983 = __m_cycle_78.phase();
			t_sample mul_9981 = (cycle_9982 * scale_10278);
			t_sample add_9984 = (expr_10006 + mul_9981);
			__m_cycle_79.freq(add_9984);
			t_sample cycle_9991 = __m_cycle_79(__sinedata);
			t_sample cycleindex_9992 = __m_cycle_79.phase();
			t_sample mul_9985 = (cycle_9991 * ((t_sample)0.5));
			t_sample mul_9989 = (mul_9985 * slide_9973);
			t_sample expr_9929 = safediv(((expr_10006 * ((int)2)) * ((t_sample)3.1415926535898)), samplerate);
			t_sample mul_9926 = (expr_9929 * mul_9918);
			t_sample mul_9925 = (expr_9929 * ((t_sample)1.5));
			t_sample plusequals_9924 = __m_pluseq_80.post(mul_9925, ((int)0), 0);
			t_sample sin_9923 = sin(plusequals_9924);
			t_sample mul_9922 = (mul_9926 * sin_9923);
			t_sample plusequals_9928 = __m_pluseq_81.post((expr_9929 + mul_9922), ((int)0), 0);
			t_sample sin_9927 = sin(plusequals_9928);
			t_sample mul_9921 = (sin_9927 * ((int)1));
			t_sample mul_9948 = (mul_9921 * ((t_sample)0.15));
			t_sample clamp_9961 = ((slide_9943 <= ((int)0)) ? ((int)0) : ((slide_9943 >= ((t_sample)0.5)) ? ((t_sample)0.5) : slide_9943));
			t_sample sub_10285 = (clamp_9961 - ((int)0));
			t_sample scale_10282 = ((safepow((sub_10285 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample clamp_9957 = ((slide_9943 <= ((t_sample)0.5)) ? ((t_sample)0.5) : ((slide_9943 >= ((int)1)) ? ((int)1) : slide_9943));
			t_sample sub_10289 = (clamp_9957 - ((t_sample)0.5));
			t_sample scale_10286 = ((safepow((sub_10289 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample mix_10290 = (gen_10033 + (scale_10282 * (mul_9948 - gen_10033)));
			t_sample mix_10291 = (mix_10290 + (scale_10286 * (mul_9989 - mix_10290)));
			t_sample mul_10015 = (mix_10291 * dbtoa_10014);
			t_sample out1 = mul_10015;
			t_sample mul_10041 = (mul_10015 * ((int)10));
			t_sample iup_83 = (1 / maximum(1, abs(mstosamps_10049)));
			t_sample idown_84 = (1 / maximum(1, abs(mstosamps_10048)));
			__m_slide_82 = fixdenorm((__m_slide_82 + (((mul_10041 > __m_slide_82) ? iup_83 : idown_84) * (mul_10041 - __m_slide_82))));
			t_sample slide_10050 = __m_slide_82;
			t_sample mul_10013 = (slide_10050 * m_midi_cc_14);
			int choice_85 = add_10010;
			t_sample selector_10009 = ((choice_85 >= 2) ? sub_10008 : ((choice_85 >= 1) ? mul_10013 : 0));
			t_sample rsub_10040 = (((int)1) - selector_10009);
			t_sample out2 = rsub_10040;
			m_delay_4.write(mix_10019);
			m_history_3 = history_10021_next_10032;
			m_delay_4.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			(*(__out3++)) = out3;
			
		};
		return __exception;
		
	};
	inline void set_intervals(void * _value) {
		m_intervals_2.setbuffer(_value);
	};
	inline void set_info(void * _value) {
		m_info_5.setbuffer(_value);
	};
	inline void set_midi_cc10(t_param _value) {
		m_midi_cc_6 = (_value < -1 ? -1 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc8(t_param _value) {
		m_midi_cc_7 = (_value < 36 ? 36 : (_value > 84 ? 84 : _value));
	};
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_8 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_9 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc9(t_param _value) {
		m_midi_cc_10 = (_value < 0 ? 0 : (_value > 7 ? 7 : _value));
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_11 = (_value < -80 ? -80 : (_value > 0 ? 0 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_12 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_13 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_14 = (_value < 0 ? 0 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_15 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_16.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_27 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_28 = ((int)0);
		bool chan_ignore_29 = ((chan_28 < 0) || (chan_28 >= _dat_channels));
		t_sample previous = ((chan_ignore_29 || index_ignore_27) ? 0 : _dat.read(_instance, chan_28));
		bool index_ignore_30 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_30)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_31 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_32 = ((int)1);
		bool chan_ignore_33 = ((chan_32 < 0) || (chan_32 >= _dat_channels));
		t_sample count = ((chan_ignore_33 || index_ignore_31) ? 0 : _dat.read(_instance, chan_32));
		t_sample iffalse_34 = (count + ((int)1));
		t_sample count_10065 = (_trig ? ((int)0) : iffalse_34);
		bool chan_ignore_35 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_36 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_35 || index_ignore_36))) {
			_dat.write(count_10065, _instance, ((int)1));
			
		};
		return count_10065;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_37 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_38 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_37 || index_ignore_38))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_39 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_40 = _channel;
			bool chan_ignore_41 = ((chan_40 < 0) || (chan_40 >= _dat_channels));
			val = ((chan_ignore_41 || index_ignore_39) ? 0 : _dat.read(_instance, chan_40));
			
		};
		return val;
		
	};
	
} State;


///
///	Configuration for the genlib API
///

/// Number of signal inputs and outputs

int gen_kernel_numins = 1;
int gen_kernel_numouts = 3;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 13; }

/// Assistive lables for the signal inputs and outputs

const char *gen_kernel_innames[] = { "in1" };
const char *gen_kernel_outnames[] = { "out1", "led", "laser" };

/// Invoke the signal process of a State object

int perform(CommonState *cself, t_sample **ins, long numins, t_sample **outs, long numouts, long n) {
	State* self = (State *)cself;
	return self->perform(ins, outs, n);
}

/// Reset all parameters and stateful operators of a State object

void reset(CommonState *cself) {
	State* self = (State *)cself;
	self->reset(cself->sr, cself->vs);
}

/// Set a parameter of a State object

void setparameter(CommonState *cself, long index, t_param value, void *ref) {
	State *self = (State *)cself;
	switch (index) {
		case 0: self->set_info(ref); break;
		case 1: self->set_intervals(ref); break;
		case 2: self->set_loop_wav(ref); break;
		case 3: self->set_midi_cc1(value); break;
		case 4: self->set_midi_cc10(value); break;
		case 5: self->set_midi_cc2(value); break;
		case 6: self->set_midi_cc3(value); break;
		case 7: self->set_midi_cc4(value); break;
		case 8: self->set_midi_cc5(value); break;
		case 9: self->set_midi_cc6(value); break;
		case 10: self->set_midi_cc7(value); break;
		case 11: self->set_midi_cc8(value); break;
		case 12: self->set_midi_cc9(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		
		
		
		case 3: *value = self->m_midi_cc_9; break;
		case 4: *value = self->m_midi_cc_6; break;
		case 5: *value = self->m_midi_cc_15; break;
		case 6: *value = self->m_midi_cc_13; break;
		case 7: *value = self->m_midi_cc_12; break;
		case 8: *value = self->m_midi_cc_11; break;
		case 9: *value = self->m_midi_cc_14; break;
		case 10: *value = self->m_midi_cc_8; break;
		case 11: *value = self->m_midi_cc_7; break;
		case 12: *value = self->m_midi_cc_10; break;
		
		default: break;
	}
}

/// Get the name of a parameter of a State object

const char *getparametername(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].name;
	}
	return 0;
}

/// Get the minimum value of a parameter of a State object

t_param getparametermin(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].outputmin;
	}
	return 0;
}

/// Get the maximum value of a parameter of a State object

t_param getparametermax(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].outputmax;
	}
	return 0;
}

/// Get parameter of a State object has a minimum and maximum value

char getparameterhasminmax(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].hasminmax;
	}
	return 0;
}

/// Get the units of a parameter of a State object

const char *getparameterunits(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].units;
	}
	return 0;
}

/// Get the size of the state of all parameters of a State object

size_t getstatesize(CommonState *cself) {
	return genlib_getstatesize(cself, &getparameter);
}

/// Get the state of all parameters of a State object

short getstate(CommonState *cself, char *state) {
	return genlib_getstate(cself, state, &getparameter);
}

/// set the state of all parameters of a State object

short setstate(CommonState *cself, const char *state) {
	return genlib_setstate(cself, state, &setparameter);
}

/// Allocate and configure a new State object and it's internal CommonState:

void *create(t_param sr, long vs) {
	State *self = new State;
	self->reset(sr, vs);
	ParamInfo *pi;
	self->__commonstate.inputnames = gen_kernel_innames;
	self->__commonstate.outputnames = gen_kernel_outnames;
	self->__commonstate.numins = gen_kernel_numins;
	self->__commonstate.numouts = gen_kernel_numouts;
	self->__commonstate.sr = sr;
	self->__commonstate.vs = vs;
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(13 * sizeof(ParamInfo));
	self->__commonstate.numparams = 13;
	// initialize parameter 0 ("m_info_5")
	pi = self->__commonstate.params + 0;
	pi->name = "info";
	pi->paramtype = GENLIB_PARAMTYPE_SYM;
	pi->defaultvalue = 0.;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = false;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 1 ("m_intervals_2")
	pi = self->__commonstate.params + 1;
	pi->name = "intervals";
	pi->paramtype = GENLIB_PARAMTYPE_SYM;
	pi->defaultvalue = 0.;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = false;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 2 ("m_loop_wav_16")
	pi = self->__commonstate.params + 2;
	pi->name = "loop_wav";
	pi->paramtype = GENLIB_PARAMTYPE_SYM;
	pi->defaultvalue = 0.;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = false;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 3 ("m_midi_cc_9")
	pi = self->__commonstate.params + 3;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_9;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.001;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_midi_cc_6")
	pi = self->__commonstate.params + 4;
	pi->name = "midi_cc10";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_6;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -1;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_midi_cc_15")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc2";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_15;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_midi_cc_13")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_13;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_midi_cc_12")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc4";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_12;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_midi_cc_11")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_11;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -80;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_midi_cc_14")
	pi = self->__commonstate.params + 9;
	pi->name = "midi_cc6";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_14;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 2;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_midi_cc_8")
	pi = self->__commonstate.params + 10;
	pi->name = "midi_cc7";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_8;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_midi_cc_7")
	pi = self->__commonstate.params + 11;
	pi->name = "midi_cc8";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_7;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 36;
	pi->outputmax = 84;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 12 ("m_midi_cc_10")
	pi = self->__commonstate.params + 12;
	pi->name = "midi_cc9";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_10;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 7;
	pi->exp = 0;
	pi->units = "";		// no units defined
	
	return self;
}

/// Release all resources and memory used by a State object:

void destroy(CommonState *cself) {
	State *self = (State *)cself;
	genlib_sysmem_freeptr(cself->params);
		
	delete self;
}


} // granular::
