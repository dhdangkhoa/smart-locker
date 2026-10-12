import 'package:flutter/material.dart';
import '../widgets/logout_button.dart';
import 'package:provider/provider.dart';
import '../providers/locker_provider.dart';

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

    final targetBox = lockerProvider.findBoxByTrackingCode(trackingCode);

    if (targetBox == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Couldn't find the package/Invalid code!"),
          backgroundColor: Colors.red,
        ),
      );
      return;
    }
    
    lockerProvider.releaseCompartment(targetBox.id);

    // display 
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (ctx) => AlertDialog(
        icon: const Icon(Icons.lock_open_rounded, size: 52, color: Colors.green),
        title: Text('${targetBox.boxNumber} has been opened!'),
        content: Text(
          'Order: $trackingCode\nYour package is ready for pickup.',
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
  }

  @override
  Widget build(BuildContext context) {
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