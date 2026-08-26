#include "pulsar2.h"

namespace pulsar2 {

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
	Change __m_change_23;
	DCBlock __m_dcblock_39;
	DCBlock __m_dcblock_53;
	DCBlock __m_dcblock_22;
	PlusEquals __m_pluseq_24;
	SineCycle __m_cycle_21;
	SineData __sinedata;
	int vectorsize;
	int __exception;
	t_sample m_history_7;
	t_sample __m_latch_35;
	t_sample __m_latch_36;
	t_sample __m_latch_37;
	t_sample __m_latch_38;
	t_sample __m_latch_34;
	t_sample __m_latch_32;
	t_sample __m_latch_33;
	t_sample __m_latch_31;
	t_sample __m_carry_28;
	t_sample m_history_3;
	t_sample __m_carry_42;
	t_sample __m_latch_49;
	t_sample __m_latch_50;
	t_sample __m_latch_51;
	t_sample __m_count_40;
	t_sample __m_latch_48;
	t_sample __m_latch_46;
	t_sample __m_latch_47;
	t_sample __m_latch_45;
	t_sample m_history_6;
	t_sample __m_count_26;
	t_sample m_history_1;
	t_sample m_history_12;
	t_sample m_history_13;
	t_sample m_history_14;
	t_sample m_history_15;
	t_sample m_history_11;
	t_sample m_history_9;
	t_sample m_history_10;
	t_sample m_history_8;
	t_sample __m_latch_25;
	t_sample m_history_16;
	t_sample m_midi_cc_18;
	t_sample __m_latch_52;
	t_sample samplerate;
	t_sample m_history_17;
	t_sample m_history_4;
	t_sample m_midi_cc_20;
	t_sample m_history_5;
	t_sample m_midi_cc_19;
	t_sample m_history_2;
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
		m_history_6 = ((int)0);
		m_history_7 = ((int)0);
		m_history_8 = ((int)0);
		m_history_9 = ((int)0);
		m_history_10 = ((int)0);
		m_history_11 = ((int)0);
		m_history_12 = ((int)0);
		m_history_13 = ((int)0);
		m_history_14 = ((int)0);
		m_history_15 = ((int)0);
		m_history_16 = ((int)0);
		m_history_17 = ((int)0);
		m_midi_cc_18 = ((int)0);
		m_midi_cc_19 = ((int)0);
		m_midi_cc_20 = ((int)0);
		__m_cycle_21.reset(samplerate, 0);
		__m_dcblock_22.reset();
		__m_change_23.reset(0);
		__m_pluseq_24.reset(0);
		__m_latch_25 = 480;
		__m_count_26 = 0;
		__m_carry_28 = 0;
		__m_latch_31 = 1;
		__m_latch_32 = 1;
		__m_latch_33 = 1;
		__m_latch_34 = 1;
		__m_latch_35 = 1;
		__m_latch_36 = 1;
		__m_latch_37 = 1;
		__m_latch_38 = 1;
		__m_dcblock_39.reset();
		__m_count_40 = 0;
		__m_carry_42 = 0;
		__m_latch_45 = 1;
		__m_latch_46 = 1;
		__m_latch_47 = 1;
		__m_latch_48 = 1;
		__m_latch_49 = 1;
		__m_latch_50 = 1;
		__m_latch_51 = 1;
		__m_latch_52 = 1;
		__m_dcblock_53.reset();
		genlib_reset_complete(this);
		
	};
	// the signal processing routine;
	inline int perform(t_sample ** __ins, t_sample ** __outs, int __n) {
		vectorsize = __n;
		const t_sample * __in1 = __ins[0];
		t_sample * __out1 = __outs[0];
		if (__exception) {
			return __exception;
			
		} else if (( (__in1 == 0) || (__out1 == 0) )) {
			__exception = GENLIB_ERR_NULL_BUFFER;
			return __exception;
			
		};
		t_sample mul_10153 = (m_midi_cc_19 * m_midi_cc_18);
		t_sample mul_8765 = (m_midi_cc_20 * m_midi_cc_20);
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample mul_8764 = (mul_8765 * ((int)1000));
			t_sample max_8801 = ((mul_8764 < ((int)3)) ? ((int)3) : mul_8764);
			__m_cycle_21.freq(m_midi_cc_19);
			t_sample cycle_8498 = __m_cycle_21(__sinedata);
			t_sample cycleindex_8499 = __m_cycle_21.phase();
			t_sample dcblock_8809 = __m_dcblock_22(cycle_8498);
			int gt_8808 = (dcblock_8809 > ((int)0));
			int change_8816 = __m_change_23(gt_8808);
			int gt_8799 = (change_8816 > ((int)0));
			int plusequals_8805 = __m_pluseq_24.pre(((int)1), gt_8799, 0);
			__m_latch_25 = ((gt_8799 != 0) ? m_history_17 : __m_latch_25);
			t_sample latch_8803 = __m_latch_25;
			t_sample gen_8807 = latch_8803;
			t_sample history_8804_next_8806 = fixdenorm(plusequals_8805);
			t_sample mul_8798 = (gen_8807 * mul_10153);
			__m_count_26 = (((int)0) ? 0 : (fixdenorm(__m_count_26 + gt_8799)));
			int carry_27 = 0;
			if ((((int)0) != 0)) {
				__m_count_26 = 0;
				__m_carry_28 = 0;
				
			} else if (((((int)4) > 0) && (__m_count_26 >= ((int)4)))) {
				int wraps_29 = (__m_count_26 / ((int)4));
				__m_carry_28 = (__m_carry_28 + wraps_29);
				__m_count_26 = (__m_count_26 - (wraps_29 * ((int)4)));
				carry_27 = 1;
				
			};
			int counter_8794 = __m_count_26;
			int counter_8795 = carry_27;
			int counter_8796 = __m_carry_28;
			int add_8789 = (counter_8794 + ((int)1));
			int choice_30 = add_8789;
			int gate_8790 = (((choice_30 >= 1) && (choice_30 < 2)) ? gt_8799 : 0);
			int gate_8791 = (((choice_30 >= 2) && (choice_30 < 3)) ? gt_8799 : 0);
			int gate_8792 = (((choice_30 >= 3) && (choice_30 < 4)) ? gt_8799 : 0);
			int gate_8793 = ((choice_30 >= 4) ? gt_8799 : 0);
			t_sample add_8733 = (m_history_16 + ((int)1));
			int gt_8738 = (m_history_15 > ((int)1));
			int mul_8743 = (gate_8790 * gt_8738);
			t_sample noise_8732 = noise();
			t_sample abs_8731 = fabs(noise_8732);
			t_sample switch_8735 = (mul_8743 ? abs_8731 : add_8733);
			__m_latch_31 = ((mul_8743 != 0) ? mul_8798 : __m_latch_31);
			t_sample latch_8740 = __m_latch_31;
			t_sample div_8746 = safediv(switch_8735, latch_8740);
			t_sample min_8739 = ((((int)1) < div_8746) ? ((int)1) : div_8746);
			t_sample sub_10397 = (min_8739 - ((int)0));
			t_sample scale_10394 = ((safepow((sub_10397 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8737 = (scale_10394 * ((t_sample)1.5707963267949));
			t_sample cos_8736 = cos(mul_8737);
			__m_latch_32 = ((mul_8743 != 0) ? max_8801 : __m_latch_32);
			t_sample latch_8744 = __m_latch_32;
			t_sample fract_8760 = fract(latch_8744);
			int int_8758 = int(latch_8744);
			t_sample mul_8757 = (scale_10394 * int_8758);
			int eq_8752 = (mul_8757 == ((int)0));
			t_sample sin_8755 = sin(mul_8757);
			t_sample div_8754 = safediv(sin_8755, mul_8757);
			t_sample switch_8753 = (eq_8752 ? ((int)1) : div_8754);
			int add_8759 = (int_8758 + ((int)1));
			t_sample mul_8756 = (scale_10394 * add_8759);
			int eq_8748 = (mul_8756 == ((int)0));
			t_sample sin_8751 = sin(mul_8756);
			t_sample div_8750 = safediv(sin_8751, mul_8756);
			t_sample switch_8749 = (eq_8748 ? ((int)1) : div_8750);
			t_sample mix_10398 = (switch_8753 + (fract_8760 * (switch_8749 - switch_8753)));
			t_sample mul_8741 = (cos_8736 * mix_10398);
			t_sample history_8734_next_8761 = fixdenorm(switch_8735);
			t_sample history_8742_next_8762 = fixdenorm(div_8746);
			t_sample add_8700 = (m_history_14 + ((int)1));
			int gt_8705 = (m_history_13 > ((int)1));
			int mul_8710 = (gate_8791 * gt_8705);
			t_sample noise_8699 = noise();
			t_sample abs_8698 = fabs(noise_8699);
			t_sample switch_8702 = (mul_8710 ? abs_8698 : add_8700);
			__m_latch_33 = ((mul_8710 != 0) ? mul_8798 : __m_latch_33);
			t_sample latch_8707 = __m_latch_33;
			t_sample div_8713 = safediv(switch_8702, latch_8707);
			t_sample min_8706 = ((((int)1) < div_8713) ? ((int)1) : div_8713);
			t_sample sub_10402 = (min_8706 - ((int)0));
			t_sample scale_10399 = ((safepow((sub_10402 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8704 = (scale_10399 * ((t_sample)1.5707963267949));
			t_sample cos_8703 = cos(mul_8704);
			__m_latch_34 = ((mul_8710 != 0) ? max_8801 : __m_latch_34);
			t_sample latch_8711 = __m_latch_34;
			t_sample fract_8727 = fract(latch_8711);
			int int_8725 = int(latch_8711);
			t_sample mul_8724 = (scale_10399 * int_8725);
			int eq_8719 = (mul_8724 == ((int)0));
			t_sample sin_8722 = sin(mul_8724);
			t_sample div_8721 = safediv(sin_8722, mul_8724);
			t_sample switch_8720 = (eq_8719 ? ((int)1) : div_8721);
			int add_8726 = (int_8725 + ((int)1));
			t_sample mul_8723 = (scale_10399 * add_8726);
			int eq_8715 = (mul_8723 == ((int)0));
			t_sample sin_8718 = sin(mul_8723);
			t_sample div_8717 = safediv(sin_8718, mul_8723);
			t_sample switch_8716 = (eq_8715 ? ((int)1) : div_8717);
			t_sample mix_10403 = (switch_8720 + (fract_8727 * (switch_8716 - switch_8720)));
			t_sample mul_8708 = (cos_8703 * mix_10403);
			t_sample history_8701_next_8728 = fixdenorm(switch_8702);
			t_sample history_8709_next_8729 = fixdenorm(div_8713);
			t_sample add_8634 = (m_history_12 + ((int)1));
			int gt_8639 = (m_history_11 > ((int)1));
			int mul_8644 = (gate_8793 * gt_8639);
			t_sample noise_8633 = noise();
			t_sample abs_8632 = fabs(noise_8633);
			t_sample switch_8636 = (mul_8644 ? abs_8632 : add_8634);
			__m_latch_35 = ((mul_8644 != 0) ? mul_8798 : __m_latch_35);
			t_sample latch_8641 = __m_latch_35;
			t_sample div_8647 = safediv(switch_8636, latch_8641);
			t_sample min_8640 = ((((int)1) < div_8647) ? ((int)1) : div_8647);
			t_sample sub_10407 = (min_8640 - ((int)0));
			t_sample scale_10404 = ((safepow((sub_10407 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8638 = (scale_10404 * ((t_sample)1.5707963267949));
			t_sample cos_8637 = cos(mul_8638);
			__m_latch_36 = ((mul_8644 != 0) ? max_8801 : __m_latch_36);
			t_sample latch_8645 = __m_latch_36;
			t_sample fract_8661 = fract(latch_8645);
			int int_8659 = int(latch_8645);
			t_sample mul_8658 = (scale_10404 * int_8659);
			int eq_8653 = (mul_8658 == ((int)0));
			t_sample sin_8656 = sin(mul_8658);
			t_sample div_8655 = safediv(sin_8656, mul_8658);
			t_sample switch_8654 = (eq_8653 ? ((int)1) : div_8655);
			int add_8660 = (int_8659 + ((int)1));
			t_sample mul_8657 = (scale_10404 * add_8660);
			int eq_8649 = (mul_8657 == ((int)0));
			t_sample sin_8652 = sin(mul_8657);
			t_sample div_8651 = safediv(sin_8652, mul_8657);
			t_sample switch_8650 = (eq_8649 ? ((int)1) : div_8651);
			t_sample mix_10408 = (switch_8654 + (fract_8661 * (switch_8650 - switch_8654)));
			t_sample mul_8642 = (cos_8637 * mix_10408);
			t_sample history_8635_next_8662 = fixdenorm(switch_8636);
			t_sample history_8643_next_8663 = fixdenorm(div_8647);
			t_sample add_8667 = (m_history_10 + ((int)1));
			int gt_8672 = (m_history_9 > ((int)1));
			int mul_8677 = (gate_8792 * gt_8672);
			t_sample noise_8666 = noise();
			t_sample abs_8665 = fabs(noise_8666);
			t_sample switch_8669 = (mul_8677 ? abs_8665 : add_8667);
			__m_latch_37 = ((mul_8677 != 0) ? mul_8798 : __m_latch_37);
			t_sample latch_8674 = __m_latch_37;
			t_sample div_8680 = safediv(switch_8669, latch_8674);
			t_sample min_8673 = ((((int)1) < div_8680) ? ((int)1) : div_8680);
			t_sample sub_10412 = (min_8673 - ((int)0));
			t_sample scale_10409 = ((safepow((sub_10412 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8671 = (scale_10409 * ((t_sample)1.5707963267949));
			t_sample cos_8670 = cos(mul_8671);
			__m_latch_38 = ((mul_8677 != 0) ? max_8801 : __m_latch_38);
			t_sample latch_8678 = __m_latch_38;
			t_sample fract_8694 = fract(latch_8678);
			int int_8692 = int(latch_8678);
			t_sample mul_8691 = (scale_10409 * int_8692);
			int eq_8686 = (mul_8691 == ((int)0));
			t_sample sin_8689 = sin(mul_8691);
			t_sample div_8688 = safediv(sin_8689, mul_8691);
			t_sample switch_8687 = (eq_8686 ? ((int)1) : div_8688);
			int add_8693 = (int_8692 + ((int)1));
			t_sample mul_8690 = (scale_10409 * add_8693);
			int eq_8682 = (mul_8690 == ((int)0));
			t_sample sin_8685 = sin(mul_8690);
			t_sample div_8684 = safediv(sin_8685, mul_8690);
			t_sample switch_8683 = (eq_8682 ? ((int)1) : div_8684);
			t_sample mix_10413 = (switch_8687 + (fract_8694 * (switch_8683 - switch_8687)));
			t_sample mul_8675 = (cos_8670 * mix_10413);
			t_sample history_8668_next_8695 = fixdenorm(switch_8669);
			t_sample history_8676_next_8696 = fixdenorm(div_8680);
			t_sample mul_8815 = ((((mul_8675 + mul_8642) + mul_8708) + mul_8741) * ((int)1));
			t_sample dcblock_8788 = __m_dcblock_39(mul_8815);
			t_sample mul_8812 = (dcblock_8788 * ((t_sample)0.5));
			int lt_8797 = (change_8816 < ((int)0));
			__m_count_40 = (((int)0) ? 0 : (fixdenorm(__m_count_40 + lt_8797)));
			int carry_41 = 0;
			if ((((int)0) != 0)) {
				__m_count_40 = 0;
				__m_carry_42 = 0;
				
			} else if (((((int)4) > 0) && (__m_count_40 >= ((int)4)))) {
				int wraps_43 = (__m_count_40 / ((int)4));
				__m_carry_42 = (__m_carry_42 + wraps_43);
				__m_count_40 = (__m_count_40 - (wraps_43 * ((int)4)));
				carry_41 = 1;
				
			};
			int counter_8784 = __m_count_40;
			int counter_8785 = carry_41;
			int counter_8786 = __m_carry_42;
			int add_8779 = (counter_8784 + ((int)1));
			int choice_44 = add_8779;
			int gate_8780 = (((choice_44 >= 1) && (choice_44 < 2)) ? lt_8797 : 0);
			int gate_8781 = (((choice_44 >= 2) && (choice_44 < 3)) ? lt_8797 : 0);
			int gate_8782 = (((choice_44 >= 3) && (choice_44 < 4)) ? lt_8797 : 0);
			int gate_8783 = ((choice_44 >= 4) ? lt_8797 : 0);
			t_sample add_8502 = (m_history_8 + ((int)1));
			int gt_8507 = (m_history_7 > ((int)1));
			int mul_8512 = (gate_8783 * gt_8507);
			__m_latch_45 = ((mul_8512 != 0) ? mul_8798 : __m_latch_45);
			t_sample latch_8509 = __m_latch_45;
			t_sample noise_8501 = noise();
			t_sample abs_8500 = fabs(noise_8501);
			t_sample switch_8504 = (mul_8512 ? abs_8500 : add_8502);
			t_sample div_8515 = safediv(switch_8504, latch_8509);
			t_sample min_8508 = ((((int)1) < div_8515) ? ((int)1) : div_8515);
			t_sample sub_10417 = (min_8508 - ((int)0));
			t_sample scale_10414 = ((safepow((sub_10417 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8506 = (scale_10414 * ((t_sample)1.5707963267949));
			t_sample cos_8505 = cos(mul_8506);
			__m_latch_46 = ((mul_8512 != 0) ? max_8801 : __m_latch_46);
			t_sample latch_8513 = __m_latch_46;
			t_sample fract_8529 = fract(latch_8513);
			int int_8527 = int(latch_8513);
			t_sample mul_8526 = (scale_10414 * int_8527);
			int eq_8521 = (mul_8526 == ((int)0));
			t_sample sin_8524 = sin(mul_8526);
			t_sample div_8523 = safediv(sin_8524, mul_8526);
			t_sample switch_8522 = (eq_8521 ? ((int)1) : div_8523);
			int add_8528 = (int_8527 + ((int)1));
			t_sample mul_8525 = (scale_10414 * add_8528);
			int eq_8517 = (mul_8525 == ((int)0));
			t_sample sin_8520 = sin(mul_8525);
			t_sample div_8519 = safediv(sin_8520, mul_8525);
			t_sample switch_8518 = (eq_8517 ? ((int)1) : div_8519);
			t_sample mix_10418 = (switch_8522 + (fract_8529 * (switch_8518 - switch_8522)));
			t_sample mul_8510 = (cos_8505 * mix_10418);
			t_sample history_8503_next_8530 = fixdenorm(switch_8504);
			t_sample history_8511_next_8531 = fixdenorm(div_8515);
			t_sample add_8535 = (m_history_6 + ((int)1));
			int gt_8540 = (m_history_5 > ((int)1));
			int mul_8545 = (gate_8782 * gt_8540);
			__m_latch_47 = ((mul_8545 != 0) ? mul_8798 : __m_latch_47);
			t_sample latch_8542 = __m_latch_47;
			t_sample noise_8534 = noise();
			t_sample abs_8533 = fabs(noise_8534);
			t_sample switch_8537 = (mul_8545 ? abs_8533 : add_8535);
			t_sample div_8548 = safediv(switch_8537, latch_8542);
			t_sample min_8541 = ((((int)1) < div_8548) ? ((int)1) : div_8548);
			t_sample sub_10422 = (min_8541 - ((int)0));
			t_sample scale_10419 = ((safepow((sub_10422 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8539 = (scale_10419 * ((t_sample)1.5707963267949));
			t_sample cos_8538 = cos(mul_8539);
			__m_latch_48 = ((mul_8545 != 0) ? max_8801 : __m_latch_48);
			t_sample latch_8546 = __m_latch_48;
			t_sample fract_8562 = fract(latch_8546);
			int int_8560 = int(latch_8546);
			t_sample mul_8559 = (scale_10419 * int_8560);
			int eq_8554 = (mul_8559 == ((int)0));
			t_sample sin_8557 = sin(mul_8559);
			t_sample div_8556 = safediv(sin_8557, mul_8559);
			t_sample switch_8555 = (eq_8554 ? ((int)1) : div_8556);
			int add_8561 = (int_8560 + ((int)1));
			t_sample mul_8558 = (scale_10419 * add_8561);
			int eq_8550 = (mul_8558 == ((int)0));
			t_sample sin_8553 = sin(mul_8558);
			t_sample div_8552 = safediv(sin_8553, mul_8558);
			t_sample switch_8551 = (eq_8550 ? ((int)1) : div_8552);
			t_sample mix_10423 = (switch_8555 + (fract_8562 * (switch_8551 - switch_8555)));
			t_sample mul_8543 = (cos_8538 * mix_10423);
			t_sample history_8536_next_8563 = fixdenorm(switch_8537);
			t_sample history_8544_next_8564 = fixdenorm(div_8548);
			t_sample add_8568 = (m_history_4 + ((int)1));
			int gt_8573 = (m_history_3 > ((int)1));
			int mul_8578 = (gate_8781 * gt_8573);
			__m_latch_49 = ((mul_8578 != 0) ? mul_8798 : __m_latch_49);
			t_sample latch_8575 = __m_latch_49;
			t_sample noise_8567 = noise();
			t_sample abs_8566 = fabs(noise_8567);
			t_sample switch_8570 = (mul_8578 ? abs_8566 : add_8568);
			t_sample div_8581 = safediv(switch_8570, latch_8575);
			t_sample min_8574 = ((((int)1) < div_8581) ? ((int)1) : div_8581);
			t_sample sub_10427 = (min_8574 - ((int)0));
			t_sample scale_10424 = ((safepow((sub_10427 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8572 = (scale_10424 * ((t_sample)1.5707963267949));
			t_sample cos_8571 = cos(mul_8572);
			__m_latch_50 = ((mul_8578 != 0) ? max_8801 : __m_latch_50);
			t_sample latch_8579 = __m_latch_50;
			t_sample fract_8595 = fract(latch_8579);
			int int_8593 = int(latch_8579);
			t_sample mul_8592 = (scale_10424 * int_8593);
			int eq_8587 = (mul_8592 == ((int)0));
			t_sample sin_8590 = sin(mul_8592);
			t_sample div_8589 = safediv(sin_8590, mul_8592);
			t_sample switch_8588 = (eq_8587 ? ((int)1) : div_8589);
			int add_8594 = (int_8593 + ((int)1));
			t_sample mul_8591 = (scale_10424 * add_8594);
			int eq_8583 = (mul_8591 == ((int)0));
			t_sample sin_8586 = sin(mul_8591);
			t_sample div_8585 = safediv(sin_8586, mul_8591);
			t_sample switch_8584 = (eq_8583 ? ((int)1) : div_8585);
			t_sample mix_10428 = (switch_8588 + (fract_8595 * (switch_8584 - switch_8588)));
			t_sample mul_8576 = (cos_8571 * mix_10428);
			t_sample history_8569_next_8596 = fixdenorm(switch_8570);
			t_sample history_8577_next_8597 = fixdenorm(div_8581);
			t_sample add_8601 = (m_history_2 + ((int)1));
			int gt_8606 = (m_history_1 > ((int)1));
			int mul_8611 = (gate_8780 * gt_8606);
			__m_latch_51 = ((mul_8611 != 0) ? mul_8798 : __m_latch_51);
			t_sample latch_8608 = __m_latch_51;
			t_sample noise_8600 = noise();
			t_sample abs_8599 = fabs(noise_8600);
			t_sample switch_8603 = (mul_8611 ? abs_8599 : add_8601);
			t_sample div_8614 = safediv(switch_8603, latch_8608);
			t_sample min_8607 = ((((int)1) < div_8614) ? ((int)1) : div_8614);
			t_sample sub_10432 = (min_8607 - ((int)0));
			t_sample scale_10429 = ((safepow((sub_10432 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_8605 = (scale_10429 * ((t_sample)1.5707963267949));
			t_sample cos_8604 = cos(mul_8605);
			__m_latch_52 = ((mul_8611 != 0) ? max_8801 : __m_latch_52);
			t_sample latch_8612 = __m_latch_52;
			t_sample fract_8628 = fract(latch_8612);
			int int_8626 = int(latch_8612);
			t_sample mul_8625 = (scale_10429 * int_8626);
			int eq_8620 = (mul_8625 == ((int)0));
			t_sample sin_8623 = sin(mul_8625);
			t_sample div_8622 = safediv(sin_8623, mul_8625);
			t_sample switch_8621 = (eq_8620 ? ((int)1) : div_8622);
			int add_8627 = (int_8626 + ((int)1));
			t_sample mul_8624 = (scale_10429 * add_8627);
			int eq_8616 = (mul_8624 == ((int)0));
			t_sample sin_8619 = sin(mul_8624);
			t_sample div_8618 = safediv(sin_8619, mul_8624);
			t_sample switch_8617 = (eq_8616 ? ((int)1) : div_8618);
			t_sample mix_10433 = (switch_8621 + (fract_8628 * (switch_8617 - switch_8621)));
			t_sample mul_8609 = (cos_8604 * mix_10433);
			t_sample history_8602_next_8629 = fixdenorm(switch_8603);
			t_sample history_8610_next_8630 = fixdenorm(div_8614);
			t_sample mul_8787 = ((((mul_8609 + mul_8576) + mul_8543) + mul_8510) * (-1));
			t_sample dcblock_8813 = __m_dcblock_53(mul_8787);
			t_sample mul_8811 = (dcblock_8813 * ((t_sample)0.5));
			t_sample out1 = (mul_8811 + mul_8812);
			m_history_17 = history_8804_next_8806;
			m_history_16 = history_8734_next_8761;
			m_history_15 = history_8742_next_8762;
			m_history_14 = history_8701_next_8728;
			m_history_13 = history_8709_next_8729;
			m_history_12 = history_8635_next_8662;
			m_history_11 = history_8643_next_8663;
			m_history_10 = history_8668_next_8695;
			m_history_9 = history_8676_next_8696;
			m_history_8 = history_8503_next_8530;
			m_history_7 = history_8511_next_8531;
			m_history_6 = history_8536_next_8563;
			m_history_5 = history_8544_next_8564;
			m_history_4 = history_8569_next_8596;
			m_history_3 = history_8577_next_8597;
			m_history_2 = history_8602_next_8629;
			m_history_1 = history_8610_next_8630;
			// assign results to output buffer;
			(*(__out1++)) = out1;
			
		};
		return __exception;
		
	};
	inline void set_midi_cc2(t_param _value) {
		m_midi_cc_18 = (_value < 0.01 ? 0.01 : (_value > 2 ? 2 : _value));
	};
	inline void set_midi_cc1(t_param _value) {
		m_midi_cc_19 = (_value < 0 ? 0 : (_value > 2000 ? 2000 : _value));
	};
	inline void set_midi_cc3(t_param _value) {
		m_midi_cc_20 = (_value < 0 ? 0 : (_value > 200 ? 200 : _value));
	};
	
} State;


///
///	Configuration for the genlib API
///

/// Number of signal inputs and outputs

int gen_kernel_numins = 1;
int gen_kernel_numouts = 1;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 3; }

/// Assistive lables for the signal inputs and outputs

const char *gen_kernel_innames[] = { "in1" };
const char *gen_kernel_outnames[] = { "out1" };

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
		case 0: self->set_midi_cc1(value); break;
		case 1: self->set_midi_cc2(value); break;
		case 2: self->set_midi_cc3(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		case 0: *value = self->m_midi_cc_19; break;
		case 1: *value = self->m_midi_cc_18; break;
		case 2: *value = self->m_midi_cc_20; break;
		
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
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(3 * sizeof(ParamInfo));
	self->__commonstate.numparams = 3;
	// initialize parameter 0 ("m_midi_cc_19")
	pi = self->__commonstate.params + 0;
	pi->name = "midi_cc1";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_19;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 2000;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 1 ("m_midi_cc_18")
	pi = self->__commonstate.params + 1;
	pi->name = "midi_cc2";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_18;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.01;
	pi->outputmax = 2;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 2 ("m_midi_cc_20")
	pi = self->__commonstate.params + 2;
	pi->name = "midi_cc3";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_midi_cc_20;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 200;
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


} // pulsar2::
