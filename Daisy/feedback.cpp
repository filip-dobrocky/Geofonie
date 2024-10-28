#include "feedback.h"

namespace feedback {

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
	Delay m_delay_12;
	Delay m_delay_10;
	Delay m_delay_9;
	Delay m_delay_8;
	Delay m_delay_11;
	Delay m_delay_14;
	Delay m_delay_13;
	Delay m_delay_15;
	Delay m_delay_16;
	Delay m_delay_7;
	Delay m_delay_19;
	Delay m_delay_18;
	Delay m_delay_3;
	Delay m_delay_17;
	Delay m_delay_20;
	Phasor __m_phasor_21;
	Sah __m_sah_22;
	SineCycle __m_cycle_23;
	SineCycle __m_cycle_24;
	SineData __sinedata;
	Train __m_train_25;
	int __exception;
	int vectorsize;
	t_sample samplerate;
	t_sample m_history_6;
	t_sample m_history_1;
	t_sample samples_to_seconds;
	t_sample m_history_2;
	t_sample m_history_4;
	t_sample m_history_5;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_history_1 = ((int)0);
		m_history_2 = ((int)0);
		m_delay_3.reset("m_delay_3", samplerate);
		m_history_4 = ((int)0);
		m_history_5 = ((int)0);
		m_history_6 = ((int)0);
		m_delay_7.reset("m_delay_7", ((int)924));
		m_delay_8.reset("m_delay_8", ((int)688));
		m_delay_9.reset("m_delay_9", ((int)3163));
		m_delay_10.reset("m_delay_10", ((int)4217));
		m_delay_11.reset("m_delay_11", ((int)4453));
		m_delay_12.reset("m_delay_12", ((int)1800));
		m_delay_13.reset("m_delay_13", ((int)3720));
		m_delay_14.reset("m_delay_14", ((int)2656));
		m_delay_15.reset("m_delay_15", ((int)142));
		m_delay_16.reset("m_delay_16", ((int)107));
		m_delay_17.reset("m_delay_17", ((int)379));
		m_delay_18.reset("m_delay_18", ((int)277));
		m_delay_19.reset("m_delay_19", (samplerate * 0.1));
		m_delay_20.reset("m_delay_20", (samplerate * 0.1));
		samples_to_seconds = (1 / samplerate);
		__m_phasor_21.reset(0);
		__m_sah_22.reset(0);
		__m_cycle_23.reset(samplerate, 0);
		__m_cycle_24.reset(samplerate, 0);
		__m_train_25.reset(0);
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
		samples_to_seconds = (1 / samplerate);
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			t_sample noise_1849 = noise();
			t_sample add_8038 = (m_history_2 + ((t_sample)0.2));
			t_sample mul_15071 = (add_8038 * ((int)100));
			if ((((int)0) != 0)) {
				__m_phasor_21.phase = 0;
				
			};
			t_sample phasor_6279 = __m_phasor_21(mul_15071, samples_to_seconds);
			t_sample sah_5329 = __m_sah_22(noise_1849, phasor_6279, ((t_sample)0.5));
			t_sample mstosamps_106 = (((int)10) * (samplerate * 0.001));
			t_sample tap_135 = m_delay_20.read_linear(mstosamps_106);
			t_sample mix_16855 = (m_history_1 + (((t_sample)0.8) * (tap_135 - m_history_1)));
			t_sample mix_133 = mix_16855;
			t_sample mul_4846 = (mix_133 * ((t_sample)0.4));
			t_sample mstosamps_10513 = (((int)10) * (samplerate * 0.001));
			t_sample tap_10544 = m_delay_19.read_linear(mstosamps_10513);
			t_sample mix_16856 = (tap_10544 + (((t_sample)0.5) * (m_history_4 - tap_10544)));
			t_sample mix_10542 = mix_16856;
			t_sample tap_10519 = m_delay_18.read_step(((int)277));
			t_sample mul_10517 = (tap_10519 * ((t_sample)0.625));
			t_sample tap_10525 = m_delay_17.read_step(((int)379));
			t_sample mul_10523 = (tap_10525 * ((t_sample)0.625));
			t_sample tap_10531 = m_delay_16.read_step(((int)107));
			t_sample mul_10529 = (tap_10531 * ((t_sample)0.75));
			t_sample tap_10537 = m_delay_15.read_step(((int)142));
			t_sample mul_10535 = (tap_10537 * ((t_sample)0.75));
			t_sample sub_10533 = (mix_10542 - mul_10535);
			t_sample mul_10534 = (sub_10533 * ((t_sample)0.75));
			t_sample add_10532 = (mul_10534 + tap_10537);
			t_sample sub_10527 = (add_10532 - mul_10529);
			t_sample mul_10528 = (sub_10527 * ((t_sample)0.75));
			t_sample add_10526 = (mul_10528 + tap_10531);
			t_sample sub_10521 = (add_10526 - mul_10523);
			t_sample mul_10522 = (sub_10521 * ((t_sample)0.625));
			t_sample add_10520 = (mul_10522 + tap_10525);
			t_sample sub_10515 = (add_10520 - mul_10517);
			t_sample mul_10516 = (sub_10515 * ((t_sample)0.625));
			t_sample add_10514 = (mul_10516 + tap_10519);
			t_sample tap_10454 = m_delay_14.read_step(((int)2656));
			t_sample tap_10456 = m_delay_14.read_step(((int)1913));
			t_sample tap_10474 = m_delay_13.read_step(((int)3720));
			t_sample tap_10475 = m_delay_13.read_step(((int)1066));
			t_sample tap_10482 = m_delay_12.read_step(((int)1800));
			t_sample tap_10483 = m_delay_12.read_step(((int)187));
			t_sample tap_10489 = m_delay_11.read_step(((int)4453));
			t_sample tap_10492 = m_delay_11.read_step(((int)1990));
			t_sample gen_10508 = (((tap_10492 + tap_10483) + tap_10475) + tap_10456);
			t_sample mix_16857 = (tap_10489 + (((t_sample)0.5) * (m_history_6 - tap_10489)));
			t_sample mix_10487 = mix_16857;
			t_sample tap_10458 = m_delay_10.read_step(((int)4217));
			t_sample tap_10459 = m_delay_10.read_step(((int)266));
			t_sample tap_10460 = m_delay_10.read_step(((int)2974));
			t_sample mix_16858 = (tap_10458 + (((t_sample)0.5) * (m_history_5 - tap_10458)));
			t_sample mix_10443 = mix_16858;
			t_sample tap_10446 = m_delay_9.read_step(((int)3163));
			t_sample tap_10448 = m_delay_9.read_step(((int)1996));
			t_sample gen_10507 = ((tap_10448 + tap_10460) + tap_10459);
			t_sample mul_10441 = (mix_10443 * ((t_sample)0.8));
			t_sample mul_10485 = (mix_10487 * ((t_sample)0.8));
			t_sample mul_10450 = (tap_10454 * ((t_sample)0.5));
			t_sample sub_10451 = (mul_10441 - mul_10450);
			t_sample mul_10478 = (tap_10482 * ((t_sample)0.5));
			t_sample sub_10479 = (mul_10485 - mul_10478);
			t_sample mul_10477 = (sub_10479 * ((t_sample)0.5));
			t_sample add_10480 = (mul_10477 + tap_10482);
			t_sample mul_10449 = (sub_10451 * ((t_sample)0.5));
			t_sample add_10452 = (mul_10449 + tap_10454);
			t_sample mul_10444 = (tap_10446 * ((t_sample)0.8));
			t_sample add_10504 = (add_10514 + mul_10444);
			__m_cycle_23.freq(((t_sample)0.1));
			t_sample cycle_10493 = __m_cycle_23(__sinedata);
			t_sample cycleindex_10494 = __m_cycle_23.phase();
			t_sample mul_10495 = (cycle_10493 * ((int)16));
			t_sample add_10496 = (mul_10495 + ((int)672));
			t_sample tap_10502 = m_delay_8.read_linear(add_10496);
			t_sample mul_10498 = (tap_10502 * ((t_sample)0.7));
			t_sample add_10499 = (add_10504 + mul_10498);
			t_sample mul_10497 = (add_10499 * ((t_sample)0.7));
			t_sample rsub_10500 = (tap_10502 - mul_10497);
			t_sample mul_10472 = (tap_10474 * ((t_sample)0.8));
			t_sample add_10503 = (mul_10472 + add_10514);
			__m_cycle_24.freq(((t_sample)0.07));
			t_sample cycle_10462 = __m_cycle_24(__sinedata);
			t_sample cycleindex_10463 = __m_cycle_24.phase();
			t_sample mul_10464 = (cycle_10462 * ((int)16));
			t_sample add_10465 = (mul_10464 + ((int)908));
			t_sample tap_10471 = m_delay_7.read_linear(add_10465);
			t_sample mul_10467 = (tap_10471 * ((t_sample)0.7));
			t_sample add_10468 = (add_10503 + mul_10467);
			t_sample mul_10466 = (add_10468 * ((t_sample)0.7));
			t_sample rsub_10469 = (tap_10471 - mul_10466);
			t_sample history_10486_next_10505 = fixdenorm(mix_10487);
			t_sample history_10442_next_10506 = fixdenorm(mix_10443);
			t_sample sub_10426 = (gen_10507 - gen_10508);
			t_sample mul_10424 = (sub_10426 * ((t_sample)0.6));
			t_sample mix_16860 = (mul_4846 + (((t_sample)0.2) * (mul_10424 - mul_4846)));
			t_sample history_10541_next_10949 = fixdenorm(mix_10542);
			t_sample out1 = mix_16860;
			int orange_16863 = (samplerate - (0.2 * samplerate));
			t_sample sub_16864 = (sah_5329 - ((int)0));
			t_sample scale_16861 = ((safepow((sub_16864 * ((t_sample)1)), ((int)1)) * orange_16863) + (0.2 * samplerate));
			t_sample tap_155 = m_delay_3.read_linear(scale_16861);
			t_sample mul_15373 = (tap_155 * ((t_sample)0.5));
			t_sample mul_16157 = (sah_5329 * ((int)100));
			t_sample train_16766 = __m_train_25(mul_16157, ((t_sample)0.5), ((int)0));
			t_sample mul_276 = (mix_133 * ((t_sample)0.7));
			t_sample history_6335_next_16769 = fixdenorm(sah_5329);
			t_sample history_132_next_16770 = fixdenorm(mix_133);
			m_delay_20.write(mul_15373);
			m_delay_19.write(mul_4846);
			m_history_4 = history_10541_next_10949;
			m_delay_13.write(add_10480);
			m_delay_12.write(sub_10479);
			m_delay_11.write(rsub_10500);
			m_delay_10.write(rsub_10469);
			m_delay_9.write(add_10452);
			m_delay_8.write(add_10499);
			m_delay_7.write(add_10468);
			m_history_6 = history_10486_next_10505;
			m_history_5 = history_10442_next_10506;
			m_delay_14.write(sub_10451);
			m_delay_17.write(sub_10521);
			m_delay_16.write(sub_10527);
			m_delay_15.write(sub_10533);
			m_delay_18.write(sub_10515);
			m_delay_3.write((train_16766 + mul_276));
			m_history_2 = history_6335_next_16769;
			m_history_1 = history_132_next_16770;
			m_delay_3.step();
			m_delay_7.step();
			m_delay_8.step();
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

int gen_kernel_numins = 1;
int gen_kernel_numouts = 1;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 0; }

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


} // feedback::
