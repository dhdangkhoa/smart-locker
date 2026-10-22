import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'providers/auth_provider.dart';
import 'routes/app_router.dart';
import 'providers/locker_provider.dart';

void main() {
  runApp(
    // ChangeNotifierProvider(
    //   create: (_) => AuthProvider(),
    //   child: const SmartLockerApp(),
    // ),

    MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => AuthProvider()),
        ChangeNotifierProvider(create: (_) => LockerProvider()),
      ],
      child: const SmartLockerApp(),
    ),
  );
}



class SmartLockerApp extends StatelessWidget {
  const SmartLockerApp({super.key});

  @override
  Widget build(BuildContext context) {
    final authProvider = context.watch<AuthProvider>();
    final router = createrouter(authProvider);

    return MaterialApp.router(
      title: 'Smart Locker System',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      routerConfig: router,
    );
  }
}