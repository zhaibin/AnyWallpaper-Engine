import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:anywp_engine_example/main.dart';

void main() {
  testWidgets('renders AnyWP demo shell', (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());
    await tester.pump();

    expect(find.text('AnyWallpaper Engine'), findsOneWidget);
    expect(find.widgetWithText(Tab, 'Wallpaper'), findsOneWidget);
    expect(find.widgetWithText(Tab, 'Optimization'), findsOneWidget);
    expect(find.widgetWithText(Tab, 'Carousel Control'), findsOneWidget);
    expect(find.widgetWithText(Tab, 'Debug'), findsOneWidget);
  });
}
