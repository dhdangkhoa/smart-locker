import 'package:flutter/material.dart';
import '../models/locker_models.dart';
import '../services/api_service.dart';

class LockerProvider extends ChangeNotifier {

  LockerProvider() {
    loadLockersFromBackend();
  }

  final List<CompartmentModel> _compartments = [
    CompartmentModel(id: '1', boxNumber: 'C1', status: CompartmentStatus.available),
    CompartmentModel(id: '2', boxNumber: 'C2', status: CompartmentStatus.available),
  ];

  Future<void> loadLockersFromBackend() async {
    final data = await APIService.fetchLockers();
    if (data != null && data.isNotEmpty) {
      _compartments.clear();
      for (final item in data) {
        CompartmentStatus status = CompartmentStatus.available;
        final s = item['status']?.toString().toLowerCase();
        if (s == 'occupied') {
          status = CompartmentStatus.occupied;
        } else if (s == 'maintenance') {
          status = CompartmentStatus.maintenance;
        }

        _compartments.add(
          CompartmentModel(
            id: item['id'].toString(),
            boxNumber: item['box_number']?.toString() ?? '',
            status: status,
            trackingCode: item['tracking_code'],
            recipient: item['recipient'],
          ),
        );
      }
      notifyListeners();
    }
  }

  void updateCompartmentStatus(String lockerNumberOrId, CompartmentStatus status, {String? trackingCode, String? recipient}) {
    final index = _compartments.indexWhere((c) => c.id == lockerNumberOrId || c.boxNumber == lockerNumberOrId);
    if (index != -1) {
      final box = _compartments[index];
      _compartments[index] = CompartmentModel(
        id: box.id,
        boxNumber: box.boxNumber,
        status: status,
        trackingCode: trackingCode,
        recipient: recipient,
      );
      notifyListeners();
    }
  }

  List<CompartmentModel> get compartments => _compartments;

  // get a list of lockers with avaiable compartments.
  List<CompartmentModel> get availableCompartments => _compartments.where((c) => c.status == CompartmentStatus.available).toList();

  // handle courier entering the order code and recipient.
  // find the first avaiable compartment and assign it to the order.
  CompartmentModel? assignCompartmentforPackage(String trackingCode, String recipient) {
    final availableList = availableCompartments;
    if (availableList.isEmpty) return null;

    // find the first avaiable box
    final assignedBox = availableList.first;

    // change the box's status to OCCUPIED
    final index = _compartments.indexWhere((c) => c.id == assignedBox.id);
    if (index != -1) {
      _compartments[index] = CompartmentModel(
        id: assignedBox.id, 
        boxNumber: assignedBox.boxNumber, 
        status: CompartmentStatus.occupied,
        trackingCode: trackingCode,
        recipient: recipient,
      );
      notifyListeners();
    }

    return assignedBox;
  }

  CompartmentModel? findBoxByTrackingCode(String trackingCode) {
    try {
      return _compartments.firstWhere((c) => c.status == CompartmentStatus.occupied && c.trackingCode == trackingCode,
      );
    } catch (_) {
      return null;
    }
  }

  void releaseCompartment(String compartmentId) {
    final index = _compartments.indexWhere((c) => c.id == compartmentId);
    if (index != -1) {
      final box = _compartments[index];
      _compartments[index] = CompartmentModel(
        id: box.id,
        boxNumber: box.boxNumber,
        status: CompartmentStatus.available,
        trackingCode: null,
        recipient: null,
      );
      notifyListeners();
    }
  }

  void adminUpdateTrackingCode(String compartmentId, String newTrackingCode, {String? newRecipient}) {
    final index = _compartments.indexWhere((c) => c.id == compartmentId);
    if (index != -1) {
      final box = _compartments[index];
      _compartments[index] = CompartmentModel(
        id: box.id,
        boxNumber: box.boxNumber,
        status: newTrackingCode.isEmpty ? CompartmentStatus.available : CompartmentStatus.occupied,
        trackingCode: newTrackingCode.isEmpty ? null : newTrackingCode,
        recipient: newTrackingCode.isEmpty ? null : (newRecipient ?? box.recipient),
      );
      notifyListeners();
    }
  }

  Future<bool> adminToggleMaintenance(String compartmentId) async {
    final index = _compartments.indexWhere((c) => c.id == compartmentId || c.boxNumber == compartmentId);
    if (index != -1) {
      final box = _compartments[index];
      final newStatusStr = box.status == CompartmentStatus.maintenance
          ? 'available'
          : 'maintenance';

      // Call API to persist new status in Database
      final result = await APIService.updateLockerStatus(box.id, newStatusStr);
      if (result['success'] == true) {
        final newStatus = newStatusStr == 'maintenance'
            ? CompartmentStatus.maintenance
            : CompartmentStatus.available;

        _compartments[index] = CompartmentModel(
          id: box.id,
          boxNumber: box.boxNumber,
          status: newStatus,
          trackingCode: newStatus == CompartmentStatus.maintenance ? null : box.trackingCode,
          recipient: newStatus == CompartmentStatus.maintenance ? null : box.recipient,
        );
        notifyListeners();
        return true;
      }
    }
    return false;
  }

}