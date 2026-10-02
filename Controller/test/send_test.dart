import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:geo_controller/geo.dart';
import 'package:geo_controller/osc.dart';
import 'package:shared_preferences/shared_preferences.dart';

// Real sockets: the app sends to 127.0.0.1:54345, we listen there.
void main() {
  test('global fan-out, /global/ packets, flush cap, ramp', () async {
    SharedPreferences.setMockInitialValues({'manualIp': '127.0.0.1'});
    final rx = await RawDatagramSocket.bind(InternetAddress.loopbackIPv4, cmdPort);
    final got = <(String, List<Object>)>[];
    rx.listen((_) {
      for (Datagram? dg; (dg = rx.receive()) != null;) {
        got.add(decodeOsc(dg!.data)!);
      }
    });
    final geo = Geo();
    await geo.start();

    geo.set(misc[0], -1, 0.5); // global VOL: one packet
    geo.set(fine, 2, 0.05);
    await Future<void>.delayed(const Duration(milliseconds: 100));
    geo.set(fine, -1, 0.01);
    geo.set(speed, -1, 0.25); // one /global/ packet, node fines zeroed
    await Future<void>.delayed(const Duration(milliseconds: 300));
    expect(got.where((m) => m.$1 == '/toRoto/global/misc/1').single.$2.single,
        closeTo(0.5, 1e-6));
    expect(got.where((m) => m.$1.contains('misc/1') && m.$2.length == 2), isEmpty);
    expect(geo.get(misc[0], 3), 0.5);
    expect(got.where((m) => m.$1 == '/toRoto/global/rotation/speed').last.$2.single,
        closeTo(0.26, 1e-6));
    // Only node 2's own fine move went out per node, before the global one.
    expect(got.where((m) => m.$1 == '/toRoto/rotation/speed').single.$2[0], 2.0);
    expect(geo.get(fine, 2), 0);
    expect(geo.out(speed, 3), closeTo(0.26, 1e-6));

    // Ramp: a 400 ms glide sends intermediate values before landing.
    got.clear();
    geo.setRamp(400);
    geo.set(servo, 1, 1.0);
    await Future<void>.delayed(const Duration(milliseconds: 600));
    final servoVals = [for (final m in got) m.$2[1] as double];
    expect(servoVals.length, greaterThan(3));
    expect(servoVals.first, lessThan(0.9));
    expect(servoVals.last, 1.0);
    rx.close();
  });
}
