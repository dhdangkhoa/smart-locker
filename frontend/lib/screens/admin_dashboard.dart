import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../models/locker_models.dart';
import '../providers/locker_provider.dart';
import '../widgets/logout_button.dart';

class AdminDashboard extends StatelessWidget {
  const AdminDashboard({super.key});

  // Edit or Delete the Order Code Assigned to a Locker Compartment
  void _showEditTrackingDialog(BuildContext context, CompartmentModel box) {
    final controller = TextEditingController(text: box.trackingCode ?? '');

    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('Quản lý Ngăn số ${box.boxNumber}'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Trạng thái hiện tại: ${box.status.name.toUpperCase()}'),
            const SizedBox(height: 12),
            TextField(
              controller: controller,
              decoration: const InputDecoration(
                labelText: "Package's code",
                hintText: 'Leave blank to remove the order from the compartment.',
                border: OutlineInputBorder(),
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text('Cancel'),
          ),
          ElevatedButton(
            style: ElevatedButton.styleFrom(backgroundColor: Colors.blue, foregroundColor: Colors.white),
            onPressed: () {
              final newCode = controller.text.trim();
              context.read<LockerProvider>().adminUpdateTrackingCode(box.id, newCode);
              Navigator.pop(ctx);
              
              ScaffoldMessenger.of(context).showSnackBar(
                SnackBar(content: Text('Compartment ${box.boxNumber} updated successfully!')),
              );
            },
            child: const Text('Save'),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final lockerProvider = context.watch<LockerProvider>();
    final compartments = lockerProvider.compartments;

    // Thống kê nhanh
    final total = compartments.length;
    final available = compartments.where((c) => c.status == CompartmentStatus.available).length;
    final occupied = compartments.where((c) => c.status == CompartmentStatus.occupied).length;
    final maintenance = compartments.where((c) => c.status == CompartmentStatus.maintenance).length;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Smart Locker System Administration'),
        backgroundColor: Colors.blue,
        actions: const [
          LogoutButton(),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Overview Statistics Cards
            Row(
              children: [
                _buildStatCard('Total Compartments', '$total', Colors.blue),
                const SizedBox(width: 8),
                _buildStatCard('AVAILABLE:', '$available', Colors.green),
                const SizedBox(width: 8),
                _buildStatCard('OCCUPIED:', '$occupied', Colors.orange),
                const SizedBox(width: 8),
                _buildStatCard('MAINTENANCE: ', '$maintenance', Colors.grey),
              ],
            ),
            const SizedBox(height: 20),

            const Text(
              'List of box',
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),

            // boxes
            Expanded(
              child: ListView.builder(
                itemCount: compartments.length,
                itemBuilder: (context, index) {
                  final box = compartments[index];
                  
                  Color statusColor;
                  switch (box.status) {
                    case CompartmentStatus.available:
                      statusColor = Colors.green;
                      break;
                    case CompartmentStatus.occupied:
                      statusColor = Colors.orange;
                      break;
                    case CompartmentStatus.maintenance:
                      statusColor = Colors.grey;
                      break;
                  }

                  return Card(
                    margin: const EdgeInsets.only(bottom: 8),
                    child: ListTile(
                      leading: CircleAvatar(
                        backgroundColor: Colors.black,
                        child: Text(
                          box.boxNumber,
                          style: TextStyle(fontWeight: FontWeight.bold, color: statusColor),
                        ),
                      ),
                      title: Text(
                        'Box Number: ${box.boxNumber}',
                        style: const TextStyle(fontWeight: FontWeight.bold),
                      ),
                      subtitle: Text(
                        box.status == CompartmentStatus.occupied
                            ? 'Code: ${box.trackingCode}'
                            : 'Current state: ${box.status.name.toUpperCase()}',
                        style: TextStyle(
                          color: box.status == CompartmentStatus.occupied ? Colors.blue : Colors.black,
                          fontWeight: box.status == CompartmentStatus.occupied ? FontWeight.w600 : FontWeight.normal,
                        ),
                      ),
                      trailing: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          // Edit/Add/Delete
                          IconButton(
                            icon: const Icon(Icons.edit_note, color: Colors.blue),
                            tooltip: 'Edit / Delete Order Code',
                            onPressed: () => _showEditTrackingDialog(context, box),
                          ),
                          // Quick Maintenance Status Toggle
                          IconButton(
                            icon: Icon(
                              box.status == CompartmentStatus.maintenance ? Icons.lock_open : Icons.build,
                              color: box.status == CompartmentStatus.maintenance ? Colors.green : Colors.grey.shade700,
                            ),
                            tooltip: box.status == CompartmentStatus.maintenance ? 'Reopen Compartment' : 'Maintenance',
                            onPressed: () {
                              context.read<LockerProvider>().adminToggleMaintenance(box.id);
                            },
                          ),
                        ],
                      ),
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStatCard(String title, String value, Color color) {
    return Expanded(
      child: Container(
        padding: const EdgeInsets.symmetric(vertical: 12, horizontal: 8),
        decoration: BoxDecoration(
          color: Colors.black,
          borderRadius: BorderRadius.circular(8),
          border: Border.all(color: Colors.black),
        ),
        child: Column(
          children: [
            Text(title, style: TextStyle(fontSize: 12, color: color, fontWeight: FontWeight.bold)),
            const SizedBox(height: 4),
            Text(value, style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: color)),
          ],
        ),
      ),
    );
  }
}