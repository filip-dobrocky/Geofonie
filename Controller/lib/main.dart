import 'dart:io';
import 'dart:math';

import 'package:file_selector/file_selector.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'geo.dart';
import 'score.dart';

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
        home: ListenableBuilder(listenable: Listenable.merge([geo, geo.midi]), builder: (_, _) => Home()), // no const below: must rebuild on geo changes
      );
}

class Home extends StatelessWidget {
  const Home({super.key});

  @override
  Widget build(BuildContext context) => DefaultTabController(
        length: 3,
        child: Scaffold(
          appBar: AppBar(
            toolbarHeight: 0,
            bottom: const TabBar(tabs: [Tab(text: 'ROTO'), Tab(text: 'ACID'), Tab(text: 'SEQ')]),
          ),
          body: Column(children: [
            ConnectionBar(),
            Expanded(child: TabBarView(children: [RotoPage(), AcidPage(), SeqPage()])),
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
                tooltip: geo.midi.devices.isEmpty
                    ? 'MIDI learn (no device)'
                    : 'MIDI learn — ${geo.midi.devices.join(', ')}',
                isSelected: geo.midi.learning,
                selectedIcon: const Icon(Icons.piano, color: Colors.amber),
                icon: Icon(Icons.piano,
                    color: geo.midi.devices.isEmpty ? Colors.grey : null),
                onPressed: geo.midi.toggleLearn),
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
          Learnable(
            'cal -1',
            OutlinedButton.icon(
              icon: const Icon(Icons.radar),
              label: const Text('calibrate all'),
              onPressed: () => geo.calibrate(-1),
            ),
          ),
          Learnable(
            'stop',
            OutlinedButton.icon(
              style: OutlinedButton.styleFrom(foregroundColor: Colors.redAccent),
              icon: const Icon(Icons.stop_circle),
              label: const Text('stop'),
              onPressed: geo.stop,
            ),
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
        Learnable(
          'cal $id',
          IconButton(
              tooltip: 'auto-calibrate',
              icon: const Icon(Icons.radar, size: 18),
              onPressed: () => geo.calibrate(id)),
        ),
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

void snack(BuildContext context, String text) => ScaffoldMessenger.of(context)
  ..hideCurrentSnackBar()
  ..showSnackBar(SnackBar(content: Text(text), duration: const Duration(seconds: 2)));

/// Long-press / right-click on a control: copy it as a Score.h Message line,
/// or append it to the sequencer's target state.
Future<void> controlMenu(BuildContext context, Offset at, Param p, int id) async {
  final t = geo.score.isEmpty ? 'new state' : 'state ${min(geo.seqTarget, geo.score.length - 1)}';
  final pick = await showMenu<int>(
    context: context,
    position: RelativeRect.fromLTRB(at.dx, at.dy, at.dx, at.dy),
    items: [
      const PopupMenuItem(value: 0, child: Text('Copy Score line')),
      PopupMenuItem(value: 1, child: Text('Add to sequencer → $t')),
    ],
  );
  if (!context.mounted || pick == null) return;
  final q = p == fine ? speed : p; // fine is part of SPEED's value
  if (pick == 0) {
    final line = scoreLine(q.path, id, geo.out(q, id), q.label);
    Clipboard.setData(ClipboardData(text: line));
    snack(context, 'Copied: $line');
  } else {
    geo.addToSeq(p, id);
    snack(context, 'Added ${q.label} to state ${geo.seqTarget}');
  }
}

/// MIDI learn wrapper: in learn mode a tap picks this control as the target
/// (the control itself is inert) and right-click / long-press unbinds it.
/// Bound controls carry a small CC/note badge.
class Learnable extends StatelessWidget {
  const Learnable(this.target, this.child, {super.key});
  final String target;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final m = geo.midi;
    final b = m.badge(target);
    var w = child;
    if (m.learning) {
      final sel = m.learnTarget == target;
      w = GestureDetector(
        behavior: HitTestBehavior.opaque,
        onTap: () => m.select(target),
        onSecondaryTap: () => m.unbind(target),
        onLongPress: () => m.unbind(target),
        child: DecoratedBox(
          decoration: BoxDecoration(
            border: Border.all(
                color: sel ? Colors.amber : Colors.teal.withAlpha(120), width: sel ? 2 : 1),
            borderRadius: BorderRadius.circular(4),
          ),
          child: AbsorbPointer(child: child),
        ),
      );
    }
    return b == null
        ? w
        : Badge(
            label: Text(b),
            backgroundColor: Colors.teal,
            alignment: AlignmentDirectional.topStart,
            offset: const Offset(-4, -4),
            child: w);
  }
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
      onLongPressStart: (d) => controlMenu(context, d.globalPosition, p, id),
      onSecondaryTapUp: (d) => controlMenu(context, d.globalPosition, p, id),
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
    return Learnable('${p.path} $id', p.hint == null ? cell : Tooltip(message: p.hint!, child: cell));
  }
}

class ToggleCell extends StatelessWidget {
  const ToggleCell(this.p, this.id, {super.key});
  final Param p;
  final int id;

  @override
  Widget build(BuildContext context) => Learnable(
        '${p.path} $id',
        GestureDetector(
          onLongPressStart: (d) => controlMenu(context, d.globalPosition, p, id),
          onSecondaryTapUp: (d) => controlMenu(context, d.globalPosition, p, id),
          child: FilterChip(
            label: Text(p.label),
            selected: geo.get(p, id) > 0,
            onSelected: (on) => geo.set(p, id, on ? 1 : 0),
          ),
        ),
      );
}

// --- sequencer ----------------------------------------------------------------

const _scoreTypes = [
  XTypeGroup(
      label: 'Score.h',
      extensions: ['h'],
      mimeTypes: ['text/*'],
      uniformTypeIdentifiers: ['public.c-header']),
];

/// Score.h editor + in-app runner. One reorderable list of state headers and
/// their messages: dragging a message past a header moves it to that state.
class SeqPage extends StatelessWidget {
  const SeqPage({super.key});

  Future<void> _import(BuildContext context) async {
    final f = await openFile(acceptedTypeGroups: _scoreTypes);
    if (f == null) return;
    try {
      final sc = parseScore(await f.readAsString());
      geo.seqStop();
      geo.score = sc;
      geo.seqTarget = 0;
      geo.scoreChanged();
      if (context.mounted) snack(context, 'Imported ${sc.length} states from ${f.name}');
    } on FormatException catch (e) {
      if (context.mounted) snack(context, 'Import failed: ${e.message}');
    }
  }

  Future<void> _export(BuildContext context) async {
    for (final (i, st) in geo.score.indexed) {
      final bad = st.msgs.isEmpty
          ? 'no messages'
          : st.msgs.where((m) => !validGen(m.gen)).firstOrNull?.gen;
      if (bad != null) return snack(context, 'State $i: $bad — fix before export');
    }
    final text = exportScore(geo.score);
    if (Platform.isAndroid || Platform.isIOS) {
      await Clipboard.setData(ClipboardData(text: text));
      if (context.mounted) snack(context, 'Score.h copied to clipboard');
      return;
    }
    final loc = await getSaveLocation(suggestedName: 'Score.h', acceptedTypeGroups: _scoreTypes);
    if (loc == null) return;
    await File(loc.path).writeAsString(text);
    if (context.mounted) snack(context, 'Saved ${loc.path}');
  }

  // [from]/[to] are ReorderableListView indices into the flat header+msg list.
  void _reorder(int from, int to) {
    final flat = <Object>[for (final st in geo.score) ...[st, ...st.msgs]];
    if (to > from) to--;
    flat.insert(max(to, 1), flat.removeAt(from)); // state 0's header stays first
    for (final st in geo.score) {
      st.msgs = [];
    }
    late SeqState cur;
    for (final o in flat) {
      if (o is SeqState) {
        cur = o;
      } else {
        cur.msgs.add(o as Msg);
      }
    }
    geo.scoreChanged();
  }

  @override
  Widget build(BuildContext context) {
    final items = [
      for (final (i, st) in geo.score.indexed) ...[
        (i, -1),
        for (var m = 0; m < st.msgs.length; m++) (i, m),
      ],
    ];
    final r = geo.runner;
    return Column(children: [
      Padding(
        padding: const EdgeInsets.all(8),
        child: Wrap(spacing: 8, runSpacing: 4, crossAxisAlignment: WrapCrossAlignment.center, children: [
          Learnable(
            'seq',
            FilledButton.icon(
              style: r.running ? FilledButton.styleFrom(backgroundColor: Colors.redAccent) : null,
              icon: Icon(r.running ? Icons.stop : Icons.play_arrow),
              label: Text(r.running ? 'stop' : 'run from ${geo.seqTarget}'),
              onPressed: geo.score.isEmpty ? null : (r.running ? geo.seqStop : geo.seqStart),
            ),
          ),
          Text(
              r.running
                  ? 'state ${r.state} · msg ${r.msg} · ESP AUTO off'
                  : 'run turns ESP AUTO off',
              style: Theme.of(context).textTheme.bodySmall),
          OutlinedButton.icon(
              icon: const Icon(Icons.file_open),
              label: const Text('import'),
              onPressed: () => _import(context)),
          OutlinedButton.icon(
              icon: const Icon(Icons.save_alt),
              label: const Text('export'),
              onPressed: () => _export(context)),
          OutlinedButton.icon(
            icon: const Icon(Icons.add),
            label: const Text('state'),
            onPressed: () {
              geo.score.add(SeqState('new state', [_newMsg()], []));
              geo.seqTarget = geo.score.length - 1;
              geo.scoreChanged();
            },
          ),
        ]),
      ),
      Expanded(
        child: ReorderableListView.builder(
          buildDefaultDragHandles: false,
          padding: const EdgeInsets.fromLTRB(8, 0, 8, 8),
          itemCount: items.length,
          onReorder: _reorder,
          itemBuilder: (_, i) {
            final (s, m) = items[i];
            final st = geo.score[s];
            return m < 0
                ? StateHeader(s, key: ObjectKey(st))
                : MsgRow(s, m, i, key: ObjectKey(st.msgs[m]));
          },
        ),
      ),
    ]);
  }
}

Msg _newMsg() => Msg(globalPath(speed.path), -1, 'CONST(0.0f)');

class StateHeader extends StatelessWidget {
  const StateHeader(this.s, {super.key});
  final int s;

  @override
  Widget build(BuildContext context) {
    final sc = geo.score;
    final st = sc[s];
    final weights = <int, int>{for (final n in st.next) n: st.next.where((x) => x == n).length};
    void edit(void Function() f) {
      f();
      geo.scoreChanged();
    }

    return Card(
      margin: const EdgeInsets.only(top: 12, bottom: 2),
      color: geo.runner.state == s ? Colors.green.withAlpha(60) : null,
      shape: geo.seqTarget == s
          ? RoundedRectangleBorder(
              side: const BorderSide(color: Colors.amber), borderRadius: BorderRadius.circular(8))
          : null,
      child: InkWell(
        onTap: () => edit(() => geo.seqTarget = s), // target for "Add to sequencer" / run
        child: Padding(
          padding: const EdgeInsets.fromLTRB(8, 4, 4, 4),
          child: Wrap(spacing: 8, runSpacing: 2, crossAxisAlignment: WrapCrossAlignment.center, children: [
            Text('STATE $s', style: Theme.of(context).textTheme.titleSmall),
            EditField(st.name, (v) => edit(() => st.name = v), width: 180, hint: 'name'),
            EditField('${st.dur}', (v) => edit(() => st.dur = int.tryParse(v) ?? st.dur),
                width: 80, label: 'dur ms', number: true),
            FilterChip(
                label: const Text('loop'),
                selected: st.loop,
                onSelected: (v) => edit(() => st.loop = v)),
            IconButton(
                tooltip: 'run from here',
                icon: const Icon(Icons.play_arrow, size: 18),
                onPressed: () {
                  geo.seqTarget = s;
                  geo.seqStart();
                }),
            IconButton(
                tooltip: 'add message',
                icon: const Icon(Icons.add, size: 18),
                onPressed: () => edit(() => st.msgs.add(_newMsg()))),
            IconButton(
                tooltip: 'move up',
                icon: const Icon(Icons.arrow_upward, size: 18),
                onPressed: s == 0 || geo.runner.running
                    ? null
                    : () => edit(() => swapStates(sc, s, s - 1))),
            IconButton(
                tooltip: 'move down',
                icon: const Icon(Icons.arrow_downward, size: 18),
                onPressed: s == sc.length - 1 || geo.runner.running
                    ? null
                    : () => edit(() => swapStates(sc, s, s + 1))),
            IconButton(
                tooltip: 'delete state',
                icon: const Icon(Icons.delete_outline, size: 18),
                onPressed: () => edit(() {
                      geo.seqStop();
                      removeState(sc, s);
                      geo.seqTarget = max(0, min(geo.seqTarget, sc.length - 1));
                    })),
            const Text('→'),
            for (final MapEntry(key: n, value: w) in weights.entries)
              Tooltip(
                message: 'tap: weight +1 · ✕: weight −1',
                child: InputChip(
                  label: Text('$n${sc[n].name.isEmpty ? '' : ' ${sc[n].name}'} ×$w'),
                  onPressed: () => edit(() => st.next.add(n)),
                  onDeleted: () => edit(() => st.next.remove(n)),
                ),
              ),
            PopupMenuButton<int>(
              tooltip: 'add transition',
              icon: const Icon(Icons.add_link, size: 18),
              itemBuilder: (_) => [
                for (final (i, x) in sc.indexed) PopupMenuItem(value: i, child: Text('$i ${x.name}')),
              ],
              onSelected: (n) => edit(() => st.next.add(n)),
            ),
          ]),
        ),
      ),
    );
  }
}

// Params a message can target (fine is never sent on its own).
final _seqParams = allParams.where((p) => p != fine).toList();

class MsgRow extends StatelessWidget {
  const MsgRow(this.s, this.m, this.index, {super.key});
  final int s, m, index;

  @override
  Widget build(BuildContext context) {
    final msg = geo.score[s].msgs[m];
    final base = msg.path.replaceFirst('/global/', '/');
    final known = _seqParams.any((p) => p.path == base);
    final ok = validGen(msg.gen);
    var preview = ok ? 'rand' : '!';
    if (ok && msg.gen.startsWith('CONST')) {
      final v = evalGen(msg.gen);
      preview = base == misc[3].path ? '${v.toStringAsFixed(3)} ${midiNote(v)}' : v.toStringAsFixed(3);
    }
    void edit(void Function() f) {
      f();
      geo.scoreChanged();
    }

    void setPath(String b, int id) => msg
      ..id = id
      ..path = id < 0 ? globalPath(b) : b;

    return Material(
      color: geo.runner.state == s && geo.runner.msg == m
          ? Colors.green.withAlpha(60)
          : Colors.transparent,
      child: Padding(
        padding: const EdgeInsets.only(left: 8),
        child: Wrap(spacing: 8, crossAxisAlignment: WrapCrossAlignment.center, children: [
          ReorderableDragStartListener(
              index: index,
              child: const Icon(Icons.drag_indicator, size: 18, color: Colors.grey)),
          DropdownButton<String>(
            value: base,
            isDense: true,
            items: [
              for (final p in _seqParams)
                DropdownMenuItem(
                    value: p.path,
                    child: Text('${p.label} · ${p.path.substring(1).split('/').first}')),
              if (!known) DropdownMenuItem(value: base, child: Text(base)),
            ],
            onChanged: (b) => edit(() => setPath(b!, msg.id)),
          ),
          DropdownButton<int>(
            value: msg.id,
            isDense: true,
            items: [
              for (final id in [-1, ...nodeIds])
                DropdownMenuItem(value: id, child: Text(id < 0 ? 'G' : '$id')),
            ],
            onChanged: (id) => edit(() => setPath(base, id!)),
          ),
          EditField(msg.gen, (v) => edit(() => msg.gen = v), width: 230, error: !ok),
          PopupMenuButton<String>(
            tooltip: 'generator',
            icon: const Icon(Icons.casino, size: 18),
            itemBuilder: (_) => [for (final k in genKinds.keys) PopupMenuItem(value: k, child: Text(k))],
            onSelected: (k) => edit(() => msg.gen = genKinds[k]!),
          ),
          PopupMenuButton<String>(
            tooltip: 'add interval (CONST)',
            icon: const Icon(Icons.music_note, size: 18),
            enabled: msg.gen.trimRight().startsWith('CONST(') && msg.gen.trimRight().endsWith(')'),
            itemBuilder: (_) => [
              for (final MapEntry(:key, :value) in intervals.entries)
                PopupMenuItem(value: key, child: Text('$key  +$value st')),
            ],
            onSelected: (k) => edit(() {
              final g = msg.gen.trimRight();
              msg.gen = '${g.substring(0, g.length - 1)} + $k)';
            }),
          ),
          SizedBox(width: 90, child: Text(preview, style: Theme.of(context).textTheme.bodySmall)),
          EditField('${msg.dur}', (v) => edit(() => msg.dur = int.tryParse(v) ?? msg.dur),
              width: 70, label: 'ms', number: true),
          EditField(msg.note, (v) => edit(() => msg.note = v), width: 150, hint: 'comment'),
          IconButton(
              tooltip: 'send once',
              icon: const Icon(Icons.send, size: 16),
              onPressed: () => geo.applyScoreMsg(msg)),
          IconButton(
              tooltip: 'delete',
              icon: const Icon(Icons.close, size: 16),
              onPressed: () => edit(() => geo.score[s].msgs.removeAt(m))),
        ]),
      ),
    );
  }
}

/// Text field that follows its model value when that changes elsewhere
/// (menus, import) without fighting the user's typing.
class EditField extends StatefulWidget {
  const EditField(this.value, this.onChanged,
      {required this.width,
      this.label,
      this.hint,
      this.number = false,
      this.error = false,
      super.key});
  final String value;
  final ValueChanged<String> onChanged;
  final double width;
  final String? label, hint;
  final bool number, error;

  @override
  State<EditField> createState() => _EditFieldState();
}

class _EditFieldState extends State<EditField> {
  late final c = TextEditingController(text: widget.value);

  @override
  void didUpdateWidget(EditField old) {
    super.didUpdateWidget(old);
    if (widget.value != c.text) c.text = widget.value;
  }

  @override
  void dispose() {
    c.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => SizedBox(
        width: widget.width,
        child: TextField(
          controller: c,
          onChanged: widget.onChanged,
          keyboardType: widget.number ? TextInputType.number : null,
          style: Theme.of(context).textTheme.bodySmall,
          decoration: InputDecoration(
            isDense: true,
            labelText: widget.label,
            hintText: widget.hint,
            enabledBorder: widget.error
                ? const UnderlineInputBorder(borderSide: BorderSide(color: Colors.redAccent))
                : null,
          ),
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
