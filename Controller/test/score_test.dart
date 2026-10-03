import 'package:flutter_test/flutter_test.dart';
import 'dart:io';

import 'package:geo_controller/geo.dart';
import 'package:geo_controller/score.dart';

void main() {
  test('global path', () {
    expect(globalPath('/toRoto/misc/4'), '/toRoto/global/misc/4');
    expect(globalPath('/toAcid/param1'), '/toAcid/global/param1');
    expect(globalPath('/toRoto/auto'), '/toRoto/global/auto');
  });

  test('score line', () {
    expect(scoreLine('/toRoto/misc/4', 2, 0.164, 'ROOT'),
        '{"/toRoto/misc/4", 2, CONST(0.164f), 100}, // root');
    expect(scoreLine('/toRoto/misc/6', -1, 0.55, 'RESO'),
        '{"/toRoto/global/misc/6", -1, CONST(0.550f), 100}, // reso');
    expect(scoreLine('/toRoto/rotation/speed', 0, 1.03, 'SPEED'),
        '{"/toRoto/rotation/speed", 0, CONST(1.000f), 100}, // speed');
  });

  test('midi note', () {
    expect(midiNote(0), 'C2');
    expect(midiNote(1), 'C6');
  });

  test('Score.h parse / export round-trip', () {
    final src = File('../PlatformIO/common/Score.h').readAsStringSync();
    final score = parseScore(src);
    expect(score.map((s) => s.msgs.length), [14, 14, 15, 15, 16]);
    expect(score[0].next, [3, 3, 3, 1, 1, 2]);
    expect(score[0].name, 'resonant granular');
    expect(score[2].dur, 25000);
    expect(score[0].msgs[4].gen, 'CONST(0.164f + I_PERFECT5TH)');
    expect(score[0].msgs[4].note, 'root');
    for (final s in score) {
      for (final m in s.msgs) {
        expect(validGen(m.gen), isTrue, reason: m.gen);
      }
    }
    final again = parseScore(exportScore(score));
    expect([for (final s in again) s.toJson()], [for (final s in score) s.toJson()]);
  });

  test('evalGen', () {
    expect(evalGen('CONST(0.164f + I_PERFECT5TH)'), closeTo(0.164 + 7 / 48, 1e-9));
    expect(evalGen('CONST(1.3f)'), 1.0);
    expect(evalGen('CONST(0.5f - I_OCTAVE)'), closeTo(0.25, 1e-9));
    for (var i = 0; i < 50; i++) {
      expect(evalGen('RANDF_RANGE(0.05f, 0.2f)'), inInclusiveRange(0.05, 0.2));
      expect(evalGen('RAND_DIR()'), anyOf(1.0, -1.0));
      expect(evalGen('RANDI(3)'), anyOf(0.0, 1.0, 2.0));
    }
    expect(validGen('CONST(I_BOGUS)'), isFalse);
    expect(validGen('NOPE(1)'), isFalse);
  });

  test('state removal and swap keep transitions', () {
    SeqState s(List<int> next) => SeqState('', [], next);
    final score = [s([1, 2]), s([2]), s([0, 1])];
    swapStates(score, 0, 2);
    expect([for (final x in score) x.next], [[2, 1], [0], [1, 0]]);
    removeState(score, 1);
    expect([for (final x in score) x.next], [[1], [0]]);
  });

  testWidgets('runner mirrors Sequencer.h timing', (tester) async {
    final sent = <String>[];
    final r = ScoreRunner((m) => sent.add(m.note), () {});
    r.start([
      SeqState('a', [Msg('/x', 0, 'CONST(0)', 100, 'a0'), Msg('/x', 0, 'CONST(0)', 200, 'a1')], [1], 450),
      SeqState('b', [Msg('/x', 0, 'CONST(0)', 100, 'b0')], [], 150, false),
    ], 0);
    expect(sent, ['a0']);
    await tester.pump(const Duration(milliseconds: 100));
    expect(sent, ['a0', 'a1']);
    await tester.pump(const Duration(milliseconds: 200)); // loop wraps
    expect(sent, ['a0', 'a1', 'a0']);
    await tester.pump(const Duration(milliseconds: 150)); // 450: state b
    expect(sent.last, 'b0');
    expect(r.state, 1);
    await tester.pump(const Duration(milliseconds: 150)); // no next: stop
    expect(r.running, isFalse);
    expect(sent.where((n) => n == 'b0').length, 1); // loop false: no repeat
  });
}
