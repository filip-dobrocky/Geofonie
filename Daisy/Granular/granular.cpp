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
	Data m_loop_wav_34;
	Data m_info_24;
	Delay m_delay_16;
	Delay m_delay_17;
	Delay m_delay_14;
	Delay m_delay_13;
	Delay m_delay_12;
	Delay m_delay_18;
	Delay m_delay_20;
	Delay m_delay_19;
	Delay m_delay_23;
	Delay m_delay_21;
	Delay m_delay_11;
	Delay m_delay_15;
	Delay m_delay_9;
	Delay m_delay_10;
	Delay m_delay_6;
	Delta __m_delta_87;
	Delta __m_delta_90;
	Delta __m_delta_82;
	Delta __m_delta_36;
	Delta __m_delta_85;
	Phasor __m_phasor_74;
	Phasor __m_phasor_37;
	Phasor __m_phasor_81;
	Phasor __m_phasor_35;
	Sah __m_sah_89;
	Sah __m_sah_91;
	Sah __m_sah_83;
	Sah __m_sah_84;
	Sah __m_sah_86;
	Sah __m_sah_88;
	SineCycle __m_cycle_79;
	SineCycle __m_cycle_80;
	SineData __sinedata;
	int __loopcount;
	int __exception;
	int vectorsize;
	t_sample samples_to_seconds;
	t_sample m_history_5;
	t_sample m_history_7;
	t_sample m_history_4;
	t_sample m_history_8;
	t_sample m_history_3;
	t_sample m_history_1;
	t_sample m_history_2;
	t_sample samplerate;
	t_sample m_midi_cc_33;
	t_sample m_history_22;
	t_sample m_pitchshift_26;
	t_sample m_midi_cc_30;
	t_sample m_midi_cc_31;
	t_sample m_midi_cc_32;
	t_sample m_shimmer_25;
	t_sample __m_count_38;
	t_sample m_midi_cc_29;
	t_sample __m_carry_40;
	t_sample __m_slide_75;
	t_sample m_midi_cc_28;
	t_sample m_midi_cc_27;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_history_1 = ((int)0);
		m_history_2 = ((int)0);
		m_history_3 = ((int)0);
		m_history_4 = ((int)0);
		m_history_5 = ((int)0);
		m_delay_6.reset("m_delay_6", ((int)88200));
		m_history_7 = ((int)0);
		m_history_8 = ((int)0);
		m_delay_9.reset("m_delay_9", ((int)688));
		m_delay_10.reset("m_delay_10", ((int)924));
		m_delay_11.reset("m_delay_11", ((int)3720));
		m_delay_12.reset("m_delay_12", ((int)4453));
		m_delay_13.reset("m_delay_13", ((int)4217));
		m_delay_14.reset("m_delay_14", ((int)2656));
		m_delay_15.reset("m_delay_15", ((int)3163));
		m_delay_16.reset("m_delay_16", ((int)1800));
		m_delay_17.reset("m_delay_17", ((int)142));
		m_delay_18.reset("m_delay_18", ((int)107));
		m_delay_19.reset("m_delay_19", ((int)379));
		m_delay_20.reset("m_delay_20", ((int)277));
		m_delay_21.reset("m_delay_21", (samplerate * 0.1));
		m_history_22 = ((int)0);
		m_delay_23.reset("m_delay_23", ((int)1000));
		m_info_24.reset("info", ((int)30), ((int)5));
		m_shimmer_25 = ((t_sample)0.66);
		m_pitchshift_26 = ((int)2);
		m_midi_cc_27 = 0.5;
		m_midi_cc_28 = 10;
		m_midi_cc_29 = 0;
		m_midi_cc_30 = 0.1;
		m_midi_cc_31 = 0;
		m_midi_cc_32 = 0.5;
		m_midi_cc_33 = 0.1;
		m_loop_wav_34.reset("loop_wav", ((int)3131843), ((int)1));
		samples_to_seconds = (1 / samplerate);
		__m_phasor_35.reset(0);
		__m_delta_36.reset(0);
		__m_phasor_37.reset(0);
		__m_count_38 = 0;
		__m_carry_40 = 0;
		__m_phasor_74.reset(0);
		__m_slide_75 = 0;
		__m_cycle_79.reset(samplerate, 0);
		__m_cycle_80.reset(samplerate, 0);
		__m_phasor_81.reset(0);
		__m_delta_82.reset(0);
		__m_sah_83.reset(0);
		__m_sah_84.reset(0);
		__m_delta_85.reset(0);
		__m_sah_86.reset(0);
		__m_delta_87.reset(0);
		__m_sah_88.reset(0);
		__m_sah_89.reset(0);
		__m_delta_90.reset(0);
		__m_sah_91.reset(0);
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
		t_sample pow_224 = safepow(m_midi_cc_32, ((t_sample)0.6));
		samples_to_seconds = (1 / samplerate);
		t_sample orange_3714 = (m_midi_cc_33 - m_midi_cc_31);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample min_78 = (-0.99);
		t_sample clamp_216 = ((m_midi_cc_29 <= min_78) ? min_78 : ((m_midi_cc_29 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_29));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample sub_3703 = (m_midi_cc_27 - ((int)0));
			t_sample scale_3700 = ((safepow((sub_3703 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_2467 = scale_3700;
			t_sample sub_3707 = (m_midi_cc_32 - ((int)0));
			t_sample scale_3704 = ((safepow((sub_3707 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_234 = scale_3704;
			t_sample mul_243 = (scale_234 * (-1));
			t_sample sub_3711 = (m_midi_cc_27 - ((int)0));
			t_sample scale_3708 = ((safepow((sub_3711 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_233 = scale_3708;
			if ((((int)0) != 0)) {
				__m_phasor_35.phase = 0;
				
			};
			t_sample phasor_244 = __m_phasor_35(scale_233, samples_to_seconds);
			t_sample sub_3715 = (phasor_244 - ((int)0));
			t_sample scale_3712 = ((safepow((sub_3715 * ((t_sample)1)), ((int)1)) * orange_3714) + m_midi_cc_31);
			t_sample scale_245 = scale_3712;
			t_sample rsub_227 = (((int)1) - m_midi_cc_27);
			t_sample mul_228 = (rsub_227 * ((int)200));
			t_sample add_226 = (mul_228 + ((int)200));
			t_sample orange_3718 = (add_226 - mul_228);
			t_sample sub_3719 = (pow_224 - ((int)0));
			t_sample scale_3716 = ((safepow((sub_3719 * ((t_sample)1)), ((int)1)) * orange_3718) + mul_228);
			t_sample scale_231 = scale_3716;
			if ((((int)0) != 0)) {
				__m_phasor_37.phase = 0;
				
			};
			int p = (__m_delta_36(__m_phasor_37(scale_2467, samples_to_seconds)) < ((int)0));
			__m_count_38 = (((int)0) ? 0 : (fixdenorm(__m_count_38 + p)));
			int carry_39 = 0;
			if ((((int)0) != 0)) {
				__m_count_38 = 0;
				__m_carry_40 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_38 >= ((int)10)))) {
				int wraps_41 = (__m_count_38 / ((int)10));
				__m_carry_40 = (__m_carry_40 + wraps_41);
				__m_count_38 = (__m_count_38 - (wraps_41 * ((int)10)));
				carry_39 = 1;
				
			};
			int c = __m_count_38;
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
				int trigger = get_trigger_i_i_dat(c, i, m_info_24);
				t_sample count = get_count_dat_i_i(m_info_24, i, trigger);
				t_sample sz = (scale_231 * (samplerate * 0.001));
				t_sample sz_3455 = latchy_i_d_dat_i_i(trigger, sz, m_info_24, i, ((int)2));
				t_sample minb_57 = safediv(count, sz_3455);
				t_sample phase = ((minb_57 < ((int)1)) ? minb_57 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_243 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_3456 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_24, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_34.dim;
				int loop_wav_channels = m_loop_wav_34.channels;
				t_sample start_pos = ((scale_245 + spry) * loop_wav_dim);
				t_sample start_pos_3457 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_24, i, ((int)4));
				t_sample playhead = (start_pos_3457 + (count * pitch_hz_3456));
				int index_trunc_58 = fixnan(floor(playhead));
				double index_fract_59 = (playhead - index_trunc_58);
				int index_trunc_60 = (index_trunc_58 - 1);
				int index_trunc_61 = (index_trunc_58 + 1);
				int index_trunc_62 = (index_trunc_58 + 2);
				int index_wrap_63 = ((index_trunc_60 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_60 + 1) % loop_wav_dim)) : (index_trunc_60 % loop_wav_dim));
				int index_wrap_64 = ((index_trunc_58 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_58 + 1) % loop_wav_dim)) : (index_trunc_58 % loop_wav_dim));
				int index_wrap_65 = ((index_trunc_61 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_61 + 1) % loop_wav_dim)) : (index_trunc_61 % loop_wav_dim));
				int index_wrap_66 = ((index_trunc_62 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_62 + 1) % loop_wav_dim)) : (index_trunc_62 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_67 = ((int)0);
				bool chan_ignore_68 = ((chan_67 < 0) || (chan_67 >= loop_wav_channels));
				double read_loop_wav_69 = (chan_ignore_68 ? 0 : m_loop_wav_34.read(index_wrap_63, chan_67));
				double read_loop_wav_70 = (chan_ignore_68 ? 0 : m_loop_wav_34.read(index_wrap_64, chan_67));
				double read_loop_wav_71 = (chan_ignore_68 ? 0 : m_loop_wav_34.read(index_wrap_65, chan_67));
				double read_loop_wav_72 = (chan_ignore_68 ? 0 : m_loop_wav_34.read(index_wrap_66, chan_67));
				double readinterp_73 = cubic_interp(index_fract_59, read_loop_wav_69, read_loop_wav_70, read_loop_wav_71, read_loop_wav_72);
				t_sample smp = readinterp_73;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_3458 = sum;
			t_sample mul_230 = (expr_3458 * ((int)10));
			if ((((int)0) != 0)) {
				__m_phasor_74.phase = 0;
				
			};
			t_sample phasor_242 = __m_phasor_74(scale_2467, samples_to_seconds);
			t_sample mul_223 = (phasor_242 * ((t_sample)0.5));
			t_sample pow_241 = safepow(mul_223, ((int)2));
			t_sample rsub_240 = (((int)1) - pow_241);
			t_sample out3 = rsub_240;
			t_sample mstosamps_238 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_237 = (((int)300) * (samplerate * 0.001));
			t_sample iup_76 = (1 / maximum(1, abs(mstosamps_238)));
			t_sample idown_77 = (1 / maximum(1, abs(mstosamps_237)));
			__m_slide_75 = fixdenorm((__m_slide_75 + (((mul_230 > __m_slide_75) ? iup_76 : idown_77) * (mul_230 - __m_slide_75))));
			t_sample slide_239 = __m_slide_75;
			t_sample rsub_229 = (((int)1) - slide_239);
			t_sample out2 = rsub_229;
			t_sample sub_3723 = (m_midi_cc_32 - ((int)0));
			t_sample scale_3720 = ((safepow((sub_3723 * ((t_sample)1)), ((int)1)) * ((int)1000)) + (-500));
			t_sample scale_2745 = scale_3720;
			t_sample add_1902 = (m_midi_cc_30 + scale_2745);
			t_sample rdiv_214 = safediv(((int)1), add_1902);
			t_sample mul_213 = (rdiv_214 * ((int)1000));
			t_sample mstosamps_215 = (mul_213 * (samplerate * 0.001));
			t_sample tap_220 = m_delay_23.read_linear(mstosamps_215);
			t_sample mul_218 = (tap_220 * clamp_216);
			t_sample add_217 = (expr_3458 + mul_218);
			t_sample gen_222 = add_217;
			t_sample mul_206 = (add_1902 * m_midi_cc_28);
			t_sample abs_207 = fabs(mul_206);
			t_sample mul_209 = (abs_207 * safediv((-6.2831853071796), samplerate));
			t_sample exp_211 = exp(mul_209);
			t_sample clamp_212 = ((exp_211 <= ((int)0)) ? ((int)0) : ((exp_211 >= ((int)1)) ? ((int)1) : exp_211));
			t_sample mix_3724 = (add_217 + (clamp_212 * (m_history_22 - add_217)));
			t_sample mix_208 = mix_3724;
			t_sample history_210_next_221 = fixdenorm(mix_208);
			t_sample mstosamps_174 = (((int)10) * (samplerate * 0.001));
			t_sample tap_203 = m_delay_21.read_linear(mstosamps_174);
			t_sample mix_3725 = (m_history_1 + (((t_sample)0.5) * (tap_203 - m_history_1)));
			t_sample mix_201 = mix_3725;
			t_sample tap_180 = m_delay_20.read_step(((int)277));
			t_sample mul_178 = (tap_180 * ((t_sample)0.625));
			t_sample tap_186 = m_delay_19.read_step(((int)379));
			t_sample mul_184 = (tap_186 * ((t_sample)0.625));
			t_sample tap_192 = m_delay_18.read_step(((int)107));
			t_sample mul_190 = (tap_192 * ((t_sample)0.75));
			t_sample tap_198 = m_delay_17.read_step(((int)142));
			t_sample mul_196 = (tap_198 * ((t_sample)0.75));
			t_sample sub_194 = (mix_201 - mul_196);
			t_sample mul_195 = (sub_194 * ((t_sample)0.75));
			t_sample add_193 = (mul_195 + tap_198);
			t_sample sub_188 = (add_193 - mul_190);
			t_sample mul_189 = (sub_188 * ((t_sample)0.75));
			t_sample add_187 = (mul_189 + tap_192);
			t_sample sub_182 = (add_187 - mul_184);
			t_sample mul_183 = (sub_182 * ((t_sample)0.625));
			t_sample add_181 = (mul_183 + tap_186);
			t_sample sub_176 = (add_181 - mul_178);
			t_sample mul_177 = (sub_176 * ((t_sample)0.625));
			t_sample add_175 = (mul_177 + tap_180);
			t_sample tap_145 = m_delay_16.read_step(((int)1800));
			t_sample tap_146 = m_delay_16.read_step(((int)187));
			t_sample tap_147 = m_delay_16.read_step(((int)1228));
			t_sample tap_109 = m_delay_15.read_step(((int)3163));
			t_sample tap_110 = m_delay_15.read_step(((int)121));
			t_sample tap_111 = m_delay_15.read_step(((int)1996));
			t_sample tap_117 = m_delay_14.read_step(((int)2656));
			t_sample tap_118 = m_delay_14.read_step(((int)335));
			t_sample tap_119 = m_delay_14.read_step(((int)1913));
			t_sample tap_121 = m_delay_13.read_step(((int)4217));
			t_sample tap_122 = m_delay_13.read_step(((int)266));
			t_sample tap_123 = m_delay_13.read_step(((int)2974));
			t_sample tap_124 = m_delay_13.read_step(((int)2111));
			t_sample gen_173 = (((tap_124 + tap_118) + tap_110) + tap_147);
			t_sample gen_170 = ((tap_122 + tap_123) + tap_111);
			t_sample mix_3726 = (tap_121 + (((t_sample)0.5) * (m_history_8 - tap_121)));
			t_sample mix_106 = mix_3726;
			t_sample tap_152 = m_delay_12.read_step(((int)4453));
			t_sample tap_153 = m_delay_12.read_step(((int)353));
			t_sample tap_154 = m_delay_12.read_step(((int)3627));
			t_sample tap_155 = m_delay_12.read_step(((int)1990));
			t_sample mix_3727 = (tap_152 + (((t_sample)0.5) * (m_history_7 - tap_152)));
			t_sample mix_150 = mix_3727;
			t_sample tap_137 = m_delay_11.read_step(((int)3720));
			t_sample tap_138 = m_delay_11.read_step(((int)1066));
			t_sample tap_139 = m_delay_11.read_step(((int)2673));
			t_sample gen_171 = (((tap_119 + tap_138) + tap_146) + tap_155);
			t_sample gen_172 = ((tap_139 + tap_153) + tap_154);
			t_sample mul_148 = (mix_150 * ((t_sample)0.5));
			t_sample mul_104 = (mix_106 * ((t_sample)0.5));
			t_sample mul_141 = (tap_145 * ((t_sample)0.5));
			t_sample sub_142 = (mul_148 - mul_141);
			t_sample mul_113 = (tap_117 * ((t_sample)0.5));
			t_sample sub_114 = (mul_104 - mul_113);
			t_sample mul_112 = (sub_114 * ((t_sample)0.5));
			t_sample add_115 = (mul_112 + tap_117);
			t_sample mul_140 = (sub_142 * ((t_sample)0.5));
			t_sample add_143 = (mul_140 + tap_145);
			t_sample mul_135 = (tap_137 * ((t_sample)0.5));
			t_sample add_166 = (mul_135 + add_175);
			__m_cycle_79.freq(((t_sample)0.07));
			t_sample cycle_125 = __m_cycle_79(__sinedata);
			t_sample cycleindex_126 = __m_cycle_79.phase();
			t_sample mul_127 = (cycle_125 * ((int)16));
			t_sample add_128 = (mul_127 + ((int)908));
			t_sample tap_134 = m_delay_10.read_linear(add_128);
			t_sample mul_130 = (tap_134 * ((t_sample)0.7));
			t_sample add_131 = (add_166 + mul_130);
			t_sample mul_129 = (add_131 * ((t_sample)0.7));
			t_sample rsub_132 = (tap_134 - mul_129);
			t_sample mul_107 = (tap_109 * ((t_sample)0.5));
			t_sample add_167 = (add_175 + mul_107);
			__m_cycle_80.freq(((t_sample)0.1));
			t_sample cycle_156 = __m_cycle_80(__sinedata);
			t_sample cycleindex_157 = __m_cycle_80.phase();
			t_sample mul_158 = (cycle_156 * ((int)16));
			t_sample add_159 = (mul_158 + ((int)672));
			t_sample tap_165 = m_delay_9.read_linear(add_159);
			t_sample mul_161 = (tap_165 * ((t_sample)0.7));
			t_sample add_162 = (add_167 + mul_161);
			t_sample mul_160 = (add_162 * ((t_sample)0.7));
			t_sample rsub_163 = (tap_165 - mul_160);
			t_sample history_105_next_168 = fixdenorm(mix_106);
			t_sample history_149_next_169 = fixdenorm(mix_150);
			t_sample sub_89 = (gen_170 - gen_171);
			t_sample mul_87 = (sub_89 * ((t_sample)0.6));
			t_sample mul_5 = (mul_87 * ((t_sample)0.5));
			t_sample sub_88 = (gen_172 - gen_173);
			t_sample mul_86 = (sub_88 * ((t_sample)0.6));
			t_sample mul_4 = (mul_86 * ((t_sample)0.5));
			t_sample add_3 = (mul_5 + mul_4);
			t_sample noise_10 = noise();
			t_sample abs_28 = fabs(noise_10);
			t_sample mul_38 = (abs_28 * ((int)0));
			t_sample noise_7 = noise();
			t_sample abs_25 = fabs(noise_7);
			t_sample mul_29 = (abs_25 * ((int)0));
			t_sample noise_8 = noise();
			t_sample abs_26 = fabs(noise_8);
			t_sample mul_32 = (abs_26 * ((int)0));
			t_sample noise_9 = noise();
			t_sample abs_27 = fabs(noise_9);
			t_sample mul_35 = (abs_27 * ((int)0));
			t_sample mstosamps_72 = (((int)100) * (samplerate * 0.001));
			if ((((int)0) != 0)) {
				__m_phasor_81.phase = 0;
				
			};
			t_sample phasor_78 = __m_phasor_81(((int)-10), samples_to_seconds);
			t_sample add_77 = ((m_history_5 + phasor_78) + ((int)0));
			t_sample mod_76 = safemod(add_77, ((int)1));
			t_sample delta_18 = __m_delta_82(mod_76);
			t_sample sah_17 = __m_sah_83(mul_38, delta_18, ((int)0));
			t_sample sah_39 = __m_sah_84(mstosamps_72, delta_18, ((int)0));
			t_sample mul_24 = (sah_39 * mod_76);
			t_sample sub_75 = (mod_76 - ((t_sample)0.5));
			t_sample mul_74 = (sub_75 * ((t_sample)3.1415926535898));
			t_sample cos_73 = cos(mul_74);
			t_sample mul_43 = (cos_73 * cos_73);
			t_sample add_49 = ((m_history_4 + phasor_78) + ((t_sample)0.75));
			t_sample mod_48 = safemod(add_49, ((int)1));
			t_sample delta_31 = __m_delta_85(mod_48);
			t_sample sah_11 = __m_sah_86(mul_29, delta_31, ((int)0));
			t_sample add_55 = ((m_history_3 + phasor_78) + ((t_sample)0.5));
			t_sample mod_54 = safemod(add_55, ((int)1));
			t_sample delta_34 = __m_delta_87(mod_54);
			t_sample sah_13 = __m_sah_88(mul_32, delta_34, ((int)0));
			t_sample sah_33 = __m_sah_89(mstosamps_72, delta_34, ((int)0));
			t_sample mul_22 = (sah_33 * mod_54);
			t_sample sub_53 = (mod_54 - ((t_sample)0.5));
			t_sample mul_52 = (sub_53 * ((t_sample)3.1415926535898));
			t_sample cos_51 = cos(mul_52);
			t_sample mul_41 = (cos_51 * cos_51);
			t_sample add_71 = ((m_history_2 + phasor_78) + ((t_sample)0.25));
			t_sample mod_70 = safemod(add_71, ((int)1));
			t_sample delta_37 = __m_delta_90(mod_70);
			t_sample sah_15 = __m_sah_91(mul_35, delta_37, ((int)0));
			t_sample tap_63 = m_delay_6.read_linear(mul_24);
			t_sample tap_65 = m_delay_6.read_linear(mul_22);
			t_sample mul_61 = (tap_63 * mul_43);
			t_sample mul_50 = (tap_65 * mul_41);
			t_sample add_3729 = (mul_50 + mul_61);
			t_sample mix_3728 = (add_3 + (((int)1) * (add_3729 - add_3)));
			t_sample history_19_next_79 = fixdenorm(sah_17);
			t_sample history_12_next_80 = fixdenorm(sah_11);
			t_sample history_14_next_81 = fixdenorm(sah_13);
			t_sample history_16_next_82 = fixdenorm(sah_15);
			t_sample mix_3732 = (add_3 + (((t_sample)0.66) * (mix_3728 - add_3)));
			t_sample mix_3733 = (gen_222 + (((t_sample)0.5) * (mix_3732 - gen_222)));
			t_sample history_200_next_204 = fixdenorm(mix_201);
			t_sample out1 = mix_3733;
			m_delay_23.write(mix_208);
			m_history_22 = history_210_next_221;
			m_delay_21.write(gen_222);
			m_history_1 = history_200_next_204;
			m_history_5 = history_19_next_79;
			m_history_4 = history_12_next_80;
			m_history_3 = history_14_next_81;
			m_history_2 = history_16_next_82;
			m_delay_6.write(add_3);
			m_delay_15.write(add_115);
			m_delay_14.write(sub_114);
			m_delay_13.write(rsub_132);
			m_delay_12.write(rsub_163);
			m_delay_11.write(add_143);
			m_delay_10.write(add_131);
			m_delay_9.write(add_162);
			m_history_8 = history_105_next_168;
			m_history_7 = history_149_next_169;
			m_delay_16.write(sub_142);
			m_delay_19.write(sub_182);
			m_delay_18.write(sub_188);
			m_delay_17.write(sub_194);
			m_delay_20.write(sub_176);
			m_delay_6.step();
			m_delay_9.step();
			m_delay_10.step();
			m_delay_11.step();
			m_delay_12.step();
			m_delay_13.step();
			m_delay_14.step();
			m_delay_15.step();
			m_delay_16.step();
			m_delay_17.step();
			m_delay_18.step();
			m_delay_19.step();
			m_delay_20.step();
			m_delay_21.step();
			m_delay_23.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			(*(__out3++)) = out3;
			
		};
		return __exception;
		
	};
	inline void set_info(void * _value) {
		m_info_24.setbuffer(_value);
	};
	inline void set_shimmer(t_param _value) {
		m_shimmer_25 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_pitchshift(t_param _value) {
		m_pitchshift_26 = (_value < 0.1 ? 0.1 : (_value > 10 ? 10 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_27 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_28 = (_value < 1 ? 1 : (_value > 20 ? 20 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_29 = (_value < -1 ? -1 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_30 = (_value < 50 ? 50 : (_value > 1000 ? 1000 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_31 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_32 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_33 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_34.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_42 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_43 = ((int)0);
		bool chan_ignore_44 = ((chan_43 < 0) || (chan_43 >= _dat_channels));
		t_sample previous = ((chan_ignore_44 || index_ignore_42) ? 0 : _dat.read(_instance, chan_43));
		bool index_ignore_45 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_45)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_46 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_47 = ((int)1);
		bool chan_ignore_48 = ((chan_47 < 0) || (chan_47 >= _dat_channels));
		t_sample count = ((chan_ignore_48 || index_ignore_46) ? 0 : _dat.read(_instance, chan_47));
		t_sample iffalse_49 = (count + ((int)1));
		t_sample count_3454 = (_trig ? ((int)0) : iffalse_49);
		bool chan_ignore_50 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_51 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_50 || index_ignore_51))) {
			_dat.write(count_3454, _instance, ((int)1));
			
		};
		return count_3454;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_52 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_53 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_52 || index_ignore_53))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_54 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_55 = _channel;
			bool chan_ignore_56 = ((chan_55 < 0) || (chan_55 >= _dat_channels));
			val = ((chan_ignore_56 || index_ignore_54) ? 0 : _dat.read(_instance, chan_55));
			
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
int num_params() { return 11; }

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
		case 2: self->set_midi_cc1(value); break;
		case 3: self->set_midi_cc2(value); break;
		case 4: self->set_midi_cc3(value); break;
		case 5: self->set_midi_cc4(value); break;
		case 6: self->set_midi_cc5(value); break;
		case 7: self->set_midi_cc6(value); break;
		case 8: self->set_midi_cc7(value); break;
		case 9: self->set_pitchshift(value); break;
		case 10: self->set_shimmer(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		
		
		case 2: *value = self->m_midi_cc_32; break;
		case 3: *value = self->m_midi_cc_31; break;
		case 4: *value = self->m_midi_cc_33; break;
		case 5: *value = self->m_midi_cc_27; break;
		case 6: *value = self->m_midi_cc_30; break;
		case 7: *value = self->m_midi_cc_29; break;
		case 8: *value = self->m_midi_cc_28; break;
		case 9: *value = self->m_pitchshift_26; break;
		case 10: *value = self->m_shimmer_25; break;
		
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
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(11 * sizeof(ParamInfo));
	self->__commonstate.numparams = 11;
	// initialize parameter 0 ("m_info_24")
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
	// initialize parameter 1 ("m_loop_wav_34")
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
	// initialize parameter 2 ("m_midi_cc_32")
	pi = self->__commonstate.params + 2;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_32;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.001;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 3 ("m_midi_cc_31")
	pi = self->__commonstate.params + 3;
	pi->name = "midi_cc2";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_31;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_midi_cc_33")
	pi = self->__commonstate.params + 4;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_33;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_midi_cc_27")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc4";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_27;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_midi_cc_30")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_30;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 50;
	pi->outputmax = 1000;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_midi_cc_29")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc6";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_29;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -1;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_midi_cc_28")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc7";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_28;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 1;
	pi->outputmax = 20;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_pitchshift_26")
	pi = self->__commonstate.params + 9;
	pi->name = "pitchshift";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_pitchshift_26;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.1;
	pi->outputmax = 10;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_shimmer_25")
	pi = self->__commonstate.params + 10;
	pi->name = "shimmer";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_shimmer_25;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
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
