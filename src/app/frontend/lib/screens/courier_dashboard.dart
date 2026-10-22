import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../models/locker_models.dart';
import '../providers/locker_provider.dart';
import '../providers/auth_provider.dart';
import '../services/api_service.dart';
import '../widgets/logout_button.dart';

class CourierDashboard extends StatefulWidget {
  const CourierDashboard({super.key});

  @override
  State<CourierDashboard> createState() => _CourierDashboardState();
}

class _CourierDashboardState extends State<CourierDashboard> {
  final _trackingCodeController = TextEditingController();
  final _customerController = TextEditingController(); 

  @override
  void dispose() {
    _trackingCodeController.dispose();
    _customerController.dispose();
    super.dispose();
  }

  void _showDropoffDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (dialogContext) => AlertDialog(
        title: const Text('Place package in locker'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Text('Enter tracking code and recipient details:'),
            const SizedBox(height: 12),
            TextField(
              controller: _trackingCodeController,
              decoration: const InputDecoration(
                labelText: 'Tracking Code (e.g., SPX123456)',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 12),
            // enter recipient details
            TextField(
              controller: _customerController,
              decoration: const InputDecoration(
                labelText: 'Recipient Username / ID',
                border: OutlineInputBorder(),
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () {
              _trackingCodeController.clear();
              _customerController.clear();
              Navigator.pop(dialogContext);
            },
            child: const Text('Cancel'),
          ),
          ElevatedButton(
            onPressed: () {
              final code = _trackingCodeController.text.trim();
              final customer = _customerController.text.trim();

              if (code.isEmpty || customer.isEmpty) {
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(content: Text('Please fill in all fields!')),
                );
                return;
              }

              Navigator.pop(dialogContext);
              _processDropoff(code, customer);
              
              _trackingCodeController.clear();
              _customerController.clear();
            },
            child: const Text('Confirm'),
          )
        ],
      ),
    );
  }

  void _processDropoff(String trackingCode, String customer) async {
    final authProvider = context.read<AuthProvider>();
    final courierId = int.tryParse(authProvider.currentUser?.id ?? '1') ?? 1;

    // Call backend API to save order in DB with status 'waiting'
    final result = await APIService.storeOrder(
      trackingCode: trackingCode,
      recipient: customer,
      courierId: courierId,
    );

    if (!mounted) return;

    if (result['success'] == true) {
      final lockerNumber = result['data']?['locker_number'] ?? 'Locker';
      final lockerProvider = context.read<LockerProvider>();
      lockerProvider.updateCompartmentStatus(
        lockerNumber,
        CompartmentStatus.occupied,
        trackingCode: trackingCode,
        recipient: customer,
      );

      showDialog(
        context: context,
        barrierDismissible: false,
        builder: (ctx) => AlertDialog(
          icon: const Icon(Icons.lock_open_rounded, size: 48, color: Colors.green),
          title: Text('$lockerNumber has been opened.'),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                'Order: $trackingCode',
                style: const TextStyle(fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 4),
              Text(
                'Recipient: $customer',
                style: const TextStyle(fontWeight: FontWeight.bold, color: Colors.blue),
              ),
              const SizedBox(height: 4),
              const Text(
                'Status: waiting',
                style: TextStyle(fontWeight: FontWeight.bold, color: Colors.orange),
              ),
              const SizedBox(height: 8),
              const Text(
                'Please place the package in the compartment and close the locker door.',
                textAlign: TextAlign.center,
              ),
            ],
          ),
          actions: [
            ElevatedButton(
              onPressed: () => Navigator.pop(ctx),
              child: const Text('Closed'),
            ),
          ],
        ),
      );
    } else {
      final message = result['message']?.toString() ?? 'No available compartments or error occurred.';
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(message),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    final lockerProvider = context.watch<LockerProvider>();
    final compartments = lockerProvider.compartments;
    final availableCount = lockerProvider.availableCompartments.length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Locker Station'),
        actions: const [
          LogoutButton(),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Card(
              color: Colors.blue,
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text('Current Locker Status:', style: TextStyle(fontSize: 14)),
                        const SizedBox(height: 4),
                        Text(
                          'Available $availableCount / ${compartments.length}',
                          style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: Colors.white),
                        ),
                      ],
                    ),
                    ElevatedButton.icon(
                      onPressed: () => _showDropoffDialog(context),
                      icon: const Icon(Icons.add_box),
                      label: const Text("Deposit Package"),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.white,
                        foregroundColor: Colors.blue,
                      ),
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 20),
            Row(
              children: [
                _buildStatusLegend(Colors.green, 'Available'),
                const SizedBox(width: 16),
                _buildStatusLegend(Colors.red, 'Occupied'),
                const SizedBox(width: 16),
                _buildStatusLegend(Colors.grey, 'Maintenance'),
              ],
            ),
            const SizedBox(height: 16),
            const Text(
              'Compartment Layout:',
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Expanded(
              child: LayoutBuilder(
                builder: (context, constraints) {
                  int crossAxisCount = 3;
                  int totalItems = compartments.length;
                  int rowCount = (totalItems / crossAxisCount).ceil();
                  if (rowCount == 0) rowCount = 1;

                  double crossAxisSpacing = 8.0;
                  double mainAxisSpacing = 8.0;

                  double totalHorizontalSpacing = (crossAxisCount - 1) * crossAxisSpacing;
                  double totalVerticalSpacing = (rowCount - 1) * mainAxisSpacing;

                  double itemWidth = (constraints.maxWidth - totalHorizontalSpacing) / crossAxisCount;
                  double itemHeight = (constraints.maxHeight - totalVerticalSpacing) / rowCount;

                  double aspectRatio = itemWidth / itemHeight;

                  return GridView.builder(
                    physics: const NeverScrollableScrollPhysics(),
                    gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
                      crossAxisCount: crossAxisCount,
                      crossAxisSpacing: crossAxisSpacing,
                      mainAxisSpacing: mainAxisSpacing,
                      childAspectRatio: aspectRatio > 0 ? aspectRatio : 1.0,
                    ),
                    itemCount: totalItems,
                    itemBuilder: (context, index) {
                      final box = compartments[index];
                      return _buildCompartmentCard(box);
                    },
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStatusLegend(Color color, String label) {
    return Row(
      children: [
        Container(width: 12, height: 12, decoration: BoxDecoration(color: color, shape: BoxShape.circle)),
        const SizedBox(width: 6),
        Text(label, style: const TextStyle(fontSize: 12)),
      ],
    );
  }

  Widget _buildCompartmentCard(CompartmentModel box) {
    Color cardColor;
    IconData icon;

    switch (box.status) {
      case CompartmentStatus.available:
        cardColor = Colors.green;
        icon = Icons.lock_open;
        break;
      case CompartmentStatus.occupied:
        cardColor = Colors.red;
        icon = Icons.lock;
        break;
      case CompartmentStatus.maintenance:
        cardColor = Colors.grey;
        icon = Icons.build;
        break;
    }

    return Container(
      decoration: BoxDecoration(
        color: cardColor,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: Colors.black),
      ),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(icon, size: 28, color: Colors.black),
          const SizedBox(height: 1),
          Text(
            'Box: ${box.boxNumber}',
            style: const TextStyle(fontWeight: FontWeight.bold, fontSize: 16),
          ),
        ],
      ),
    );
  }
}