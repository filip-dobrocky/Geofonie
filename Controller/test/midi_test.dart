import 'package:flutter_test/flutter_test.dart';
import 'package:geo_controller/midi.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('learn binds the next event, then dispatches it', () async {
    SharedPreferences.setMockInitialValues({});
    final prefs = await SharedPreferences.getInstance();
    final got = <(String, double, bool)>[];
    final m = Midi((t, v, n) => got.add((t, v, n)));
    await m.start(prefs); // no MIDI backend in tests: must not throw

    m.handle('cc:0:12', 1, false); // unbound: ignored
    m.toggleLearn();
    m.select('/toRoto/rotation/speed 2');
    m.handle('cc:0:12', 0.5, false); // learnt, not dispatched
    expect(got, isEmpty);
    expect(m.badge('/toRoto/rotation/speed 2'), 'CC12');

    m.select('stop');
    m.handle('note:9:36', 1, true);
    expect(m.badge('stop'), 'N36/10');

    m.select('/toRoto/rotation/speed 2'); // rebinding replaces the old source
    m.handle('cc:0:13', 0.1, false);
    m.toggleLearn();

    m.handle('cc:0:12', 0.3, false);
    m.handle('cc:0:13', 0.7, false);
    m.handle('note:9:36', 1, true);
    expect(got, [('/toRoto/rotation/speed 2', 0.7, false), ('stop', 1.0, true)]);
    expect(prefs.getString('midi_map'), contains('cc:0:13'));

    m.unbind('stop');
    expect(m.badge('stop'), isNull);
  });
}
