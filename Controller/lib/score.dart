import 'dart:async';
import 'dart:math';

/// Editable mirror of PlatformIO/common/Score.h. A message's value generator
/// is kept as its C source (`CONST(0.164f + I_PERFECT5TH)`), so import →
/// export round-trips exactly.
class Msg {
  Msg(this.path, this.id, this.gen, [this.dur = 100, this.note = '']);
  String path, gen, note;
  int id, dur;

  Msg copy() => Msg(path, id, gen, dur, note);
  Map<String, Object> toJson() => {'p': path, 'i': id, 'g': gen, 'd': dur, 'n': note};
  factory Msg.fromJson(Map j) =>
      Msg(j['p'] as String, j['i'] as int, j['g'] as String, j['d'] as int, j['n'] as String);
}

/// State ID = its index in the score (the firmware indexes score[] by ID).
class SeqState {
  SeqState(this.name, this.msgs, this.next, [this.dur = 30000, this.loop = true]);
  String name;
  List<Msg> msgs;
  List<int> next; // repeats = weight
  int dur;
  bool loop;

  Map<String, Object> toJson() => {
        'name': name,
        'msgs': [for (final m in msgs) m.toJson()],
        'next': next,
        'dur': dur,
        'loop': loop,
      };
  factory SeqState.fromJson(Map j) => SeqState(
        j['name'] as String,
        [for (final m in j['msgs'] as List) Msg.fromJson(m as Map)],
        (j['next'] as List).cast<int>(),
        j['dur'] as int,
        j['loop'] as bool,
      );
}

const intervals = {
  'I_MINOR2ND': 1,
  'I_MAJOR2ND': 2,
  'I_MINOR3RD': 3,
  'I_MAJOR3RD': 4,
  'I_PERFECT4TH': 5,
  'I_TRITONE': 6,
  'I_PERFECT5TH': 7,
  'I_MINOR6TH': 8,
  'I_MAJOR6TH': 9,
  'I_MINOR7TH': 10,
  'I_MAJOR7TH': 11,
  'I_OCTAVE': 12,
};

const genKinds = {
  'CONST': 'CONST(0.5f)',
  'RANDF': 'RANDF()',
  'RANDF_RANGE': 'RANDF_RANGE(0.0f, 1.0f)',
  'RANDI': 'RANDI(2)',
  'RAND_DIR': 'RAND_DIR()',
};

final _rng = Random();

/// `0.164f + I_PERFECT5TH - 0.01f` -> double. Throws FormatException.
double _expr(String s) {
  s = s.replaceAll(' ', '');
  const term = r'([+-]?)(\d*\.?\d+f?|[A-Z_][A-Z_0-9]*)';
  if (!RegExp('^(?:$term)+\$').hasMatch(s)) throw FormatException('bad expression: $s');
  var sum = 0.0;
  for (final m in RegExp(term).allMatches(s)) {
    final t = m[2]!;
    final semis = intervals[t];
    final v = semis != null ? semis / 48 : double.tryParse(t.replaceAll('f', ''));
    if (v == null) throw FormatException('unknown term: $t');
    sum += m[1] == '-' ? -v : v;
  }
  return sum;
}

/// Evaluates a generator like the Score.h macros do. Throws FormatException.
double evalGen(String gen) {
  final m = RegExp(r'^\s*(\w+)\((.*)\)\s*$').firstMatch(gen);
  if (m == null) throw FormatException('not KIND(args): $gen');
  final args = m[2]!.trim().isEmpty ? <double>[] : m[2]!.split(',').map(_expr).toList();
  double r() => _rng.nextInt(10001) / 10000;
  return switch ((m[1], args.length)) {
    ('CONST', 1) => args[0].clamp(0.0, 1.0),
    ('RANDF', 0) => r(),
    ('RANDF_RANGE', 2) => r() * (args[1] - args[0]) + args[0],
    ('RANDI', 1) => _rng.nextInt(args[0].toInt()).toDouble(),
    ('RAND_DIR', 0) => _rng.nextBool() ? 1 : -1,
    _ => throw FormatException('unknown generator: $gen'),
  };
}

