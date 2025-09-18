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
	Delta __m_delta_24;
	Phasor __m_phasor_73;
	Phasor __m_phasor_25;
	Phasor __m_phasor_23;
	Phasor __m_phasor_62;
	SineCycle __m_cycle_77;
	SineCycle __m_cycle_80;
	SineCycle __m_cycle_78;
	SineCycle __m_cycle_79;
	SineData __sinedata;
	int __loopcount;
	int __exception;
	int vectorsize;
	t_sample m_midi_cc_14;
	t_sample m_midi_cc_6;
	t_sample m_midi_cc_7;
	t_sample __m_slide_74;
	t_sample m_history_3;
	t_sample m_init_1;
	t_sample m_midi_cc_8;
	t_sample samplerate;
	t_sample m_midi_cc_13;
	t_sample __m_carry_28;
	t_sample m_midi_cc_9;
	t_sample __m_slide_17;
	t_sample m_midi_cc_12;
	t_sample m_midi_cc_15;
	t_sample __m_count_26;
	t_sample __m_slide_20;
	t_sample m_midi_cc_11;
	t_sample samples_to_seconds;
	t_sample m_midi_cc_10;
	t_sample __m_slide_81;
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
		m_midi_cc_6 = 1;
		m_midi_cc_7 = 1;
		m_midi_cc_8 = 0;
		m_midi_cc_9 = 0.5;
		m_midi_cc_10 = 1;
		m_midi_cc_11 = 0.5;
		m_midi_cc_12 = 1;
		m_midi_cc_13 = 0;
		m_midi_cc_14 = 0.1;
		m_midi_cc_15 = 0;
		m_loop_wav_16.reset("loop_wav", ((int)3131843), ((int)1));
		__m_slide_17 = 0;
		__m_slide_20 = 0;
		samples_to_seconds = (1 / samplerate);
		__m_phasor_23.reset(0);
		__m_delta_24.reset(0);
		__m_phasor_25.reset(0);
		__m_count_26 = 0;
		__m_carry_28 = 0;
		__m_phasor_62.reset(0);
		__m_phasor_73.reset(0);
		__m_slide_74 = 0;
		__m_cycle_77.reset(samplerate, 0);
		__m_cycle_78.reset(samplerate, 0);
		__m_cycle_79.reset(samplerate, 0);
		__m_cycle_80.reset(samplerate, 0);
		__m_slide_81 = 0;
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
		t_sample dbtoa_72 = dbtoa(m_midi_cc_13);
		t_sample pow_94 = safepow(m_midi_cc_9, ((t_sample)0.6));
		samples_to_seconds = (1 / samplerate);
		t_sample orange_5099 = (m_midi_cc_14 - m_midi_cc_8);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample floor_2 = floor(m_midi_cc_12);
		t_sample min_63 = (-0.99);
		t_sample clamp_86 = ((m_midi_cc_15 <= min_63) ? min_63 : ((m_midi_cc_15 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_15));
		t_sample mode = floor(m_midi_cc_7);
		t_sample mstosamps_30 = (((t_sample)0.5) * (samplerate * 0.001));
		t_sample iup_75 = (1 / maximum(1, abs(mstosamps_30)));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample sub_5088 = (m_midi_cc_9 - ((int)0));
			t_sample scale_5085 = ((safepow((sub_5088 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_104 = scale_5085;
			t_sample mul_113 = (scale_104 * (-1));
			t_sample mstosamps_4501 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_4500 = (((int)600) * (samplerate * 0.001));
			t_sample iup_18 = (1 / maximum(1, abs(mstosamps_4501)));
			t_sample idown_19 = (1 / maximum(1, abs(mstosamps_4500)));
			__m_slide_17 = fixdenorm((__m_slide_17 + (((m_midi_cc_11 > __m_slide_17) ? iup_18 : idown_19) * (m_midi_cc_11 - __m_slide_17))));
			t_sample slide_4504 = __m_slide_17;
			t_sample gen_4505 = slide_4504;
			t_sample sub_5092 = (gen_4505 - ((int)0));
			t_sample scale_5089 = ((safepow((sub_5092 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_102 = scale_5089;
			t_sample mstosamps_1176 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_1175 = (((int)600) * (samplerate * 0.001));
			t_sample iup_21 = (1 / maximum(1, abs(mstosamps_1176)));
			t_sample idown_22 = (1 / maximum(1, abs(mstosamps_1175)));
			__m_slide_20 = fixdenorm((__m_slide_20 + (((m_midi_cc_6 > __m_slide_20) ? iup_21 : idown_22) * (m_midi_cc_6 - __m_slide_20))));
			t_sample slide_1179 = __m_slide_20;
			t_sample sub_5096 = (gen_4505 - ((int)0));
			t_sample scale_5093 = ((safepow((sub_5096 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_103 = scale_5093;
			if ((((int)0) != 0)) {
				__m_phasor_23.phase = 0;
				
			};
			t_sample phasor_114 = __m_phasor_23(scale_103, samples_to_seconds);
			t_sample sub_5100 = (phasor_114 - ((int)0));
			t_sample scale_5097 = ((safepow((sub_5100 * ((t_sample)1)), ((int)1)) * orange_5099) + m_midi_cc_8);
			t_sample scale_115 = scale_5097;
			t_sample rsub_97 = (((int)1) - gen_4505);
			t_sample mul_98 = (rsub_97 * ((int)200));
			t_sample add_96 = (mul_98 + ((int)200));
			t_sample orange_5103 = (add_96 - mul_98);
			t_sample sub_5104 = (pow_94 - ((int)0));
			t_sample scale_5101 = ((safepow((sub_5104 * ((t_sample)1)), ((int)1)) * orange_5103) + mul_98);
			t_sample scale_101 = scale_5101;
			if ((((int)0) != 0)) {
				__m_phasor_25.phase = 0;
				
			};
			int p = (__m_delta_24(__m_phasor_25(scale_102, samples_to_seconds)) < ((int)0));
			__m_count_26 = (((int)0) ? 0 : (fixdenorm(__m_count_26 + p)));
			int carry_27 = 0;
			if ((((int)0) != 0)) {
				__m_count_26 = 0;
				__m_carry_28 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_26 >= ((int)10)))) {
				int wraps_29 = (__m_count_26 / ((int)10));
				__m_carry_28 = (__m_carry_28 + wraps_29);
				__m_count_26 = (__m_count_26 - (wraps_29 * ((int)10)));
				carry_27 = 1;
				
			};
			int c = __m_count_26;
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
				t_sample sz = (scale_101 * (samplerate * 0.001));
				t_sample sz_4507 = latchy_i_d_dat_i_i(trigger, sz, m_info_5, i, ((int)2));
				t_sample minb_45 = safediv(count, sz_4507);
				t_sample phase = ((minb_45 < ((int)1)) ? minb_45 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_113 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_4508 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_5, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_16.dim;
				int loop_wav_channels = m_loop_wav_16.channels;
				t_sample start_pos = ((scale_115 + spry) * loop_wav_dim);
				t_sample start_pos_4509 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_5, i, ((int)4));
				t_sample playhead = (start_pos_4509 + (count * pitch_hz_4508));
				int index_trunc_46 = fixnan(floor(playhead));
				double index_fract_47 = (playhead - index_trunc_46);
				int index_trunc_48 = (index_trunc_46 - 1);
				int index_trunc_49 = (index_trunc_46 + 1);
				int index_trunc_50 = (index_trunc_46 + 2);
				int index_wrap_51 = ((index_trunc_48 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_48 + 1) % loop_wav_dim)) : (index_trunc_48 % loop_wav_dim));
				int index_wrap_52 = ((index_trunc_46 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_46 + 1) % loop_wav_dim)) : (index_trunc_46 % loop_wav_dim));
				int index_wrap_53 = ((index_trunc_49 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_49 + 1) % loop_wav_dim)) : (index_trunc_49 % loop_wav_dim));
				int index_wrap_54 = ((index_trunc_50 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_50 + 1) % loop_wav_dim)) : (index_trunc_50 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_55 = ((int)0);
				bool chan_ignore_56 = ((chan_55 < 0) || (chan_55 >= loop_wav_channels));
				double read_loop_wav_57 = (chan_ignore_56 ? 0 : m_loop_wav_16.read(index_wrap_51, chan_55));
				double read_loop_wav_58 = (chan_ignore_56 ? 0 : m_loop_wav_16.read(index_wrap_52, chan_55));
				double read_loop_wav_59 = (chan_ignore_56 ? 0 : m_loop_wav_16.read(index_wrap_53, chan_55));
				double read_loop_wav_60 = (chan_ignore_56 ? 0 : m_loop_wav_16.read(index_wrap_54, chan_55));
				double readinterp_61 = cubic_interp(index_fract_47, read_loop_wav_57, read_loop_wav_58, read_loop_wav_59, read_loop_wav_60);
				t_sample smp = readinterp_61;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_4510 = sum;
			t_sample sub_66 = (m_midi_cc_10 - ((int)1));
			t_sample sub_5108 = (gen_4505 - ((int)0));
			t_sample scale_5105 = ((safepow((sub_5108 * ((t_sample)1)), ((int)1)) * ((t_sample)19.9)) + ((t_sample)0.1));
			if ((((int)0) != 0)) {
				__m_phasor_62.phase = 0;
				
			};
			t_sample phasor_112 = __m_phasor_62(scale_102, samples_to_seconds);
			t_sample mul_93 = (phasor_112 * ((t_sample)0.5));
			t_sample pow_111 = safepow(mul_93, ((int)2));
			t_sample rsub_110 = (((int)1) - pow_111);
			t_sample out3 = rsub_110;
			int gt_69 = (m_midi_cc_10 > ((int)1));
			int add_68 = (gt_69 + ((int)1));
			t_sample mtof_26 = mtof(floor_2, ((int)440));
			t_sample rdiv_84 = safediv(((int)1), mtof_26);
			t_sample mul_83 = (rdiv_84 * ((int)1000));
			t_sample mstosamps_85 = (mul_83 * (samplerate * 0.001));
			t_sample tap_90 = m_delay_4.read_linear(mstosamps_85);
			t_sample mul_88 = (tap_90 * clamp_86);
			t_sample add_87 = (expr_4510 + mul_88);
			t_sample gen_92 = add_87;
			t_sample mul_76 = (mtof_26 * ((int)10));
			t_sample abs_77 = fabs(mul_76);
			t_sample mul_79 = (abs_77 * safediv((-6.2831853071796), samplerate));
			t_sample exp_81 = exp(mul_79);
			t_sample clamp_82 = ((exp_81 <= ((int)0)) ? ((int)0) : ((exp_81 >= ((int)1)) ? ((int)1) : exp_81));
			t_sample mix_5109 = (add_87 + (clamp_82 * (m_history_3 - add_87)));
			t_sample mix_78 = mix_5109;
			t_sample history_80_next_91 = fixdenorm(mix_78);
			t_sample mod_52 = safemod(floor_2, ((int)12));
			t_sample mstosamps_108 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_107 = (((int)300) * (samplerate * 0.001));
			t_sample rsub_1 = (((int)1) - m_midi_cc_9);
			t_sample mul_75 = (rsub_1 * ((int)1000));
			t_sample add_74 = (mul_75 + mtof_26);
			t_sample root = floor(mod_52);
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
			t_sample note = (ftom(add_74, ((int)440)) - root);
			t_sample octave = floor((note * ((t_sample)0.083333333333333)));
			t_sample degree = safemod(note, ((int)12));
			t_sample min_dif = ((int)127);
			t_sample q_deg = ((int)0);
			t_sample prev = ((int)0);
			// for loop initializer;
			int i_4512 = ((int)0);
			// for loop condition;
			while ((i_4512 < ((int)7))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				t_sample d = ((int)0);
				if ((i_4512 > ((int)0))) {
					int intervals_dim = m_intervals_2.dim;
					int intervals_channels = m_intervals_2.channels;
					int index_trunc_69 = fixnan(floor(safemod(((i_4512 - ((int)1)) + mode), ((int)6))));
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
				i_4512 = (i_4512 + ((int)1));
				
			};
			t_sample expr_64 = mtof(((root + q_deg) + (octave * ((int)12))), ((int)440));
			t_sample mstosamps_29 = (((int)1) * (samplerate * 0.001));
			t_sample clamp_44 = ((scale_5105 <= ((t_sample)0.1)) ? ((t_sample)0.1) : ((scale_5105 >= ((int)1000)) ? ((int)1000) : scale_5105));
			if ((((int)0) != 0)) {
				__m_phasor_73.phase = 0;
				
			};
			t_sample phasor_48 = __m_phasor_73(clamp_44, samples_to_seconds);
			t_sample rsub_45 = (((int)1) - phasor_48);
			t_sample pow_46 = safepow(rsub_45, ((int)2));
			t_sample idown_76 = (1 / maximum(1, abs(mstosamps_29)));
			__m_slide_74 = fixdenorm((__m_slide_74 + (((pow_46 > __m_slide_74) ? iup_75 : idown_76) * (pow_46 - __m_slide_74))));
			t_sample slide_31 = __m_slide_74;
			t_sample mul_32 = (scale_5105 * ((int)10));
			__m_cycle_77.freq(mul_32);
			t_sample cycle_37 = __m_cycle_77(__sinedata);
			t_sample cycleindex_38 = __m_cycle_77.phase();
			t_sample sub_5113 = (cycle_37 - (-1));
			t_sample scale_5110 = ((safepow((sub_5113 * ((t_sample)0.5)), ((int)1)) * ((int)2000)) + ((int)0));
			t_sample floor_34 = floor(scale_5105);
			t_sample mul_35 = (expr_64 * floor_34);
			t_sample clamp_33 = ((mul_35 <= ((int)1000)) ? ((int)1000) : ((mul_35 >= ((int)20000)) ? ((int)20000) : mul_35));
			__m_cycle_78.freq(clamp_33);
			t_sample cycle_40 = __m_cycle_78(__sinedata);
			t_sample cycleindex_41 = __m_cycle_78.phase();
			t_sample mul_39 = (cycle_40 * scale_5110);
			t_sample add_42 = (expr_64 + mul_39);
			__m_cycle_79.freq(add_42);
			t_sample cycle_49 = __m_cycle_79(__sinedata);
			t_sample cycleindex_50 = __m_cycle_79.phase();
			t_sample mul_43 = (cycle_49 * ((t_sample)0.5));
			t_sample mul_47 = (mul_43 * slide_31);
			__m_cycle_80.freq(expr_64);
			t_sample cycle_24 = __m_cycle_80(__sinedata);
			t_sample cycleindex_25 = __m_cycle_80.phase();
			t_sample mul_2526 = (cycle_24 * ((t_sample)0.2));
			t_sample clamp_17 = ((slide_1179 <= ((int)0)) ? ((int)0) : ((slide_1179 >= ((t_sample)0.5)) ? ((t_sample)0.5) : slide_1179));
			t_sample sub_5117 = (clamp_17 - ((int)0));
			t_sample scale_5114 = ((safepow((sub_5117 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample clamp_13 = ((slide_1179 <= ((t_sample)0.5)) ? ((t_sample)0.5) : ((slide_1179 >= ((int)1)) ? ((int)1) : slide_1179));
			t_sample sub_5121 = (clamp_13 - ((t_sample)0.5));
			t_sample scale_5118 = ((safepow((sub_5121 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample mix_5122 = (gen_92 + (scale_5114 * (mul_2526 - gen_92)));
			t_sample mix_5123 = (mix_5122 + (scale_5118 * (mul_47 - mix_5122)));
			t_sample mul_73 = (mix_5123 * dbtoa_72);
			t_sample out1 = mul_73;
			t_sample mul_100 = (mul_73 * ((int)10));
			t_sample iup_82 = (1 / maximum(1, abs(mstosamps_108)));
			t_sample idown_83 = (1 / maximum(1, abs(mstosamps_107)));
			__m_slide_81 = fixdenorm((__m_slide_81 + (((mul_100 > __m_slide_81) ? iup_82 : idown_83) * (mul_100 - __m_slide_81))));
			t_sample slide_109 = __m_slide_81;
			t_sample mul_71 = (slide_109 * m_midi_cc_10);
			int choice_84 = add_68;
			t_sample selector_67 = ((choice_84 >= 2) ? sub_66 : ((choice_84 >= 1) ? mul_71 : 0));
			t_sample rsub_99 = (((int)1) - selector_67);
			t_sample out2 = rsub_99;
			m_delay_4.write(mix_78);
			m_history_3 = history_80_next_91;
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
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_6 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc9(t_param _value) {
		m_midi_cc_7 = (_value < 0 ? 0 : (_value > 7 ? 7 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_8 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_9 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_10 = (_value < 0 ? 0 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_11 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc8(t_param _value) {
		m_midi_cc_12 = (_value < 36 ? 36 : (_value > 84 ? 84 : _value));
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_13 = (_value < -80 ? -80 : (_value > 0 ? 0 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_14 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc10(t_param _value) {
		m_midi_cc_15 = (_value < -1 ? -1 : (_value > 1 ? 1 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_16.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_30 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_31 = ((int)0);
		bool chan_ignore_32 = ((chan_31 < 0) || (chan_31 >= _dat_channels));
		t_sample previous = ((chan_ignore_32 || index_ignore_30) ? 0 : _dat.read(_instance, chan_31));
		bool index_ignore_33 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_33)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_34 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_35 = ((int)1);
		bool chan_ignore_36 = ((chan_35 < 0) || (chan_35 >= _dat_channels));
		t_sample count = ((chan_ignore_36 || index_ignore_34) ? 0 : _dat.read(_instance, chan_35));
		t_sample iffalse_37 = (count + ((int)1));
		t_sample count_4506 = (_trig ? ((int)0) : iffalse_37);
		bool chan_ignore_38 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_39 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_38 || index_ignore_39))) {
			_dat.write(count_4506, _instance, ((int)1));
			
		};
		return count_4506;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_40 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_41 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_40 || index_ignore_41))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_42 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_43 = _channel;
			bool chan_ignore_44 = ((chan_43 < 0) || (chan_43 >= _dat_channels));
			val = ((chan_ignore_44 || index_ignore_42) ? 0 : _dat.read(_instance, chan_43));
			
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
		case 4: *value = self->m_midi_cc_15; break;
		case 5: *value = self->m_midi_cc_8; break;
		case 6: *value = self->m_midi_cc_14; break;
		case 7: *value = self->m_midi_cc_11; break;
		case 8: *value = self->m_midi_cc_13; break;
		case 9: *value = self->m_midi_cc_10; break;
		case 10: *value = self->m_midi_cc_6; break;
		case 11: *value = self->m_midi_cc_12; break;
		case 12: *value = self->m_midi_cc_7; break;
		
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
	// initialize parameter 4 ("m_midi_cc_15")
	pi = self->__commonstate.params + 4;
	pi->name = "midi_cc10";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_15;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -1;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_midi_cc_8")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc2";
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
	// initialize parameter 6 ("m_midi_cc_14")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_14;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_midi_cc_11")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc4";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_11;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_midi_cc_13")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_13;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -80;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_midi_cc_10")
	pi = self->__commonstate.params + 9;
	pi->name = "midi_cc6";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_10;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 2;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_midi_cc_6")
	pi = self->__commonstate.params + 10;
	pi->name = "midi_cc7";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_6;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_midi_cc_12")
	pi = self->__commonstate.params + 11;
	pi->name = "midi_cc8";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_12;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 36;
	pi->outputmax = 84;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 12 ("m_midi_cc_7")
	pi = self->__commonstate.params + 12;
	pi->name = "midi_cc9";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_7;
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
