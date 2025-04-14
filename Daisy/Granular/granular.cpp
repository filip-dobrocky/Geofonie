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
	Delta __m_delta_18;
	Phasor __m_phasor_67;
	Phasor __m_phasor_56;
	Phasor __m_phasor_17;
	Phasor __m_phasor_19;
	SineCycle __m_cycle_74;
	SineCycle __m_cycle_71;
	SineCycle __m_cycle_73;
	SineCycle __m_cycle_72;
	SineData __sinedata;
	int __exception;
	int vectorsize;
	int __loopcount;
	t_sample __m_count_20;
	t_sample __m_carry_22;
	t_sample __m_slide_68;
	t_sample m_midi_cc_15;
	t_sample m_midi_cc_14;
	t_sample m_midi_cc_6;
	t_sample m_midi_cc_7;
	t_sample m_history_3;
	t_sample samplerate;
	t_sample m_init_1;
	t_sample samples_to_seconds;
	t_sample m_midi_cc_8;
	t_sample m_midi_cc_10;
	t_sample m_midi_cc_13;
	t_sample m_midi_cc_9;
	t_sample m_midi_cc_12;
	t_sample m_midi_cc_11;
	t_sample __m_slide_75;
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
		m_midi_cc_7 = 0.1;
		m_midi_cc_8 = 0.5;
		m_midi_cc_9 = 1;
		m_midi_cc_10 = 0;
		m_midi_cc_11 = 0.5;
		m_midi_cc_12 = 1;
		m_midi_cc_13 = 1;
		m_midi_cc_14 = 0;
		m_midi_cc_15 = 1;
		m_loop_wav_16.reset("loop_wav", ((int)3131843), ((int)1));
		samples_to_seconds = (1 / samplerate);
		__m_phasor_17.reset(0);
		__m_delta_18.reset(0);
		__m_phasor_19.reset(0);
		__m_count_20 = 0;
		__m_carry_22 = 0;
		__m_phasor_56.reset(0);
		__m_phasor_67.reset(0);
		__m_slide_68 = 0;
		__m_cycle_71.reset(samplerate, 0);
		__m_cycle_72.reset(samplerate, 0);
		__m_cycle_73.reset(samplerate, 0);
		__m_cycle_74.reset(samplerate, 0);
		__m_slide_75 = 0;
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
		t_sample pow_95 = safepow(m_midi_cc_11, ((t_sample)0.6));
		t_sample dbtoa_73 = dbtoa(m_midi_cc_10);
		samples_to_seconds = (1 / samplerate);
		t_sample orange_10418 = (m_midi_cc_7 - m_midi_cc_14);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample floor_8908 = floor(m_midi_cc_9);
		t_sample min_57 = (-0.99);
		t_sample clamp_87 = ((m_midi_cc_6 <= min_57) ? min_57 : ((m_midi_cc_6 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_6));
		t_sample mode = floor(m_midi_cc_12);
		t_sample mstosamps_29 = (((t_sample)0.5) * (samplerate * 0.001));
		t_sample iup_69 = (1 / maximum(1, abs(mstosamps_29)));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample sub_10407 = (m_midi_cc_8 - ((int)0));
			t_sample scale_10404 = ((safepow((sub_10407 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_103 = scale_10404;
			t_sample sub_10411 = (m_midi_cc_11 - ((int)0));
			t_sample scale_10408 = ((safepow((sub_10411 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_105 = scale_10408;
			t_sample mul_114 = (scale_105 * (-1));
			t_sample sub_10415 = (m_midi_cc_8 - ((int)0));
			t_sample scale_10412 = ((safepow((sub_10415 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_104 = scale_10412;
			if ((((int)0) != 0)) {
				__m_phasor_17.phase = 0;
				
			};
			t_sample phasor_115 = __m_phasor_17(scale_104, samples_to_seconds);
			t_sample sub_10419 = (phasor_115 - ((int)0));
			t_sample scale_10416 = ((safepow((sub_10419 * ((t_sample)1)), ((int)1)) * orange_10418) + m_midi_cc_14);
			t_sample scale_116 = scale_10416;
			t_sample rsub_98 = (((int)1) - m_midi_cc_8);
			t_sample mul_99 = (rsub_98 * ((int)200));
			t_sample add_97 = (mul_99 + ((int)200));
			t_sample orange_10422 = (add_97 - mul_99);
			t_sample sub_10423 = (pow_95 - ((int)0));
			t_sample scale_10420 = ((safepow((sub_10423 * ((t_sample)1)), ((int)1)) * orange_10422) + mul_99);
			t_sample scale_102 = scale_10420;
			if ((((int)0) != 0)) {
				__m_phasor_19.phase = 0;
				
			};
			int p = (__m_delta_18(__m_phasor_19(scale_103, samples_to_seconds)) < ((int)0));
			__m_count_20 = (((int)0) ? 0 : (fixdenorm(__m_count_20 + p)));
			int carry_21 = 0;
			if ((((int)0) != 0)) {
				__m_count_20 = 0;
				__m_carry_22 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_20 >= ((int)10)))) {
				int wraps_23 = (__m_count_20 / ((int)10));
				__m_carry_22 = (__m_carry_22 + wraps_23);
				__m_count_20 = (__m_count_20 - (wraps_23 * ((int)10)));
				carry_21 = 1;
				
			};
			int c = __m_count_20;
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
				t_sample sz = (scale_102 * (samplerate * 0.001));
				t_sample sz_9562 = latchy_i_d_dat_i_i(trigger, sz, m_info_5, i, ((int)2));
				t_sample minb_39 = safediv(count, sz_9562);
				t_sample phase = ((minb_39 < ((int)1)) ? minb_39 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_114 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_9563 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_5, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_16.dim;
				int loop_wav_channels = m_loop_wav_16.channels;
				t_sample start_pos = ((scale_116 + spry) * loop_wav_dim);
				t_sample start_pos_9564 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_5, i, ((int)4));
				t_sample playhead = (start_pos_9564 + (count * pitch_hz_9563));
				int index_trunc_40 = fixnan(floor(playhead));
				double index_fract_41 = (playhead - index_trunc_40);
				int index_trunc_42 = (index_trunc_40 - 1);
				int index_trunc_43 = (index_trunc_40 + 1);
				int index_trunc_44 = (index_trunc_40 + 2);
				int index_wrap_45 = ((index_trunc_42 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_42 + 1) % loop_wav_dim)) : (index_trunc_42 % loop_wav_dim));
				int index_wrap_46 = ((index_trunc_40 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_40 + 1) % loop_wav_dim)) : (index_trunc_40 % loop_wav_dim));
				int index_wrap_47 = ((index_trunc_43 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_43 + 1) % loop_wav_dim)) : (index_trunc_43 % loop_wav_dim));
				int index_wrap_48 = ((index_trunc_44 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_44 + 1) % loop_wav_dim)) : (index_trunc_44 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_49 = ((int)0);
				bool chan_ignore_50 = ((chan_49 < 0) || (chan_49 >= loop_wav_channels));
				double read_loop_wav_51 = (chan_ignore_50 ? 0 : m_loop_wav_16.read(index_wrap_45, chan_49));
				double read_loop_wav_52 = (chan_ignore_50 ? 0 : m_loop_wav_16.read(index_wrap_46, chan_49));
				double read_loop_wav_53 = (chan_ignore_50 ? 0 : m_loop_wav_16.read(index_wrap_47, chan_49));
				double read_loop_wav_54 = (chan_ignore_50 ? 0 : m_loop_wav_16.read(index_wrap_48, chan_49));
				double readinterp_55 = cubic_interp(index_fract_41, read_loop_wav_51, read_loop_wav_52, read_loop_wav_53, read_loop_wav_54);
				t_sample smp = readinterp_55;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_9565 = sum;
			t_sample sub_67 = (m_midi_cc_13 - ((int)1));
			if ((((int)0) != 0)) {
				__m_phasor_56.phase = 0;
				
			};
			t_sample phasor_113 = __m_phasor_56(scale_103, samples_to_seconds);
			t_sample mul_94 = (phasor_113 * ((t_sample)0.5));
			t_sample pow_112 = safepow(mul_94, ((int)2));
			t_sample rsub_111 = (((int)1) - pow_112);
			t_sample out3 = rsub_111;
			t_sample sub_10427 = (m_midi_cc_8 - ((int)0));
			t_sample scale_10424 = ((safepow((sub_10427 * ((t_sample)1)), ((int)1)) * ((t_sample)19.9)) + ((t_sample)0.1));
			int gt_70 = (m_midi_cc_13 > ((int)1));
			int add_69 = (gt_70 + ((int)1));
			t_sample mtof_24 = mtof(floor_8908, ((int)440));
			t_sample rdiv_85 = safediv(((int)1), mtof_24);
			t_sample mul_84 = (rdiv_85 * ((int)1000));
			t_sample mstosamps_86 = (mul_84 * (samplerate * 0.001));
			t_sample tap_91 = m_delay_4.read_linear(mstosamps_86);
			t_sample mul_89 = (tap_91 * clamp_87);
			t_sample add_88 = (expr_9565 + mul_89);
			t_sample gen_93 = add_88;
			t_sample mul_77 = (mtof_24 * ((int)10));
			t_sample abs_78 = fabs(mul_77);
			t_sample mul_80 = (abs_78 * safediv((-6.2831853071796), samplerate));
			t_sample exp_82 = exp(mul_80);
			t_sample clamp_83 = ((exp_82 <= ((int)0)) ? ((int)0) : ((exp_82 >= ((int)1)) ? ((int)1) : exp_82));
			t_sample mix_10428 = (add_88 + (clamp_83 * (m_history_3 - add_88)));
			t_sample mix_79 = mix_10428;
			t_sample history_81_next_92 = fixdenorm(mix_79);
			t_sample mod_7860 = safemod(floor_8908, ((int)12));
			t_sample mstosamps_109 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_108 = (((int)300) * (samplerate * 0.001));
			t_sample rsub_9465 = (((int)1) - m_midi_cc_11);
			t_sample mul_76 = (rsub_9465 * ((int)1000));
			t_sample add_8146 = (mul_76 + mtof_24);
			t_sample root = floor(mod_7860);
			if ((m_init_1 == ((int)0))) {
				int intervals_dim = m_intervals_2.dim;
				int intervals_channels = m_intervals_2.channels;
				m_intervals_2.write(((int)2), 0, 0);
				bool index_ignore_58 = (((int)1) >= intervals_dim);
				if ((!index_ignore_58)) {
					m_intervals_2.write(((int)2), ((int)1), 0);
					
				};
				bool index_ignore_59 = (((int)2) >= intervals_dim);
				if ((!index_ignore_59)) {
					m_intervals_2.write(((int)1), ((int)2), 0);
					
				};
				bool index_ignore_60 = (((int)3) >= intervals_dim);
				if ((!index_ignore_60)) {
					m_intervals_2.write(((int)2), ((int)3), 0);
					
				};
				bool index_ignore_61 = (((int)4) >= intervals_dim);
				if ((!index_ignore_61)) {
					m_intervals_2.write(((int)2), ((int)4), 0);
					
				};
				bool index_ignore_62 = (((int)5) >= intervals_dim);
				if ((!index_ignore_62)) {
					m_intervals_2.write(((int)2), ((int)5), 0);
					
				};
				m_init_1 = ((int)1);
				
			};
			t_sample note = (ftom(add_8146, ((int)440)) - root);
			t_sample octave = floor((note * ((t_sample)0.083333333333333)));
			t_sample degree = safemod(note, ((int)12));
			t_sample min_dif = ((int)127);
			t_sample q_deg = ((int)0);
			t_sample prev = ((int)0);
			// for loop initializer;
			int i_9567 = ((int)0);
			// for loop condition;
			while ((i_9567 < ((int)7))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				t_sample d = ((int)0);
				if ((i_9567 > ((int)0))) {
					int intervals_dim = m_intervals_2.dim;
					int intervals_channels = m_intervals_2.channels;
					int index_trunc_63 = fixnan(floor(safemod(((i_9567 - ((int)1)) + mode), ((int)6))));
					bool index_ignore_64 = ((index_trunc_63 >= intervals_dim) || (index_trunc_63 < 0));
					// samples intervals channel 1;
					int chan_65 = ((int)0);
					bool chan_ignore_66 = ((chan_65 < 0) || (chan_65 >= intervals_channels));
					t_sample interval = ((chan_ignore_66 || index_ignore_64) ? 0 : m_intervals_2.read(index_trunc_63, chan_65));
					d = (prev + interval);
					
				};
				t_sample dif = fabs((degree - d));
				if ((dif < min_dif)) {
					min_dif = dif;
					q_deg = d;
					
				};
				prev = d;
				// for loop increment;
				i_9567 = (i_9567 + ((int)1));
				
			};
			t_sample expr_6718 = mtof(((root + q_deg) + (octave * ((int)12))), ((int)440));
			t_sample mstosamps_28 = (((int)1) * (samplerate * 0.001));
			t_sample clamp_43 = ((scale_10424 <= ((t_sample)0.1)) ? ((t_sample)0.1) : ((scale_10424 >= ((int)1000)) ? ((int)1000) : scale_10424));
			if ((((int)0) != 0)) {
				__m_phasor_67.phase = 0;
				
			};
			t_sample phasor_47 = __m_phasor_67(clamp_43, samples_to_seconds);
			t_sample rsub_44 = (((int)1) - phasor_47);
			t_sample pow_45 = safepow(rsub_44, ((int)2));
			t_sample idown_70 = (1 / maximum(1, abs(mstosamps_28)));
			__m_slide_68 = fixdenorm((__m_slide_68 + (((pow_45 > __m_slide_68) ? iup_69 : idown_70) * (pow_45 - __m_slide_68))));
			t_sample slide_30 = __m_slide_68;
			t_sample mul_31 = (scale_10424 * ((int)10));
			__m_cycle_71.freq(mul_31);
			t_sample cycle_36 = __m_cycle_71(__sinedata);
			t_sample cycleindex_37 = __m_cycle_71.phase();
			t_sample sub_10432 = (cycle_36 - (-1));
			t_sample scale_10429 = ((safepow((sub_10432 * ((t_sample)0.5)), ((int)1)) * ((int)2000)) + ((int)0));
			t_sample floor_33 = floor(scale_10424);
			t_sample mul_34 = (expr_6718 * floor_33);
			t_sample clamp_1818 = ((mul_34 <= ((int)1000)) ? ((int)1000) : ((mul_34 >= ((int)20000)) ? ((int)20000) : mul_34));
			__m_cycle_72.freq(clamp_1818);
			t_sample cycle_39 = __m_cycle_72(__sinedata);
			t_sample cycleindex_40 = __m_cycle_72.phase();
			t_sample mul_38 = (cycle_39 * scale_10429);
			t_sample add_41 = (expr_6718 + mul_38);
			__m_cycle_73.freq(add_41);
			t_sample cycle_48 = __m_cycle_73(__sinedata);
			t_sample cycleindex_49 = __m_cycle_73.phase();
			t_sample mul_42 = (cycle_48 * ((t_sample)0.5));
			t_sample mul_46 = (mul_42 * slide_30);
			__m_cycle_74.freq(expr_6718);
			t_sample cycle_22 = __m_cycle_74(__sinedata);
			t_sample cycleindex_23 = __m_cycle_74.phase();
			t_sample mul_2 = (cycle_22 * ((t_sample)0.5));
			t_sample clamp_11 = ((m_midi_cc_15 <= ((t_sample)0.5)) ? ((t_sample)0.5) : ((m_midi_cc_15 >= ((int)1)) ? ((int)1) : m_midi_cc_15));
			t_sample sub_10436 = (clamp_11 - ((t_sample)0.5));
			t_sample scale_10433 = ((safepow((sub_10436 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample clamp_15 = ((m_midi_cc_15 <= ((int)0)) ? ((int)0) : ((m_midi_cc_15 >= ((t_sample)0.5)) ? ((t_sample)0.5) : m_midi_cc_15));
			t_sample sub_10440 = (clamp_15 - ((int)0));
			t_sample scale_10437 = ((safepow((sub_10440 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample mix_10441 = (gen_93 + (scale_10437 * (mul_2 - gen_93)));
			t_sample mix_10442 = (mix_10441 + (scale_10433 * (mul_46 - mix_10441)));
			t_sample mul_74 = (mix_10442 * dbtoa_73);
			t_sample out1 = mul_74;
			t_sample mul_101 = (mul_74 * ((int)10));
			t_sample iup_76 = (1 / maximum(1, abs(mstosamps_109)));
			t_sample idown_77 = (1 / maximum(1, abs(mstosamps_108)));
			__m_slide_75 = fixdenorm((__m_slide_75 + (((mul_101 > __m_slide_75) ? iup_76 : idown_77) * (mul_101 - __m_slide_75))));
			t_sample slide_110 = __m_slide_75;
			t_sample mul_72 = (slide_110 * m_midi_cc_13);
			int choice_78 = add_69;
			t_sample selector_68 = ((choice_78 >= 2) ? sub_67 : ((choice_78 >= 1) ? mul_72 : 0));
			t_sample rsub_100 = (((int)1) - selector_68);
			t_sample out2 = rsub_100;
			m_delay_4.write(mix_79);
			m_history_3 = history_81_next_92;
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
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_7 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_8 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc8(t_param _value) {
		m_midi_cc_9 = (_value < 36 ? 36 : (_value > 84 ? 84 : _value));
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_10 = (_value < -60 ? -60 : (_value > 0 ? 0 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_11 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc9(t_param _value) {
		m_midi_cc_12 = (_value < 0 ? 0 : (_value > 7 ? 7 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_13 = (_value < 0 ? 0 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_14 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_15 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_16.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_24 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_25 = ((int)0);
		bool chan_ignore_26 = ((chan_25 < 0) || (chan_25 >= _dat_channels));
		t_sample previous = ((chan_ignore_26 || index_ignore_24) ? 0 : _dat.read(_instance, chan_25));
		bool index_ignore_27 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_27)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_28 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_29 = ((int)1);
		bool chan_ignore_30 = ((chan_29 < 0) || (chan_29 >= _dat_channels));
		t_sample count = ((chan_ignore_30 || index_ignore_28) ? 0 : _dat.read(_instance, chan_29));
		t_sample iffalse_31 = (count + ((int)1));
		t_sample count_9561 = (_trig ? ((int)0) : iffalse_31);
		bool chan_ignore_32 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_33 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_32 || index_ignore_33))) {
			_dat.write(count_9561, _instance, ((int)1));
			
		};
		return count_9561;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_34 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_35 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_34 || index_ignore_35))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_36 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_37 = _channel;
			bool chan_ignore_38 = ((chan_37 < 0) || (chan_37 >= _dat_channels));
			val = ((chan_ignore_38 || index_ignore_36) ? 0 : _dat.read(_instance, chan_37));
			
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
		
		
		
		case 3: *value = self->m_midi_cc_11; break;
		case 4: *value = self->m_midi_cc_6; break;
		case 5: *value = self->m_midi_cc_14; break;
		case 6: *value = self->m_midi_cc_7; break;
		case 7: *value = self->m_midi_cc_8; break;
		case 8: *value = self->m_midi_cc_10; break;
		case 9: *value = self->m_midi_cc_13; break;
		case 10: *value = self->m_midi_cc_15; break;
		case 11: *value = self->m_midi_cc_9; break;
		case 12: *value = self->m_midi_cc_12; break;
		
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
	// initialize parameter 3 ("m_midi_cc_11")
	pi = self->__commonstate.params + 3;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_11;
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
	// initialize parameter 5 ("m_midi_cc_14")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc2";
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
	// initialize parameter 6 ("m_midi_cc_7")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_7;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_midi_cc_8")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc4";
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
	// initialize parameter 8 ("m_midi_cc_10")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_10;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -60;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_midi_cc_13")
	pi = self->__commonstate.params + 9;
	pi->name = "midi_cc6";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_13;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 2;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_midi_cc_15")
	pi = self->__commonstate.params + 10;
	pi->name = "midi_cc7";
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
	// initialize parameter 11 ("m_midi_cc_9")
	pi = self->__commonstate.params + 11;
	pi->name = "midi_cc8";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_9;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 36;
	pi->outputmax = 84;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 12 ("m_midi_cc_12")
	pi = self->__commonstate.params + 12;
	pi->name = "midi_cc9";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_12;
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