bool validGen(String gen) {
  try {
    evalGen(gen);
    return true;
  } on Object {
    return false;
  }
}

// --- Score.h import / export ---------------------------------------------------

List<SeqState> parseScore(String src) {
  final states = <int, SeqState>{};
  final msgArr = RegExp(r'(?://[ \t]*([^\n]*)\n\s*)?const Message state(\d+)_msgs\[\] PROGMEM = \{(.*?)\n\s*\};',
      dotAll: true);
  final line = RegExp(r'^\s*\{"([^"]+)",\s*(-?\d+),\s*(.+),\s*(\d+)\},?\s*(?://\s*(.*))?$', multiLine: true);
  for (final a in msgArr.allMatches(src)) {
    states[int.parse(a[2]!)] = SeqState((a[1] ?? '').trim(), [
      for (final l in line.allMatches(a[3]!))
        Msg(l[1]!, int.parse(l[2]!), l[3]!.trim(), int.parse(l[4]!), (l[5] ?? '').trim()),
    ], []);
  }
  for (final n in RegExp(r'state(\d+)_next\[\] PROGMEM = \{([^}]*)\}').allMatches(src)) {
    states[int.parse(n[1]!)]?.next = [
      for (final s in n[2]!.split(',')) if (s.trim().isNotEmpty) int.parse(s.trim()),
    ];
  }
  for (final r in RegExp(r'\{(\d+),\s*state\d+_msgs,[^}]*?,\s*(\d+)\s*,\s*(true|false)\s*\}').allMatches(src)) {
    final s = states[int.parse(r[1]!)];
    if (s == null) continue;
    s.dur = int.parse(r[2]!);
    s.loop = r[3] == 'true';
  }
  if (states.isEmpty) throw const FormatException('no stateN_msgs arrays found');
  final ids = states.keys.toList()..sort();
  if (ids.last != ids.length - 1) throw FormatException('state IDs not 0..n: $ids');
  return [for (final i in ids) states[i]!];
}

