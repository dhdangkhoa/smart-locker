import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../models/locker_models.dart';
import '../providers/locker_provider.dart';
import '../services/api_service.dart';
import '../widgets/logout_button.dart';

class AdminDashboard extends StatelessWidget {
  const AdminDashboard({super.key});

  void _confirmDeleteOrder(BuildContext context, CompartmentModel box) {
    if (box.trackingCode == null || box.trackingCode!.isEmpty) return;

    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('Delete Order from Box ${box.boxNumber}'),
        content: Text(
          'Are you sure you want to remove order "${box.trackingCode}" from this compartment?\n',
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text('Cancel'),
          ),
          ElevatedButton(
            style: ElevatedButton.styleFrom(backgroundColor: Colors.red, foregroundColor: Colors.white),
            onPressed: () async {
              Navigator.pop(ctx);
              final lockerProvider = context.read<LockerProvider>();
              final result = await APIService.deleteOrder(box.trackingCode!);
              if (result['success'] == true) {
                lockerProvider.releaseCompartment(box.id);
                if (context.mounted) {
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(content: Text('Order ${box.trackingCode} is deleted!')),
                  );
                }
              } else {
                if (context.mounted) {
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(
                      content: Text(result['message'] ?? 'Failed to delete order'),
                      backgroundColor: Colors.red,
                    ),
                  );
                }
              }
            },
            child: const Text('Delete'),
          ),
        ],
      ),
    );
  }

  // Edit or Delete the Order Code Assigned to a Locker Compartment
  void _showEditTrackingDialog(BuildContext context, CompartmentModel box) {
    final controller = TextEditingController(text: box.trackingCode ?? '');
    final recipientController = TextEditingController(text: box.recipient ?? '');
    final hasActiveOrder = box.trackingCode != null && box.trackingCode!.isNotEmpty;

    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text('Box No.${box.boxNumber}'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('State: ${box.status.name.toUpperCase()}'),
            const SizedBox(height: 12),
            TextField(
              controller: controller,
              decoration: const InputDecoration(
                labelText: "Package's code",
                hintText: 'Leave blank to remove the order from the compartment.',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 12),
            TextField(
              controller: recipientController,
              decoration: const InputDecoration(
                labelText: "Recipient Username / ID",
                border: OutlineInputBorder(),
              ),
            ),
          ],
        ),
        actions: [
          if (hasActiveOrder)
            TextButton.icon(
              style: TextButton.styleFrom(foregroundColor: Colors.red),
              icon: const Icon(Icons.delete_outline, size: 18),
              label: const Text('Delete Order'),
              onPressed: () async {
                final lockerProvider = context.read<LockerProvider>();
                Navigator.pop(ctx);
                
                final result = await APIService.deleteOrder(box.trackingCode!);
                if (result['success'] == true) {
                  lockerProvider.releaseCompartment(box.id);
                  if (context.mounted) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(content: Text('Order ${box.trackingCode} deleted!')),
                    );
                  }
                } else {
                  if (context.mounted) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(
                        content: Text(result['message'] ?? 'Failed to delete order'),
                        backgroundColor: Colors.red,
                      ),
                    );
                  }
                }
              },
            ),
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text('Cancel'),
          ),
          ElevatedButton(
            style: ElevatedButton.styleFrom(backgroundColor: Colors.blue, foregroundColor: Colors.white),
            onPressed: () async {
              final newCode = controller.text.trim();
              final newRecipient = recipientController.text.trim();
              final lockerProvider = context.read<LockerProvider>();
              Navigator.pop(ctx);

              if (newCode.isEmpty && hasActiveOrder) {
                // If cleared, mark order as deleted in DB
                final result = await APIService.deleteOrder(box.trackingCode!);
                if (result['success'] == true) {
                  lockerProvider.releaseCompartment(box.id);
                  if (context.mounted) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(content: Text('Order ${box.trackingCode} is deleted!')),
                    );
                  }
                } else {
                  if (context.mounted) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(
                        content: Text(result['message'] ?? 'Failed to delete order'),
                        backgroundColor: Colors.red,
                      ),
                    );
                  }
                }
              } else {
                lockerProvider.adminUpdateTrackingCode(
                  box.id, 
                  newCode,
                  newRecipient: newRecipient.isEmpty ? null : newRecipient,
                );
                if (context.mounted) {
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(content: Text('Compartment ${box.boxNumber} updated successfully!')),
                  );
                }
              }
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
                            ? 'Code: ${box.trackingCode}${box.recipient != null ? ' | Recipient: ${box.recipient}' : ''}'
                            : 'Current state: ${box.status.name.toUpperCase()}',
                        style: TextStyle(
                          color: box.status == CompartmentStatus.occupied ? Colors.blue : Colors.black,
                          fontWeight: box.status == CompartmentStatus.occupied ? FontWeight.w600 : FontWeight.normal,
                        ),
                      ),
                      trailing: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          if (box.status == CompartmentStatus.occupied && box.trackingCode != null)
                            IconButton(
                              icon: const Icon(Icons.delete_outline, color: Colors.red),
                              tooltip: 'Delete Order',
                              onPressed: () => _confirmDeleteOrder(context, box),
                            ),
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
                            onPressed: () async {
                              final wasMaintenance = box.status == CompartmentStatus.maintenance;
                              final success = await context.read<LockerProvider>().adminToggleMaintenance(box.id);
                              if (context.mounted) {
                                if (success) {
                                  final newStatusName = wasMaintenance ? 'AVAILABLE' : 'MAINTENANCE';
                                  ScaffoldMessenger.of(context).showSnackBar(
                                    SnackBar(content: Text('Compartment ${box.boxNumber} status changed to $newStatusName in DB!')),
                                  );
                                } else {
                                  ScaffoldMessenger.of(context).showSnackBar(
                                    const SnackBar(content: Text('Failed to update compartment status in DB!'), backgroundColor: Colors.red),
                                  );
                                }
                              }
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