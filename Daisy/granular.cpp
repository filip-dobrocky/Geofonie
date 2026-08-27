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
	Data m_info_9;
	Data m_loop_wav_22;
	Data m_major_6;
	Delay m_delay_8;
	Delta __m_delta_27;
	Phasor __m_phasor_86;
	Phasor __m_phasor_65;
	Phasor __m_phasor_26;
	Phasor __m_phasor_28;
	PlusEquals __m_pluseq_85;
	PlusEquals __m_pluseq_84;
	int __exception;
	int __loopcount;
	int vectorsize;
	t_sample __m_count_29;
	t_sample m_midi_cc_21;
	t_sample __m_slide_23;
	t_sample samples_to_seconds;
	t_sample __m_slide_66;
	t_sample m_midi_cc_20;
	t_sample __m_carry_31;
	t_sample m_midi_cc_19;
	t_sample m_midi_cc_17;
	t_sample __m_slide_87;
	t_sample m_history_3;
	t_sample m_history_4;
	t_sample m_init_5;
	t_sample m_history_2;
	t_sample samplerate;
	t_sample m_history_1;
	t_sample m_midi_cc_18;
	t_sample m_history_7;
	t_sample m_midi_cc_11;
	t_sample m_midi_cc_15;
	t_sample m_midi_cc_16;
	t_sample m_midi_cc_10;
	t_sample m_midi_cc_14;
	t_sample m_midi_cc_12;
	t_sample m_midi_cc_13;
	t_sample __m_slide_90;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_history_1 = ((int)0);
		m_history_2 = ((int)0);
		m_history_3 = ((int)0);
		m_history_4 = ((int)0);
		m_init_5 = ((int)0);
		m_major_6.reset("major", ((int)7), ((int)1));
		m_history_7 = ((int)0);
		m_delay_8.reset("m_delay_8", ((int)1000));
		m_info_9.reset("info", ((int)10), ((int)5));
		m_midi_cc_10 = 1;
		m_midi_cc_11 = 0.5;
		m_midi_cc_12 = 1;
		m_midi_cc_13 = 0;
		m_midi_cc_14 = 0.5;
		m_midi_cc_15 = 1;
		m_midi_cc_16 = 1;
		m_midi_cc_17 = 0.1;
		m_midi_cc_18 = 0;
		m_midi_cc_19 = 1;
		m_midi_cc_20 = 1.5;
		m_midi_cc_21 = 0;
		m_loop_wav_22.reset("loop_wav", ((int)3131843), ((int)1));
		__m_slide_23 = 0;
		samples_to_seconds = (1 / samplerate);
		__m_phasor_26.reset(0);
		__m_delta_27.reset(0);
		__m_phasor_28.reset(0);
		__m_count_29 = 0;
		__m_carry_31 = 0;
		__m_phasor_65.reset(0);
		__m_slide_66 = 0;
		__m_pluseq_84.reset(0);
		__m_pluseq_85.reset(0);
		__m_phasor_86.reset(0);
		__m_slide_87 = 0;
		__m_slide_90 = 0;
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
		t_sample pow_5972 = safepow(m_midi_cc_14, ((t_sample)0.6));
		t_sample dbtoa_5951 = dbtoa(m_midi_cc_21);
		samples_to_seconds = (1 / samplerate);
		t_sample orange_9255 = (m_midi_cc_17 - m_midi_cc_13);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample gtp_5865 = ((dbtoa_5951 > ((t_sample)0.01)) ? dbtoa_5951 : 0);
		t_sample floor_5891 = floor(m_midi_cc_19);
		t_sample min_69 = (-0.99);
		t_sample clamp_5964 = ((m_midi_cc_18 <= min_69) ? min_69 : ((m_midi_cc_18 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_18));
		int major_dim = m_major_6.dim;
		int major_channels = m_major_6.channels;
		t_sample mstosamps_5917 = (((t_sample)0.5) * (samplerate * 0.001));
		t_sample iup_88 = (1 / maximum(1, abs(mstosamps_5917)));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample mstosamps_5879 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_5878 = (((int)600) * (samplerate * 0.001));
			t_sample iup_24 = (1 / maximum(1, abs(mstosamps_5879)));
			t_sample idown_25 = (1 / maximum(1, abs(mstosamps_5878)));
			__m_slide_23 = fixdenorm((__m_slide_23 + (((m_midi_cc_11 > __m_slide_23) ? iup_24 : idown_25) * (m_midi_cc_11 - __m_slide_23))));
			t_sample slide_5882 = __m_slide_23;
			t_sample gen_5883 = slide_5882;
			t_sample sub_9243 = (gen_5883 - ((int)0));
			t_sample scale_9240 = ((safepow((sub_9243 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_5980 = scale_9240;
			t_sample sub_9247 = (m_midi_cc_14 - ((int)0));
			t_sample scale_9244 = ((safepow((sub_9247 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_5982 = scale_9244;
			t_sample mul_5991 = (scale_5982 * (-1));
			t_sample sub_5945 = (m_midi_cc_12 - ((int)1));
			t_sample sub_9251 = (gen_5883 - ((int)0));
			t_sample scale_9248 = ((safepow((sub_9251 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_5981 = scale_9248;
			if ((((int)0) != 0)) {
				__m_phasor_26.phase = 0;
				
			};
			t_sample phasor_5992 = __m_phasor_26(scale_5981, samples_to_seconds);
			t_sample add_9253 = (gen_5883 + phasor_5992);
			t_sample sub_9256 = (add_9253 - ((int)0));
			t_sample scale_9252 = ((safepow((sub_9256 * ((t_sample)1)), ((int)1)) * orange_9255) + m_midi_cc_13);
			t_sample scale_5993 = scale_9252;
			t_sample rsub_5975 = (((int)1) - gen_5883);
			t_sample mul_5976 = (rsub_5975 * ((int)200));
			t_sample add_5974 = (mul_5976 + ((int)200));
			t_sample orange_9259 = (add_5974 - mul_5976);
			t_sample sub_9260 = (pow_5972 - ((int)0));
			t_sample scale_9257 = ((safepow((sub_9260 * ((t_sample)1)), ((int)1)) * orange_9259) + mul_5976);
			t_sample scale_5979 = scale_9257;
			if ((((int)0) != 0)) {
				__m_phasor_28.phase = 0;
				
			};
			int p = (__m_delta_27(__m_phasor_28(scale_5980, samples_to_seconds)) < ((int)0));
			__m_count_29 = (((int)0) ? 0 : (fixdenorm(__m_count_29 + p)));
			int carry_30 = 0;
			if ((((int)0) != 0)) {
				__m_count_29 = 0;
				__m_carry_31 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_29 >= ((int)10)))) {
				int wraps_32 = (__m_count_29 / ((int)10));
				__m_carry_31 = (__m_carry_31 + wraps_32);
				__m_count_29 = (__m_count_29 - (wraps_32 * ((int)10)));
				carry_30 = 1;
				
			};
			int c = __m_count_29;
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
				int trigger = get_trigger_i_i_dat(c, i, m_info_9);
				t_sample count = get_count_dat_i_i(m_info_9, i, trigger);
				t_sample sz = (scale_5979 * (samplerate * 0.001));
				t_sample sz_9150 = latchy_i_d_dat_i_i(trigger, sz, m_info_9, i, ((int)2));
				t_sample minb_48 = safediv(count, sz_9150);
				t_sample phase = ((minb_48 < ((int)1)) ? minb_48 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_5991 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_9151 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_9, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_22.dim;
				int loop_wav_channels = m_loop_wav_22.channels;
				t_sample start_pos = ((scale_5993 + spry) * loop_wav_dim);
				t_sample start_pos_9152 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_9, i, ((int)4));
				t_sample playhead = (start_pos_9152 + (count * pitch_hz_9151));
				int index_trunc_49 = fixnan(floor(playhead));
				double index_fract_50 = (playhead - index_trunc_49);
				int index_trunc_51 = (index_trunc_49 - 1);
				int index_trunc_52 = (index_trunc_49 + 1);
				int index_trunc_53 = (index_trunc_49 + 2);
				int index_wrap_54 = ((index_trunc_51 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_51 + 1) % loop_wav_dim)) : (index_trunc_51 % loop_wav_dim));
				int index_wrap_55 = ((index_trunc_49 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_49 + 1) % loop_wav_dim)) : (index_trunc_49 % loop_wav_dim));
				int index_wrap_56 = ((index_trunc_52 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_52 + 1) % loop_wav_dim)) : (index_trunc_52 % loop_wav_dim));
				int index_wrap_57 = ((index_trunc_53 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_53 + 1) % loop_wav_dim)) : (index_trunc_53 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_58 = ((int)0);
				bool chan_ignore_59 = ((chan_58 < 0) || (chan_58 >= loop_wav_channels));
				double read_loop_wav_60 = (chan_ignore_59 ? 0 : m_loop_wav_22.read(index_wrap_54, chan_58));
				double read_loop_wav_61 = (chan_ignore_59 ? 0 : m_loop_wav_22.read(index_wrap_55, chan_58));
				double read_loop_wav_62 = (chan_ignore_59 ? 0 : m_loop_wav_22.read(index_wrap_56, chan_58));
				double read_loop_wav_63 = (chan_ignore_59 ? 0 : m_loop_wav_22.read(index_wrap_57, chan_58));
				double readinterp_64 = cubic_interp(index_fract_50, read_loop_wav_60, read_loop_wav_61, read_loop_wav_62, read_loop_wav_63);
				t_sample smp = readinterp_64;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_9153 = sum;
			int gt_5948 = (m_midi_cc_12 > ((int)1));
			int add_5947 = (gt_5948 + ((int)1));
			if ((((int)0) != 0)) {
				__m_phasor_65.phase = 0;
				
			};
			t_sample phasor_5990 = __m_phasor_65(scale_5980, samples_to_seconds);
			t_sample mul_5971 = (phasor_5990 * ((t_sample)0.5));
			t_sample pow_5989 = safepow(mul_5971, ((int)2));
			t_sample rsub_5988 = (((int)1) - pow_5989);
			t_sample out3 = rsub_5988;
			t_sample mstosamps_5885 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_5884 = (((int)600) * (samplerate * 0.001));
			t_sample iup_67 = (1 / maximum(1, abs(mstosamps_5885)));
			t_sample idown_68 = (1 / maximum(1, abs(mstosamps_5884)));
			__m_slide_66 = fixdenorm((__m_slide_66 + (((m_midi_cc_10 > __m_slide_66) ? iup_67 : idown_68) * (m_midi_cc_10 - __m_slide_66))));
			t_sample slide_5888 = __m_slide_66;
			t_sample gen_5889 = slide_5888;
			t_sample sub_9264 = (gen_5883 - ((int)0));
			t_sample scale_9261 = ((safepow((sub_9264 * ((t_sample)1)), ((int)1)) * ((t_sample)19.9)) + ((t_sample)0.1));
			t_sample scale_5892 = scale_9261;
			t_sample mstosamps_5986 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_5985 = (((int)300) * (samplerate * 0.001));
			t_sample mtof_5913 = mtof(floor_5891, ((int)440));
			t_sample rdiv_5962 = safediv(((int)1), mtof_5913);
			t_sample mul_5961 = (rdiv_5962 * ((int)1000));
			t_sample mstosamps_5963 = (mul_5961 * (samplerate * 0.001));
			t_sample tap_5968 = m_delay_8.read_linear(mstosamps_5963);
			t_sample mul_5966 = (tap_5968 * clamp_5964);
			t_sample add_5965 = (expr_9153 + mul_5966);
			t_sample gen_5970 = add_5965;
			t_sample mul_5954 = (mtof_5913 * ((int)10));
			t_sample abs_5955 = fabs(mul_5954);
			t_sample mul_5957 = (abs_5955 * safediv((-6.2831853071796), samplerate));
			t_sample exp_5959 = exp(mul_5957);
			t_sample clamp_5960 = ((exp_5959 <= ((int)0)) ? ((int)0) : ((exp_5959 >= ((int)1)) ? ((int)1) : exp_5959));
			t_sample mix_9265 = (add_5965 + (clamp_5960 * (m_history_7 - add_5965)));
			t_sample mix_5956 = mix_9265;
			t_sample history_5958_next_5969 = fixdenorm(mix_5956);
			t_sample mod_5925 = safemod(floor_5891, ((int)12));
			t_sample rsub_5890 = (((int)1) - m_midi_cc_14);
			t_sample mul_5877 = (rsub_5890 * ((int)48));
			t_sample add_5953 = (mul_5877 + floor_5891);
			if ((m_init_5 == ((int)0))) {
				int major_dim = m_major_6.dim;
				int major_channels = m_major_6.channels;
				m_major_6.write(((int)0), 0, 0);
				bool index_ignore_70 = (((int)1) >= major_dim);
				if ((!index_ignore_70)) {
					m_major_6.write(((int)2), ((int)1), 0);
					
				};
				bool index_ignore_71 = (((int)2) >= major_dim);
				if ((!index_ignore_71)) {
					m_major_6.write(((int)4), ((int)2), 0);
					
				};
				bool index_ignore_72 = (((int)3) >= major_dim);
				if ((!index_ignore_72)) {
					m_major_6.write(((int)5), ((int)3), 0);
					
				};
				bool index_ignore_73 = (((int)4) >= major_dim);
				if ((!index_ignore_73)) {
					m_major_6.write(((int)7), ((int)4), 0);
					
				};
				bool index_ignore_74 = (((int)5) >= major_dim);
				if ((!index_ignore_74)) {
					m_major_6.write(((int)9), ((int)5), 0);
					
				};
				bool index_ignore_75 = (((int)6) >= major_dim);
				if ((!index_ignore_75)) {
					m_major_6.write(((int)11), ((int)6), 0);
					
				};
				m_init_5 = ((int)1);
				
			};
			t_sample root = floor(mod_5925);
			t_sample mode = safemod(floor(m_midi_cc_16), ((int)7));
			int index_trunc_76 = fixnan(floor(mode));
			bool index_ignore_77 = ((index_trunc_76 >= major_dim) || (index_trunc_76 < 0));
			// samples major channel 1;
			int chan_78 = ((int)0);
			bool chan_ignore_79 = ((chan_78 < 0) || (chan_78 >= major_channels));
			t_sample tonic = ((chan_ignore_79 || index_ignore_77) ? 0 : m_major_6.read(index_trunc_76, chan_78));
			t_sample note = (add_5953 - root);
			t_sample octave = floor((note * ((t_sample)0.083333333333333)));
			t_sample degree = (note - (octave * ((int)12)));
			t_sample min_dif = ((int)127);
			t_sample q_deg = ((int)0);
			// for loop initializer;
			int i_9155 = ((int)0);
			// for loop condition;
			while ((i_9155 < ((int)7))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				int index_trunc_80 = fixnan(floor(safemod((i_9155 + mode), ((int)7))));
				bool index_ignore_81 = ((index_trunc_80 >= major_dim) || (index_trunc_80 < 0));
				// samples major channel 1;
				int chan_82 = ((int)0);
				bool chan_ignore_83 = ((chan_82 < 0) || (chan_82 >= major_channels));
				t_sample peek_5935 = ((chan_ignore_83 || index_ignore_81) ? 0 : m_major_6.read(index_trunc_80, chan_82));
				t_sample peek_5936 = safemod((i_9155 + mode), ((int)7));
				t_sample d = safemod(((peek_5935 - tonic) + ((int)12)), ((int)12));
				t_sample dif = fabs((degree - d));
				if ((dif < min_dif)) {
					min_dif = dif;
					q_deg = d;
					
				};
				// for loop increment;
				i_9155 = (i_9155 + ((int)1));
				
			};
			t_sample expr_5943 = mtof(((root + q_deg) + (octave * ((int)12))), ((int)440));
			t_sample gen_5944 = expr_5943;
			t_sample expr_5875 = safediv(((gen_5944 * ((int)2)) * ((t_sample)3.1415926535898)), samplerate);
			t_sample mul_5872 = (expr_5875 * m_midi_cc_15);
			t_sample mul_5871 = (expr_5875 * m_midi_cc_20);
			t_sample plusequals_5870 = __m_pluseq_84.post(mul_5871, ((int)0), 0);
			t_sample sin_5869 = sin(plusequals_5870);
			t_sample mul_5868 = (mul_5872 * sin_5869);
			t_sample plusequals_5874 = __m_pluseq_85.post((expr_5875 + mul_5868), ((int)0), 0);
			t_sample sin_5873 = sin(plusequals_5874);
			t_sample mul_5867 = (sin_5873 * ((int)1));
			t_sample gen_5876 = mul_5867;
			t_sample mul_5893 = (gen_5876 * ((t_sample)0.15));
			t_sample mstosamps_5916 = (((int)1) * (samplerate * 0.001));
			t_sample clamp_5919 = ((scale_5892 <= ((t_sample)0.1)) ? ((t_sample)0.1) : ((scale_5892 >= ((int)1000)) ? ((int)1000) : scale_5892));
			if ((((int)0) != 0)) {
				__m_phasor_86.phase = 0;
				
			};
			t_sample phasor_5923 = __m_phasor_86(clamp_5919, samples_to_seconds);
			t_sample rsub_5920 = (((int)1) - phasor_5923);
			t_sample pow_5921 = safepow(rsub_5920, ((int)2));
			t_sample idown_89 = (1 / maximum(1, abs(mstosamps_5916)));
			__m_slide_87 = fixdenorm((__m_slide_87 + (((pow_5921 > __m_slide_87) ? iup_88 : idown_89) * (pow_5921 - __m_slide_87))));
			t_sample slide_5918 = __m_slide_87;
			t_sample mul_5922 = (mul_5893 * slide_5918);
			t_sample gen_5924 = mul_5922;
			t_sample clamp_5902 = ((gen_5889 <= ((t_sample)0.5)) ? ((t_sample)0.5) : ((gen_5889 >= ((int)1)) ? ((int)1) : gen_5889));
			t_sample sub_9269 = (clamp_5902 - ((t_sample)0.5));
			t_sample scale_9266 = ((safepow((sub_9269 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample scale_5901 = scale_9266;
			t_sample gen_5909 = scale_5901;
			t_sample clamp_5906 = ((gen_5889 <= ((int)0)) ? ((int)0) : ((gen_5889 >= ((t_sample)0.5)) ? ((t_sample)0.5) : gen_5889));
			t_sample sub_9273 = (clamp_5906 - ((int)0));
			t_sample scale_9270 = ((safepow((sub_9273 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample scale_5905 = scale_9270;
			t_sample gen_5908 = scale_5905;
			t_sample mix_9274 = (gen_5970 + (gen_5908 * (mul_5893 - gen_5970)));
			t_sample mix_5911 = mix_9274;
			t_sample mix_9275 = (mix_5911 + (gen_5909 * (gen_5924 - mix_5911)));
			t_sample mix_5910 = mix_9275;
			t_sample gen_5912 = mix_5910;
			t_sample omega = safediv(((t_sample)125.66370614359), samplerate);
			t_sample sn = sin(omega);
			t_sample cs = cos(omega);
			t_sample alpha = ((sn * ((t_sample)0.5)) * ((t_sample)1));
			t_sample b0 = safediv(((int)1), (((int)1) + alpha));
			t_sample a2 = (((((int)1) + cs) * ((t_sample)0.5)) * b0);
			t_sample a1 = ((-(((int)1) + cs)) * b0);
			t_sample b1 = ((((int)-2) * cs) * b0);
			t_sample b2 = ((((int)1) - alpha) * b0);
			t_sample expr_8538 = a2;
			t_sample expr_8539 = a1;
			t_sample expr_8540 = a2;
			t_sample expr_8541 = b1;
			t_sample expr_8542 = b2;
			t_sample mul_8527 = (m_history_4 * expr_8540);
			t_sample mul_8529 = (m_history_2 * expr_8539);
			t_sample mul_8532 = (gen_5912 * expr_8538);
			t_sample mul_8525 = (m_history_1 * expr_8541);
			t_sample mul_8523 = (m_history_3 * expr_8542);
			t_sample sub_8531 = (((mul_8532 + mul_8529) + mul_8527) - (mul_8523 + mul_8525));
			t_sample gen_8537 = sub_8531;
			t_sample history_8528_next_8533 = fixdenorm(m_history_2);
			t_sample history_8524_next_8534 = fixdenorm(m_history_1);
			t_sample history_8530_next_8535 = fixdenorm(gen_5912);
			t_sample history_8526_next_8536 = fixdenorm(sub_8531);
			t_sample tanh_6149 = tanh(gen_8537);
			t_sample mul_5952 = (tanh_6149 * gtp_5865);
			t_sample out1 = mul_5952;
			t_sample mul_5978 = (mul_5952 * ((int)10));
			t_sample iup_91 = (1 / maximum(1, abs(mstosamps_5986)));
			t_sample idown_92 = (1 / maximum(1, abs(mstosamps_5985)));
			__m_slide_90 = fixdenorm((__m_slide_90 + (((mul_5978 > __m_slide_90) ? iup_91 : idown_92) * (mul_5978 - __m_slide_90))));
			t_sample slide_5987 = __m_slide_90;
			t_sample mul_5950 = (slide_5987 * m_midi_cc_12);
			int choice_93 = add_5947;
			t_sample selector_5946 = ((choice_93 >= 2) ? sub_5945 : ((choice_93 >= 1) ? mul_5950 : 0));
			t_sample rsub_5977 = (((int)1) - selector_5946);
			t_sample out2 = rsub_5977;
			m_delay_8.write(mix_5956);
			m_history_7 = history_5958_next_5969;
			m_history_4 = history_8528_next_8533;
			m_history_3 = history_8524_next_8534;
			m_history_2 = history_8530_next_8535;
			m_history_1 = history_8526_next_8536;
			m_delay_8.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			(*(__out3++)) = out3;
			
		};
		return __exception;
		
	};
	inline void set_major(void * _value) {
		m_major_6.setbuffer(_value);
	};
	inline void set_info(void * _value) {
		m_info_9.setbuffer(_value);
	};
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_10 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_11 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_12 = (_value < 0 ? 0 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_13 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_14 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc12(t_param _value) {
		m_midi_cc_15 = (_value < 0 ? 0 : (_value > 50 ? 50 : _value));
	};
	inline void set_midi_cc9(t_param _value) {
		m_midi_cc_16 = (_value < 0 ? 0 : (_value > 7 ? 7 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_17 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc10(t_param _value) {
		m_midi_cc_18 = (_value < -1 ? -1 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc8(t_param _value) {
		m_midi_cc_19 = (_value < 36 ? 36 : (_value > 84 ? 84 : _value));
	};
	inline void set_midi_cc11(t_param _value) {
		m_midi_cc_20 = (_value < 0 ? 0 : (_value > 8 ? 8 : _value));
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_21 = (_value < -40 ? -40 : (_value > 0 ? 0 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_22.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_33 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_34 = ((int)0);
		bool chan_ignore_35 = ((chan_34 < 0) || (chan_34 >= _dat_channels));
		t_sample previous = ((chan_ignore_35 || index_ignore_33) ? 0 : _dat.read(_instance, chan_34));
		bool index_ignore_36 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_36)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_37 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_38 = ((int)1);
		bool chan_ignore_39 = ((chan_38 < 0) || (chan_38 >= _dat_channels));
		t_sample count = ((chan_ignore_39 || index_ignore_37) ? 0 : _dat.read(_instance, chan_38));
		t_sample iffalse_40 = (count + ((int)1));
		t_sample count_9149 = (_trig ? ((int)0) : iffalse_40);
		bool chan_ignore_41 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_42 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_41 || index_ignore_42))) {
			_dat.write(count_9149, _instance, ((int)1));
			
		};
		return count_9149;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_43 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_44 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_43 || index_ignore_44))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_45 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_46 = _channel;
			bool chan_ignore_47 = ((chan_46 < 0) || (chan_46 >= _dat_channels));
			val = ((chan_ignore_47 || index_ignore_45) ? 0 : _dat.read(_instance, chan_46));
			
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
int num_params() { return 15; }

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
		case 1: self->set_loop_wav(ref); break;
		case 2: self->set_major(ref); break;
		case 3: self->set_midi_cc1(value); break;
		case 4: self->set_midi_cc10(value); break;
		case 5: self->set_midi_cc11(value); break;
		case 6: self->set_midi_cc12(value); break;
		case 7: self->set_midi_cc2(value); break;
		case 8: self->set_midi_cc3(value); break;
		case 9: self->set_midi_cc4(value); break;
		case 10: self->set_midi_cc5(value); break;
		case 11: self->set_midi_cc6(value); break;
		case 12: self->set_midi_cc7(value); break;
		case 13: self->set_midi_cc8(value); break;
		case 14: self->set_midi_cc9(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		
		
		
		case 3: *value = self->m_midi_cc_14; break;
		case 4: *value = self->m_midi_cc_18; break;
		case 5: *value = self->m_midi_cc_20; break;
		case 6: *value = self->m_midi_cc_15; break;
		case 7: *value = self->m_midi_cc_13; break;
		case 8: *value = self->m_midi_cc_17; break;
		case 9: *value = self->m_midi_cc_11; break;
		case 10: *value = self->m_midi_cc_21; break;
		case 11: *value = self->m_midi_cc_12; break;
		case 12: *value = self->m_midi_cc_10; break;
		case 13: *value = self->m_midi_cc_19; break;
		case 14: *value = self->m_midi_cc_16; break;
		
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
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(15 * sizeof(ParamInfo));
	self->__commonstate.numparams = 15;
	// initialize parameter 0 ("m_info_9")
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
	// initialize parameter 1 ("m_loop_wav_22")
	pi = self->__commonstate.params + 1;
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
	// initialize parameter 2 ("m_major_6")
	pi = self->__commonstate.params + 2;
	pi->name = "major";
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
	// initialize parameter 3 ("m_midi_cc_14")
	pi = self->__commonstate.params + 3;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_14;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.001;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_midi_cc_18")
	pi = self->__commonstate.params + 4;
	pi->name = "midi_cc10";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_18;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -1;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_midi_cc_20")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc11";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_20;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 8;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_midi_cc_15")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc12";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_15;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 50;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_midi_cc_13")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc2";
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
	// initialize parameter 8 ("m_midi_cc_17")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_17;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_midi_cc_11")
	pi = self->__commonstate.params + 9;
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
	// initialize parameter 10 ("m_midi_cc_21")
	pi = self->__commonstate.params + 10;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_21;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -40;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_midi_cc_12")
	pi = self->__commonstate.params + 11;
	pi->name = "midi_cc6";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_12;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 2;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 12 ("m_midi_cc_10")
	pi = self->__commonstate.params + 12;
	pi->name = "midi_cc7";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_10;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 13 ("m_midi_cc_19")
	pi = self->__commonstate.params + 13;
	pi->name = "midi_cc8";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_19;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 36;
	pi->outputmax = 84;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 14 ("m_midi_cc_16")
	pi = self->__commonstate.params + 14;
	pi->name = "midi_cc9";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_16;
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
