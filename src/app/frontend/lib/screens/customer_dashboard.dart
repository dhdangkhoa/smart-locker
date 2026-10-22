import 'package:flutter/material.dart';
import '../widgets/logout_button.dart';
import 'package:provider/provider.dart';
import '../providers/locker_provider.dart';
import '../providers/auth_provider.dart';
import '../services/api_service.dart';

class CustomerDashboard extends StatefulWidget {
  const CustomerDashboard({super.key});

  @override
  State<CustomerDashboard> createState() => _CustomerDashboardState();
}

class _CustomerDashboardState extends State<CustomerDashboard> {
  final _trackingController = TextEditingController();

  void _handlePickup() {
    final trackingCode = _trackingController.text.trim();
    if (trackingCode.isEmpty) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text("Enter the package's code:"), backgroundColor: Colors.orange),
      );
      return;
    }

    final lockerProvider = context.read<LockerProvider>();
    final authProvider = context.read<AuthProvider>();
    final currentUser = authProvider.currentUser;

    final targetBox = lockerProvider.findBoxByTrackingCode(trackingCode);

    if (targetBox == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("This order is not in the locker!"),
          backgroundColor: Colors.red,
        ),
      );
      return;
    }

    // check username/id
    final recipient = targetBox.recipient?.trim().toLowerCase();
    final currentUsername = currentUser?.username.trim().toLowerCase();
    final currentUserId = currentUser?.id.trim().toLowerCase();

    final isCorrectRecipient = recipient != null &&
        (recipient == currentUsername || recipient == currentUserId);

    if (!isCorrectRecipient) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("You can not get this package!"),
          backgroundColor: Colors.red,
        ),
      );
      return;
    }
    
    final int? customerId = int.tryParse(currentUser?.id ?? '');
    final String? customerUsername = currentUser?.username;

    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (ctx) => const Center(child: CircularProgressIndicator()),
    );

    APIService.pickupOrder(
      trackingCode: trackingCode,
      customerId: customerId,
      customerUsername: customerUsername,
    ).then((result) {
      if (mounted) {
        Navigator.pop(context); // Dismiss loading dialog
      }

      if (result['success'] == true) {
        lockerProvider.releaseCompartment(targetBox.id);

        // display success dialog
        showDialog(
          context: context,
          barrierDismissible: false,
          builder: (ctx) => AlertDialog(
            icon: const Icon(Icons.lock_open_rounded, size: 52, color: Colors.green),
            title: Text('${targetBox.boxNumber} has been opened!'),
            content: Text(
              'Order: $trackingCode\nRecipient: ${currentUser?.username ?? targetBox.recipient}\nYour package is ready for pickup.',
              textAlign: TextAlign.center,
            ),
            actions: [
              ElevatedButton(
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  foregroundColor: Colors.white,
                ),
                onPressed: () {
                  Navigator.pop(ctx);
                  _trackingController.clear();
                },
                child: const Text('Done!'),
              ),
            ],
          ),
        );
      } else {
        if (mounted) {
          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(
              content: Text(result['message'] ?? 'Pickup failed!'),
              backgroundColor: Colors.red,
            ),
          );
        }
      }
    }).catchError((err) {
      if (mounted) {
        Navigator.pop(context); // Dismiss loading dialog
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Error: $err'),
            backgroundColor: Colors.red,
          ),
        );
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    final currentUser = context.watch<AuthProvider>().currentUser;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Pickup package from Smart Locker'),
        backgroundColor: Colors.green,
        actions: const [
          LogoutButton(),
        ],
      ),
      body: Center(
        child: ConstrainedBox(
          constraints: const BoxConstraints(maxWidth: 420),
          child: Padding(
            padding: const EdgeInsets.all(24.0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Container(
                  padding: const EdgeInsets.all(20),
                  decoration: BoxDecoration(
                    color: Colors.green.shade50,
                    shape: BoxShape.circle,
                  ),
                  child: Icon(Icons.local_shipping_outlined, size: 64, color: Colors.green.shade700),
                ),
                const SizedBox(height: 24),
                
                const Text(
                  'Enter your order code to collect your package.',
                  style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                if (currentUser != null) ...[
                  Text(
                    'Logged in as: ${currentUser.username}',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: Colors.green.shade800,
                    ),
                  ),
                  const SizedBox(height: 8),
                ],
                const Text(
                  'Enter the exact tracking code or order number provided to unlock the locker and collect your package.',
                  textAlign: TextAlign.center,
                  style: TextStyle(color: Colors.black54, fontSize: 13),
                ),
                const SizedBox(height: 32),

                // confirmation box
                TextField(
                  controller: _trackingController,
                  textAlign: TextAlign.center,
                  style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
                  decoration: InputDecoration(
                    border: OutlineInputBorder(
                      borderRadius: BorderRadius.circular(12),
                    ),
                    filled: true,
                    fillColor: Colors.grey.shade50,
                    prefixIcon: const Icon(Icons.qr_code_scanner),
                  ),
                ),
                const SizedBox(height: 24),

                // Instant Feedback Confirmation Button
                SizedBox(
                  width: double.infinity,
                  height: 50,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.green,
                      foregroundColor: Colors.white,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12),
                      ),
                    ),
                    onPressed: _handlePickup,
                    child: const Text(
                      'Confirm Unlock',
                      style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                    ),
                  ),
                ),
                const SizedBox(height: 16),
              ],
            ),
          ),
        ),
      ),
    );
  }
}