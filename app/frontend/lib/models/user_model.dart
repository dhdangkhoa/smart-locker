enum UserRole {admin, courier, customer, unknown}

class UserModel {
  final String id;
  final String username;
  final UserRole role;
  final String token;

  UserModel({
    required this.id,
    required this.username,
    required this.role,
    required this.token
  });
  factory UserModel.fromJson(Map<String, dynamic> json, String token) {
    return UserModel(
      id: json['id']?.toString() ?? '',
      username: json['username'] ?? '',
      role: _parseRole(json['role']),
      token: token,
    );
  }

  static UserRole _parseRole(String? roleStr) {
    switch (roleStr?.toUpperCase()) {
      case 'ADMIN':
        return UserRole.admin;
      case 'COURIER':
        return UserRole.courier;
      case 'CUSTOMER':
        return UserRole.customer;
      default:
        return UserRole.unknown;
    }
  }
}