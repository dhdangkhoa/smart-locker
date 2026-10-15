import 'package:flutter/material.dart';
import '../models/locker_models.dart';


class LockerProvider extends ChangeNotifier {

  final List<CompartmentModel> _compartments = [
    CompartmentModel(id: '1', boxNumber: '01', status: CompartmentStatus.available),
    CompartmentModel(id: '2', boxNumber: '02', status: CompartmentStatus.available),
    // CompartmentModel(id: '3', boxNumber: '03', status: CompartmentStatus.available),
    // CompartmentModel(id: '4', boxNumber: '04', status: CompartmentStatus.available),
    // CompartmentModel(id: '5', boxNumber: '05', status: CompartmentStatus.available),
    // CompartmentModel(id: '6', boxNumber: '06', status: CompartmentStatus.available),
  ];

  List<CompartmentModel> get compartments => _compartments;

  // get a list of lockers with avaiable compartments.
  List<CompartmentModel> get availableCompartments => _compartments.where((c) => c.status == CompartmentStatus.available).toList();

  // handle courier entering the order code.
  // find the first avaiable compartment and assign it to the order.
  CompartmentModel? assignCompartmentforPackage(String trackingCode) {
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
        trackingCode: trackingCode
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
      );
      notifyListeners();
    }
  }

  void adminUpdateTrackingCode(String compartmentId, String newTrackingCode) {
    final index = _compartments.indexWhere((c) => c.id == compartmentId);
    if (index != -1) {
      final box = _compartments[index];
      _compartments[index] = CompartmentModel(
        id: box.id,
        boxNumber: box.boxNumber,
        status: newTrackingCode.isEmpty ? CompartmentStatus.available : CompartmentStatus.occupied,
        trackingCode: newTrackingCode.isEmpty ? null : newTrackingCode,
      );
      notifyListeners();
    }
  }

  void adminToggleMaintenance(String compartmentId) {
    final index = _compartments.indexWhere((c) => c.id == compartmentId);
    if (index != -1) {
      final box = _compartments[index];
      final newStatus = box.status == CompartmentStatus.maintenance
          ? CompartmentStatus.available
          : CompartmentStatus.maintenance;

      _compartments[index] = CompartmentModel(
        id: box.id,
        boxNumber: box.boxNumber,
        status: newStatus,
        trackingCode: newStatus == CompartmentStatus.maintenance ? null : box.trackingCode,
      );
      notifyListeners();
    }
  }

}