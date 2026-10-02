import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'geo.dart';

final geo = Geo();

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await geo.start();
  // ponytail: semantics off — the Windows accessibility bridge (Flutter 3.41)
  // crashes the app after ~30 s ("Failed to update ui::AXTree"). Re-enable
  // when a Flutter upgrade fixes it; costs screen-reader support meanwhile.
  runApp(const ExcludeSemantics(child: App()));
}

const _green = Color(0xFF2ECC71);
const _cellWidth = 150.0;

class App extends StatelessWidget {
  const App({super.key});

  @override
  Widget build(BuildContext context) => MaterialApp(
        title: 'Geofonie',
        debugShowCheckedModeBanner: false,
        theme: ThemeData(
          brightness: Brightness.dark,
          colorSchemeSeed: const Color(0xFFFF8F00),
          scaffoldBackgroundColor: const Color(0xFF111111),
          visualDensity: VisualDensity.compact,
          sliderTheme: const SliderThemeData(
            trackHeight: 3,
            thumbShape: RoundSliderThumbShape(enabledThumbRadius: 6),
            overlayShape: RoundSliderOverlayShape(overlayRadius: 12),
          ),
        ),
        home: ListenableBuilder(listenable: geo, builder: (_, _) => Home()), // no const below: must rebuild on geo changes
      );
}

class Home extends StatelessWidget {
  const Home({super.key});

  @override
  Widget build(BuildContext context) => DefaultTabController(
        length: 2,
        child: Scaffold(
          appBar: AppBar(
            toolbarHeight: 0,
            bottom: const TabBar(tabs: [Tab(text: 'ROTO'), Tab(text: 'ACID')]),
          ),
          body: Column(children: [
            ConnectionBar(),
            Expanded(child: TabBarView(children: [RotoPage(), AcidPage()])),
          ]),
        ),
      );
}

class ConnectionBar extends StatelessWidget {
  const ConnectionBar({super.key});

  @override
  Widget build(BuildContext context) => Material(
        color: const Color(0xFF1C1C1C),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(12, 4, 4, 4),
          child: Row(children: [
            Icon(Icons.circle, size: 12, color: geo.live ? _green : Colors.grey),
            const SizedBox(width: 8),
            Expanded(
              child: Text('${geo.ip} · ${geo.status}',
                  maxLines: 1, overflow: TextOverflow.ellipsis),
            ),
            IconButton(
                tooltip: 'Presets',
                icon: const Icon(Icons.bookmarks),
                onPressed: () => showDialog<void>(
                    context: context, builder: (_) => const PresetDialog())),
            IconButton(
                tooltip: 'Find node',
                icon: const Icon(Icons.travel_explore),
                onPressed: geo.findNode),
            IconButton(
                tooltip: 'Reconnect Wi-Fi',
                icon: geo.busy
                    ? const SizedBox.square(
                        dimension: 20, child: CircularProgressIndicator(strokeWidth: 2))
                    : const Icon(Icons.wifi_find),
                onPressed: geo.busy ? null : geo.reconnectWifi),
            IconButton(
                tooltip: 'Settings',
                icon: const Icon(Icons.settings),
                onPressed: () => showDialog<void>(
                    context: context, builder: (_) => const SettingsDialog())),
          ]),
        ),
      );
}

// --- pages --------------------------------------------------------------------

class RotoPage extends StatelessWidget {
  const RotoPage({super.key});

