import 'package:flutter_test/flutter_test.dart';
import 'package:geo_controller/geo.dart';

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
}
