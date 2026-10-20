import 'dart:convert';
import 'package:http/http.dart' as http;

class APIService {

  static const String baseURL = "http://localhost:8000";


  static Future<bool> register(String username, String password, String role) async {
    try {

      final String endpoint = (role.toUpperCase() == 'CUSTOMER')
          ? '/auth/customers/register'
          : '/auth/couriers/register';

      final url = Uri.parse('$baseURL$endpoint');

      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'username': username,
          'password': password,

          if (role.toUpperCase() == 'CUSTOMER') 'face_id': null,
        }),
      );

      return response.statusCode == 200 || response.statusCode == 201;
    } catch (e) {

      return false;
    }
  }


  static Future<Map<String, dynamic>?> login(String username, String password) async {
    try {
      final url = Uri.parse('$baseURL/auth/login');

      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'username': username,
          'password': password,
        }),
      );

      if (response.statusCode == 200) {
        return jsonDecode(response.body) as Map<String, dynamic>;
      }
      return null;
    } catch (e) {

      return null;
    }
  }


  static Future<Map<String, dynamic>> storeOrder({
    required String trackingCode,
    required String recipient,
    required int courierId,
    int? lockerId,
  }) async {
    try {
      final url = Uri.parse('$baseURL/orders/store');
      final int? recipientId = int.tryParse(recipient);

      final Map<String, dynamic> body = {
        'tracking_code': trackingCode,
        'courier_id': courierId,
      };
      if (recipientId != null) {
        body['customer_id'] = recipientId;
      } else {
        body['customer_username'] = recipient;
      }
      if (lockerId != null) {
        body['locker_id'] = lockerId;
      }

      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode(body),
      );

      final data = jsonDecode(response.body);
      if (response.statusCode == 200 || response.statusCode == 201) {
        return {'success': true, 'data': data};
      } else {
        return {'success': false, 'message': data['detail'] ?? 'Deposit failed'};
      }
    } catch (e) {
      return {'success': false, 'message': e.toString()};
    }
  }

  static Future<Map<String, dynamic>> pickupOrder({
    required String trackingCode,
    int? customerId,
    String? customerUsername,
  }) async {
    try {
      final url = Uri.parse('$baseURL/orders/pickup');

      final Map<String, dynamic> body = {
        'tracking_code': trackingCode,
      };
      if (customerId != null) {
        body['customer_id'] = customerId;
      }
      if (customerUsername != null) {
        body['customer_username'] = customerUsername;
      }

      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode(body),
      );

      final data = jsonDecode(response.body);
      if (response.statusCode == 200) {
        return {'success': true, 'data': data};
      } else {
        return {
          'success': false,
          'statusCode': response.statusCode,
          'message': data['detail'] ?? 'Pickup failed'
        };
      }
    } catch (e) {
      return {'success': false, 'statusCode': 500, 'message': e.toString()};
    }
  }

  static Future<List<Map<String, dynamic>>?> fetchLockers() async {
    try {
      final url = Uri.parse('$baseURL/lockers/');
      final response = await http.get(url);
      if (response.statusCode == 200) {
        final List<dynamic> list = jsonDecode(response.body);
        return list.cast<Map<String, dynamic>>();
      }
      return null;
    } catch (e) {
      return null;
    }
  }

  static Future<Map<String, dynamic>> deleteOrder(String trackingCode) async {
    try {
      final url = Uri.parse('$baseURL/orders/$trackingCode');
      final response = await http.delete(url);
      final data = jsonDecode(response.body);
      if (response.statusCode == 200) {
        return {'success': true, 'data': data};
      } else {
        return {'success': false, 'message': data['detail'] ?? 'Delete failed'};
      }
    } catch (e) {
      return {'success': false, 'message': e.toString()};
    }
  }

  static Future<Map<String, dynamic>> removeOrderFromLocker(String lockerIdOrNumber) async {
    try {
      final url = Uri.parse('$baseURL/lockers/$lockerIdOrNumber/order');
      final response = await http.delete(url);
      final data = jsonDecode(response.body);
      if (response.statusCode == 200) {
        return {'success': true, 'data': data};
      } else {
        return {'success': false, 'message': data['detail'] ?? 'Clear failed'};
      }
    } catch (e) {
      return {'success': false, 'message': e.toString()};
    }
  }

  static Future<Map<String, dynamic>> updateLockerStatus(String lockerIdOrNumber, String status) async {
    try {
      final url = Uri.parse('$baseURL/lockers/$lockerIdOrNumber/status');
      final response = await http.patch(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'status': status}),
      );
      final data = jsonDecode(response.body);
      if (response.statusCode == 200) {
        return {'success': true, 'data': data};
      } else {
        return {'success': false, 'message': data['detail'] ?? 'Update failed'};
      }
    } catch (e) {
      return {'success': false, 'message': e.toString()};
    }
  }
}