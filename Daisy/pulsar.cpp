#include "pulsar.h"

namespace pulsar {

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
	Change __m_change_21;
	DCBlock __m_dcblock_50;
	DCBlock __m_dcblock_20;
	Phasor __m_phasor_18;
	PlusEquals __m_pluseq_22;
	SineCycle __m_cycle_19;
	SineData __sinedata;
	int __exception;
	int vectorsize;
	t_sample m_history_7;
	t_sample __m_latch_33;
	t_sample __m_latch_34;
	t_sample __m_latch_35;
	t_sample __m_latch_36;
	t_sample __m_latch_32;
	t_sample __m_latch_30;
	t_sample __m_latch_31;
	t_sample __m_latch_29;
	t_sample __m_carry_26;
	t_sample __m_count_37;
	t_sample __m_latch_42;
	t_sample __m_latch_47;
	t_sample __m_latch_48;
	t_sample __m_carry_39;
	t_sample __m_latch_46;
	t_sample __m_latch_44;
	t_sample __m_latch_45;
	t_sample __m_latch_43;
	t_sample m_history_6;
	t_sample __m_count_24;
	t_sample m_history_2;
	t_sample m_history_12;
	t_sample m_history_13;
	t_sample m_history_14;
	t_sample m_history_11;
	t_sample m_history_9;
	t_sample m_history_10;
	t_sample m_history_8;
	t_sample __m_latch_23;
	t_sample m_history_15;
	t_sample m_history_17;
	t_sample __m_latch_49;
	t_sample samplerate;
	t_sample m_history_16;
	t_sample m_history_3;
	t_sample m_history_5;
	t_sample m_history_4;
	t_sample samples_to_seconds;
	t_sample m_history_1;
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
		samples_to_seconds = (1 / samplerate);
		__m_phasor_18.reset(0);
		__m_cycle_19.reset(samplerate, 0);
		__m_dcblock_20.reset();
		__m_change_21.reset(0);
		__m_pluseq_22.reset(0);
		__m_latch_23 = 480;
		__m_count_24 = 0;
		__m_carry_26 = 0;
		__m_latch_29 = 1;
		__m_latch_30 = 1;
		__m_latch_31 = 1;
		__m_latch_32 = 1;
		__m_latch_33 = 1;
		__m_latch_34 = 1;
		__m_latch_35 = 1;
		__m_latch_36 = 1;
		__m_count_37 = 0;
		__m_carry_39 = 0;
		__m_latch_42 = 1;
		__m_latch_43 = 1;
		__m_latch_44 = 1;
		__m_latch_45 = 1;
		__m_latch_46 = 1;
		__m_latch_47 = 1;
		__m_latch_48 = 1;
		__m_latch_49 = 1;
		__m_dcblock_50.reset();
		genlib_reset_complete(this);
		
	};
	// the signal processing routine;
	inline int perform(t_sample ** __ins, t_sample ** __outs, int __n) {
		vectorsize = __n;
		const t_sample * __in1 = __ins[0];
		const t_sample * __in2 = __ins[1];
		t_sample * __out1 = __outs[0];
		if (__exception) {
			return __exception;
			
		} else if (( (__in1 == 0) || (__in2 == 0) || (__out1 == 0) )) {
			__exception = GENLIB_ERR_NULL_BUFFER;
			return __exception;
			
		};
		samples_to_seconds = (1 / samplerate);
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			const t_sample in2 = (*(__in2++));
			if ((((int)0) != 0)) {
				__m_phasor_18.phase = 0;
				
			};
			t_sample phasor_12118 = __m_phasor_18(in2, samples_to_seconds);
			t_sample rsub_13321 = (((int)1) - phasor_12118);
			t_sample pow_13160 = safepow(rsub_13321, ((int)2));
			t_sample mul_13963 = (in2 * ((t_sample)0.1));
			t_sample clamp_11264 = ((mul_13963 <= ((t_sample)0.001)) ? ((t_sample)0.001) : ((mul_13963 >= ((int)4)) ? ((int)4) : mul_13963));
			t_sample mul_15578 = (mul_13963 * ((t_sample)0.5));
			t_sample clamp_15659 = ((mul_15578 <= ((t_sample)0.01)) ? ((t_sample)0.01) : ((mul_15578 >= ((t_sample)0.5)) ? ((t_sample)0.5) : mul_15578));
			t_sample mul_10699 = (clamp_15659 * clamp_15659);
			t_sample mul_10698 = (mul_10699 * ((int)1000));
			t_sample max_10734 = ((mul_10698 < ((int)3)) ? ((int)3) : mul_10698);
			__m_cycle_19.freq(in1);
			t_sample cycle_11133 = __m_cycle_19(__sinedata);
			t_sample cycleindex_11134 = __m_cycle_19.phase();
			t_sample dcblock_10741 = __m_dcblock_20(cycle_11133);
			int gt_10740 = (dcblock_10741 > ((int)0));
			int change_10747 = __m_change_21(gt_10740);
			int gt_10733 = (change_10747 > ((int)0));
			int plusequals_10737 = __m_pluseq_22.pre(((int)1), gt_10733, 0);
			__m_latch_23 = ((gt_10733 != 0) ? m_history_17 : __m_latch_23);
			t_sample latch_10735 = __m_latch_23;
			t_sample gen_10739 = latch_10735;
			t_sample history_10736_next_10738 = fixdenorm(plusequals_10737);
			t_sample mul_10732 = (gen_10739 * clamp_11264);
			__m_count_24 = (((int)0) ? 0 : (fixdenorm(__m_count_24 + gt_10733)));
			int carry_25 = 0;
			if ((((int)0) != 0)) {
				__m_count_24 = 0;
				__m_carry_26 = 0;
				
			} else if (((((int)4) > 0) && (__m_count_24 >= ((int)4)))) {
				int wraps_27 = (__m_count_24 / ((int)4));
				__m_carry_26 = (__m_carry_26 + wraps_27);
				__m_count_24 = (__m_count_24 - (wraps_27 * ((int)4)));
				carry_25 = 1;
				
			};
			int counter_10728 = __m_count_24;
			int counter_10729 = carry_25;
			int counter_10730 = __m_carry_26;
			int add_10723 = (counter_10728 + ((int)1));
			int choice_28 = add_10723;
			int gate_10724 = (((choice_28 >= 1) && (choice_28 < 2)) ? gt_10733 : 0);
			int gate_10725 = (((choice_28 >= 2) && (choice_28 < 3)) ? gt_10733 : 0);
			int gate_10726 = (((choice_28 >= 3) && (choice_28 < 4)) ? gt_10733 : 0);
			int gate_10727 = ((choice_28 >= 4) ? gt_10733 : 0);
			t_sample add_10667 = (m_history_16 + ((int)1));
			int gt_10672 = (m_history_15 > ((int)1));
			int mul_10677 = (gate_10724 * gt_10672);
			__m_latch_29 = ((mul_10677 != 0) ? mul_10732 : __m_latch_29);
			t_sample latch_10674 = __m_latch_29;
			t_sample noise_10666 = noise();
			t_sample abs_10665 = fabs(noise_10666);
			t_sample switch_10669 = (mul_10677 ? abs_10665 : add_10667);
			t_sample div_10680 = safediv(switch_10669, latch_10674);
			t_sample min_10673 = ((((int)1) < div_10680) ? ((int)1) : div_10680);
			t_sample sub_15783 = (min_10673 - ((int)0));
			t_sample scale_15780 = ((safepow((sub_15783 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10671 = (scale_15780 * ((t_sample)1.5707963267949));
			t_sample cos_10670 = cos(mul_10671);
			__m_latch_30 = ((mul_10677 != 0) ? max_10734 : __m_latch_30);
			t_sample latch_10678 = __m_latch_30;
			t_sample fract_10694 = fract(latch_10678);
			int int_10692 = int(latch_10678);
			t_sample mul_10691 = (scale_15780 * int_10692);
			int eq_10686 = (mul_10691 == ((int)0));
			t_sample sin_10689 = sin(mul_10691);
			t_sample div_10688 = safediv(sin_10689, mul_10691);
			t_sample switch_10687 = (eq_10686 ? ((int)1) : div_10688);
			int add_10693 = (int_10692 + ((int)1));
			t_sample mul_10690 = (scale_15780 * add_10693);
			int eq_10682 = (mul_10690 == ((int)0));
			t_sample sin_10685 = sin(mul_10690);
			t_sample div_10684 = safediv(sin_10685, mul_10690);
			t_sample switch_10683 = (eq_10682 ? ((int)1) : div_10684);
			t_sample mix_15784 = (switch_10687 + (fract_10694 * (switch_10683 - switch_10687)));
			t_sample mul_10675 = (cos_10670 * mix_15784);
			t_sample history_10668_next_10695 = fixdenorm(switch_10669);
			t_sample history_10676_next_10696 = fixdenorm(div_10680);
			t_sample add_10634 = (m_history_14 + ((int)1));
			int gt_10639 = (m_history_13 > ((int)1));
			int mul_10644 = (gate_10725 * gt_10639);
			t_sample noise_10633 = noise();
			t_sample abs_10632 = fabs(noise_10633);
			t_sample switch_10636 = (mul_10644 ? abs_10632 : add_10634);
			__m_latch_31 = ((mul_10644 != 0) ? mul_10732 : __m_latch_31);
			t_sample latch_10641 = __m_latch_31;
			t_sample div_10647 = safediv(switch_10636, latch_10641);
			t_sample min_10640 = ((((int)1) < div_10647) ? ((int)1) : div_10647);
			t_sample sub_15788 = (min_10640 - ((int)0));
			t_sample scale_15785 = ((safepow((sub_15788 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10638 = (scale_15785 * ((t_sample)1.5707963267949));
			t_sample cos_10637 = cos(mul_10638);
			__m_latch_32 = ((mul_10644 != 0) ? max_10734 : __m_latch_32);
			t_sample latch_10645 = __m_latch_32;
			t_sample fract_10661 = fract(latch_10645);
			int int_10659 = int(latch_10645);
			t_sample mul_10658 = (scale_15785 * int_10659);
			int eq_10653 = (mul_10658 == ((int)0));
			t_sample sin_10656 = sin(mul_10658);
			t_sample div_10655 = safediv(sin_10656, mul_10658);
			t_sample switch_10654 = (eq_10653 ? ((int)1) : div_10655);
			int add_10660 = (int_10659 + ((int)1));
			t_sample mul_10657 = (scale_15785 * add_10660);
			int eq_10649 = (mul_10657 == ((int)0));
			t_sample sin_10652 = sin(mul_10657);
			t_sample div_10651 = safediv(sin_10652, mul_10657);
			t_sample switch_10650 = (eq_10649 ? ((int)1) : div_10651);
			t_sample mix_15789 = (switch_10654 + (fract_10661 * (switch_10650 - switch_10654)));
			t_sample mul_10642 = (cos_10637 * mix_15789);
			t_sample history_10635_next_10662 = fixdenorm(switch_10636);
			t_sample history_10643_next_10663 = fixdenorm(div_10647);
			t_sample add_10568 = (m_history_12 + ((int)1));
			int gt_10573 = (m_history_11 > ((int)1));
			int mul_10578 = (gate_10727 * gt_10573);
			t_sample noise_10567 = noise();
			t_sample abs_10566 = fabs(noise_10567);
			t_sample switch_10570 = (mul_10578 ? abs_10566 : add_10568);
			__m_latch_33 = ((mul_10578 != 0) ? mul_10732 : __m_latch_33);
			t_sample latch_10575 = __m_latch_33;
			t_sample div_10581 = safediv(switch_10570, latch_10575);
			t_sample min_10574 = ((((int)1) < div_10581) ? ((int)1) : div_10581);
			t_sample sub_15793 = (min_10574 - ((int)0));
			t_sample scale_15790 = ((safepow((sub_15793 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10572 = (scale_15790 * ((t_sample)1.5707963267949));
			t_sample cos_10571 = cos(mul_10572);
			__m_latch_34 = ((mul_10578 != 0) ? max_10734 : __m_latch_34);
			t_sample latch_10579 = __m_latch_34;
			t_sample fract_10595 = fract(latch_10579);
			int int_10593 = int(latch_10579);
			t_sample mul_10592 = (scale_15790 * int_10593);
			int eq_10587 = (mul_10592 == ((int)0));
			t_sample sin_10590 = sin(mul_10592);
			t_sample div_10589 = safediv(sin_10590, mul_10592);
			t_sample switch_10588 = (eq_10587 ? ((int)1) : div_10589);
			int add_10594 = (int_10593 + ((int)1));
			t_sample mul_10591 = (scale_15790 * add_10594);
			int eq_10583 = (mul_10591 == ((int)0));
			t_sample sin_10586 = sin(mul_10591);
			t_sample div_10585 = safediv(sin_10586, mul_10591);
			t_sample switch_10584 = (eq_10583 ? ((int)1) : div_10585);
			t_sample mix_15794 = (switch_10588 + (fract_10595 * (switch_10584 - switch_10588)));
			t_sample mul_10576 = (cos_10571 * mix_15794);
			t_sample history_10569_next_10596 = fixdenorm(switch_10570);
			t_sample history_10577_next_10597 = fixdenorm(div_10581);
			t_sample add_10601 = (m_history_10 + ((int)1));
			int gt_10606 = (m_history_9 > ((int)1));
			int mul_10611 = (gate_10726 * gt_10606);
			__m_latch_35 = ((mul_10611 != 0) ? mul_10732 : __m_latch_35);
			t_sample latch_10608 = __m_latch_35;
			t_sample noise_10600 = noise();
			t_sample abs_10599 = fabs(noise_10600);
			t_sample switch_10603 = (mul_10611 ? abs_10599 : add_10601);
			t_sample div_10614 = safediv(switch_10603, latch_10608);
			t_sample min_10607 = ((((int)1) < div_10614) ? ((int)1) : div_10614);
			t_sample sub_15798 = (min_10607 - ((int)0));
			t_sample scale_15795 = ((safepow((sub_15798 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10605 = (scale_15795 * ((t_sample)1.5707963267949));
			t_sample cos_10604 = cos(mul_10605);
			__m_latch_36 = ((mul_10611 != 0) ? max_10734 : __m_latch_36);
			t_sample latch_10612 = __m_latch_36;
			t_sample fract_10628 = fract(latch_10612);
			int int_10626 = int(latch_10612);
			t_sample mul_10625 = (scale_15795 * int_10626);
			int eq_10620 = (mul_10625 == ((int)0));
			t_sample sin_10623 = sin(mul_10625);
			t_sample div_10622 = safediv(sin_10623, mul_10625);
			t_sample switch_10621 = (eq_10620 ? ((int)1) : div_10622);
			int add_10627 = (int_10626 + ((int)1));
			t_sample mul_10624 = (scale_15795 * add_10627);
			int eq_10616 = (mul_10624 == ((int)0));
			t_sample sin_10619 = sin(mul_10624);
			t_sample div_10618 = safediv(sin_10619, mul_10624);
			t_sample switch_10617 = (eq_10616 ? ((int)1) : div_10618);
			t_sample mix_15799 = (switch_10621 + (fract_10628 * (switch_10617 - switch_10621)));
			t_sample mul_10609 = (cos_10604 * mix_15799);
			t_sample history_10602_next_10629 = fixdenorm(switch_10603);
			t_sample history_10610_next_10630 = fixdenorm(div_10614);
			t_sample mul_10746 = ((((mul_10609 + mul_10576) + mul_10642) + mul_10675) * ((int)1));
			int lt_10731 = (change_10747 < ((int)0));
			__m_count_37 = (((int)0) ? 0 : (fixdenorm(__m_count_37 + lt_10731)));
			int carry_38 = 0;
			if ((((int)0) != 0)) {
				__m_count_37 = 0;
				__m_carry_39 = 0;
				
			} else if (((((int)4) > 0) && (__m_count_37 >= ((int)4)))) {
				int wraps_40 = (__m_count_37 / ((int)4));
				__m_carry_39 = (__m_carry_39 + wraps_40);
				__m_count_37 = (__m_count_37 - (wraps_40 * ((int)4)));
				carry_38 = 1;
				
			};
			int counter_10718 = __m_count_37;
			int counter_10719 = carry_38;
			int counter_10720 = __m_carry_39;
			int add_10713 = (counter_10718 + ((int)1));
			int choice_41 = add_10713;
			int gate_10714 = (((choice_41 >= 1) && (choice_41 < 2)) ? lt_10731 : 0);
			int gate_10715 = (((choice_41 >= 2) && (choice_41 < 3)) ? lt_10731 : 0);
			int gate_10716 = (((choice_41 >= 3) && (choice_41 < 4)) ? lt_10731 : 0);
			int gate_10717 = ((choice_41 >= 4) ? lt_10731 : 0);
			t_sample add_10436 = (m_history_8 + ((int)1));
			int gt_10441 = (m_history_7 > ((int)1));
			int mul_10446 = (gate_10717 * gt_10441);
			t_sample noise_10435 = noise();
			t_sample abs_10434 = fabs(noise_10435);
			t_sample switch_10438 = (mul_10446 ? abs_10434 : add_10436);
			__m_latch_42 = ((mul_10446 != 0) ? mul_10732 : __m_latch_42);
			t_sample latch_10443 = __m_latch_42;
			t_sample div_10449 = safediv(switch_10438, latch_10443);
			t_sample min_10442 = ((((int)1) < div_10449) ? ((int)1) : div_10449);
			t_sample sub_15803 = (min_10442 - ((int)0));
			t_sample scale_15800 = ((safepow((sub_15803 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10440 = (scale_15800 * ((t_sample)1.5707963267949));
			t_sample cos_10439 = cos(mul_10440);
			__m_latch_43 = ((mul_10446 != 0) ? max_10734 : __m_latch_43);
			t_sample latch_10447 = __m_latch_43;
			t_sample fract_10463 = fract(latch_10447);
			int int_10461 = int(latch_10447);
			t_sample mul_10460 = (scale_15800 * int_10461);
			int eq_10455 = (mul_10460 == ((int)0));
			t_sample sin_10458 = sin(mul_10460);
			t_sample div_10457 = safediv(sin_10458, mul_10460);
			t_sample switch_10456 = (eq_10455 ? ((int)1) : div_10457);
			int add_10462 = (int_10461 + ((int)1));
			t_sample mul_10459 = (scale_15800 * add_10462);
			int eq_10451 = (mul_10459 == ((int)0));
			t_sample sin_10454 = sin(mul_10459);
			t_sample div_10453 = safediv(sin_10454, mul_10459);
			t_sample switch_10452 = (eq_10451 ? ((int)1) : div_10453);
			t_sample mix_15804 = (switch_10456 + (fract_10463 * (switch_10452 - switch_10456)));
			t_sample mul_10444 = (cos_10439 * mix_15804);
			t_sample history_10437_next_10464 = fixdenorm(switch_10438);
			t_sample history_10445_next_10465 = fixdenorm(div_10449);
			t_sample add_10469 = (m_history_6 + ((int)1));
			int gt_10474 = (m_history_5 > ((int)1));
			int mul_10479 = (gate_10716 * gt_10474);
			t_sample noise_10468 = noise();
			t_sample abs_10467 = fabs(noise_10468);
			t_sample switch_10471 = (mul_10479 ? abs_10467 : add_10469);
			__m_latch_44 = ((mul_10479 != 0) ? mul_10732 : __m_latch_44);
			t_sample latch_10476 = __m_latch_44;
			t_sample div_10482 = safediv(switch_10471, latch_10476);
			t_sample min_10475 = ((((int)1) < div_10482) ? ((int)1) : div_10482);
			t_sample sub_15808 = (min_10475 - ((int)0));
			t_sample scale_15805 = ((safepow((sub_15808 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10473 = (scale_15805 * ((t_sample)1.5707963267949));
			t_sample cos_10472 = cos(mul_10473);
			__m_latch_45 = ((mul_10479 != 0) ? max_10734 : __m_latch_45);
			t_sample latch_10480 = __m_latch_45;
			t_sample fract_10496 = fract(latch_10480);
			int int_10494 = int(latch_10480);
			t_sample mul_10493 = (scale_15805 * int_10494);
			int eq_10488 = (mul_10493 == ((int)0));
			t_sample sin_10491 = sin(mul_10493);
			t_sample div_10490 = safediv(sin_10491, mul_10493);
			t_sample switch_10489 = (eq_10488 ? ((int)1) : div_10490);
			int add_10495 = (int_10494 + ((int)1));
			t_sample mul_10492 = (scale_15805 * add_10495);
			int eq_10484 = (mul_10492 == ((int)0));
			t_sample sin_10487 = sin(mul_10492);
			t_sample div_10486 = safediv(sin_10487, mul_10492);
			t_sample switch_10485 = (eq_10484 ? ((int)1) : div_10486);
			t_sample mix_15809 = (switch_10489 + (fract_10496 * (switch_10485 - switch_10489)));
			t_sample mul_10477 = (cos_10472 * mix_15809);
			t_sample history_10470_next_10497 = fixdenorm(switch_10471);
			t_sample history_10478_next_10498 = fixdenorm(div_10482);
			t_sample add_10502 = (m_history_4 + ((int)1));
			int gt_10507 = (m_history_3 > ((int)1));
			int mul_10512 = (gate_10715 * gt_10507);
			t_sample noise_10501 = noise();
			t_sample abs_10500 = fabs(noise_10501);
			t_sample switch_10504 = (mul_10512 ? abs_10500 : add_10502);
			__m_latch_46 = ((mul_10512 != 0) ? mul_10732 : __m_latch_46);
			t_sample latch_10509 = __m_latch_46;
			t_sample div_10515 = safediv(switch_10504, latch_10509);
			t_sample min_10508 = ((((int)1) < div_10515) ? ((int)1) : div_10515);
			t_sample sub_15813 = (min_10508 - ((int)0));
			t_sample scale_15810 = ((safepow((sub_15813 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10506 = (scale_15810 * ((t_sample)1.5707963267949));
			t_sample cos_10505 = cos(mul_10506);
			__m_latch_47 = ((mul_10512 != 0) ? max_10734 : __m_latch_47);
			t_sample latch_10513 = __m_latch_47;
			t_sample fract_10529 = fract(latch_10513);
			int int_10527 = int(latch_10513);
			t_sample mul_10526 = (scale_15810 * int_10527);
			int eq_10521 = (mul_10526 == ((int)0));
			t_sample sin_10524 = sin(mul_10526);
			t_sample div_10523 = safediv(sin_10524, mul_10526);
			t_sample switch_10522 = (eq_10521 ? ((int)1) : div_10523);
			int add_10528 = (int_10527 + ((int)1));
			t_sample mul_10525 = (scale_15810 * add_10528);
			int eq_10517 = (mul_10525 == ((int)0));
			t_sample sin_10520 = sin(mul_10525);
			t_sample div_10519 = safediv(sin_10520, mul_10525);
			t_sample switch_10518 = (eq_10517 ? ((int)1) : div_10519);
			t_sample mix_15814 = (switch_10522 + (fract_10529 * (switch_10518 - switch_10522)));
			t_sample mul_10510 = (cos_10505 * mix_15814);
			t_sample history_10503_next_10530 = fixdenorm(switch_10504);
			t_sample history_10511_next_10531 = fixdenorm(div_10515);
			t_sample add_10535 = (m_history_2 + ((int)1));
			int gt_10540 = (m_history_1 > ((int)1));
			int mul_10545 = (gate_10714 * gt_10540);
			__m_latch_48 = ((mul_10545 != 0) ? mul_10732 : __m_latch_48);
			t_sample latch_10542 = __m_latch_48;
			t_sample noise_10534 = noise();
			t_sample abs_10533 = fabs(noise_10534);
			t_sample switch_10537 = (mul_10545 ? abs_10533 : add_10535);
			t_sample div_10548 = safediv(switch_10537, latch_10542);
			t_sample min_10541 = ((((int)1) < div_10548) ? ((int)1) : div_10548);
			t_sample sub_15818 = (min_10541 - ((int)0));
			t_sample scale_15815 = ((safepow((sub_15818 * ((t_sample)1)), ((int)1)) * ((int)2)) + (-1));
			t_sample mul_10539 = (scale_15815 * ((t_sample)1.5707963267949));
			t_sample cos_10538 = cos(mul_10539);
			__m_latch_49 = ((mul_10545 != 0) ? max_10734 : __m_latch_49);
			t_sample latch_10546 = __m_latch_49;
			t_sample fract_10562 = fract(latch_10546);
			int int_10560 = int(latch_10546);
			t_sample mul_10559 = (scale_15815 * int_10560);
			int eq_10554 = (mul_10559 == ((int)0));
			t_sample sin_10557 = sin(mul_10559);
			t_sample div_10556 = safediv(sin_10557, mul_10559);
			t_sample switch_10555 = (eq_10554 ? ((int)1) : div_10556);
			int add_10561 = (int_10560 + ((int)1));
			t_sample mul_10558 = (scale_15815 * add_10561);
			int eq_10550 = (mul_10558 == ((int)0));
			t_sample sin_10553 = sin(mul_10558);
			t_sample div_10552 = safediv(sin_10553, mul_10558);
			t_sample switch_10551 = (eq_10550 ? ((int)1) : div_10552);
			t_sample mix_15819 = (switch_10555 + (fract_10562 * (switch_10551 - switch_10555)));
			t_sample mul_10543 = (cos_10538 * mix_15819);
			t_sample history_10536_next_10563 = fixdenorm(switch_10537);
			t_sample history_10544_next_10564 = fixdenorm(div_10548);
			t_sample mul_10721 = ((((mul_10543 + mul_10510) + mul_10477) + mul_10444) * (-1));
			t_sample dcblock_10722 = __m_dcblock_50((mul_10721 + mul_10746));
			t_sample mul_12439 = (dcblock_10722 * pow_13160);
			t_sample out1 = mul_12439;
			m_history_17 = history_10736_next_10738;
			m_history_16 = history_10668_next_10695;
			m_history_15 = history_10676_next_10696;
			m_history_14 = history_10635_next_10662;
			m_history_13 = history_10643_next_10663;
			m_history_12 = history_10569_next_10596;
			m_history_11 = history_10577_next_10597;
			m_history_10 = history_10602_next_10629;
			m_history_9 = history_10610_next_10630;
			m_history_8 = history_10437_next_10464;
			m_history_7 = history_10445_next_10465;
			m_history_6 = history_10470_next_10497;
			m_history_5 = history_10478_next_10498;
			m_history_4 = history_10503_next_10530;
			m_history_3 = history_10511_next_10531;
			m_history_2 = history_10536_next_10563;
			m_history_1 = history_10544_next_10564;
			// assign results to output buffer;
			(*(__out1++)) = out1;
			
		};
		return __exception;
		
	};
	
} State;


///
///	Configuration for the genlib API
///

/// Number of signal inputs and outputs

int gen_kernel_numins = 2;
int gen_kernel_numouts = 1;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 0; }

/// Assistive lables for the signal inputs and outputs

const char *gen_kernel_innames[] = { "freqency", "mod" };
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
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		
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
	self->__commonstate.params = 0;
	self->__commonstate.numparams = 0;
	
	return self;
}

/// Release all resources and memory used by a State object:

void destroy(CommonState *cself) {
	State *self = (State *)cself;
	
	delete self;
}


} // pulsar::
