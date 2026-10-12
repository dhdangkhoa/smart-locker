import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import '../models/user_model.dart';

class AuthProvider extends ChangeNotifier {
  final _storage = const FlutterSecureStorage();

  UserModel? _currentUser;
  bool _isLoading = false;

  UserModel? get currentUser => _currentUser;
  bool get isAuthenticated => _currentUser != null;
  UserRole get role => _currentUser?.role ?? UserRole.unknown;
  bool get isLoading => _isLoading;

  // simulate the login states
  Future<bool> login(String username, String password) async {
    _isLoading = true;
    notifyListeners();

    await Future.delayed(const Duration(seconds: 1));
    if (username == 'admin' && password == '123') {
      _currentUser = UserModel(
        id: '1', 
        username: 'Admin user', 
        role: UserRole.admin, 
        token: 'test_admin',
        );
    } else if (username == 'courier' && password == '123') {
      _currentUser = UserModel(
        id: '2',
        username: 'Courier 01',
        role: UserRole.courier,
        token: 'test_courier',
      );
    } else if (username == 'customer' && password == '123') {
      _currentUser = UserModel(
        id: '3', 
        username: 'Customer A',
        role: UserRole.customer, 
        token: 'test_customer',
        );
    } else {
      _isLoading = false;
      notifyListeners();
      return false; // incorrect login info
    }

    await _storage.write(key: 'storage_token', value: _currentUser!.token);
    _isLoading = false;

    notifyListeners();
  
    return true;
  }

  Future<void> logout() async {
    _currentUser = null;
    await _storage.delete(key: 'storage_token');
    notifyListeners();
  }
}