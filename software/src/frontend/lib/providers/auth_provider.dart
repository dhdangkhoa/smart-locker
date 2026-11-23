import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import '../models/user_model.dart';
import '../services/api_service.dart';
class AuthProvider extends ChangeNotifier {
  final _storage = const FlutterSecureStorage();

  UserModel? _currentUser;
  bool _isLoading = false;

  UserModel? get currentUser => _currentUser;
  bool get isAuthenticated => _currentUser != null;
  UserRole get role => _currentUser?.role ?? UserRole.unknown;
  bool get isLoading => _isLoading;

  Future<bool> login(String username, String password) async {
    _isLoading = true;
    notifyListeners();
    
    final data = await APIService.login(username, password);

    if (data != null) {
      _currentUser = UserModel(
        id: data['id'].toString(), 
        username: data['username'], 
        role: _parseRole(data['role']), 
        token: data['token'] ?? 'fake_token'
      );

      await _storage.write(key: 'storage_token', value: _currentUser!.token);
      _isLoading = false;
      notifyListeners();
      return true;
    } else {
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }
  
  Future<bool> register(String username, String password, String role) async {
    _isLoading = true;
    notifyListeners();

    final success = await APIService.register(username, password, role);

    _isLoading = false;
    notifyListeners();
    return success;
  }

  UserRole _parseRole(String roleStr) {
    switch (roleStr.toLowerCase()) {
      case 'admin':
        return UserRole.admin;
      case 'courier':
        return UserRole.courier;
      case 'customer':
        return UserRole.customer;
      default:
        return UserRole.unknown;
    }
  }

  Future<void> logout() async {
    _currentUser = null;
    await _storage.delete(key: 'storage_token');
    notifyListeners();
  }
}