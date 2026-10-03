import 'dart:async';
import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter_midi_command/flutter_midi_command.dart';
import 'package:flutter_midi_command/flutter_midi_command_messages.dart';
import 'package:shared_preferences/shared_preferences.dart';

/// MIDI input + learn. Bindings map a source (`cc:<ch>:<num>` /
/// `note:<ch>:<num>`) to a target: a control key (`<path> <id>`, as in Geo's
/// values) or an action name (`stop`, `cal <id>`, `seq`).
class Midi extends ChangeNotifier {
  Midi(this.onControl);

  /// (target, value 0..1, isNote): the app decides what a target does with it.
  final void Function(String target, double v, bool isNote) onControl;

  final bindings = <String, String>{};
  List<String> devices = [];
  bool learning = false;
  String? learnTarget;

  late SharedPreferences _prefs;
  MidiCommand? _cmd;

  Future<void> start(SharedPreferences prefs) async {
    _prefs = prefs;
    final s = prefs.getString('midi_map');
    if (s != null) bindings.addAll((jsonDecode(s) as Map).cast<String, String>());
    try {
      final cmd = _cmd = MidiCommand();
      cmd.configureTransportPolicy(
          const MidiTransportPolicy(excludedTransports: {MidiTransport.ble}));
      cmd.onMidiDataReceived?.listen((e) => _onMessage(e.message), onError: (Object _) {});
      cmd.onMidiSetupChanged?.listen((_) => _connectAll(), onError: (Object _) {});
      await _connectAll();
    } catch (e) {
      // No MIDI backend (e.g. ALSA missing): the app works without it.
      debugPrint('MIDI unavailable: $e');
    }
  }

  // Hot-plug: connect every device with an input that isn't connected yet.
  Future<void> _connectAll() async {
    final cmd = _cmd;
    if (cmd == null) return;
    final all = await cmd.devices ?? [];
    for (final d in all) {
      if (d.connected || d.inputPorts.isEmpty) continue;
      try {
        await cmd.connectToDevice(d);
      } catch (e) {
        debugPrint('MIDI connect ${d.name}: $e');
      }
    }
    devices = [for (final d in all) if (d.inputPorts.isNotEmpty) d.name];
    notifyListeners();
  }

  void _onMessage(MidiMessage m) {
    switch (m) {
      case CCMessage():
        handle('cc:${m.channel}:${m.controller}', m.value / 127, false);
      case NoteOnMessage() when m.velocity > 0:
        handle('note:${m.channel}:${m.note}', m.velocity / 127, true);
    }
  }

  /// One incoming event. Public for tests.
  void handle(String src, double v, bool isNote) {
    if (learning && learnTarget != null) {
      bindings.removeWhere((k, t) => t == learnTarget);
      bindings[src] = learnTarget!;
      learnTarget = null;
      _save();
      return;
    }
    final t = bindings[src];
    if (t != null) onControl(t, v, isNote);
  }

  void toggleLearn() {
    learning = !learning;
    learnTarget = null;
    notifyListeners();
  }

  void select(String target) {
    learnTarget = target;
    notifyListeners();
  }

  void unbind(String target) {
    bindings.removeWhere((k, t) => t == target);
    _save();
  }

  /// Short label of what's bound to [target], e.g. 'CC12' / 'N36'.
  String? badge(String target) {
    for (final MapEntry(:key, :value) in bindings.entries) {
      if (value != target) continue;
      final p = key.split(':');
      return '${p[0] == 'cc' ? 'CC' : 'N'}${p[2]}${p[1] == '0' ? '' : '/${int.parse(p[1]) + 1}'}';
    }
    return null;
  }

  void _save() {
    _prefs.setString('midi_map', jsonEncode(bindings));
    notifyListeners();
  }
}
