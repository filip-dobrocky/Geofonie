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
	Data m_info_11;
	Data m_major_8;
	Data m_loop_wav_24;
	Delay m_delay_10;
	Delta __m_delta_29;
	Phasor __m_phasor_28;
	Phasor __m_phasor_67;
	Phasor __m_phasor_30;
	Phasor __m_phasor_88;
	SineCycle __m_cycle_87;
	SineCycle __m_cycle_86;
	SineData __sinedata;
	int __exception;
	int __loopcount;
	int vectorsize;
	t_sample __m_slide_25;
	t_sample m_midi_cc_23;
	t_sample samples_to_seconds;
	t_sample __m_count_31;
	t_sample __m_carry_33;
	t_sample m_midi_cc_22;
	t_sample __m_slide_68;
	t_sample m_midi_cc_21;
	t_sample m_midi_cc_19;
	t_sample __m_slide_89;
	t_sample m_history_4;
	t_sample m_pc_5;
	t_sample m_pm_6;
	t_sample m_history_3;
	t_sample m_history_1;
	t_sample m_history_2;
	t_sample samplerate;
	t_sample m_midi_cc_20;
	t_sample m_init_7;
	t_sample m_midi_cc_12;
	t_sample m_midi_cc_17;
	t_sample m_midi_cc_18;
	t_sample m_history_9;
	t_sample m_midi_cc_16;
	t_sample m_midi_cc_14;
	t_sample m_midi_cc_15;
	t_sample m_midi_cc_13;
	t_sample __m_slide_92;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_history_1 = ((int)0);
		m_history_2 = ((int)0);
		m_history_3 = ((int)0);
		m_history_4 = ((int)0);
		m_pc_5 = ((int)0);
		m_pm_6 = ((int)0);
		m_init_7 = ((int)0);
		m_major_8.reset("major", ((int)7), ((int)1));
		m_history_9 = ((int)0);
		m_delay_10.reset("m_delay_10", ((int)1000));
		m_info_11.reset("info", ((int)10), ((int)5));
		m_midi_cc_12 = 0;
		m_midi_cc_13 = 1;
		m_midi_cc_14 = 0.1;
		m_midi_cc_15 = 0.5;
		m_midi_cc_16 = 0.5;
		m_midi_cc_17 = 0;
		m_midi_cc_18 = 1;
		m_midi_cc_19 = 0;
		m_midi_cc_20 = 0.1;
		m_midi_cc_21 = 1;
		m_midi_cc_22 = 1;
		m_midi_cc_23 = 1.5;
		m_loop_wav_24.reset("loop_wav", ((int)3131843), ((int)1));
		__m_slide_25 = 0;
		samples_to_seconds = (1 / samplerate);
		__m_phasor_28.reset(0);
		__m_delta_29.reset(0);
		__m_phasor_30.reset(0);
		__m_count_31 = 0;
		__m_carry_33 = 0;
		__m_phasor_67.reset(0);
		__m_slide_68 = 0;
		__m_cycle_86.reset(samplerate, 0);
		__m_cycle_87.reset(samplerate, 0);
		__m_phasor_88.reset(0);
		__m_slide_89 = 0;
		__m_slide_92 = 0;
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
		t_sample pow_1671 = safepow(m_midi_cc_16, ((t_sample)0.6));
		samples_to_seconds = (1 / samplerate);
		t_sample orange_2395 = (m_midi_cc_20 - m_midi_cc_19);
		__loopcount = (__n * GENLIB_LOOPCOUNT_BAIL);
		t_sample dbtoa_1650 = dbtoa(m_midi_cc_12);
		t_sample gtp_1568 = ((dbtoa_1650 > ((t_sample)0.01)) ? dbtoa_1650 : 0);
		t_sample floor_1590 = floor(m_midi_cc_21);
		t_sample min_71 = (-0.99);
		t_sample clamp_1663 = ((m_midi_cc_17 <= min_71) ? min_71 : ((m_midi_cc_17 >= ((t_sample)0.99)) ? ((t_sample)0.99) : m_midi_cc_17));
		int major_dim = m_major_8.dim;
		int major_channels = m_major_8.channels;
		t_sample mstosamps_1616 = (((t_sample)0.5) * (samplerate * 0.001));
		t_sample iup_90 = (1 / maximum(1, abs(mstosamps_1616)));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample mstosamps_1578 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_1577 = (((int)600) * (samplerate * 0.001));
			t_sample iup_26 = (1 / maximum(1, abs(mstosamps_1578)));
			t_sample idown_27 = (1 / maximum(1, abs(mstosamps_1577)));
			__m_slide_25 = fixdenorm((__m_slide_25 + (((m_midi_cc_15 > __m_slide_25) ? iup_26 : idown_27) * (m_midi_cc_15 - __m_slide_25))));
			t_sample slide_1581 = __m_slide_25;
			t_sample gen_1582 = slide_1581;
			t_sample sub_2383 = (gen_1582 - ((int)0));
			t_sample scale_2380 = ((safepow((sub_2383 * ((t_sample)1)), ((int)1)) * ((t_sample)99.5)) + ((t_sample)0.5));
			t_sample scale_1679 = scale_2380;
			t_sample sub_2387 = (m_midi_cc_16 - ((int)0));
			t_sample scale_2384 = ((safepow((sub_2387 * ((t_sample)1)), ((int)1)) * ((int)100)) + (-50));
			t_sample scale_1681 = scale_2384;
			t_sample mul_1690 = (scale_1681 * (-1));
			t_sample sub_2391 = (gen_1582 - ((int)0));
			t_sample scale_2388 = ((safepow((sub_2391 * ((t_sample)1)), ((int)1)) * ((int)90)) + ((int)10));
			t_sample scale_1680 = scale_2388;
			if ((((int)0) != 0)) {
				__m_phasor_28.phase = 0;
				
			};
			t_sample phasor_1691 = __m_phasor_28(scale_1680, samples_to_seconds);
			t_sample add_2393 = (gen_1582 + phasor_1691);
			t_sample sub_2396 = (add_2393 - ((int)0));
			t_sample scale_2392 = ((safepow((sub_2396 * ((t_sample)1)), ((int)1)) * orange_2395) + m_midi_cc_19);
			t_sample scale_1692 = scale_2392;
			t_sample rsub_1674 = (((int)1) - gen_1582);
			t_sample mul_1675 = (rsub_1674 * ((int)200));
			t_sample add_1673 = (mul_1675 + ((int)200));
			t_sample orange_2399 = (add_1673 - mul_1675);
			t_sample sub_2400 = (pow_1671 - ((int)0));
			t_sample scale_2397 = ((safepow((sub_2400 * ((t_sample)1)), ((int)1)) * orange_2399) + mul_1675);
			t_sample scale_1678 = scale_2397;
			if ((((int)0) != 0)) {
				__m_phasor_30.phase = 0;
				
			};
			int p = (__m_delta_29(__m_phasor_30(scale_1679, samples_to_seconds)) < ((int)0));
			__m_count_31 = (((int)0) ? 0 : (fixdenorm(__m_count_31 + p)));
			int carry_32 = 0;
			if ((((int)0) != 0)) {
				__m_count_31 = 0;
				__m_carry_33 = 0;
				
			} else if (((((int)10) > 0) && (__m_count_31 >= ((int)10)))) {
				int wraps_34 = (__m_count_31 / ((int)10));
				__m_carry_33 = (__m_carry_33 + wraps_34);
				__m_count_31 = (__m_count_31 - (wraps_34 * ((int)10)));
				carry_32 = 1;
				
			};
			int c = __m_count_31;
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
				int trigger = get_trigger_i_i_dat(c, i, m_info_11);
				t_sample count = get_count_dat_i_i(m_info_11, i, trigger);
				t_sample sz = (scale_1678 * (samplerate * 0.001));
				t_sample sz_1702 = latchy_i_d_dat_i_i(trigger, sz, m_info_11, i, ((int)2));
				t_sample minb_50 = safediv(count, sz_1702);
				t_sample phase = ((minb_50 < ((int)1)) ? minb_50 : ((int)1));
				t_sample amp = (((t_sample)0.5) - (((t_sample)0.5) * cos((phase * ((t_sample)6.2831853071796)))));
				t_sample sprd = (noise() * ((int)12));
				t_sample pitch_hz = safepow(((int)2), ((mul_1690 + sprd) * ((t_sample)0.083333333333333)));
				t_sample pitch_hz_1703 = latchy_i_d_dat_i_i(trigger, pitch_hz, m_info_11, i, ((int)3));
				t_sample spry = (noise() * ((t_sample)0.5));
				int loop_wav_dim = m_loop_wav_24.dim;
				int loop_wav_channels = m_loop_wav_24.channels;
				t_sample start_pos = ((scale_1692 + spry) * loop_wav_dim);
				t_sample start_pos_1704 = latchy_i_d_dat_i_i(trigger, start_pos, m_info_11, i, ((int)4));
				t_sample playhead = (start_pos_1704 + (count * pitch_hz_1703));
				int index_trunc_51 = fixnan(floor(playhead));
				double index_fract_52 = (playhead - index_trunc_51);
				int index_trunc_53 = (index_trunc_51 - 1);
				int index_trunc_54 = (index_trunc_51 + 1);
				int index_trunc_55 = (index_trunc_51 + 2);
				int index_wrap_56 = ((index_trunc_53 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_53 + 1) % loop_wav_dim)) : (index_trunc_53 % loop_wav_dim));
				int index_wrap_57 = ((index_trunc_51 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_51 + 1) % loop_wav_dim)) : (index_trunc_51 % loop_wav_dim));
				int index_wrap_58 = ((index_trunc_54 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_54 + 1) % loop_wav_dim)) : (index_trunc_54 % loop_wav_dim));
				int index_wrap_59 = ((index_trunc_55 < 0) ? ((loop_wav_dim - 1) + ((index_trunc_55 + 1) % loop_wav_dim)) : (index_trunc_55 % loop_wav_dim));
				// samples loop_wav channel 1;
				int chan_60 = ((int)0);
				bool chan_ignore_61 = ((chan_60 < 0) || (chan_60 >= loop_wav_channels));
				double read_loop_wav_62 = (chan_ignore_61 ? 0 : m_loop_wav_24.read(index_wrap_56, chan_60));
				double read_loop_wav_63 = (chan_ignore_61 ? 0 : m_loop_wav_24.read(index_wrap_57, chan_60));
				double read_loop_wav_64 = (chan_ignore_61 ? 0 : m_loop_wav_24.read(index_wrap_58, chan_60));
				double read_loop_wav_65 = (chan_ignore_61 ? 0 : m_loop_wav_24.read(index_wrap_59, chan_60));
				double readinterp_66 = cubic_interp(index_fract_52, read_loop_wav_62, read_loop_wav_63, read_loop_wav_64, read_loop_wav_65);
				t_sample smp = readinterp_66;
				t_sample grain = (amp * smp);
				sum = (sum + grain);
				// for loop increment;
				i = (i + ((int)1));
				
			};
			t_sample expr_1705 = sum;
			t_sample sub_1644 = (m_midi_cc_13 - ((int)1));
			if ((((int)0) != 0)) {
				__m_phasor_67.phase = 0;
				
			};
			t_sample phasor_1689 = __m_phasor_67(scale_1679, samples_to_seconds);
			t_sample mul_1670 = (phasor_1689 * ((t_sample)0.5));
			t_sample pow_1688 = safepow(mul_1670, ((int)2));
			t_sample rsub_1687 = (((int)1) - pow_1688);
			t_sample out3 = rsub_1687;
			int gt_1647 = (m_midi_cc_13 > ((int)1));
			int add_1646 = (gt_1647 + ((int)1));
			t_sample mstosamps_1584 = (((int)600) * (samplerate * 0.001));
			t_sample mstosamps_1583 = (((int)600) * (samplerate * 0.001));
			t_sample iup_69 = (1 / maximum(1, abs(mstosamps_1584)));
			t_sample idown_70 = (1 / maximum(1, abs(mstosamps_1583)));
			__m_slide_68 = fixdenorm((__m_slide_68 + (((m_midi_cc_22 > __m_slide_68) ? iup_69 : idown_70) * (m_midi_cc_22 - __m_slide_68))));
			t_sample slide_1587 = __m_slide_68;
			t_sample gen_1588 = slide_1587;
			t_sample sub_2404 = (gen_1582 - ((int)0));
			t_sample scale_2401 = ((safepow((sub_2404 * ((t_sample)1)), ((int)1)) * ((t_sample)19.9)) + ((t_sample)0.1));
			t_sample scale_1591 = scale_2401;
			t_sample mstosamps_1685 = (((int)10) * (samplerate * 0.001));
			t_sample mstosamps_1684 = (((int)300) * (samplerate * 0.001));
			t_sample mtof_1612 = mtof(floor_1590, ((int)440));
			t_sample rdiv_1661 = safediv(((int)1), mtof_1612);
			t_sample mul_1660 = (rdiv_1661 * ((int)1000));
			t_sample mstosamps_1662 = (mul_1660 * (samplerate * 0.001));
			t_sample tap_1667 = m_delay_10.read_linear(mstosamps_1662);
			t_sample mul_1665 = (tap_1667 * clamp_1663);
			t_sample add_1664 = (expr_1705 + mul_1665);
			t_sample gen_1669 = add_1664;
			t_sample mul_1653 = (mtof_1612 * ((int)10));
			t_sample abs_1654 = fabs(mul_1653);
			t_sample mul_1656 = (abs_1654 * safediv((-6.2831853071796), samplerate));
			t_sample exp_1658 = exp(mul_1656);
			t_sample clamp_1659 = ((exp_1658 <= ((int)0)) ? ((int)0) : ((exp_1658 >= ((int)1)) ? ((int)1) : exp_1658));
			t_sample mix_2405 = (add_1664 + (clamp_1659 * (m_history_9 - add_1664)));
			t_sample mix_1655 = mix_2405;
			t_sample history_1657_next_1668 = fixdenorm(mix_1655);
			t_sample mod_1624 = safemod(floor_1590, ((int)12));
			t_sample rsub_1589 = (((int)1) - m_midi_cc_16);
			t_sample mul_1576 = (rsub_1589 * ((int)48));
			t_sample add_1652 = (mul_1576 + floor_1590);
			if ((m_init_7 == ((int)0))) {
				int major_dim = m_major_8.dim;
				int major_channels = m_major_8.channels;
				m_major_8.write(((int)0), 0, 0);
				bool index_ignore_72 = (((int)1) >= major_dim);
				if ((!index_ignore_72)) {
					m_major_8.write(((int)2), ((int)1), 0);
					
				};
				bool index_ignore_73 = (((int)2) >= major_dim);
				if ((!index_ignore_73)) {
					m_major_8.write(((int)4), ((int)2), 0);
					
				};
				bool index_ignore_74 = (((int)3) >= major_dim);
				if ((!index_ignore_74)) {
					m_major_8.write(((int)5), ((int)3), 0);
					
				};
				bool index_ignore_75 = (((int)4) >= major_dim);
				if ((!index_ignore_75)) {
					m_major_8.write(((int)7), ((int)4), 0);
					
				};
				bool index_ignore_76 = (((int)5) >= major_dim);
				if ((!index_ignore_76)) {
					m_major_8.write(((int)9), ((int)5), 0);
					
				};
				bool index_ignore_77 = (((int)6) >= major_dim);
				if ((!index_ignore_77)) {
					m_major_8.write(((int)11), ((int)6), 0);
					
				};
				m_init_7 = ((int)1);
				
			};
			t_sample root = floor(mod_1624);
			t_sample mode = safemod(floor(m_midi_cc_18), ((int)7));
			int index_trunc_78 = fixnan(floor(mode));
			bool index_ignore_79 = ((index_trunc_78 >= major_dim) || (index_trunc_78 < 0));
			// samples major channel 1;
			int chan_80 = ((int)0);
			bool chan_ignore_81 = ((chan_80 < 0) || (chan_80 >= major_channels));
			t_sample tonic = ((chan_ignore_81 || index_ignore_79) ? 0 : m_major_8.read(index_trunc_78, chan_80));
			t_sample note = (add_1652 - root);
			t_sample octave = floor((note * ((t_sample)0.083333333333333)));
			t_sample degree = (note - (octave * ((int)12)));
			t_sample min_dif = ((int)127);
			t_sample q_deg = ((int)0);
			// for loop initializer;
			int i_1707 = ((int)0);
			// for loop condition;
			while ((i_1707 < ((int)7))) {
				// abort processing if an infinite loop is suspected;
				if (((__loopcount--) <= 0)) {
					__exception = GENLIB_ERR_LOOP_OVERFLOW;
					break ;
					
				};
				int index_trunc_82 = fixnan(floor(safemod((i_1707 + mode), ((int)7))));
				bool index_ignore_83 = ((index_trunc_82 >= major_dim) || (index_trunc_82 < 0));
				// samples major channel 1;
				int chan_84 = ((int)0);
				bool chan_ignore_85 = ((chan_84 < 0) || (chan_84 >= major_channels));
				t_sample peek_1634 = ((chan_ignore_85 || index_ignore_83) ? 0 : m_major_8.read(index_trunc_82, chan_84));
				t_sample peek_1635 = safemod((i_1707 + mode), ((int)7));
				t_sample d = safemod(((peek_1634 - tonic) + ((int)12)), ((int)12));
				t_sample dif = fabs((degree - d));
				if ((dif < min_dif)) {
					min_dif = dif;
					q_deg = d;
					
				};
				// for loop increment;
				i_1707 = (i_1707 + ((int)1));
				
			};
			t_sample expr_1642 = mtof(((root + q_deg) + (octave * ((int)12))), ((int)440));
			t_sample gen_1643 = expr_1642;
			t_sample f = gen_1643;
			m_pm_6 = wrap((m_pm_6 + safediv((f * m_midi_cc_23), samplerate)), ((int)0), ((int)1));
			__m_cycle_86.phase(m_pm_6);
			t_sample cycle_1572 = __m_cycle_86(__sinedata);
			t_sample cycle_1573 = __m_cycle_86.phase();
			m_pc_5 = wrap((m_pc_5 + safediv((f * (((int)1) + (((m_midi_cc_14 * m_midi_cc_14) * ((int)50)) * cycle_1572))), samplerate)), ((int)0), ((int)1));
			__m_cycle_87.phase(m_pc_5);
			t_sample expr_1574 = __m_cycle_87(__sinedata);
			t_sample gen_1575 = expr_1574;
			t_sample mul_1592 = (gen_1575 * ((t_sample)0.15));
			t_sample mstosamps_1615 = (((int)1) * (samplerate * 0.001));
			t_sample clamp_1618 = ((scale_1591 <= ((t_sample)0.1)) ? ((t_sample)0.1) : ((scale_1591 >= ((int)1000)) ? ((int)1000) : scale_1591));
			if ((((int)0) != 0)) {
				__m_phasor_88.phase = 0;
				
			};
			t_sample phasor_1622 = __m_phasor_88(clamp_1618, samples_to_seconds);
			t_sample rsub_1619 = (((int)1) - phasor_1622);
			t_sample pow_1620 = safepow(rsub_1619, ((int)2));
			t_sample idown_91 = (1 / maximum(1, abs(mstosamps_1615)));
			__m_slide_89 = fixdenorm((__m_slide_89 + (((pow_1620 > __m_slide_89) ? iup_90 : idown_91) * (pow_1620 - __m_slide_89))));
			t_sample slide_1617 = __m_slide_89;
			t_sample mul_1621 = (mul_1592 * slide_1617);
			t_sample gen_1623 = mul_1621;
			t_sample clamp_1605 = ((gen_1588 <= ((int)0)) ? ((int)0) : ((gen_1588 >= ((t_sample)0.5)) ? ((t_sample)0.5) : gen_1588));
			t_sample sub_2409 = (clamp_1605 - ((int)0));
			t_sample scale_2406 = ((safepow((sub_2409 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample scale_1604 = scale_2406;
			t_sample gen_1607 = scale_1604;
			t_sample clamp_1601 = ((gen_1588 <= ((t_sample)0.5)) ? ((t_sample)0.5) : ((gen_1588 >= ((int)1)) ? ((int)1) : gen_1588));
			t_sample sub_2413 = (clamp_1601 - ((t_sample)0.5));
			t_sample scale_2410 = ((safepow((sub_2413 * ((t_sample)2)), ((int)1)) * ((int)1)) + ((int)0));
			t_sample scale_1600 = scale_2410;
			t_sample gen_1608 = scale_1600;
			t_sample mix_2414 = (gen_1669 + (gen_1607 * (mul_1592 - gen_1669)));
			t_sample mix_1610 = mix_2414;
			t_sample mix_2415 = (mix_1610 + (gen_1608 * (gen_1623 - mix_1610)));
			t_sample mix_1609 = mix_2415;
			t_sample gen_1611 = mix_1609;
			t_sample omega = safediv(((t_sample)125.66370614359), samplerate);
			t_sample sn = sin(omega);
			t_sample cs = cos(omega);
			t_sample alpha = ((sn * ((t_sample)0.5)) * ((t_sample)1));
			t_sample b0 = safediv(((int)1), (((int)1) + alpha));
			t_sample a2 = (((((int)1) + cs) * ((t_sample)0.5)) * b0);
			t_sample a1 = ((-(((int)1) + cs)) * b0);
			t_sample b1 = ((((int)-2) * cs) * b0);
			t_sample b2 = ((((int)1) - alpha) * b0);
			t_sample expr_1559 = a2;
			t_sample expr_1560 = a1;
			t_sample expr_1561 = a2;
			t_sample expr_1562 = b1;
			t_sample expr_1563 = b2;
			t_sample mul_1553 = (gen_1611 * expr_1559);
			t_sample mul_1550 = (m_history_2 * expr_1560);
			t_sample mul_1548 = (m_history_4 * expr_1561);
			t_sample mul_1544 = (m_history_3 * expr_1563);
			t_sample mul_1546 = (m_history_1 * expr_1562);
			t_sample sub_1552 = (((mul_1548 + mul_1550) + mul_1553) - (mul_1546 + mul_1544));
			t_sample gen_1558 = sub_1552;
			t_sample history_1549_next_1554 = fixdenorm(m_history_2);
			t_sample history_1545_next_1555 = fixdenorm(m_history_1);
			t_sample history_1551_next_1556 = fixdenorm(gen_1611);
			t_sample history_1547_next_1557 = fixdenorm(sub_1552);
			t_sample tanh_1565 = tanh(gen_1558);
			t_sample mul_1651 = (tanh_1565 * gtp_1568);
			t_sample out1 = mul_1651;
			t_sample mul_1677 = (mul_1651 * ((int)10));
			t_sample iup_93 = (1 / maximum(1, abs(mstosamps_1685)));
			t_sample idown_94 = (1 / maximum(1, abs(mstosamps_1684)));
			__m_slide_92 = fixdenorm((__m_slide_92 + (((mul_1677 > __m_slide_92) ? iup_93 : idown_94) * (mul_1677 - __m_slide_92))));
			t_sample slide_1686 = __m_slide_92;
			t_sample mul_1649 = (slide_1686 * m_midi_cc_13);
			int choice_95 = add_1646;
			t_sample selector_1645 = ((choice_95 >= 2) ? sub_1644 : ((choice_95 >= 1) ? mul_1649 : 0));
			t_sample rsub_1676 = (((int)1) - selector_1645);
			t_sample out2 = rsub_1676;
			m_delay_10.write(mix_1655);
			m_history_9 = history_1657_next_1668;
			m_history_4 = history_1549_next_1554;
			m_history_3 = history_1545_next_1555;
			m_history_2 = history_1551_next_1556;
			m_history_1 = history_1547_next_1557;
			m_delay_10.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			(*(__out3++)) = out3;
			
		};
		return __exception;
		
	};
	inline void set_major(void * _value) {
		m_major_8.setbuffer(_value);
	};
	inline void set_info(void * _value) {
		m_info_11.setbuffer(_value);
	};
	inline void set_midi_cc5(t_param _value) {
		m_midi_cc_12 = (_value < -40 ? -40 : (_value > 0 ? 0 : _value));
	};
	inline void set_midi_cc6(t_param _value) {
		m_midi_cc_13 = (_value < 0 ? 0 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc12(t_param _value) {
		m_midi_cc_14 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc4(t_param _value) {
		m_midi_cc_15 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_16 = (_value < 0.001 ? 0.001 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc10(t_param _value) {
		m_midi_cc_17 = (_value < -1 ? -1 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc9(t_param _value) {
		m_midi_cc_18 = (_value < 0 ? 0 : (_value > 7 ? 7 : _value));
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_19 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_20 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc8(t_param _value) {
		m_midi_cc_21 = (_value < 36 ? 36 : (_value > 84 ? 84 : _value));
	};
	inline void set_midi_cc7(t_param _value) {
		m_midi_cc_22 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_midi_cc11(t_param _value) {
		m_midi_cc_23 = (_value < 1 ? 1 : (_value > 8 ? 8 : _value));
	};
	inline void set_loop_wav(void * _value) {
		m_loop_wav_24.setbuffer(_value);
	};
	inline int get_trigger_i_i_dat(int _count, int _instance, Data& _dat) {
		int current = (_count == _instance);
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_35 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_36 = ((int)0);
		bool chan_ignore_37 = ((chan_36 < 0) || (chan_36 >= _dat_channels));
		t_sample previous = ((chan_ignore_37 || index_ignore_35) ? 0 : _dat.read(_instance, chan_36));
		bool index_ignore_38 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!index_ignore_38)) {
			_dat.write(current, _instance, 0);
			
		};
		return ((current - previous) == ((int)1));
		
	};
	inline t_sample get_count_dat_i_i(Data& _dat, int _instance, int _trig) {
		int _dat_dim = _dat.dim;
		int _dat_channels = _dat.channels;
		bool index_ignore_39 = ((_instance >= _dat_dim) || (_instance < 0));
		// samples _dat channel 1;
		int chan_40 = ((int)1);
		bool chan_ignore_41 = ((chan_40 < 0) || (chan_40 >= _dat_channels));
		t_sample count = ((chan_ignore_41 || index_ignore_39) ? 0 : _dat.read(_instance, chan_40));
		t_sample iffalse_42 = (count + ((int)1));
		t_sample count_1701 = (_trig ? ((int)0) : iffalse_42);
		bool chan_ignore_43 = ((((int)1) < 0) || (((int)1) >= _dat_channels));
		bool index_ignore_44 = ((_instance >= _dat_dim) || (_instance < 0));
		if ((!(chan_ignore_43 || index_ignore_44))) {
			_dat.write(count_1701, _instance, ((int)1));
			
		};
		return count_1701;
		
	};
	inline t_sample latchy_i_d_dat_i_i(int _trigger, t_sample _val, Data& _dat, int _instance, int _channel) {
		t_sample val = _val;
		if (_trigger) {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool chan_ignore_45 = ((_channel < 0) || (_channel >= _dat_channels));
			bool index_ignore_46 = ((_instance >= _dat_dim) || (_instance < 0));
			if ((!(chan_ignore_45 || index_ignore_46))) {
				_dat.write(val, _instance, _channel);
				
			};
			
		} else {
			int _dat_dim = _dat.dim;
			int _dat_channels = _dat.channels;
			bool index_ignore_47 = ((_instance >= _dat_dim) || (_instance < 0));
			// samples _dat channel 1;
			int chan_48 = _channel;
			bool chan_ignore_49 = ((chan_48 < 0) || (chan_48 >= _dat_channels));
			val = ((chan_ignore_49 || index_ignore_47) ? 0 : _dat.read(_instance, chan_48));
			
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
		
		
		
		case 3: *value = self->m_midi_cc_16; break;
		case 4: *value = self->m_midi_cc_17; break;
		case 5: *value = self->m_midi_cc_23; break;
		case 6: *value = self->m_midi_cc_14; break;
		case 7: *value = self->m_midi_cc_19; break;
		case 8: *value = self->m_midi_cc_20; break;
		case 9: *value = self->m_midi_cc_15; break;
		case 10: *value = self->m_midi_cc_12; break;
		case 11: *value = self->m_midi_cc_13; break;
		case 12: *value = self->m_midi_cc_22; break;
		case 13: *value = self->m_midi_cc_21; break;
		case 14: *value = self->m_midi_cc_18; break;
		
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
	// initialize parameter 0 ("m_info_11")
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
	// initialize parameter 1 ("m_loop_wav_24")
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
	// initialize parameter 2 ("m_major_8")
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
	// initialize parameter 3 ("m_midi_cc_16")
	pi = self->__commonstate.params + 3;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_16;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.001;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_midi_cc_17")
	pi = self->__commonstate.params + 4;
	pi->name = "midi_cc10";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_17;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -1;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_midi_cc_23")
	pi = self->__commonstate.params + 5;
	pi->name = "midi_cc11";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_23;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 1;
	pi->outputmax = 8;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_midi_cc_14")
	pi = self->__commonstate.params + 6;
	pi->name = "midi_cc12";
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
	// initialize parameter 7 ("m_midi_cc_19")
	pi = self->__commonstate.params + 7;
	pi->name = "midi_cc2";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_19;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_midi_cc_20")
	pi = self->__commonstate.params + 8;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_20;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_midi_cc_15")
	pi = self->__commonstate.params + 9;
	pi->name = "midi_cc4";
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
	// initialize parameter 10 ("m_midi_cc_12")
	pi = self->__commonstate.params + 10;
	pi->name = "midi_cc5";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_12;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -40;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_midi_cc_13")
	pi = self->__commonstate.params + 11;
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
	// initialize parameter 12 ("m_midi_cc_22")
	pi = self->__commonstate.params + 12;
	pi->name = "midi_cc7";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_22;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 13 ("m_midi_cc_21")
	pi = self->__commonstate.params + 13;
	pi->name = "midi_cc8";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_21;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 36;
	pi->outputmax = 84;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 14 ("m_midi_cc_18")
	pi = self->__commonstate.params + 14;
	pi->name = "midi_cc9";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_18;
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
