import 'package:go_router/go_router.dart';
import '../models/user_model.dart';
import '../providers/auth_provider.dart';
import '../screens/login_screen.dart';
import '../screens/admin_dashboard.dart';
import '../screens/courier_dashboard.dart';
import '../screens/customer_dashboard.dart';

GoRouter createrouter(AuthProvider authProvider) {
  return GoRouter(
    refreshListenable: authProvider,
    initialLocation: '/login',
    routes: [
      GoRoute(
        path: '/login',
        builder: (context, state) => const LoginScreen(), 
        ),
      GoRoute(
        path: '/admin',
        builder: (context, state) => const AdminDashboard(),
        ),
      GoRoute(
        path: '/courier',
        builder: (context, state) => const CourierDashboard(),
      ),
      GoRoute(
        path: '/customer',
        builder: (context, state) => const CustomerDashboard(),
      ),
    ],
    redirect: (context, state) {
      final isLoggedIn = authProvider.isAuthenticated;
      final isGoingToLogin = state.matchedLocation == '/login';

      if (!isLoggedIn) {
        return isGoingToLogin ? null : '/login';
      }

      if (isGoingToLogin) {
        switch (authProvider.role) {
          case UserRole.admin:
            return '/admin';
          case UserRole.courier:
            return '/courier';
          case UserRole.customer:
            return '/customer';
          default:
            return '/login';
        }
      }
      return null;
    }
  );
}