  @override
  Widget build(BuildContext context) => ListView(padding: const EdgeInsets.all(8), children: [
        Section(title: 'GLOBAL', children: [
          ToggleCell(auto, -1),
          ToggleCell(dir, -1),
          SizedBox(
            width: _cellWidth,
            child: TextFormField(
              initialValue: '${geo.rampMs}',
              keyboardType: TextInputType.number,
              decoration: const InputDecoration(
                  labelText: 'RAMP ms', helperText: 'speed + servo glide', isDense: true),
              onChanged: (s) => geo.setRamp(int.tryParse(s) ?? 0),
            ),
          ),
          OutlinedButton.icon(
            icon: const Icon(Icons.radar),
            label: const Text('calibrate all'),
            onPressed: () => geo.calibrate(-1),
          ),
          OutlinedButton.icon(
            style: OutlinedButton.styleFrom(foregroundColor: Colors.redAccent),
            icon: const Icon(Icons.stop_circle),
            label: const Text('stop'),
            onPressed: geo.stop,
          ),
          ParamCell(speed, -1),
          ParamCell(fine, -1, max: 0.05),
          for (final (i, p) in misc.take(6).indexed)
            ParamCell(p, -1, quick: switch (i) {
              2 => const [0, 0.5, 1],
              5 => const [0.5],
              _ => const [],
            }),
        ]),
        for (final id in nodeIds) RotoNode(id),
      ]);
}

class RotoNode extends StatelessWidget {
  const RotoNode(this.id, {super.key});
  final int id;

  @override
  Widget build(BuildContext context) {
    final n = geo.roto[id]!;
    return Section(
      title: 'ROTO $id',
      alive: n.alive,
      trailing: [
        if (n.fw != null) Text('fw ${n.fw}', style: Theme.of(context).textTheme.bodySmall),
        const SizedBox(width: 8),
        Tooltip(
          message: 'sensor reading',
          child: SizedBox(
              width: 80,
              child: LinearProgressIndicator(value: n.reading ?? 0, minHeight: 6)),
        ),
        IconButton(
            tooltip: 'auto-calibrate',
            icon: const Icon(Icons.radar, size: 18),
            onPressed: () => geo.calibrate(id)),
      ],
      children: [
        ToggleCell(dir, id),
        ParamCell(speed, id),
        ParamCell(fine, id, max: 0.05),
        ParamCell(servo, id),
        for (final p in misc) ParamCell(p, id),
        ParamCell(minDist, id),
        ParamCell(maxDist, id),
      ],
    );
  }
}

class AcidPage extends StatelessWidget {
  const AcidPage({super.key});

  @override
  Widget build(BuildContext context) => ListView(padding: const EdgeInsets.all(8), children: [
        Section(title: 'GLOBAL', children: [for (final p in acid) ParamCell(p, -1)]),
        for (final id in nodeIds)
          Section(
            title: 'ACID $id',
            alive: geo.acidInfo[id]!.alive,
            children: [for (final p in acid) ParamCell(p, id)],
          ),
      ]);
}

// --- building blocks ----------------------------------------------------------

class Section extends StatelessWidget {
  const Section(
      {required this.title, required this.children, this.alive, this.trailing = const [], super.key});
  final String title;
  final bool? alive;
  final List<Widget> children, trailing;

  @override
  Widget build(BuildContext context) => Card(
        margin: const EdgeInsets.only(bottom: 8),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(10, 4, 10, 8),
          child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
            Row(children: [
              if (alive != null) ...[
                Icon(Icons.circle, size: 10, color: alive! ? _green : Colors.grey),
                const SizedBox(width: 6),
              ],
              Text(title, style: Theme.of(context).textTheme.titleSmall),
              const Spacer(),
              ...trailing,
            ]),
            const SizedBox(height: 4),
            Wrap(
              spacing: 8,
              runSpacing: 4,
              crossAxisAlignment: WrapCrossAlignment.center,
              children: children,
            ),
          ]),
        ),
      );
}

/// Long-press / right-click: copy this control as a Score.h Message line.
void copyScoreLine(BuildContext context, Param p, int id) {
  final q = p == fine ? speed : p; // fine is part of SPEED's value
  final line = scoreLine(q.path, id, geo.out(q, id), q.label);
  Clipboard.setData(ClipboardData(text: line));
  ScaffoldMessenger.of(context)
    ..hideCurrentSnackBar()
    ..showSnackBar(SnackBar(content: Text('Copied: $line'), duration: const Duration(seconds: 2)));
}

class ParamCell extends StatelessWidget {
  const ParamCell(this.p, this.id, {this.max = 1, this.quick = const [], super.key});
  final Param p;
  final int id;
  final double max;
  final List<double> quick;