String exportScore(List<SeqState> score) {
  final b = StringBuffer(_header);
  for (final (i, s) in score.indexed) {
    b.writeln();
    if (s.name.isNotEmpty) b.writeln('    // ${s.name}');
    b.writeln('    const Message state${i}_msgs[] PROGMEM = {');
    for (final m in s.msgs) {
      b.writeln('    {"${m.path}", ${m.id}, ${m.gen}, ${m.dur}},${m.note.isEmpty ? '' : ' // ${m.note}'}');
    }
    b.writeln('    };');
  }
  b.writeln('\n    // Define next_states arrays separately (repeats = weight)');
  for (final (i, s) in score.indexed) {
    b.writeln('    const uint8_t state${i}_next[] PROGMEM = {${s.next.join(', ')}};');
  }
  b.writeln('\n    const State score[] PROGMEM = {');
  for (final (i, s) in score.indexed) {
    b.writeln('    {$i, state${i}_msgs, COUNT_OF(state${i}_msgs), state${i}_next, '
        'COUNT_OF(state${i}_next), ${s.dur}, ${s.loop}},');
  }
  b.writeln('    };\n}');
  return b.toString();
}

/// Removes state [i] and fixes up every next list (indices shift down).
void removeState(List<SeqState> score, int i) {
  score.removeAt(i);
  for (final s in score) {
    s.next = [for (final n in s.next) if (n != i) n > i ? n - 1 : n];
  }
}

/// Swaps states [a] and [b], keeping transitions pointing at the same states.
void swapStates(List<SeqState> score, int a, int b) {
  final t = score[a];
  score[a] = score[b];
  score[b] = t;
  for (final s in score) {
    s.next = [for (final n in s.next) n == a ? b : n == b ? a : n];
  }
}

// --- runner ----------------------------------------------------------------------

/// Timer mirror of PlatformIO/common/Sequencer.h.
class ScoreRunner {
  ScoreRunner(this.send, this.changed);
  final void Function(Msg m) send;
  final void Function() changed;
  List<SeqState> score = [];
  int state = -1, msg = -1; // -1 = stopped
  Timer? _msgT, _stateT;

  bool get running => state >= 0;

  void start(List<SeqState> score, int at) {
    this.score = score;
    _enter(at);
  }

  void stop() {
    _msgT?.cancel();
    _stateT?.cancel();
    state = msg = -1;
    changed();
  }

  void _enter(int i) {
    _stateT?.cancel();
    if (i < 0 || i >= score.length) return stop();
    state = i;
    msg = -1;
    _next();
    final s = score[i];
    if (s.dur > 0) {
      _stateT = Timer(Duration(milliseconds: s.dur), () {
        if (s.next.isEmpty) return stop();
        _enter(s.next[_rng.nextInt(s.next.length)]);
      });
    }
  }

  void _next() {
    _msgT?.cancel();
    final s = score[state];
    if (s.msgs.isEmpty) return changed();
    msg = msg + 1 < s.msgs.length ? msg + 1 : 0;
    final m = s.msgs[msg];
    send(m);
    if (msg + 1 < s.msgs.length || s.loop) {
      _msgT = Timer(Duration(milliseconds: max(m.dur, 1)), _next);
    }
    changed();
  }
}

const _header = r'''/*
 * Score.h
 * Part of Geofonie project
 * Copyright (C) 2025 Filip Dobrocky, Trychtyr collective
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Generated by the Geofonie Controller app (SEQ tab).

#pragma once
#include <Arduino.h>

// Value generator macros
#define RANDF() []() -> float { return (float)random(0, 10001) / 10000.0f; }
#define RANDF_RANGE(a, b) []() -> float { return (float)random(0, 10001) / 10000.0f * ((b) - (a)) + (a); }
#define RANDI(n) []() -> float { return (float)random(0, n); }
#define CONST(f) []() -> float { return (float)(constrain(f, 0.0f, 1.0f)); }
#define RAND_DIR() []() -> float { return (random(0, 2) * 2 - 1); }

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

// Musical interval macros (root float offset for MIDI mapping 36-84)
#define I_MINOR2ND    (1.0f/48.0f)
#define I_MAJOR2ND    (2.0f/48.0f)
#define I_MINOR3RD    (3.0f/48.0f)
#define I_MAJOR3RD    (4.0f/48.0f)
#define I_PERFECT4TH  (5.0f/48.0f)
#define I_TRITONE     (6.0f/48.0f)
#define I_PERFECT5TH  (7.0f/48.0f)
#define I_MINOR6TH    (8.0f/48.0f)
#define I_MAJOR6TH    (9.0f/48.0f)
#define I_MINOR7TH    (10.0f/48.0f)
#define I_MAJOR7TH    (11.0f/48.0f)
#define I_OCTAVE      (12.0f/48.0f)

namespace Score {
    struct Message {
        const char *msg;
        int8_t dest_ID;
        float (*get_value)();
        uint32_t duration;
    };

    struct State {
        int8_t ID;
        const Message *msgs;
        uint8_t msgs_count;
        const uint8_t *next_states;
        uint8_t next_states_count;
        uint32_t duration = 0;
        bool loop = false;
    };

    // misc: 2 brightness, 3 engine (0 granular+reso / 0.5 FM drone / 1 FM pluck),
    //       4 root, 5 scale, 6 reso fb (0 = -fb, 0.5 = clean, 1 = +fb),
    //       7 FM ratio (x8, min 1), 8 FM index
    // Messages loop within a state; states last < 1 min.
''';