  @override
  Widget build(BuildContext context) {
    final v = geo.get(p, id).clamp(0.0, max);
    final small = Theme.of(context).textTheme.bodySmall;
    final text = p == misc[3] ? '${v.toStringAsFixed(3)} ${midiNote(v)}' : v.toStringAsFixed(3);
    final cell = GestureDetector(
      onLongPress: () => copyScoreLine(context, p, id),
      onSecondaryTap: () => copyScoreLine(context, p, id),
      child: SizedBox(
        width: _cellWidth,
        child: Column(crossAxisAlignment: CrossAxisAlignment.stretch, children: [
          Row(children: [
            Text(p.label, style: small?.copyWith(fontWeight: FontWeight.bold)),
            const Spacer(),
            for (final q in quick)
              InkWell(
                onTap: () => geo.set(p, id, q),
                child: Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 3),
                  child: Text('$q', style: small?.copyWith(color: Colors.amber)),
                ),
              ),
            const SizedBox(width: 4),
            Text(text, style: small),
          ]),
          SizedBox(
            height: 24,
            child: Slider(value: v, max: max, onChanged: (x) => geo.set(p, id, x)),
          ),
        ]),
      ),
    );
    return p.hint == null ? cell : Tooltip(message: p.hint!, child: cell);
  }
}

class ToggleCell extends StatelessWidget {
  const ToggleCell(this.p, this.id, {super.key});
  final Param p;
  final int id;

  @override
  Widget build(BuildContext context) => GestureDetector(
        onLongPress: () => copyScoreLine(context, p, id),
        onSecondaryTap: () => copyScoreLine(context, p, id),
        child: FilterChip(
          label: Text(p.label),
          selected: geo.get(p, id) > 0,
          onSelected: (on) => geo.set(p, id, on ? 1 : 0),
        ),
      );
}

// --- dialogs ------------------------------------------------------------------

class PresetDialog extends StatelessWidget {
  const PresetDialog({super.key});

  @override
  Widget build(BuildContext context) => ListenableBuilder(
        listenable: geo,
        builder: (context, _) => AlertDialog(
          title: const Text('Presets'),
          content: Column(mainAxisSize: MainAxisSize.min, children: [
            for (var slot = 1; slot <= 8; slot++)
              Row(children: [
                SizedBox(width: 60, child: Text('Slot $slot')),
                TextButton(
                  onPressed: geo.hasPreset(slot)
                      ? () {
                          geo.recallPreset(slot);
                          Navigator.pop(context);
                        }
                      : null,
                  child: const Text('Recall'),
                ),
                TextButton(
                  onPressed: () => geo.savePreset(slot),
                  child: Text(geo.hasPreset(slot) ? 'Overwrite' : 'Save'),
                ),
              ]),
          ]),
        ),
      );
}

class SettingsDialog extends StatefulWidget {
  const SettingsDialog({super.key});

  @override
  State<SettingsDialog> createState() => _SettingsDialogState();
}

class _SettingsDialogState extends State<SettingsDialog> {
  final ssid = TextEditingController(text: geo.ssid);
  final pass = TextEditingController(text: geo.pass);
  final ip = TextEditingController(text: geo.manualIp);

  @override
  Widget build(BuildContext context) => AlertDialog(
        title: const Text('Settings'),
        content: SingleChildScrollView(
          child: Column(mainAxisSize: MainAxisSize.min, children: [
            TextField(controller: ssid, decoration: const InputDecoration(labelText: 'Mesh SSID')),
            TextField(
                controller: pass,
                obscureText: true,
                decoration: const InputDecoration(labelText: 'Mesh password')),
            TextField(
                controller: ip,
                decoration: const InputDecoration(
                    labelText: 'Manual node IP', hintText: 'empty = auto (gateway)')),
          ]),
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: const Text('Cancel')),
          FilledButton(
            onPressed: () {
              geo.saveSettings(
                  ssid: ssid.text.trim(), pass: pass.text, manualIp: ip.text.trim());
              Navigator.pop(context);
            },
            child: const Text('Save'),
          ),
        ],
      );
}
